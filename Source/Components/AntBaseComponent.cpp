#include "Precomp.h"
#include "Components/AntBaseComponent.h"

#include "Commands/EmitPheromoneCommand.h"
#include "Components/AntNestComponent.h"
#include "Components/AntSimulationComponent.h"
#include "Components/FoodPelletTag.h"
#include "Components/PheromoneComponent.h"
#include "World/World.h"
#include "World/Physics.h"
#include "Components/TransformComponent.h"
#include "Utilities/DrawDebugHelpers.h"
#include "Utilities/Random.h"
#include "Utilities/Reflect/ReflectComponentType.h"

bool Ant::AntBaseComponent::IsCarryingFood(const CE::World& world, entt::entity owner)
{
	const CE::Registry& reg = world.GetRegistry();
	const AntBaseComponent* ant = reg.TryGet<AntBaseComponent>(owner);

	if (ant == nullptr)
	{
		LOG(LogGame, Error, "No ant component");
		return false;
	}

	return ant->mIsHoldingFood;
}

void Ant::AntBaseComponent::Interact(CE::World& world, entt::entity owner)
{
	SenseResult result = Sense(world, owner, CE::sForward * sInteractRange);

	if (result.mHitEntity == entt::null)
	{
		LOG(LogGame, Verbose, "Cannot interact, no interactable in range");
		return;
	}

	AntSimulationComponent::RecordCommand<InteractCommand>(world, { owner, result.mHitEntity });
}

void Ant::AntBaseComponent::Move(CE::World& world, entt::entity owner, glm::vec2 towardsLocation)
{
	if (towardsLocation == glm::vec2{})
	{
		return;
	}

	CE::Registry& reg = world.GetRegistry();
	AntBaseComponent* ant = reg.TryGet<AntBaseComponent>(owner);

	if (ant == nullptr)
	{
		LOG(LogGame, Error, "No ant component");
		return;
	}

	const glm::vec2 worldStart = ant->mWorldPosition;
	const glm::vec2 delta = CE::Math::RotateVec2ByAngleInRadians(towardsLocation, ant->mWorldOrientation);
	const glm::vec2 newPosition = worldStart + delta;

	const float newOrientation = CE::Math::Vec2ToAngle(delta);

	AntSimulationComponent::RecordCommand<MoveCommand>(world, { owner, newPosition, newOrientation });
}

Ant::SenseResult Ant::AntBaseComponent::Sense(CE::World& world, entt::entity owner, glm::vec2 senseLocation)
{
	SenseResult senseResult{};

	if (senseLocation == glm::vec2{})
	{
		return senseResult;
	}

	const CE::Registry& reg = world.GetRegistry();
	const AntBaseComponent* ant = reg.TryGet<AntBaseComponent>(owner);

	if (ant == nullptr)
	{
		LOG(LogGame, Error, "No ant component");
		return senseResult;
	}

	const glm::vec2 startWorld = ant->mWorldPosition;

	senseLocation = CE::Math::ClampLength(senseLocation, 0.0f, sMaxSenseRange);

	const glm::vec2 endWorld = startWorld + 
		CE::Math::RotateVec2ByAngleInRadians(senseLocation, ant->mWorldOrientation);

	CE::CollisionRules rules{};
	rules.mLayer = CE::CollisionLayer::Query;
	rules.SetResponse(CE::CollisionLayer::Projectiles, CE::CollisionResponse::Overlap);

	const CE::Physics::LineTraceResult physicsResult = world.GetPhysics().LineTrace({ startWorld, endWorld }, rules);

	senseResult.mDist = physicsResult.mDist;
	senseResult.mHitEntity = physicsResult.mHitEntity;
	AntSimulationComponent::RecordCommand<SenseCommand>(world, { owner, endWorld, senseResult.mDist });
	return senseResult;
}

namespace
{
	struct PheromoneSmellingOnIntersect
	{
		static void Callback(const CE::TransformedDisk& shape,
			entt::entity entity,
			const entt::storage_for_t<Ant::PheromoneComponent>& storage,
			const CE::TransformedDisk& queryShape,
			float& totalSmelled,
			Ant::PheromoneId pheromoneId)
		{
			if (!storage.contains(entity))
			{
				LOG(LogGame, Error, "Entity unexpectedly did not have pheromone component, while being in pheromone channel");
				return;
			}
			const Ant::PheromoneComponent& pheromoneComponent = storage.get(entity);

			if (pheromoneComponent.mPheromoneId != pheromoneId)
			{
				return;
			}

			const float distToPheromoneCentre = glm::distance(shape.mCentre, queryShape.mCentre) - queryShape.mRadius;
			totalSmelled += pheromoneComponent.GetPheromoneAmountAtDist(distToPheromoneCentre);
		}

		template<typename... Args>
		static void Callback(const CE::TransformedAABB&, Args&&...)
		{
			LOG(LogGame, Error, "Entity unexpectedly had a non-disk collider, while being in pheromone channel");
		}

		template<typename... Args>
		static void Callback(const CE::TransformedPolygon&, Args&&...)
		{
			LOG(LogGame, Error, "Entity unexpectedly had a non-disk collider, while being in pheromone channel");
		}
	};
}

float Ant::AntBaseComponent::DetectPheromones(const CE::World& world, 
	entt::entity owner, 
	glm::vec2 senseLocation,
	PheromoneId pheromoneId)
{
	const CE::Registry& reg = world.GetRegistry();
	const AntBaseComponent* ant = reg.TryGet<AntBaseComponent>(owner);

	if (ant == nullptr)
	{
		LOG(LogGame, Error, "No ant component");
		return 0.0f;
	}

	const entt::storage_for_t<PheromoneComponent>* pheromoneStorage = reg.Storage<PheromoneComponent>();

	if (pheromoneStorage == nullptr)
	{
		LOG(LogGame, Warning, "Pheromone storage was nullptr, uninitialised storage could lead to threading issues");
		return 0.0f;
	}

	const glm::vec2 senseLocationWorld = ant->mWorldPosition +
		CE::Math::RotateVec2ByAngleInRadians(senseLocation, ant->mWorldOrientation);

	CE::TransformedDisk queryShape{ senseLocationWorld, 1.0f };

	CE::CollisionRules rules{};
	rules.mLayer = CE::CollisionLayer::Query;
	rules.SetResponse(CE::CollisionLayer::Query, CE::CollisionResponse::Overlap);

	float totalSmelled = 0.0f;

	world.GetPhysics().Query<PheromoneSmellingOnIntersect, CE::BVH::DefaultShouldCheckFunction<true>, CE::BVH::DefaultShouldReturnFunction<false>>(
		queryShape, 
		rules, 
		*pheromoneStorage, 
		queryShape,
		totalSmelled,
		pheromoneId);

	return totalSmelled;
}

void Ant::AntBaseComponent::EmitPheromones(CE::World& world, entt::entity owner, PheromoneId pheromoneId)
{
	const CE::Registry& reg = world.GetRegistry();
	const AntBaseComponent* ant = reg.TryGet<AntBaseComponent>(owner);

	if (ant == nullptr)
	{
		LOG(LogGame, Error, "No ant component");
		return;
	}

	AntSimulationComponent::RecordCommand<EmitPheromoneCommand>(world, { ant->mPreviousWorldPosition, pheromoneId });
}

bool Ant::SenseResult::SensedComponent(const CE::World& world, CE::TypeId componentTypeId) const
{
	return mHitEntity != entt::null
		&& world.GetRegistry().HasComponent(componentTypeId, mHitEntity);
}

bool Ant::SenseResult::SensedFood(const CE::World& world) const
{
	return SensedComponent(world, CE::MakeTypeId<FoodPelletTag>());
}

bool Ant::SenseResult::SensedNest(const CE::World& world) const
{
	return SensedComponent(world, CE::MakeTypeId<AntNestComponent>());
}

float Ant::SenseResult::GetDistance() const
{
	return mDist;
}

CE::MetaType Ant::SenseResult::Reflect()
{
	CE::MetaType metaType{CE::MetaType::T<SenseResult>{}, "SenseResult" };

	metaType.GetProperties()
		.Add(CE::Props::sIsScriptOwnableTag)
		.Add(CE::Props::sIsScriptableTag);

	metaType.AddFunc(&SenseResult::SensedFood, "SensedFood").GetProperties().Add(CE::Props::sIsScriptableTag);
	metaType.AddFunc(&SenseResult::SensedNest, "SensedNest").GetProperties().Add(CE::Props::sIsScriptableTag);
	metaType.AddFunc(&SenseResult::GetDistance, "GetDistance").GetProperties().Add(CE::Props::sIsScriptableTag);

	return metaType;
}

CE::MetaType Ant::AntBaseComponent::Reflect()
{
	CE::MetaType metaType{ CE::MetaType::T<AntBaseComponent>{}, "AntBaseComponent" };

	metaType.GetProperties()
		.Add(CE::Props::sIsScriptableTag);

	metaType.AddFunc(&AntBaseComponent::Sense, "Sense").GetProperties()
		.Add(CE::Props::sIsScriptableTag)
		.Set(CE::Props::sIsScriptPure, false);

	metaType.AddFunc(&AntBaseComponent::DetectPheromones, "DetectPheromones").GetProperties()
		.Add(CE::Props::sIsScriptableTag)
		.Set(CE::Props::sIsScriptPure, false);

	metaType.AddFunc(&AntBaseComponent::Move, "Move").GetProperties()
		.Add(CE::Props::sIsScriptableTag)
		.Set(CE::Props::sIsScriptPure, false);

	metaType.AddFunc(&AntBaseComponent::Interact, "Interact").GetProperties()
		.Add(CE::Props::sIsScriptableTag)
		.Set(CE::Props::sIsScriptPure, false);

	metaType.AddFunc(&AntBaseComponent::EmitPheromones, "EmitPheromones").GetProperties()
		.Add(CE::Props::sIsScriptableTag)
		.Set(CE::Props::sIsScriptPure, false);

	metaType.AddFunc([] { return sInteractRange;  }, "GetInteractRange").GetProperties()
		.Add(CE::Props::sIsScriptableTag)
		.Set(CE::Props::sIsScriptPure, true);

	metaType.AddFunc([] { return sMaxSenseRange;  }, "GetSenseRange").GetProperties()
		.Add(CE::Props::sIsScriptableTag)
		.Set(CE::Props::sIsScriptPure, true);

	metaType.AddFunc(&AntBaseComponent::IsCarryingFood, "IsCarryingFood").GetProperties()
		.Add(CE::Props::sIsScriptableTag)
		.Set(CE::Props::sIsScriptPure, true);

	CE::ReflectComponentType<AntBaseComponent>(metaType);

	return metaType;
}

