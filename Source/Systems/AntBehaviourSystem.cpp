#include "Precomp.h"
#include "Systems/AntBehaviourSystem.h"

#include "Components/AntBaseComponent.h"
#include "Core/ThreadPool.h"
#include "World/EventManager.h"
#include "World/Registry.h"
#include "World/World.h"

namespace Internal
{
	template<typename T>
	static void ProcessCommands(CE::World& world, Ant::CommandBuffer<T>& commandBuffer)
	{
		auto antView = world.GetRegistry().View<Ant::AntBaseComponent>();

		T::Execute(world, commandBuffer.GetSubmittedCommands());

		commandBuffer.Clear();
		commandBuffer.mCommands.resize(antView.size());
	}
}

Ant::AntBehaviourSystem::~AntBehaviourSystem()
{
	if (mCollectCommandsFuture.valid())
	{
		mCollectCommandsFuture.wait();
	}
}

void Ant::AntBehaviourSystem::Update(CE::World& world, float)
{
	if (mCollectCommandsFuture.valid())
	{
		mCollectCommandsFuture.get();
	}

	Internal::ProcessCommands(world, mMoveCommandBuffer);
	Internal::ProcessCommands(world, mInteractCommandBuffer);

	//mCollectCommandsFuture = CE::ThreadPool::Get().Enqueue([&world]()
		{
			world.GetEventManager().InvokeEventsForAllComponents(sOnAntTick);
		}
	//);
}

CE::MetaType Ant::AntBehaviourSystem::Reflect()
{
	return { CE::MetaType::T<AntBehaviourSystem>{}, "AntBehaviourSystem", CE::MetaType::Base<System>{} };
}
