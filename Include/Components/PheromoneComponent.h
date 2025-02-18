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
		static constexpr float sEvaporationPerSecond = 0.01f;

		// TODO: Requires updating disk collider component as well
		static constexpr float sRadius = 5.0f;

		PheromoneId mPheromoneId{};

		float mAmount = sInitialAmount;

	private:
		friend CE::ReflectAccess;
		static CE::MetaType Reflect();
		REFLECT_AT_START_UP(PheromoneComponent);
	};
}
