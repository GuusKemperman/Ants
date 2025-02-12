#pragma once
#include "World/World.h"

namespace Ant
{
	class GameStep;

	class GameState
	{
	public:
		GameState();

		void Step(const GameStep& step);

		CE::World& GetWorld() { return mWorld; }
		const CE::World& GetWorld() const { return mWorld; }

	private:
		CE::World mWorld;
	};
}
