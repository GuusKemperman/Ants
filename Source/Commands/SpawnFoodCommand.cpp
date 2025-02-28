#include "Precomp.h"
#include "Commands/SpawnFoodCommand.h"

#include "World/World.h"
#include "World/Registry.h"
#include "Components/FoodPelletTag.h"
#include "Components/TransformComponent.h"
#include "Components/Physics2D/DiskColliderComponent.h"
#include "Components/Physics2D/PhysicsBody2DComponent.h"

void Ant::SpawnFoodCommand::Execute(CE::World& world, std::span<const SpawnFoodCommand> commands)
{
	CE::Registry& reg = world.GetRegistry();

	auto& foods = reg.Storage<FoodPelletTag>();
	auto& transforms = reg.Storage<CE::TransformComponent>();
	auto& disks = reg.Storage<CE::DiskColliderComponent>();
	auto& physics = reg.Storage<CE::PhysicsBody2DComponent>();

	for (const SpawnFoodCommand& command : commands)
	{
		entt::entity entity = reg.Create();

		foods.emplace(entity);
		transforms.emplace(entity).SetWorldPosition(command.mLocation);
		disks.emplace(entity, CE::DiskColliderComponent{ .mRadius = 1.0f });

		CE::PhysicsBody2DComponent& body = physics.emplace(entity);
		body.mIsAffectedByForces = false;
		body.mRules = {};
		body.mRules.mLayer = CE::CollisionLayer::WorldDynamic;
		body.mRules.SetResponse(CE::CollisionLayer::Query, CE::CollisionResponse::Overlap);
	}
}
