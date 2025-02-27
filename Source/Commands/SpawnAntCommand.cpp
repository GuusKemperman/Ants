#include "Precomp.h"
#include "Commands/SpawnAntCommand.h"

#include "World/World.h"
#include "World/Registry.h"
#include "Components/AntBaseComponent.h"

void Ant::SpawnAntCommand::Execute(CE::World& world, std::span<const SpawnAntCommand> commands)
{
	CE::Registry& reg = world.GetRegistry();

	auto& antStorage = reg.Storage<AntBaseComponent>();
	const CE::MetaType* playerScript = CE::MetaManager::Get().TryGetType("S_WorkerAnt");

	if (playerScript == nullptr)
	{
		LOG(LogGame, Error, "No player script! Cannot spawn ant");
		return;
	}

	for (const SpawnAntCommand& command : commands)
	{
		entt::entity entity = reg.Create();
		reg.AddComponent(*playerScript, entity);

		antStorage.emplace(entity,
			AntBaseComponent{
				.mWorldPosition = {},
				.mPreviousWorldPosition = {},
				.mWorldOrientation = command.mOrientation,
				.mPreviousWorldOrientation = command.mOrientation,
			});
	}
}
