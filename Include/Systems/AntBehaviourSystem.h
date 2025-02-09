#pragma once
#include "Systems/System.h"
#include "Utilities/Events.h"

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
		CE::SystemStaticTraits GetStaticTraits() const override
		{
			CE::SystemStaticTraits traits{};
			traits.mFixedTickInterval = sAntTickInterval;
			return traits;
		}

		void Update(CE::World& world, float dt) override;

	private:
		friend CE::ReflectAccess;
		static CE::MetaType Reflect();
		REFLECT_AT_START_UP(AntBehaviourSystem);
	};
}
