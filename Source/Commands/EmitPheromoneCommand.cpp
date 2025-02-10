#include "Precomp.h"
#include "Commands/EmitPheromoneCommand.h"

#include "Assets/Prefabs/Prefab.h"
#include "Components/TransformComponent.h"
#include "Core/AssetManager.h"
#include "World/Registry.h"
#include "World/World.h"

void Ant::EmitPheromoneCommand::Execute(CE::World& world, std::span<const EmitPheromoneCommand> commands)
{
	CE::AssetHandle pheromonePrefab = CE::AssetManager::Get().TryGetAsset<CE::Prefab>("PF_Pheromone");

	if (pheromonePrefab == nullptr)
	{
		LOG(LogGame, Error, "No pheromone prefab!");
		return;
	}

	auto& pheromoneComponentStorage = world.GetRegistry().Storage<PheromoneComponent>();

	for (const EmitPheromoneCommand& command : commands)
	{
		glm::vec3 pos = CE::To3D(command.mLocation);
		entt::entity createdEntity = world.GetRegistry().CreateFromPrefab(*pheromonePrefab, entt::null, &pos);

		if (!pheromoneComponentStorage.contains(createdEntity))
		{
			LOG(LogGame, Error, "Pheromone prefab did not spawn entity, or entity did not contain pheromone component");
			world.GetRegistry().Destroy(createdEntity, true);
			return;
		}

		PheromoneComponent& pheromoneComponent = pheromoneComponentStorage.get(createdEntity);
		pheromoneComponent.mPheromoneId = command.mPheromoneId;
	}
}
