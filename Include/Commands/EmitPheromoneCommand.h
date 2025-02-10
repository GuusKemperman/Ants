#pragma once
#include "Components/PheromoneComponent.h"

namespace CE
{
	class World;
}

namespace Ant
{
	struct EmitPheromoneCommand
	{
		static void Execute(CE::World& world, std::span<const EmitPheromoneCommand> commands);

		glm::vec2 mLocation{};
		PheromoneId mPheromoneId{};
	};
}
