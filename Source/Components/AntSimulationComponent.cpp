#include "Precomp.h"
#include "Components/AntSimulationComponent.h"

#include <numeric>

#include "Commands/GameStep.h"
#include "Commands/SpawnFoodCommand.h"
#include "Components/AntBaseComponent.h"
#include "Components/AntNestComponent.h"
#include "Components/FoodPelletTag.h"
#include "Components/SimulationRenderingComponent.h"
#include "Systems/SimulationRenderingSystem.h"
#include "Utilities/Random.h"
#include "Utilities/Reflect/ReflectComponentType.h"
#include "World/EventManager.h"
#include "World/Physics.h"

Ant::AntSimulationComponent::~AntSimulationComponent()
{
	mSimulationStopped = true;
	if (mSimulateThread.joinable())
	{
		mSimulateThread.join();
	}
}

void Ant::AntSimulationComponent::OnBeginPlay(CE::World& world, entt::entity)
{
	StartSimulation(&world);
}

namespace
{
	struct InternalSimulationSystem : CE::System
	{
		InternalSimulationSystem(Ant::AntSimulationComponent& component) : mOuterSimulationComponent(component) {}

		Ant::AntSimulationComponent& mOuterSimulationComponent;
		Ant::GameStep mNextStep{};
	};
	
}

const Ant::AntSimulationComponent* Ant::AntSimulationComponent::TryGetOwningSimulationComponent(const CE::World& world)
{
	const InternalSimulationSystem* system = world.TryGetSystem<InternalSimulationSystem>();

	if (system == nullptr)
	{
		return nullptr;
	}

	return &system->mOuterSimulationComponent;
}

const Ant::GameState* Ant::AntSimulationComponent::TryGetGameState(const CE::World& world)
{
	const AntSimulationComponent* component = TryGetOwningSimulationComponent(world);

	if (component == nullptr)
	{
		return nullptr;
	}
	return &component->GetGameState();
}

Ant::GameStep* Ant::AntSimulationComponent::TryGetNextGameStep(CE::World& world)
{
	InternalSimulationSystem* system = world.TryGetSystem<InternalSimulationSystem>();

	if (system == nullptr)
	{
		LOG(LogGame, Error, "World did not have Ant::GameStep");
		return nullptr;
	}

	return &system->mNextStep;
}

void Ant::AntSimulationComponent::StartSimulation(CE::World* viewportWorld)
{
	CE::World& world = mCurrentState.GetWorld();
	world.AddSystem<InternalSimulationSystem>(*this);

	if (!viewportWorld->GetRegistry().View<SimulationRenderingComponent>().empty())
	{
		SimulationRenderingSystem& rendering = viewportWorld->AddSystem<SimulationRenderingSystem>();
		mOnStepCompletedCallback = [&](const GameStep& step) { rendering.RecordStep(step); };
	}

	mSimulateThread = std::thread
	{
		[this]
		{
			while (!mSimulationStopped)
			{
				CE::World& world = mCurrentState.GetWorld();
				GameStep* nextStep = TryGetNextGameStep(world);

				if (nextStep == nullptr)
				{
					LOG(LogGame, Error, "No next step, stopped simulation");
					break;
				}

				nextStep->ForEachCommandBuffer(
					[&]<typename T>(CommandBuffer<T>& commandBuffer)
					{
						size_t numStored = commandBuffer.GetStoredCommands().size();
						size_t numSubmitted = commandBuffer.GetNumSubmittedCommands();

						commandBuffer.Clear();

						size_t numReserved = numStored;

						if (numStored == 0)
						{
							numReserved = 1024;
						}
						else if (numStored < numSubmitted * 2)
						{
							numReserved = numSubmitted * 2;
						}

						commandBuffer.Reserve(numReserved);
					});

				// Collect commands
				world.GetEventManager().InvokeEventsForAllComponents(sOnAntTick);
				CollectSpawnAntsCommands(world, *nextStep);
				CollectSpawnFoodCommands(world, nextStep->GetBuffer<SpawnFoodCommand>());

				mCurrentState.Step(*nextStep);
				mStepsSimulated++;

				CE::Physics::UpdateBVHConfig config
				{
					.mOnlyRebuildForNewColliders = mStepsSimulated % 500 != 1
				};
				world.GetPhysics().UpdateBVHs(config);

				if (mOnStepCompletedCallback != nullptr)
				{
					mOnStepCompletedCallback(*nextStep);
				}
			}
		}
	};
}

void Ant::AntSimulationComponent::CollectSpawnFoodCommands(CE::World& world, CommandBuffer<SpawnFoodCommand>& commandBuffer)
{
	size_t numOfFood = world.GetRegistry().Storage<FoodPelletTag>().size();
	size_t numOfFoodToSpawn = numOfFood < mMinNumOfFoodInWorld ? mNumOfFoodToSpawn : 0;

	if (numOfFoodToSpawn == 0)
	{
		return;
	}

	commandBuffer.Reserve(numOfFoodToSpawn);

	float clusterAngle = CE::Random::Range(0.0f, glm::two_pi<float>());
	const glm::vec2 clusterCentre = mFoodSpawnDist * CE::Math::AngleToVec2(clusterAngle);
	mFoodSpawnDist *= mFoodSpawnDistanceIncrementFactor;

	const float clusterRadius = mFoodClusterRadiusPerPellet * static_cast<float>(numOfFoodToSpawn);

	for (size_t i = 0; i < numOfFoodToSpawn; i++)
	{
		const float dist = CE::Random::Range(0.0f, clusterRadius);
		const float angle = CE::Random::Range(0.0f, glm::two_pi<float>());
		const glm::vec2 pos = clusterCentre + dist * CE::Math::AngleToVec2(angle);
		commandBuffer.AddCommand(pos);
	}
}

void Ant::AntSimulationComponent::CollectSpawnAntsCommands(CE::World& world, GameStep& nextStep)
{
	for (auto [entity, nest] : world.GetRegistry().View<AntNestComponent>().each())
	{
		nest.SpendFoodOnSpawning(nextStep);
	}
}

CE::MetaType Ant::AntSimulationComponent::Reflect()
{
	CE::MetaType metaType{ CE::MetaType::T<AntSimulationComponent>{}, "AntSimulationComponent" };

	metaType.AddField(&AntSimulationComponent::mStepsSimulated, "mStepsSimulated")
		.GetProperties().Add(CE::Props::sIsEditorReadOnlyTag);

	metaType.AddField(&AntSimulationComponent::mMinNumOfFoodInWorld, "mMinNumOfFoodInWorld");
	metaType.AddField(&AntSimulationComponent::mNumOfFoodToSpawn, "mNumOfFoodToSpawn");
	metaType.AddField(&AntSimulationComponent::mFoodSpawnDistanceIncrementFactor, "mFoodSpawnDistanceIncrementFactor");
	metaType.AddField(&AntSimulationComponent::mFoodSpawnDist, "mFoodSpawnDist");
	metaType.AddField(&AntSimulationComponent::mFoodClusterRadiusPerPellet, "mFoodClusterRadiusPerPellet");


	CE::BindEvent(metaType, CE::sOnBeginPlay, &AntSimulationComponent::OnBeginPlay);

	CE::ReflectComponentType<AntSimulationComponent>(metaType);

	return metaType;
}
