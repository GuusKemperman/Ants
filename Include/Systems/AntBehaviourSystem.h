#pragma once
#include <future>

#include "Commands/CommandBuffer.h"
#include "Commands/InteractCommand.h"
#include "Commands/MoveCommand.h"
#include "Systems/System.h"
#include "Utilities/Events.h"
#include "Utilities/Time.h"

namespace Ant
{
	struct OnAntTick :
		CE::EventType<OnAntTick>
	{
		OnAntTick() :
			EventType("OnAntTick")
		{
		}
	};
	inline OnAntTick sOnAntTick{};

	static constexpr float sAntTickInterval = 1.0f;

	class AntBehaviourSystem :
		public CE::System
	{
	public:
		~AntBehaviourSystem();

		CE::SystemStaticTraits GetStaticTraits() const override
		{
			CE::SystemStaticTraits traits{};
			traits.mFixedTickInterval = sAntTickInterval;
			return traits;
		}

		void Update(CE::World& world, float dt) override;

		CommandBuffer<MoveCommand> mMoveCommandBuffer{};
		CommandBuffer<InteractCommand> mInteractCommandBuffer{};

	private:
		std::future<void> mCollectCommandsFuture{};

		friend CE::ReflectAccess;
		static CE::MetaType Reflect();
		REFLECT_AT_START_UP(AntBehaviourSystem);
	};
}
