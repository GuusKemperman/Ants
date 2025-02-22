#pragma once
#include "Meta/Fwd/MetaReflectFwd.h"

namespace Ant
{
	using PheromoneId = int32;

	class PheromoneComponent
	{
	public:
		float GetPheromoneAmountAtDist(float distToCentre) const;

		static constexpr float sInitialAmount = 1.0f;
		static constexpr float sPheromoneDuration = 60.0f;
		static constexpr float sEvaporationPerSecond = sInitialAmount / sPheromoneDuration;

		static constexpr float sRadius = 1.0f;

		PheromoneId mPheromoneId{};

		float mAmount = sInitialAmount;

	private:
		friend CE::ReflectAccess;
		static CE::MetaType Reflect();
		REFLECT_AT_START_UP(PheromoneComponent);
	};
}
