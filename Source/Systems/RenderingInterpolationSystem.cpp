#include "Precomp.h"
#include "Systems/RenderingInterpolationSystem.h"

#include "Components/AntBaseComponent.h"
#include "Components/FoodPelletTag.h"
#include "Components/TransformComponent.h"
#include "Core/Renderer.h"
#include "Meta/MetaType.h"
#include "Systems/AntBehaviourSystem.h"
#include "World/Registry.h"
#include "World/World.h"

Ant::RenderingInterpolationSystem::RenderingInterpolationSystem()
{
	CE::AssetManager& assetManager = CE::AssetManager::Get();

	mMat = assetManager.TryGetAsset<CE::Material>("MT_White");
	mAntMesh = assetManager.TryGetAsset<CE::StaticMesh>("SM_Suzanne");
	mPheromoneMesh = assetManager.TryGetAsset<CE::StaticMesh>("SM_Plane");
	mFoodMesh = assetManager.TryGetAsset<CE::StaticMesh>("SM_Icosphere");

	if (mMat == nullptr
		|| mAntMesh == nullptr
		|| mPheromoneMesh == nullptr
		|| mFoodMesh == nullptr)
	{
		LOG(LogGame, Error, "Missing assets");
	}
}

void Ant::RenderingInterpolationSystem::Render(const CE::World& world, CE::RenderCommandQueue& renderQueue) const
{
	if (mMat == nullptr)
	{
		return;
	}

	//if (mPheromoneMesh != nullptr)
	//{
	//	const glm::mat4 pheromoneMatrix = CE::TransformComponent::ToMatrix({}, 
	//		glm::vec3{ PheromoneComponent::sRadius, PheromoneComponent::sRadius, 0.1f },
	//		glm::quat{ glm::vec3{ glm::pi<float>(), 0.0f, 0.0f } });

	//	for (auto [entity, transform, pheromone] : world.GetRegistry().View<CE::TransformComponent, PheromoneComponent>().each())
	//	{
	//		glm::vec4 col{ 0.0f, 0.0f, 0.0f, 1.0f };
	//		col[0] = static_cast<float>(static_cast<bool>(pheromone.mPheromoneId & 1));
	//		col[1] = static_cast<float>(static_cast<bool>(pheromone.mPheromoneId & 2));
	//		col[2] = static_cast<float>(static_cast<bool>(pheromone.mPheromoneId & 4));

	//		CE::Renderer::Get().AddStaticMesh(renderQueue,
	//			mPheromoneMesh,
	//			mMat,
	//			transform.GetWorldMatrix() * pheromoneMatrix,
	//			glm::vec4{ 0.0f },
	//			col);
	//	}
	//}

	if (mAntMesh != nullptr
		&& mFoodMesh != nullptr)
	{
		const float totalTimePassed = world.GetCurrentTimeScaled();
		const float interpolationFactor = glm::clamp(fmodf(totalTimePassed, sAntTickInterval), 0.0f, 1.0f);

		const glm::mat4 foodOffsetMatrix = CE::TransformComponent::ToMatrix(sFoodPelletHoldOffset, sFoodPelletScale, {1.0f, 0.0f, 0.0f, 0.0f});

		for (auto [entity, ant] : world.GetRegistry().View<AntBaseComponent>().each())
		{
			const glm::vec3 interpolatedPos = CE::To3D(CE::Math::lerp(ant.mPreviousWorldPosition, ant.mWorldPosition, interpolationFactor));
			const glm::quat interpolatedRot = glm::slerp(ant.mPreviousWorldOrientation, ant.mWorldOrientation, interpolationFactor);

			const glm::mat4 antMatrix = CE::TransformComponent::ToMatrix(interpolatedPos, glm::vec3{ 1.0f, 1.0f, 1.0f }, interpolatedRot);

			CE::Renderer::Get().AddStaticMesh(renderQueue,
				mAntMesh,
				mMat,
				antMatrix,
				glm::vec4{ 0.0f },
				sAntCol);

			if (ant.mIsHoldingFood)
			{
				const glm::mat4 foodMatrix = antMatrix * foodOffsetMatrix;

				CE::Renderer::Get().AddStaticMesh(renderQueue,
					mFoodMesh,
					mMat,
					foodMatrix,
					glm::vec4{ 0.0f },
					sFoodCol);
			}
		}
	}

	if (mFoodMesh != nullptr)
	{
		for (auto [entity, transform] : world.GetRegistry().View<CE::TransformComponent, FoodPelletTag>().each())
		{
			CE::Renderer::Get().AddStaticMesh(renderQueue,
				mFoodMesh,
				mMat,
				transform.GetWorldMatrix(),
				glm::vec4{ 0.0f },
				sFoodCol);
		}
	}


}

CE::MetaType Ant::RenderingInterpolationSystem::Reflect()
{
	return { CE::MetaType::T<RenderingInterpolationSystem>{}, "RenderingInterpolationSystem", CE::MetaType::Base<System>{} };
}
