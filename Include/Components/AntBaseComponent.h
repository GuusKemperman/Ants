#pragma once
#include "Meta\Fwd\MetaReflectFwd.h"

namespace CE
{
	class World;
}

namespace Ant
{
	struct SenseResult
	{
		bool HitFood(const CE::World& world) const;

		float GetDistance() const;

		entt::entity mHitEntity{};
		float mDist{};

	private:
		friend CE::ReflectAccess;
		static CE::MetaType Reflect();
		REFLECT_AT_START_UP(SenseResult);
	};

	class AntBaseComponent
	{
	public:
		// static float GetCurrentEnergy(const CE::World& world, entt::entity owner);

		// static bool IsCarryingFood(const CE::World& world, entt::entity owner);

		// static void PickUp(const CE::World& world, entt::entity owner);

		// static void Drop(const CE::World& world, entt::entity owner);

		// static void Rest(const CE::World& world, entt::entity owner);

		static void Move(CE::World& world, entt::entity owner, glm::vec2 towardsLocation);

		static SenseResult Sense(const CE::World& world, entt::entity owner, glm::vec2 senseLocation);

	private:
		float mEnergy = 100.0f;



		friend CE::ReflectAccess;
		static CE::MetaType Reflect();
		REFLECT_AT_START_UP(AntBaseComponent);
	};
}