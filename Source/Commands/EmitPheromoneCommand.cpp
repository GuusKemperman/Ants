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

	for (const EmitPheromoneCommand& command : commands)
	{
		entt::entity entity = reg.Create();

		pheromoneComponentStorage.emplace(entity, command.mPheromoneId);
		transforms.emplace(entity).SetWorldPosition(command.mLocation);
		disks.emplace(entity, CE::DiskColliderComponent{ PheromoneComponent::sRadius });

		CE::PhysicsBody2DComponent& body = physics.emplace(entity);
		body.mIsAffectedByForces = false;
		body.mRules = {};
		body.mRules.mLayer = CE::CollisionLayer::Query;
		body.mRules.SetResponse(CE::CollisionLayer::Query, CE::CollisionResponse::Overlap);
	}
}

bool Ant::EmitPheromoneCommand::CanSpawnPheromoneNextTick(const GameState& state)
{
	return state.GetNumOfStepsCompleted() % sPheromoneSpawnInterval == 0;
}
