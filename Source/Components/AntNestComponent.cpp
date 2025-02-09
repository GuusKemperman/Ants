#include "Precomp.h"
#include "Components/AntNestComponent.h"

#include "Assets/Prefabs/Prefab.h"
#include "Components/TransformComponent.h"
#include "Utilities/Random.h"
#include "Utilities/Reflect/ReflectComponentType.h"
#include "World/Registry.h"
#include "World/World.h"

void Ant::AntNestComponent::DepositFood(float amount)
{
	mFood += amount;
}

void Ant::AntNestComponent::SpendFoodOnSpawning(CE::World& world, entt::entity owner)
{
	if (mWorkerAnt == nullptr)
	{
		LOG(LogGame, Warning, "No ant prefab set");
		return;
	}

	const CE::TransformComponent* nestTransform = world.GetRegistry().TryGet<CE::TransformComponent>(owner);

	if (nestTransform == nullptr)
	{
		LOG(LogGame, Warning, "Nest does not have a transform");
		return;
	}

	size_t numAnts = static_cast<size_t>(mFood / mAntCost);
	mFood -= static_cast<float>(numAnts) * mAntCost;

	for (size_t i = 0; i < numAnts; i++)
	{
		float randomAngle = CE::Random::Range(0.0f, glm::two_pi<float>());

		glm::vec3 pos = nestTransform->GetWorldPosition();
		glm::vec3 orientationEuler{};
		orientationEuler[CE::Axis::Up] = randomAngle;

		glm::quat orientation{ orientationEuler };

		world.GetRegistry().CreateFromPrefab(*mWorkerAnt, entt::null, &pos, &orientation);
	}
}

CE::MetaType Ant::AntNestComponent::Reflect()
{
	CE::MetaType metaType{ CE::MetaType::T<AntNestComponent>{}, "AntNestComponent" };

	metaType.AddField(&AntNestComponent::mWorkerAnt, "mWorkerAnt");
	metaType.AddField(&AntNestComponent::mFood, "mFood");
	metaType.AddField(&AntNestComponent::mAntCost, "mAntCost");

	CE::ReflectComponentType<AntNestComponent>(metaType);

	return metaType;
}
