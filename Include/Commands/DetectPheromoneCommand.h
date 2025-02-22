#pragma once

namespace CE
{
	class World;
}

namespace Ant
{
	// Mostly used for visualising ant sensing
	struct DetectPheromoneCommand
	{
		static void Execute(CE::World&, std::span<const DetectPheromoneCommand>) {}

		entt::entity mAnt = entt::null;
		PheromoneId mPheromoneId{};
		glm::vec2 mSenseLocationWorld{};
		float mAmountDetected{};
	};
}