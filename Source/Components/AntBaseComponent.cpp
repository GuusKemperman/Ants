#include "Precomp.h"
#include "Components/AntBaseComponent.h"

#include "Components/FoodPelletTag.h"
#include "World/World.h"
#include "World/Physics.h"
#include "Components/TransformComponent.h"
#include "Utilities/DrawDebugHelpers.h"
#include "Utilities/Reflect/ReflectComponentType.h"

void Ant::AntBaseComponent::Move(CE::World& world, entt::entity owner, glm::vec2 direction)
{
	CE::Registry& reg = world.GetRegistry();
	CE::TransformComponent* antTransform = reg.TryGet<CE::TransformComponent>(owner);

	if (antTransform == nullptr)
	{
		LOG(LogGame, Error, "Ant did not have transform");
		return;
	}

	antTransform->SetWorldPosition(antTransform->GetWorldPosition() + CE::To3D(direction));
}

Ant::SenseResult Ant::AntBaseComponent::Sense(const CE::World& world, entt::entity owner, glm::vec2 senseLocation)
{
	SenseResult senseResult{};

	const CE::Registry& reg = world.GetRegistry();
	const CE::TransformComponent* antTransform = reg.TryGet<CE::TransformComponent>(owner);

	if (antTransform == nullptr)
	{
		LOG(LogGame, Error, "Ant did not have transform");
		return senseResult;
	}

	const glm::vec2 start = antTransform->GetWorldPosition();

	const float dirLength = glm::length(senseLocation);

	if (dirLength == 0.0f)
	{
		return senseResult;
	}

	const glm::vec2 normalisedDir = senseLocation / dirLength;

	const glm::vec2 end = start + CE::To2D(CE::Math::RotateVector(CE::To3D(normalisedDir),
		antTransform->GetWorldOrientation())) * dirLength;

	CE::CollisionRules rules{};
	rules.mLayer = CE::CollisionLayer::Query;
	rules.SetResponse(CE::CollisionLayer::Projectiles, CE::CollisionResponse::Overlap);

	const CE::Physics::LineTraceResult physicsResult = world.GetPhysics().LineTrace({ start, end }, rules);

	senseResult.mSenseStart = start;
	senseResult.mSenseEnd = start + normalisedDir * physicsResult.mDist;

	senseResult.mDist = physicsResult.mDist;
	senseResult.mHitEntity = physicsResult.mHitEntity;

	if (CE::IsDebugDrawCategoryVisible(CE::DebugDraw::Gameplay))
	{
		const glm::mat4 invMat = glm::inverse(antTransform->GetWorldMatrix());

		CE::AddDebugLine(const_cast<CE::RenderCommandQueue&>(world.GetRenderCommandQueue()),
			CE::DebugDraw::Gameplay,
			glm::vec4{ CE::To3D(senseResult.mSenseStart), 0.0f } *invMat,
			glm::vec4{ CE::To3D(senseResult.mSenseEnd), 0.0f } *invMat,
			glm::vec4{ 1.0f, 0.0f, 0.0f, 1.0f });
	}

	return senseResult;
}

bool Ant::SenseResult::HitFood(const CE::World& world) const
{
	return mHitEntity != entt::null
		&& world.GetRegistry().HasComponent<FoodPelletTag>(mHitEntity);
}

glm::vec2 Ant::SenseResult::GetSenseDirection() const
{
	if (mSenseStart == mSenseEnd)
	{
		return CE::sForward;
	}
	return glm::normalize(mSenseEnd - mSenseStart);
}

glm::vec2 Ant::SenseResult::GetHitPosition() const
{
	return mSenseEnd;
}

float Ant::SenseResult::GetDistance() const
{
	return glm::distance(mSenseEnd, mSenseStart);
}

CE::MetaType Ant::SenseResult::Reflect()
{
	CE::MetaType metaType{CE::MetaType::T<SenseResult>{}, "SenseResult" };

	metaType.GetProperties()
		.Add(CE::Props::sIsScriptOwnableTag)
		.Add(CE::Props::sIsScriptableTag);

	metaType.AddFunc(&SenseResult::HitFood, "HitFood").GetProperties().Add(CE::Props::sIsScriptableTag);
	metaType.AddFunc(&SenseResult::GetSenseDirection, "GetSenseDirection").GetProperties().Add(CE::Props::sIsScriptableTag);
	metaType.AddFunc(&SenseResult::GetHitPosition, "GetHitPosition").GetProperties().Add(CE::Props::sIsScriptableTag);
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

	metaType.AddFunc(&AntBaseComponent::Move, "Move").GetProperties()
		.Add(CE::Props::sIsScriptableTag)
		.Set(CE::Props::sIsScriptPure, false);

	CE::ReflectComponentType<AntBaseComponent>(metaType);

	return metaType;
}

