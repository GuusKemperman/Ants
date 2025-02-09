#pragma once
#include "Assets/Core/AssetHandle.h"
#include "Meta/Fwd/MetaReflectFwd.h"

namespace CE
{
	class World;
	class Prefab;
}

namespace Ant
{
	class AntNestComponent
	{
	public:
		void DepositFood(float amount);

		void SpendFoodOnSpawning(CE::World& world, entt::entity owner);

		CE::AssetHandle<CE::Prefab> mWorkerAnt{};

		float mFood{};

		float mAntCost = 1.0f;

	private:
		friend CE::ReflectAccess;
		static CE::MetaType Reflect();
		REFLECT_AT_START_UP(AntNestComponent);
	};
}
