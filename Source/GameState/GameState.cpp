#include "Precomp.h"
#include "GameState/GameState.h"

#include "Assets/Level.h"
#include "Commands/GameStep.h"
#include "Components/AntBaseComponent.h"
#include "Components/TransformComponent.h"
#include "Core/AssetManager.h"
#include "World/Registry.h"
#include "World/World.h"

namespace Internal
{
	template<typename T>
	static void ProcessCommands(CE::World& world, const Ant::CommandBuffer<T>& commandBuffer)
	{
		T::Execute(world, commandBuffer.GetStoredCommands());
	}
}

Ant::GameState::GameState() :
	mWorld(true)
{
	CE::AssetHandle level = CE::AssetManager::Get().TryGetAsset<CE::Level>("L_StartingGameState");

	if (level == nullptr)
	{
		LOG(LogGame, Error, "No starting level for gamestate");
		return;
	}

	level->LoadIntoWorld(mWorld);
}

void Ant::GameState::Step(const GameStep& step)
{
	if (!mWorld.HasBegunPlay())
	{
		mWorld.BeginPlay();
	}

	mScore += static_cast<uint32>(step.GetBuffer<SpawnAntCommand>().GetNumSubmittedCommands());

	step.ForEachCommandBuffer(
		[&](const auto& commandBuffer)
		{
			Internal::ProcessCommands(mWorld, commandBuffer);
		});

	EvaporatePheromones();
	AgeAnts();
	mWorld.GetRegistry().RemovedDestroyed();

	mNumStepsCompleted++;
}

void Ant::GameState::EvaporatePheromones()
{
	CE::Registry& reg = mWorld.GetRegistry();
	auto view = reg.View<CE::TransformComponent, PheromoneComponent>(entt::exclude_t<InactivePheromoneTag>{});
	auto& inactiveStorage = reg.Storage<InactivePheromoneTag>();
	for (auto entity : view)
	{
		auto& pheromone = view.get<PheromoneComponent>(entity);
		pheromone.mAmount -= PheromoneComponent::sEvaporationPerSecond;

		if (pheromone.mAmount <= 0.0f)
		{
			CE::TransformComponent& transform = view.get<CE::TransformComponent>(entity);
			transform.SetLocalPosition(transform.GetLocalPosition() + CE::To3D({ 1'000'000, 1'000'000 }));
			inactiveStorage.emplace(entity);
		}
	}
}

void Ant::GameState::AgeAnts()
{
	CE::Registry& reg = mWorld.GetRegistry();
	for (auto [entity, ant] : reg.View<AntBaseComponent>().each())
	{
		ant.mTimeLeftAlive -= sStepDurationSeconds;

		if (ant.mTimeLeftAlive <= 0.0f)
		{
			reg.Destroy(entity, false);
		}
	}
}
