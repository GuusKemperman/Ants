#pragma once
#include "Meta/Fwd/MetaReflectFwd.h"

namespace Ant
{
	class FoodPelletTag
	{
		friend CE::ReflectAccess;
		static CE::MetaType Reflect();
		REFLECT_AT_START_UP(FoodPelletTag);
	};
}