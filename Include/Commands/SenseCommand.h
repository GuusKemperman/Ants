#pragma once

namespace CE
{
	class World;
}

namespace Ant
{
	// Mostly used for visualising ant sensing
	struct SenseCommand
	{
		static void Execute(CE::World& , std::span<const SenseCommand>) {}

		entt::entity mAnt = entt::null;
		glm::vec2 mSenseLocationWorld{};
		float mDist{};
	};
}