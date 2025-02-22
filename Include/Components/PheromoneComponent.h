#pragma once
#include "Meta/Fwd/MetaReflectFwd.h"

namespace Ant
{
	using PheromoneId = int32;

	glm::vec4 PheromoneIdToColor(PheromoneId id);

	class PheromoneComponent
	{
	public:
		static constexpr float sInitialAmount = 1.0f;
		static constexpr float sPheromoneDuration = 250.0f;
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
