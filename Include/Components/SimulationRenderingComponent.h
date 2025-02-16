#pragma once
#include "Meta/Fwd/MetaReflectFwd.h"


namespace Ant
{
	struct SimulationRenderingComponent
	{
		float mTimeStamp{};
		float mDesiredPlaySpeed = 1.0f;

	private:
		friend CE::ReflectAccess;
		static CE::MetaType Reflect();
		REFLECT_AT_START_UP(SimulationRenderingComponent);
	};
}
