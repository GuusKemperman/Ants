#pragma once

namespace CE
{
	class World;
}

namespace Ant
{
	struct InteractCommand
	{
		static void Execute(CE::World& world, std::span<const InteractCommand> commands);

		entt::entity mAnt = entt::null;
		entt::entity mInteractedWith = entt::null;
	};
}
