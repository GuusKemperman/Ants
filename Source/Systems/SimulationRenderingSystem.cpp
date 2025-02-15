#include "Precomp.h"
#include "Systems/SimulationRenderingSystem.h"

#include "Components/AntBaseComponent.h"
#include "Components/AntSimulationComponent.h"
#include "Components/FoodPelletTag.h"
#include "Components/SimulationRenderingComponent.h"
#include "Components/TransformComponent.h"
#include "Core/AssetManager.h"
#include "Core/Renderer.h"
#include "World/Registry.h"
#include "World/World.h"

Ant::SimulationRenderingSystem::SimulationRenderingSystem()
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

void Ant::SimulationRenderingSystem::RecordStep(const GameStep& step)
{
	std::lock_guard guard{ mRenderingQueueMutex };
	mRenderingQueue.push(step);
}

void Ant::SimulationRenderingSystem::Update(CE::World& world, float dt)
{
	auto renderingView = world.GetRegistry().View<SimulationRenderingComponent>();
	entt::entity renderingEntity = renderingView.front();

	if (renderingEntity == entt::null)
	{
		return;
	}

	SimulationRenderingComponent& renderingComponent = renderingView.get<SimulationRenderingComponent>(renderingEntity);
	renderingComponent.mTimeStamp += renderingComponent.mPlaySpeed * dt;

	uint64 numOfStepsAtTimeStamp = static_cast<uint64>(renderingComponent.mTimeStamp);

	if (numOfStepsAtTimeStamp > mRenderingState.GetNumOfStepsCompleted())
	{
		std::lock_guard guard{ mRenderingQueueMutex };

		while (numOfStepsAtTimeStamp > mRenderingState.GetNumOfStepsCompleted())
		{
			if (mRenderingQueue.empty())
			{
				LOG(LogGame, Warning, "Rendering has caught up to simulation end!");
				return;
			}
			mRenderingState.Step(mRenderingQueue.front());
			mRenderingQueue.pop();
		}
	}
}

void Ant::SimulationRenderingSystem::Render(const CE::World& viewportWorld, CE::RenderCommandQueue& renderQueue) const
{
	const CE::World& world = mRenderingState.GetWorld();

	auto renderingView = viewportWorld.GetRegistry().View<SimulationRenderingComponent>();
	entt::entity renderingEntity = renderingView.front();

	if (renderingEntity == entt::null)
	{
		return;
	}

	const SimulationRenderingComponent& renderingComponent = renderingView.get<SimulationRenderingComponent>(renderingEntity);
	const float totalTimePassed = renderingComponent.mTimeStamp;
	const float interpolationFactor = glm::clamp(
		fmodf(totalTimePassed, GameState::sStepDurationSeconds) * (1 / GameState::sStepDurationSeconds),
		0.0f, 1.0f);

	if (mMat == nullptr)
	{
		return;
	}

	if (mAntMesh != nullptr
		&& mFoodMesh != nullptr)
	{
		const glm::mat4 foodOffsetMatrix = CE::TransformComponent::ToMatrix(sFoodPelletHoldOffset, sFoodPelletScale, { 1.0f, 0.0f, 0.0f, 0.0f });

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
