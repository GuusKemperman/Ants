#include "Precomp.h"
#include "Commands/InteractCommand.h"

#include "Components/AntBaseComponent.h"
#include "Components/FoodPelletTag.h"
#include "Components/Physics2D/DiskColliderComponent.h"
#include "Components/Physics2D/PhysicsBody2DComponent.h"
#include "Components/TransformComponent.h"
#include "World/Registry.h"
#include "World/World.h"

void Ant::InteractCommand::Execute(CE::World& world, std::span<const InteractCommand> commands)
{
	CE::Registry& reg = world.GetRegistry();
	auto antView = reg.View<AntBaseComponent>();
	auto foodView = reg.View<FoodPelletTag>();
	auto transformView = reg.View<CE::TransformComponent>();

	for (const InteractCommand& command : commands)
	{
		AntBaseComponent& ant = antView.get<AntBaseComponent>(command.mAnt);

		if (ant.mIsHoldingFood)
		{
			continue;
		}

		if (!foodView.contains(command.mInteractedWith))
		{
			continue;
		}

		ant.mIsHoldingFood = true;
		reg.RemoveComponent<FoodPelletTag>(command.mInteractedWith);
		reg.RemoveComponent<CE::PhysicsBody2DComponent>(command.mInteractedWith);
		reg.RemoveComponent<CE::DiskColliderComponent>(command.mInteractedWith);

		CE::TransformComponent& antTransform = transformView.get<CE::TransformComponent>(command.mAnt);
		CE::TransformComponent& foodTransform = transformView.get<CE::TransformComponent>(command.mInteractedWith);

		foodTransform.SetLocalPosition(CE::sForward * 1.5f);
		foodTransform.SetParent(&antTransform, false);
	}
}
