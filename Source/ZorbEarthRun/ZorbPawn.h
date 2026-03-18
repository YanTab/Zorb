#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ZorbTuningTypes.h"
#include "ZorbPawn.generated.h"

UENUM(BlueprintType)
enum class EZorbTypePreset : uint8
{
    Classic UMETA(DisplayName = "Classic"),
    Agile UMETA(DisplayName = "Agile"),
    Heavy UMETA(DisplayName = "Heavy"),
    Wild UMETA(DisplayName = "Wild")
};

UCLASS(Blueprintable)
class AZorbPawn : public APawn
{
    GENERATED_BODY()

public:
    AZorbPawn();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    void MoveForward(float Value);
    void MoveRight(float Value);
    void StartBoost();
    void StopBoost();

    float GetEnergyPercent() const;
    float GetHeatPercent() const;
    int32 GetOverheatLevel() const;
    bool IsBoosting() const;
    float GetCriticalFlashAlpha() const;
    float GetGroundSlopePercent() const;
    bool IsGrounded() const;
    void PrepareForAutomatedScenario(const FTransform& StartTransform, const FVector& InitialLinearVelocity);
    void ApplyAutomatedScenarioInput(float ForwardValue, float RightValue, bool bBoostEnabled);
    void ResetAutomatedScenarioInput();
    void RestartTelemetrySession();
    void EndTelemetrySession();

private:
    void RespawnToCheckpoint();
    void UpdateEnergyAndHeat(float DeltaTime);
    void TriggerCriticalOverheat();
    void ApplyProjectTuningIfEnabled();
    void ResetInternalMassState();
    void UpdateInternalMassModel(float DeltaTime, const FVector& WorldLinearAcceleration, bool bGroundedForMovement);
    void BeginTelemetrySession(bool bIgnoreAutoStart = false);
    void RecordTelemetrySample(float TimeSeconds, const FVector& Position, const FVector& Velocity, bool bGrounded, const FVector& GroundNormal);
    float ComputeGroundSlopePercent(const FVector& GroundNormal, bool bGrounded) const;

    UPROPERTY(VisibleAnywhere)
    class USphereComponent* CollisionComponent;

    UPROPERTY(VisibleAnywhere)
    class UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere)
    class USpringArmComponent* SpringArmComponent;

    UPROPERTY(VisibleAnywhere)
    class UCameraComponent* CameraComponent;

    UPROPERTY(VisibleAnywhere)
    class UStaticMeshComponent* HeatHaloComponent;

    UPROPERTY(Transient)
    class UMaterialInstanceDynamic* HeatHaloMID;

    UPROPERTY(EditAnywhere, Category = "Zorb Physics|Setup")
    bool bUseProjectTuningSettings;

    UPROPERTY(EditAnywhere, Category = "Zorb Physics|Setup")
    EZorbTypePreset ZorbTypePreset;

    UPROPERTY(EditAnywhere, Category = "Zorb Physics|Local Override", meta = (ShowOnlyInnerProperties))
    FZorbMovementTuning MovementTuning;

    UPROPERTY(EditAnywhere, Category = "Zorb Physics|Local Override", meta = (ShowOnlyInnerProperties))
    FZorbEnergyTuning EnergyTuning;

    UPROPERTY(EditAnywhere, Category = "Zorb Physics|Local Override", meta = (ShowOnlyInnerProperties))
    FZorbRespawnTuning RespawnTuning;

    UPROPERTY(EditAnywhere, Category = "Zorb Physics|Local Override", meta = (ShowOnlyInnerProperties))
    FZorbFeedbackTuning FeedbackTuning;

    float CurrentEnergy;
    float CurrentHeat;
    float ForwardInputValue;
    float RightInputValue;
    float SmoothedForwardInputValue;
    float SmoothedRightInputValue;
    float SteeringRightInputValue;
    float OverheatLockRemaining;
    float CriticalFlashRemaining;
    float HeatHaloCurrentOpacity;
    float HeatHaloCurrentScale;
    FLinearColor HeatHaloCurrentColor;
    bool bAutomationInputOverrideActive;
    bool bBoostRequested;
    bool bBoostActive;
    FRotator DesiredCameraRotation;
    float CameraGroundOffsetZ;
    float FrozenAirbornePitch;
    bool bHasFrozenAirbornePitch;
    FVector CameraPlanarForward;
    FVector CameraStableTravelDir;
    float CameraReverseIntentHoldTime;
    float CameraLookAheadDistance;
    float CameraCollisionLiftZ;
    float CameraBaseFov;
    float CameraTargetFov;
    FVector InternalMassOffsetLocal;
    FVector InternalMassVelocityLocal;
    FVector PreviousLinearVelocity;
    bool bHasPreviousLinearVelocity;

    // Camera target-follow state
    FVector CameraPivotWorld; // smoothed pivot world position
    float CameraMaxVerticalStepPerSecond;
    float TelemetrySampleAccumulator;
    bool bTelemetrySessionStarted;
    FString ActiveTelemetryFilePath;

    void UpdateCameraRotation(float DeltaTime);
    void UpdateCameraPivotAndHeight(float DeltaTime, const FVector& BallPos, const FVector& VelocityXY);
    FVector ComputeSafePlanarDirection(const FVector& VelocityXY) const;
};
