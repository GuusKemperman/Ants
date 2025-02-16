#include "Precomp.h"
#include "Components/SimulationRenderingComponent.h"

#include "Meta/MetaType.h"
#include "Utilities/Reflect/ReflectComponentType.h"

CE::MetaType Ant::SimulationRenderingComponent::Reflect()
{
	CE::MetaType metaType{ CE::MetaType::T<SimulationRenderingComponent>{}, "SimulationRenderingComponent" };

	metaType.AddField(&SimulationRenderingComponent::mTimeStamp, "mTimeStamp");
	metaType.AddField(&SimulationRenderingComponent::mDesiredPlaySpeed, "mDesiredPlaySpeed");

	CE::ReflectComponentType<SimulationRenderingComponent>(metaType);
	return metaType;
}
