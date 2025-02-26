#pragma once
#include "Meta/Fwd/MetaReflectFwd.h"


namespace Ant
{
	struct SimulationRenderingComponent
	{
		float mTimeStamp{};
		float mDesiredPlaySpeed = 1.0f;

		float mAntAnimationSpeed = 5.0f;

		uint32 mNumOfAnts{};
		uint32 mNumOfPheromones{};

		uint32 mNumFoodInWorld{};
		
		uint32 mScore{};

	private:
		friend CE::ReflectAccess;
		static CE::MetaType Reflect();
		REFLECT_AT_START_UP(SimulationRenderingComponent);
	};
}
