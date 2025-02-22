#pragma once
#include "Components/PheromoneComponent.h"

namespace Ant
{
	class GameState;
}

namespace CE
{
	class World;
}

namespace Ant
{
	struct EmitPheromoneCommand
	{
		static void Execute(CE::World& world, std::span<const EmitPheromoneCommand> commands);

		static constexpr uint32 sPheromoneSpawnInterval = 3;

		static bool CanSpawnPheromoneNextTick(const GameState& state);

		glm::vec2 mLocation{};
		PheromoneId mPheromoneId{};
	};
}
