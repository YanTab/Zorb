#include "ZorbTuningSettings.h"

const UZorbTuningSettings* UZorbTuningSettings::Get()
{
    return GetDefault<UZorbTuningSettings>();
}

FName UZorbTuningSettings::GetCategoryName() const
{
    return TEXT("Game");
}
