#pragma once
#include "Assets/Material.h"
#include "Assets/StaticMesh.h"
#include "Assets/Core/AssetHandle.h"
#include "Components/TransformComponent.h"
#include "Core/AssetManager.h"
#include "Systems/System.h"

namespace Ant
{
	class RenderingInterpolationSystem :
		public CE::System
	{
	public:
		RenderingInterpolationSystem();

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

		friend CE::ReflectAccess;
		static CE::MetaType Reflect();
		REFLECT_AT_START_UP(RenderingInterpolationSystem);
	};
}
