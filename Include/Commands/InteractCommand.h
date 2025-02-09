#pragma once
#include "CommandBase.h"

namespace CE
{
	class World;
}

namespace Ant
{
	struct InteractCommand : CommandBase
	{
		static void Execute(CE::World& world, std::span<const InteractCommand> commands);

		entt::entity mInteractedWith{};
	};
}
