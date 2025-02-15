#pragma once
#include <queue>

#include "Assets/Material.h"
#include "Assets/StaticMesh.h"
#include "Assets/Core/AssetHandle.h"
#include "Commands/GameStep.h"
#include "GameState/GameState.h"
#include "Systems/System.h"

namespace Ant
{
	class SimulationRenderingSystem :
		public CE::System
	{
	public:
		SimulationRenderingSystem();

		void RecordStep(const GameStep& step);

		void Update(CE::World& world, float dt) override;

		void Render(const CE::World& world, CE::RenderCommandQueue& renderQueue) const override;

	private:
		CE::AssetHandle<CE::Material> mMat{};
		CE::AssetHandle<CE::StaticMesh> mAntMesh{};
		CE::AssetHandle<CE::StaticMesh> mPheromoneMesh{};
		CE::AssetHandle<CE::StaticMesh> mFoodMesh{};

		static constexpr glm::vec4 sAntCol{ 1.0f, .9f, .9f, 1.0f };
		static constexpr glm::vec4 sFoodCol{ 0.0f, 1.0f, 0.0f, 1.0f };

		static constexpr glm::vec3 sFoodPelletScale{ 0.2f };
		static constexpr glm::vec3 sFoodPelletHoldOffset = { 1.5f, 0.0f, 0.0f };

		std::mutex mRenderingQueueMutex{};
		std::queue<GameStep> mRenderingQueue{};
		GameState mRenderingState{};
	};
}
