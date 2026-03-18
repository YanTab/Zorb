#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ZorbTuningTypes.h"
#include "ZorbGameMode.generated.h"

UCLASS()
class AZorbGameMode : public AGameModeBase
{
    GENERATED_BODY()

    struct FGhostSample
    {
        float TimeSeconds = 0.f;
        FVector Position = FVector::ZeroVector;
    };

public:
    AZorbGameMode();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

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
    void TryStartAutomationScenario();
    void UpdateAutomationScenario(float DeltaSeconds);
    void FinishAutomationScenario();
    bool ResolveAutomationScenario(FZorbScenarioDefinition& OutScenario) const;
    void TryLoadGhostReplay();
    bool ResolveGhostCsvPath(FString& OutCsvPath) const;
    bool LoadGhostSamplesFromCsv(const FString& CsvPath);
    void UpdateGhostReplay(float ScenarioTimeSeconds);
    FVector EvaluateGhostPosition(float ScenarioTimeSeconds) const;
    void ResetGhostReplay();

    float RaceStartTime;
    bool bRaceFinished;
    FTransform LastCheckpointTransform;
    bool bAutomationScenarioStarted;
    bool bAutomationScenarioCompleted;
    float AutomationScenarioElapsed;
    FZorbScenarioDefinition ActiveAutomationScenario;
    TArray<FGhostSample> GhostSamples;
    int32 GhostSampleCursor;
    class AStaticMeshActor* GhostActor;
    float GhostVerticalOffset;
};
