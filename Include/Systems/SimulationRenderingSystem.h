#pragma once
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

		std::array<CE::AssetHandle<CE::Material>, 3> mAntWalkFrames{};

		CE::AssetHandle<CE::StaticMesh> mAntMesh{};
		CE::AssetHandle<CE::StaticMesh> mPheromoneMesh{};
		CE::AssetHandle<CE::StaticMesh> mFoodMesh{};

		static constexpr glm::vec4 sFoodCol{ 0.0f, 1.0f, 0.0f, 1.0f };

		static constexpr glm::vec3 sFoodPelletHoldScale{ 0.2f };
		static constexpr glm::vec3 sFoodPelletHoldOffset = { 1.f, 0.0f, 0.0f };
		static constexpr glm::vec3 sFoodPelletHoldRotation = { 0.0f, 0.0f, 0.0f };

		mutable std::mutex mRenderingQueueMutex{};
		std::vector<GameStep> mRenderingQueue{};
		std::unique_ptr<GameState> mRenderingState = std::make_unique<GameState>();
	};
}
