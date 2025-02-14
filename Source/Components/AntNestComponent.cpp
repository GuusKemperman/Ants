#include "Precomp.h"
#include "Components/AntNestComponent.h"

#include "Utilities/Random.h"
#include "Utilities/Reflect/ReflectComponentType.h"
#include "Commands/GameStep.h"

void Ant::AntNestComponent::DepositFood(float amount)
{
	mFood += amount;
}

size_t Ant::AntNestComponent::GetMaxNumAntsToSpawnNextStep() const
{
	return static_cast<size_t>(mFood / mAntCost);
}

void Ant::AntNestComponent::SpendFoodOnSpawning(GameStep& gameStep)
{
	size_t numAnts = GetMaxNumAntsToSpawnNextStep();
	mFood -= static_cast<float>(numAnts) * mAntCost;

	for (size_t i = 0; i < numAnts; i++)
	{
		const float randomAngle = CE::Random::Range(0.0f, glm::two_pi<float>());
		gameStep.AddCommand(SpawnAntCommand{ randomAngle });
	}
}

CE::MetaType Ant::AntNestComponent::Reflect()
{
	CE::MetaType metaType{ CE::MetaType::T<AntNestComponent>{}, "AntNestComponent" };

	metaType.AddField(&AntNestComponent::mFood, "mFood");
	metaType.AddField(&AntNestComponent::mAntCost, "mAntCost");

	CE::ReflectComponentType<AntNestComponent>(metaType);

	return metaType;
}
