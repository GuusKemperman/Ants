#include "Precomp.h"
#include "Components/PheromoneComponent.h"

#include "Meta/MetaType.h"
#include "Utilities/Reflect/ReflectComponentType.h"

float Ant::PheromoneComponent::GetPheromoneAmountAtDist(float distToCentre) const
{
	float s = distToCentre / sRadius;
	float s2 = s * 2;
	float f = 3.0f;

	float intensity = CE::Math::sqr(1.0f - s2) / (1.0f + f * s2);
	float percentageRemaining = mAmount / sInitialAmount;
	return intensity * percentageRemaining;
}

CE::MetaType Ant::PheromoneComponent::Reflect()
{
	CE::MetaType metaType{ CE::MetaType::T<PheromoneComponent>{}, "PheromoneComponent" };

	metaType.AddField(&PheromoneComponent::mAmount, "mAmount");

	CE::ReflectComponentType<PheromoneComponent>(metaType);
	return metaType;
}
