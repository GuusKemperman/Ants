#pragma once
#include "CommandBase.h"

namespace CE
{
	class World;
}

namespace Ant
{
	struct MoveCommand : CommandBase
	{
		static void Execute(CE::World& world, std::span<const MoveCommand> commands);

		glm::vec2 mNewPosition{};
		glm::quat mNewOrientation{};
	};
}
