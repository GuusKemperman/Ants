#pragma once
#include "World/World.h"

namespace Ant
{
	class GameStep;

	class GameState
	{
	public:
		GameState();

		static constexpr float sStepDurationSeconds = 1.0f;

		void Step(const GameStep& step);

		uint32 GetNumOfStepsCompleted() const { return mNumStepsCompleted; }
		uint32 GetScore() const { return mScore; }

		CE::World& GetWorld() { return mWorld; }
		const CE::World& GetWorld() const { return mWorld; }

	private:
		void EvaporatePheromones();
		void AgeAnts();

		CE::World mWorld;
		uint32 mNumStepsCompleted{};
		uint32 mScore{};
	};
}
