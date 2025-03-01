#include "Precomp.h"
#include "Core/Engine.h"

namespace CE
{
	class PhysicsSystem;
}

int main(int argc, char* argv[])
{
	CE::EngineConfig config{ argc, argv };
	config.mGameDir = GAME_DIR;

	// I want better control over when the BVHs are rebuild.
	config.BanType<CE::PhysicsSystem>();

	CE::Engine engine{ config };
	engine.Run("L_Training");

	return 0;
}
