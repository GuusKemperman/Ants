#include "Precomp.h"
#include "Commands/MoveCommand.h"

#include "Components/AntBaseComponent.h"
#include "World/Registry.h"
#include "World/World.h"

void Ant::MoveCommand::Execute(CE::World& world, std::span<const MoveCommand> commands)
{
	auto antView = world.GetRegistry().View<AntBaseComponent>();

	for (const MoveCommand& command : commands)
	{
		AntBaseComponent& ant = antView.get<AntBaseComponent>(command.mAnt);

		ant.mPreviousWorldPosition = std::exchange(ant.mWorldPosition, command.mNewPosition);
		ant.mPreviousWorldOrientation = std::exchange(ant.mWorldOrientation, command.mNewOrientation);
	}
}
