#include "Precomp.h"
#include "Components/FoodPelletTag.h"

#include "Meta/MetaType.h"
#include "Utilities/Reflect/ReflectComponentType.h"

CE::MetaType Ant::FoodPelletTag::Reflect()
{
	CE::MetaType metaType{ CE::MetaType::T<FoodPelletTag>{}, "FoodPelletTag" };
	CE::ReflectComponentType<FoodPelletTag>(metaType);
	return metaType;
}
