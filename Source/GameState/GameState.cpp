#include "Precomp.h"
#include "GameState/GameState.h"

#include "Assets/Level.h"
#include "Commands/GameStep.h"
#include "Components/AntNestComponent.h"
#include "Core/AssetManager.h"
#include "World/Registry.h"
#include "World/World.h"

namespace Internal
{
	template<typename T>
	static void ProcessCommands(CE::World& world, const Ant::CommandBuffer<T>& commandBuffer)
	{
		T::Execute(world, commandBuffer.GetSubmittedCommands());
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

	CE::Registry& reg = mWorld.GetRegistry();

	for (auto [entity, nest] : reg.View<AntNestComponent>().each())
	{
		nest.SpendFoodOnSpawning(mWorld, entity);
	}

	step.ForEachCommandBuffer(
		[&](const auto& commandBuffer)
		{
			Internal::ProcessCommands(mWorld, commandBuffer);
		});

	reg.RemovedDestroyed();
}
