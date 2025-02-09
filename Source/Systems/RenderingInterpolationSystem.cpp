#include "Precomp.h"
#include "Systems/RenderingInterpolationSystem.h"

#include "Components/AntBaseComponent.h"
#include "Components/TransformComponent.h"
#include "Meta/MetaType.h"
#include "Systems/AntBehaviourSystem.h"
#include "World/Registry.h"
#include "World/World.h"

void Ant::RenderingInterpolationSystem::Update(CE::World& world, float)
{
	float totalTimePassed = world.GetCurrentTimeScaled();
	float interpolationFactor = glm::clamp(fmodf(totalTimePassed, sAntTickInterval), 0.0f, 1.0f);

	for (auto [entity, transform, ant] : world.GetRegistry().View<CE::TransformComponent, AntBaseComponent>().each())
	{
		glm::vec3 interpolatedPos = CE::To3D(CE::Math::lerp(ant.mPreviousWorldPosition, ant.mWorldPosition, interpolationFactor));
		glm::quat interpolatedRot = glm::slerp(ant.mPreviousWorldOrientation, ant.mWorldOrientation, interpolationFactor);

		transform.SetWorldPositionScaleOrientation(interpolatedPos, transform.GetWorldScale(), interpolatedRot);
	}
}

CE::MetaType Ant::RenderingInterpolationSystem::Reflect()
{
	return { CE::MetaType::T<RenderingInterpolationSystem>{}, "RenderingInterpolationSystem", CE::MetaType::Base<System>{} };
}
