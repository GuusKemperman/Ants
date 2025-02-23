#include "Precomp.h"
#include "Commands/EmitPheromoneCommand.h"

#include "Assets/Prefabs/Prefab.h"
#include "Components/TransformComponent.h"
#include "Components/Physics2D/DiskColliderComponent.h"
#include "Components/Physics2D/PhysicsBody2DComponent.h"
#include "Core/AssetManager.h"
#include "GameState/GameState.h"
#include "World/Registry.h"
#include "World/World.h"

void Ant::EmitPheromoneCommand::Execute(CE::World& world, std::span<const EmitPheromoneCommand> commands)
{
	CE::Registry& reg = world.GetRegistry();

	auto& pheromoneComponentStorage = reg.Storage<PheromoneComponent>();
	auto& transforms = reg.Storage<CE::TransformComponent>();
	auto& disks = reg.Storage<CE::DiskColliderComponent>();
	auto& physics = reg.Storage<CE::PhysicsBody2DComponent>();

	auto& inactiveStorage = reg.Storage<InactivePheromoneTag>();

	if (inactiveStorage.size() < commands.size())
	{
		for (size_t i = 0; i < commands.size() * 2; i++)
		{
			entt::entity entity = reg.Create();
			inactiveStorage.emplace(entity);

			pheromoneComponentStorage.emplace(entity);
			transforms.emplace(entity).SetWorldPosition(commands[i / 2].mLocation + glm::vec2{1'000'000} * static_cast<float>((i / 2) + i % 2));
			disks.emplace(entity, CE::DiskColliderComponent{ PheromoneComponent::sRadius });

			CE::PhysicsBody2DComponent& body = physics.emplace(entity);
			body.mIsAffectedByForces = false;
			body.mRules = {};
			body.mRules.mLayer = CE::CollisionLayer::Query;
			body.mRules.SetResponse(CE::CollisionLayer::Query, CE::CollisionResponse::Overlap);
		}
	}

	auto inactiveRange = inactiveStorage.each();
	auto currentInactive = inactiveRange.begin();

	for (const EmitPheromoneCommand& command : commands)
	{
		auto [entity] = *currentInactive;
		inactiveStorage.erase(entity);
		++currentInactive;
		auto& transform = transforms.get(entity);
		auto& pheromone = pheromoneComponentStorage.get(entity);

		transform.SetWorldPosition(command.mLocation);
		pheromone.mAmount = command.mAmount;
		pheromone.mPheromoneId = command.mPheromoneId;
	}
}

bool Ant::EmitPheromoneCommand::CanSpawnPheromoneNextTick(const GameState& state)
{
	return state.GetNumOfStepsCompleted() % sPheromoneSpawnInterval == 0;
}
