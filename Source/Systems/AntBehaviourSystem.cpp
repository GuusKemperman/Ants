#include "Precomp.h"
#include "Systems/AntBehaviourSystem.h"

#include "World/EventManager.h"
#include "World/World.h"

void Ant::AntBehaviourSystem::Update(CE::World& world, float)
{
	world.GetEventManager().InvokeEventsForAllComponents(sOnAntTick);
}

CE::MetaType Ant::AntBehaviourSystem::Reflect()
{
	return { CE::MetaType::T<AntBehaviourSystem>{}, "AntBehaviourSystem", CE::MetaType::Base<System>{} };
}
