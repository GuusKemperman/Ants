#pragma once

namespace CE
{
	class World;
}

namespace Ant
{
	struct MoveCommand
	{
		static void Execute(CE::World& world, std::span<const MoveCommand> commands);

		entt::entity mAnt = entt::null;

		glm::vec2 mNewPosition{};
		glm::quat mNewOrientation{};
	};
}
