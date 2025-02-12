#include "Precomp.h"
#include "Systems/AntBehaviourSystem.h"

#include "Components/AntBaseComponent.h"
#include "Components/AntNestComponent.h"
#include "Core/ThreadPool.h"
#include "World/EventManager.h"
#include "World/Physics.h"
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

	CE::Registry& reg = world.GetRegistry();

	for (auto [entity, nest] : reg.View<AntNestComponent>().each())
	{
		nest.SpendFoodOnSpawning(world, entity);
	}

	Internal::ProcessCommands(world, mMoveCommandBuffer);
	Internal::ProcessCommands(world, mInteractCommandBuffer);

	reg.RemovedDestroyed();
	world.GetPhysics().RebuildBVHs();

	mCollectCommandsFuture = CE::ThreadPool::Get().Enqueue([&world]()
		{
			world.GetEventManager().InvokeEventsForAllComponents(sOnAntTick);
		}
	);
}

CE::MetaType Ant::AntBehaviourSystem::Reflect()
{
	return { CE::MetaType::T<AntBehaviourSystem>{}, "AntBehaviourSystem", CE::MetaType::Base<System>{} };
}
