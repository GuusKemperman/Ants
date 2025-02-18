#include "Precomp.h"
#include "Commands/InteractCommand.h"

#include "Components/AntBaseComponent.h"
#include "Components/AntNestComponent.h"
#include "Components/FoodPelletTag.h"
#include "World/Registry.h"
#include "World/World.h"

void Ant::InteractCommand::Execute(CE::World& world, std::span<const InteractCommand> commands)
{
	CE::Registry& reg = world.GetRegistry();
	auto antView = reg.View<AntBaseComponent>();
	auto foodView = reg.View<FoodPelletTag>();
	auto nestView = reg.View<AntNestComponent>();

	for (const InteractCommand& command : commands)
	{
		AntBaseComponent& ant = antView.get<AntBaseComponent>(command.mAnt);

		if (ant.mIsHoldingFood)
		{
			if (nestView.contains(command.mInteractedWith))
			{
				ant.mIsHoldingFood = false;
				nestView.get<AntNestComponent>(command.mInteractedWith).DepositFood(1.0f);
			}
			continue;
		}

		if (!foodView.contains(command.mInteractedWith))
		{
			continue;
		}

		ant.mIsHoldingFood = true;

		// Prevents others from grabbing the same one in this turn
		reg.RemoveComponent<FoodPelletTag>(command.mInteractedWith);
		reg.Destroy(command.mInteractedWith, true);
	}
}
