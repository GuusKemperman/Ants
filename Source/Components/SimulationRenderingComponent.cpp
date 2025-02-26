#include "Precomp.h"
#include "Components/SimulationRenderingComponent.h"

#include "Meta/MetaType.h"
#include "Utilities/Reflect/ReflectComponentType.h"

CE::MetaType Ant::SimulationRenderingComponent::Reflect()
{
	CE::MetaType metaType{ CE::MetaType::T<SimulationRenderingComponent>{}, "SimulationRenderingComponent" };

	metaType.AddField(&SimulationRenderingComponent::mTimeStamp, "mTimeStamp");
	metaType.AddField(&SimulationRenderingComponent::mDesiredPlaySpeed, "mDesiredPlaySpeed");
	metaType.AddField(&SimulationRenderingComponent::mAntAnimationSpeed, "mAntAnimationSpeed");

	metaType.AddField(&SimulationRenderingComponent::mNumOfAnts, "mNumOfAnts")
		.GetProperties().Add(CE::Props::sIsEditorReadOnlyTag);

	metaType.AddField(&SimulationRenderingComponent::mNumOfPheromones, "mNumOfPheromones")
		.GetProperties().Add(CE::Props::sIsEditorReadOnlyTag);

	metaType.AddField(&SimulationRenderingComponent::mNumFoodInWorld, "mNumFoodInWorld")
		.GetProperties().Add(CE::Props::sIsEditorReadOnlyTag);

	metaType.AddField(&SimulationRenderingComponent::mScore, "mScore")
		.GetProperties().Add(CE::Props::sIsEditorReadOnlyTag);
	
	CE::ReflectComponentType<SimulationRenderingComponent>(metaType);
	return metaType;
}
