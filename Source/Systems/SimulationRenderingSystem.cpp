#include "Precomp.h"
#include "Systems/SimulationRenderingSystem.h"

#include "Commands/SenseCommand.h"
#include "Components/AntBaseComponent.h"
#include "Components/AntSimulationComponent.h"
#include "Components/CameraComponent.h"
#include "Components/FoodPelletTag.h"
#include "Components/SimulationRenderingComponent.h"
#include "Components/TransformComponent.h"
#include "Core/AssetManager.h"
#include "Core/Renderer.h"
#include "Utilities/DrawDebugHelpers.h"
#include "World/Registry.h"
#include "World/World.h"

namespace Internal
{
	static float InterpolateAngle(float a1, float a2, float alpha);
}

Ant::SimulationRenderingSystem::SimulationRenderingSystem()
{
	CE::AssetManager& assetManager = CE::AssetManager::Get();

	mMat = assetManager.TryGetAsset<CE::Material>("MT_White");
	mAntWalkFrames[0] = assetManager.TryGetAsset<CE::Material>("MT_AntWalk1");
	mAntWalkFrames[1] = assetManager.TryGetAsset<CE::Material>("MT_AntWalk2");
	mAntWalkFrames[2] = assetManager.TryGetAsset<CE::Material>("MT_AntWalk3");

	mAntMesh = assetManager.TryGetAsset<CE::StaticMesh>("SM_Plane");

	mPheromoneLODs[0] = { assetManager.TryGetAsset<CE::StaticMesh>("SM_PheromoneLOD0"), 0.0f };
	mPheromoneLODs[1] = { assetManager.TryGetAsset<CE::StaticMesh>("SM_PheromoneLOD1"), 50.0f };
	mPheromoneLODs[2] = { assetManager.TryGetAsset<CE::StaticMesh>("SM_PheromoneLOD2"), 200.0f };
	mPheromoneLODs[3] = { assetManager.TryGetAsset<CE::StaticMesh>("SM_PheromoneLOD3"), 500.0f };

	mFoodMesh = assetManager.TryGetAsset<CE::StaticMesh>("SM_Cube");
}

void Ant::SimulationRenderingSystem::RecordStep(const GameStep& step)
{
	std::lock_guard guard{ mRenderingQueueMutex };
	mRenderingQueue.emplace_back(step);
}

void Ant::SimulationRenderingSystem::Update(CE::World& world, float dt)
{
	auto renderingView = world.GetRegistry().View<AntSimulationComponent, SimulationRenderingComponent>();
	entt::entity renderingEntity = renderingView.front();

	if (renderingEntity == entt::null)
	{
		return;
	}

	auto [simulationComponent, renderingComponent] = renderingView.get(renderingEntity);

	renderingComponent.mTimeStamp = glm::max(0.0f, renderingComponent.mTimeStamp + renderingComponent.mDesiredPlaySpeed * dt);
	uint64 requiredNumSteps = glm::clamp(static_cast<int64>(renderingComponent.mTimeStamp), 
		0ll, 
		static_cast<int64>(mRenderingQueue.size()));
	uint64 currentNumSteps = mRenderingState->GetNumOfStepsCompleted();

	if (currentNumSteps > requiredNumSteps)
	{
		mRenderingState = std::make_unique<GameState>();
		currentNumSteps = 0;
	}

	if (currentNumSteps != requiredNumSteps)
	{
		std::lock_guard guard{ mRenderingQueueMutex };

		for (uint64 i = currentNumSteps; i < requiredNumSteps; i++)
		{
			mRenderingState->Step(mRenderingQueue[i]);
		}
	}
}

void Ant::SimulationRenderingSystem::Render(const CE::World& viewportWorld, CE::RenderCommandQueue& renderQueue) const
{
	CE::Renderer::Get().SetClearColor(renderQueue, { 0.5f, 0.3f, 0.15f, 1.0f });

	const CE::World& world = mRenderingState->GetWorld();

	auto renderingView = viewportWorld.GetRegistry().View<SimulationRenderingComponent>();
	entt::entity renderingEntity = renderingView.front();

	if (renderingEntity == entt::null)
	{
		return;
	}

	auto camView = viewportWorld.GetRegistry().View<CE::TransformComponent, CE::CameraComponent>();
	entt::entity camEntity = CE::CameraComponent::GetSelected(viewportWorld);
	ASSERT(camView.contains(camEntity));
	
	const CE::TransformComponent& camTransform = camView.get<CE::TransformComponent>(camEntity);

	if (CE::IsDebugDrawCategoryVisible(CE::DebugDraw::Particles))
	{
		const glm::mat4 pheromoneMatrix = CE::TransformComponent::ToMatrix({ 0.0f, 0.0f, -2.0f },
			glm::vec3{ PheromoneComponent::sRadius, PheromoneComponent::sRadius, 0.1f },
			glm::quat{ glm::vec3{ glm::pi<float>(), 0.0f, 0.0f } });

		float height = camTransform.GetWorldPosition().z;
		CE::AssetHandle pheromoneMesh = mPheromoneLODs[0].mMesh;

		for (auto& lod : mPheromoneLODs)
		{
			if (height < lod.Dist)
			{
				break;
			}

			pheromoneMesh = lod.mMesh;
		}

		for (auto [entity, transform, pheromone] : world.GetRegistry().View<CE::TransformComponent, PheromoneComponent>().each())
		{
			CE::Renderer::Get().AddStaticMesh(renderQueue,
				pheromoneMesh,
				mMat,
				transform.GetWorldMatrix() * pheromoneMatrix,
				glm::vec4{ 0.0f },
				PheromoneIdToColor(pheromone.mPheromoneId));
		}
	}

	for (auto [entity, transform] : world.GetRegistry().View<CE::TransformComponent, FoodPelletTag>().each())
	{
		CE::Renderer::Get().AddStaticMesh(renderQueue,
			mFoodMesh,
			mMat,
			transform.GetWorldMatrix(),
			sFoodCol,
			glm::vec4{ 0.0f });
	}

	const SimulationRenderingComponent& renderingComponent = renderingView.get<SimulationRenderingComponent>(renderingEntity);

	const float totalTimePassed = glm::min(renderingComponent.mTimeStamp, static_cast<float>(mRenderingState->GetNumOfStepsCompleted()) + 1.f);
	const float interpolationFactor = glm::clamp(
		fmodf(totalTimePassed, GameState::sStepDurationSeconds) * (1 / GameState::sStepDurationSeconds),
		0.0f, 1.0f);

	const glm::mat4 foodOffsetMatrix = 
		CE::TransformComponent::ToMatrix(sFoodPelletHoldOffset, sFoodPelletHoldScale, { sFoodPelletHoldRotation });

	auto antView = world.GetRegistry().View<AntBaseComponent>();
	float height = 0.0f;
	float heightStep = 1.0f / static_cast<float>(antView.size());

	for (auto [entity, ant] : antView.each())
	{
		float renderHeight = height;
		height += heightStep;

		const glm::vec3 interpolatedPos = CE::To3D(CE::Math::lerp(ant.mPreviousWorldPosition, 
			ant.mWorldPosition, 
			interpolationFactor),
			renderHeight);

		float interpolatedAngle = Internal::InterpolateAngle(ant.mPreviousWorldOrientation, 
			ant.mWorldOrientation, 
			interpolationFactor);

		glm::vec3 orientationEuler{};
		orientationEuler[CE::Axis::Up] = interpolatedAngle - glm::half_pi<float>();

		glm::quat interpolatedRot{ orientationEuler };

		const glm::mat4 antMatrix = CE::TransformComponent::ToMatrix(interpolatedPos, 
			glm::vec3{ 0.809523811f, 1.0f, 1.0f }, interpolatedRot);

		uint32 frame = static_cast<uint32>(static_cast<float>(entt::to_integral(entity) % mAntWalkFrames.size()) 
			+ totalTimePassed * renderingComponent.mAntAnimationSpeed) % mAntWalkFrames.size();

		CE::Renderer::Get().AddStaticMesh(renderQueue,
			mAntMesh,
			mAntWalkFrames[frame],
			antMatrix,
			glm::vec4{ 1.0f },
			glm::vec4{ 0.0f });

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

	if (!CE::IsDebugDrawCategoryVisible(CE::DebugDraw::AIDecision))
	{
		return;
	}

	const GameStep mostRecentGameStep = [&]() -> GameStep
		{
			std::lock_guard l{ mRenderingQueueMutex }; 
			uint64 numStepsCompleted = mRenderingState->GetNumOfStepsCompleted();

			if (numStepsCompleted == 0
				|| numStepsCompleted > mRenderingQueue.size())
			{
				return {};
			}

			return mRenderingQueue[numStepsCompleted - 1];
		}();

	{ // Visualise sense commands
		const CommandBuffer<SenseCommand>& senseCommands = mostRecentGameStep.GetBuffer<SenseCommand>();

		for (const SenseCommand& command : senseCommands.GetStoredCommands())
		{
			if (antView.contains(command.mAnt))
			{
				const AntBaseComponent& ant = antView.get<AntBaseComponent>(command.mAnt);
				constexpr float lineHeight = 2.0f;
				const glm::vec2 direction = glm::normalize(command.mSenseLocationWorld - ant.mPreviousWorldPosition);

				const glm::vec2 nonHitLineStart = ant.mPreviousWorldPosition + direction * command.mDist;
				const glm::vec2 nonHitLineEnd = command.mSenseLocationWorld;

				const glm::vec2 hitLineStart = ant.mPreviousWorldPosition;
				const glm::vec2 hitLineEnd = nonHitLineStart;

				CE::AddDebugLine(renderQueue,
					CE::DebugDraw::AIDecision,
					CE::To3D(nonHitLineStart, lineHeight),
					CE::To3D(nonHitLineEnd, lineHeight),
					glm::vec4{ 1.0f, 1.0f, 1.0f, .2f });

				CE::AddDebugLine(renderQueue,
					CE::DebugDraw::AIDecision,
					CE::To3D(hitLineStart, lineHeight),
					CE::To3D(hitLineEnd, lineHeight),
					glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
			}
		}
	}

	const auto& detectPheromoneCommands = mostRecentGameStep.GetBuffer<DetectPheromoneCommand>().GetStoredCommands();

	if (detectPheromoneCommands.empty())
	{
		return;
	}

	for (const DetectPheromoneCommand& command : detectPheromoneCommands)
	{
		if (!antView.contains(command.mAnt))
		{
			continue;
		}

		const AntBaseComponent& ant = antView.get<AntBaseComponent>(command.mAnt);
		const glm::vec4 color = PheromoneIdToColor(command.mPheromoneId);

		CE::AddDebugCircle(renderQueue,
			CE::DebugDraw::AIDecision,
			CE::To3D(command.mSenseLocationWorld, 1.0f),
			AntBaseComponent::sPheromoneDetectionSampleRadius,
			color);

		CE::AddDebugLine(renderQueue,
			CE::DebugDraw::AIDecision,
			CE::To3D(ant.mPreviousWorldPosition, 1.0f),
			CE::To3D(command.mSenseLocationWorld, 1.0f),
			color);
	}
	
}

float Internal::InterpolateAngle(float a1, float a2, float alpha)
{
	// Normalize angles to [0, 2PI)

	a1 = fmod(a1, 2 * glm::pi<float>());

	if (a1 < 0) {

		a1 += 2 * glm::pi<float>();

	}

	a2 = fmod(a2, 2 * glm::pi<float>());

	if (a2 < 0) {

		a2 += 2 * glm::pi<float>();

	}

	float delta = a2 - a1;

	// Compute the shortest delta

	delta = fmod(delta + glm::pi<float>(), 2 * glm::pi<float>()) - glm::pi<float>();

	float interpolated = a1 + delta * alpha;

	// Normalize the result to [0, 2PI)

	interpolated = fmod(interpolated, 2 * glm::pi<float>());

	if (interpolated < 0) {

		interpolated += 2 * glm::pi<float>();

	}

	return interpolated;
}