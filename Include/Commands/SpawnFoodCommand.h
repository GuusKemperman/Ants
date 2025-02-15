#pragma once

namespace CE
{
	class World;
}

namespace Ant
{
	struct SpawnFoodCommand
	{
		static void Execute(CE::World& world, std::span<const SpawnFoodCommand> commands);

		glm::vec2 mLocation{};
	};
}