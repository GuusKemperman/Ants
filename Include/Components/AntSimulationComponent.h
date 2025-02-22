#pragma once
#include "Commands/GameStep.h"
#include "GameState/GameState.h"
#include "Utilities/Events.h"

namespace Ant
{
	struct OnAntTick :
		CE::EventType<OnAntTick>
	{
		OnAntTick() :
			EventType("OnAntTick")
		{
		}
	};
	inline OnAntTick sOnAntTick{};

	class GameStep;

	class AntSimulationComponent
	{
	public:
		~AntSimulationComponent();

		void OnBeginPlay(CE::World& world, entt::entity owner);

		const GameState& GetGameState() const { return mCurrentState; }

		static const AntSimulationComponent* TryGetOwningSimulationComponent(const CE::World& world);
		static const GameState* TryGetGameState(const CE::World& world);

		static GameStep* TryGetNextGameStep(CE::World& world);

		template<typename T>
		static void RecordCommand(CE::World& world, T&& command);

		void StartSimulation(CE::World* viewportWorld);

		uint32 mMinNumOfFoodInWorld = 100;
		uint32 mNumOfFoodToSpawn = 200;
		float mFoodSpawnDistanceIncrementFactor = 1.5f;
		float mFoodSpawnDist = 1.0f;
		float mFoodClusterRadiusPerPellet = .4f;

		uint32 mStepsSimulated = 0;

	private:
		void CollectSpawnFoodCommands(CE::World& world, CommandBuffer<SpawnFoodCommand>& commandBuffer);
		static void CollectSpawnAntsCommands(CE::World& world, GameStep& nextStep);

		GameState mCurrentState{};
		std::function<void(const GameStep&)> mOnStepCompletedCallback{};

		std::thread mSimulateThread{};
		bool mSimulationStopped{};

		friend CE::ReflectAccess;
		static CE::MetaType Reflect();
		REFLECT_AT_START_UP(AntSimulationComponent);
	};

	template <typename T>
	void AntSimulationComponent::RecordCommand(CE::World& world, T&& command)
	{
		GameStep* step = TryGetNextGameStep(world);

		if (step == nullptr)
		{
			LOG(LogGame, Error, "Could not record command, no gamestep");
			return;
		}

		step->AddCommand(std::forward<T>(command));
	}
}
