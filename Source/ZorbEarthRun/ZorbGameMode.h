#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ZorbGameMode.generated.h"

UCLASS()
class AZorbGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AZorbGameMode();

    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable)
    void OnRaceFinished();

    UFUNCTION(BlueprintPure)
    float GetElapsedRaceTime() const;

    UFUNCTION(BlueprintPure)
    bool IsRaceFinished() const;

    UFUNCTION(BlueprintCallable)
    void SetLastCheckpointTransform(const FTransform& NewCheckpointTransform);

    UFUNCTION(BlueprintPure)
    FTransform GetLastCheckpointTransform() const;

private:
    float RaceStartTime;
    bool bRaceFinished;
    FTransform LastCheckpointTransform;
};
