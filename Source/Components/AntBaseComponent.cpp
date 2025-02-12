#include "Precomp.h"
#include "Components/AntBaseComponent.h"

#include "Components/AntNestComponent.h"
#include "Components/FoodPelletTag.h"
#include "Components/PheromoneComponent.h"
#include "World/World.h"
#include "World/Physics.h"
#include "Components/TransformComponent.h"
#include "Systems/AntBehaviourSystem.h"
#include "Utilities/DrawDebugHelpers.h"
#include "Utilities/Random.h"
#include "Utilities/Reflect/ReflectComponentType.h"

void Ant::AntBaseComponent::OnBeginPlay(CE::World&, entt::entity)
{
	const float randomAngle = CE::Random::Range(0.0f, glm::two_pi<float>());

	glm::vec3 orientationEuler{};
	orientationEuler[CE::Axis::Up] = randomAngle;

	mPreviousWorldOrientation = glm::quat{ orientationEuler };
	mWorldOrientation = mPreviousWorldOrientation;
}

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

	AntBehaviourSystem* antSystem = world.TryGetSystem<AntBehaviourSystem>();

	if (antSystem == nullptr)
	{
		LOG(LogGame, Error, "AntBehaviourSystem does not exist");
		return;
	}

	antSystem->mInteractCommandBuffer.AddCommand(owner, result.mHitEntity);
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
	const glm::vec2 delta = CE::To2D(
		CE::Math::RotateVector(CE::To3D(towardsLocation), ant->mWorldOrientation));
	const glm::vec2 newPosition = worldStart + delta;

	const glm::vec3 newForward = CE::To3D(glm::normalize(delta));
	const glm::quat newOrientation = CE::Math::CalculateRotationBetweenOrientations(CE::sForward, newForward);

	AntBehaviourSystem* antSystem = world.TryGetSystem<AntBehaviourSystem>();

	if (antSystem == nullptr)
	{
		LOG(LogGame, Error, "AntBehaviourSystem does not exist");
		return;
	}

	antSystem->mMoveCommandBuffer.AddCommand(owner, newPosition, newOrientation);
}

Ant::SenseResult Ant::AntBaseComponent::Sense(const CE::World& world, entt::entity owner, glm::vec2 senseLocation)
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

	const float dirLength = glm::length(senseLocation);

	const glm::vec2 normalisedDirLocal = senseLocation / dirLength;

	const glm::vec2 endWorld = startWorld + CE::To2D(CE::Math::RotateVector(CE::To3D(normalisedDirLocal),
		ant->mWorldOrientation)) * dirLength;

	CE::CollisionRules rules{};
	rules.mLayer = CE::CollisionLayer::Query;
	rules.SetResponse(CE::CollisionLayer::Projectiles, CE::CollisionResponse::Overlap);

	const CE::Physics::LineTraceResult physicsResult = world.GetPhysics().LineTrace({ startWorld, endWorld }, rules);

	senseResult.mDist = physicsResult.mDist;
	senseResult.mHitEntity = physicsResult.mHitEntity;

	if (CE::IsDebugDrawCategoryVisible(CE::DebugDraw::Gameplay))
	{
		CE::AddDebugLine(const_cast<CE::RenderCommandQueue&>(world.GetRenderCommandQueue()),
			CE::DebugDraw::Gameplay,
			CE::To3D(startWorld),
			CE::To3D(startWorld + glm::normalize(endWorld - startWorld) * senseResult.mDist),
			glm::vec4{ 1.0f, 0.0f, 0.0f, 1.0f });
	}

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

	const glm::vec2 senseLocationWorld = ant->mWorldPosition + CE::To2D(CE::Math::RotateVector(CE::To3D(senseLocation),
		ant->mWorldOrientation));

	CE::TransformedDisk queryShape{ senseLocationWorld, 1.0f };

	CE::CollisionRules rules{};
	rules.mLayer = CE::CollisionLayer::Query;
	rules.SetResponse(CE::CollisionLayer::Trigger, CE::CollisionResponse::Overlap);

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

	AntBehaviourSystem* antSystem = world.TryGetSystem<AntBehaviourSystem>();

	if (antSystem == nullptr)
	{
		LOG(LogGame, Error, "AntBehaviourSystem does not exist");
		return;
	}

	antSystem->mEmitPheromoneCommandBuffer.AddCommand(ant->mPreviousWorldPosition, pheromoneId);
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

	metaType.AddFunc(&AntBaseComponent::IsCarryingFood, "IsCarryingFood").GetProperties()
		.Add(CE::Props::sIsScriptableTag)
		.Set(CE::Props::sIsScriptPure, true);

	CE::BindEvent(metaType, CE::sOnBeginPlay, &AntBaseComponent::OnBeginPlay);

	CE::ReflectComponentType<AntBaseComponent>(metaType);

	return metaType;
}

