#pragma once
#include "PheromoneComponent.h"
#include "Meta\Fwd\MetaReflectFwd.h"

namespace CE
{
	class World;
}

namespace Ant
{
	struct SenseResult
	{
		bool SensedComponent(const CE::World& world, CE::TypeId componentTypeId) const;

		bool SensedFood(const CE::World& world) const;

		bool SensedNest(const CE::World& world) const;

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
		void OnBeginPlay(CE::World& world, entt::entity owner);

		static bool IsCarryingFood(const CE::World& world, entt::entity owner);

		static constexpr float sInteractRange = 2.0f;

		static void Interact(CE::World& world, entt::entity owner);

		static void Move(CE::World& world, entt::entity owner, glm::vec2 towardsLocation);

		static SenseResult Sense(const CE::World& world, entt::entity owner, glm::vec2 senseLocation);

		static float DetectPheromones(const CE::World& world, entt::entity owner, glm::vec2 senseLocation, PheromoneId pheromoneId);

		static void EmitPheromones(CE::World& world, entt::entity owner, PheromoneId pheromoneId);

		glm::vec2 mPreviousWorldPosition{};
		glm::quat mPreviousWorldOrientation{};

		glm::vec2 mWorldPosition{};
		glm::quat mWorldOrientation{};

		bool mIsHoldingFood{};

	private:
		friend CE::ReflectAccess;
		static CE::MetaType Reflect();
		REFLECT_AT_START_UP(AntBaseComponent);
	};
}