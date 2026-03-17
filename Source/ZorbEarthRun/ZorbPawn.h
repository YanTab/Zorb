#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ZorbTuningTypes.h"
#include "ZorbPawn.generated.h"

UCLASS(Blueprintable)
class AZorbPawn : public APawn
{
    GENERATED_BODY()

public:
    AZorbPawn();

protected:
    virtual void BeginPlay() override;

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

private:
    void RespawnToCheckpoint();
    void UpdateEnergyAndHeat(float DeltaTime);
    void TriggerCriticalOverheat();
    void ApplyProjectTuningIfEnabled();

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
    float OverheatLockRemaining;
    float CriticalFlashRemaining;
    float HeatHaloCurrentOpacity;
    float HeatHaloCurrentScale;
    FLinearColor HeatHaloCurrentColor;
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

    // Camera target-follow state
    FVector CameraPivotWorld; // smoothed pivot world position
    float CameraMaxVerticalStepPerSecond;

    void UpdateCameraRotation(float DeltaTime);
    void UpdateCameraPivotAndHeight(float DeltaTime, const FVector& BallPos, const FVector& VelocityXY);
    FVector ComputeSafePlanarDirection(const FVector& VelocityXY) const;
};
