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
		static bool IsCarryingFood(const CE::World& world, entt::entity owner);

		static constexpr float sMaxSenseRange = 5.0f;
		static constexpr float sInteractRange = 2.0f;
		static constexpr float sMaxDetectPheromonesRange = 15.0f;
		static constexpr float sPheromoneDetectionSampleRadius = 5.0f;

		static void Interact(CE::World& world, entt::entity owner);

		static void Move(CE::World& world, entt::entity owner, glm::vec2 towardsLocation);

		static SenseResult Sense(CE::World& world, entt::entity owner, glm::vec2 senseLocation);

		static float DetectPheromones(CE::World& world, entt::entity owner, glm::vec2 senseLocation, PheromoneId pheromoneId);

		static void EmitPheromones(CE::World& world, entt::entity owner, PheromoneId pheromoneId);

		glm::vec2 mWorldPosition{};
		glm::vec2 mPreviousWorldPosition{};

		float mWorldOrientation{};
		float mPreviousWorldOrientation{};

		bool mIsHoldingFood{};

	private:
		friend CE::ReflectAccess;
		static CE::MetaType Reflect();
		REFLECT_AT_START_UP(AntBaseComponent);
	};
}