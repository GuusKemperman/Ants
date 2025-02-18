#pragma once

namespace CE
{
	class World;
}


namespace Ant
{
	struct SpawnAntCommand
	{
		static void Execute(CE::World& world, std::span<const SpawnAntCommand> commands);

		float mOrientation{};
	};
}