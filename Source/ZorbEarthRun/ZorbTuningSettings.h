#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ZorbTuningTypes.h"
#include "ZorbTuningSettings.generated.h"

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Zorb Tuning"))
class ZORBEARTHRUN_API UZorbTuningSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    static const UZorbTuningSettings* Get();

    virtual FName GetCategoryName() const override;

    UPROPERTY(Config, EditAnywhere, Category = "Zorb Physics", meta = (ShowOnlyInnerProperties))
    FZorbMovementTuning Movement;

    UPROPERTY(Config, EditAnywhere, Category = "Zorb Physics", meta = (ShowOnlyInnerProperties))
    FZorbEnergyTuning Energy;

    UPROPERTY(Config, EditAnywhere, Category = "Zorb Physics", meta = (ShowOnlyInnerProperties))
    FZorbRespawnTuning Respawn;

    UPROPERTY(Config, EditAnywhere, Category = "Zorb Feedback", meta = (ShowOnlyInnerProperties))
    FZorbFeedbackTuning Feedback;
};
