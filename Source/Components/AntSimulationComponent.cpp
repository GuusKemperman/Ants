#include "Precomp.h"
#include "Components/AntSimulationComponent.h"

#include <numeric>

#include "Commands/GameStep.h"
#include "Commands/SpawnFoodCommand.h"
#include "Components/AntBaseComponent.h"
#include "Components/AntNestComponent.h"
#include "Components/FoodPelletTag.h"
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
		Ant::GameStep mNextStep{};
	};
	
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
	world.AddSystem<InternalSimulationSystem>();

	if (mShouldRecord)
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

				CE::Registry& reg = world.GetRegistry();

				size_t numOfAnts = reg.Storage<AntBaseComponent>().size();

				nextStep->ForEachCommandBuffer(
					[&]<typename T>(T& commandBuffer)
					{
						commandBuffer.Clear();

						if constexpr (std::is_same_v<T, CommandBuffer<SpawnAntCommand>>)
						{
							size_t total{};
							for (auto [entity, nest] : reg.View<AntNestComponent>().each())
							{
								total += nest.GetMaxNumAntsToSpawnNextStep();
							}
							commandBuffer.Reserve(total);
						}
						else if constexpr (std::is_same_v<T, CommandBuffer<SpawnFoodCommand>>)
						{
							// do nothing
						}
						else
						{
							commandBuffer.Reserve(numOfAnts);
						}
					});

				SpawnFood(world, nextStep->GetBuffer<SpawnFoodCommand>());
				world.GetPhysics().RebuildBVHs();
				world.GetEventManager().InvokeEventsForAllComponents(sOnAntTick);

				for (auto [entity, nest] : reg.View<AntNestComponent>().each())
				{
					nest.SpendFoodOnSpawning(*nextStep);
				}

				mCurrentState.Step(*nextStep);
				mStepsSimulated++;

				if (mOnStepCompletedCallback != nullptr)
				{
					mOnStepCompletedCallback(*nextStep);
				}
			}
		}
	};
}


void Ant::AntSimulationComponent::SpawnFood(CE::World& world, CommandBuffer<SpawnFoodCommand>& commandBuffer)
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

CE::MetaType Ant::AntSimulationComponent::Reflect()
{
	CE::MetaType metaType{ CE::MetaType::T<AntSimulationComponent>{}, "AntSimulationComponent" };

	metaType.AddField(&AntSimulationComponent::mShouldRecord, "mShouldRecord");
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
