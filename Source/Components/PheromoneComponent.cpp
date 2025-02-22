#include "Precomp.h"
#include "Components/PheromoneComponent.h"

#include "Meta/MetaType.h"
#include "Utilities/Reflect/ReflectComponentType.h"

glm::vec4 Ant::PheromoneIdToColor(PheromoneId id)
{
	glm::vec4 col{ 0.0f, 0.0f, 0.0f, 1.0f };
	col[0] = static_cast<float>(static_cast<bool>(id & 1));
	col[1] = static_cast<float>(static_cast<bool>(id & 2));
	col[2] = static_cast<float>(static_cast<bool>(id & 4));
	return col;
}

CE::MetaType Ant::PheromoneComponent::Reflect()
{
	CE::MetaType metaType{ CE::MetaType::T<PheromoneComponent>{}, "PheromoneComponent" };

	metaType.AddField(&PheromoneComponent::mAmount, "mAmount");

	CE::ReflectComponentType<PheromoneComponent>(metaType);
	return metaType;
}
