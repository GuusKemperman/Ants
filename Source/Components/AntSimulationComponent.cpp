#include "Precomp.h"
#include "Components/AntSimulationComponent.h"

#include <numeric>

#include "Commands/GameStep.h"
#include "Components/AntBaseComponent.h"
#include "Components/AntNestComponent.h"
#include "Systems/SimulationRenderingSystem.h"
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
							commandBuffer.mCommands.resize(total);
						}
						else
						{
							commandBuffer.mCommands.resize(numOfAnts);
						}
					});

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

CE::MetaType Ant::AntSimulationComponent::Reflect()
{
	CE::MetaType metaType{ CE::MetaType::T<AntSimulationComponent>{}, "AntSimulationComponent" };

	metaType.AddField(&AntSimulationComponent::mShouldRecord, "mShouldRecord");
	metaType.AddField(&AntSimulationComponent::mStepsSimulated, "mStepsSimulated")
		.GetProperties().Add(CE::Props::sIsEditorReadOnlyTag);

	CE::BindEvent(metaType, CE::sOnBeginPlay, &AntSimulationComponent::OnBeginPlay);

	CE::ReflectComponentType<AntSimulationComponent>(metaType);

	return metaType;
}
