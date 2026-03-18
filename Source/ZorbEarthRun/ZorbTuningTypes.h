#pragma once

#include "CoreMinimal.h"
#include "ZorbTuningTypes.generated.h"

USTRUCT(BlueprintType)
struct FZorbMovementTuning
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Zorb Type")
    uint8 ZorbTypePreset = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "1000.0", ClampMax = "500000.0"))
    float MoveForce = 35000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "1.0", ClampMax = "30.0"))
    float VelocityResponse = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
    float MaxDriveAcceleration = 1200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "100.0", ClampMax = "8000.0"))
    float BrakeDeceleration = 1800.f;

    // Below this planar speed, negative forward input acts as directional reverse input instead of brake.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
    float ReverseDirectionalSpeedThreshold = 120.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "200.0", ClampMax = "10000.0"))
    float MaxSpeed = 3200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.01", ClampMax = "5.0"))
    float DriveLinearDamping = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.01", ClampMax = "10.0"))
    float IdleLinearDamping = 1.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|FreeRoll", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float FreeRollLinearDamping = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|FreeRoll", meta = (ClampMin = "0.0", ClampMax = "2000.0"))
    float DownhillGravityAssist = 260.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|FreeRoll", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DownhillAssistMinSlope = 0.03f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Air", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float AirLinearDamping = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Air", meta = (ClampMin = "0.0", ClampMax = "2000.0"))
    float AirHorizontalDeceleration = 120.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0", ClampMax = "200000.0"))
    float LateralGripForce = 16000.f;

    // Centripetal turning rate (rad/s equivalent). Higher = tighter constant-radius turns at any speed.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float TurnRateScale = 1.1f;

    // Max centripetal acceleration (cm/s²) applied when steering. Caps sharpness at very high speeds.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "100.0", ClampMax = "20000.0"))
    float MaxSteeringAcceleration = 3200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.1", ClampMax = "30.0"))
    float DirectionResponse = 7.5f;

    // Dedicated steering input smoothing for digital inputs (keyboard/editor testing).
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Steering", meta = (ClampMin = "0.1", ClampMax = "30.0"))
    float SteeringInputRiseSpeed = 3.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Steering", meta = (ClampMin = "0.1", ClampMax = "30.0"))
    float SteeringInputReleaseSpeed = 2.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Steering", meta = (ClampMin = "0.1", ClampMax = "30.0"))
    float SteeringInputReverseSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Boost", meta = (ClampMin = "1.0", ClampMax = "5.0"))
    float BoostAccelerationMultiplier = 1.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Camera", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float CameraRotationSpeed = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Camera", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CameraInputDirectionThreshold = 0.18f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Camera", meta = (ClampMin = "1.0", ClampMax = "6.0"))
    float CameraInputTurnMultiplier = 1.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Camera", meta = (ClampMin = "1.0", ClampMax = "8.0"))
    float CameraTurnaroundBoostMultiplier = 2.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Camera", meta = (ClampMin = "50.0", ClampMax = "1500.0"))
    float CameraDistance = 400.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Camera", meta = (ClampMin = "-45", ClampMax = "0"))
    float CameraPitchAngle = -20.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Camera", meta = (ClampMin = "50.0", ClampMax = "1000.0"))
    float CameraGroundClearance = 260.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Camera", meta = (ClampMin = "200.0", ClampMax = "5000.0"))
    float CameraGroundTraceLength = 2200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Camera", meta = (ClampMin = "0.1", ClampMax = "30.0"))
    float CameraHeightAdjustSpeed = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Camera", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
    float CameraMaxGroundOffset = 400.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Camera|LookAhead", meta = (ClampMin = "0.0", ClampMax = "800.0"))
    float CameraLookAheadMin = 80.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Camera|LookAhead", meta = (ClampMin = "0.0", ClampMax = "1200.0"))
    float CameraLookAheadMax = 360.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Camera|LookAhead", meta = (ClampMin = "100.0", ClampMax = "10000.0"))
    float CameraLookAheadSpeedRef = 3200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Camera|LookAhead", meta = (ClampMin = "0.1", ClampMax = "30.0"))
    float CameraLookAheadInterpSpeed = 7.0f;

    // Camera target-follow tuning
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Camera|TargetFollow", meta = (ClampMin = "50.0", ClampMax = "2000.0"))
    float CameraHorizontalDistance = 600.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Camera|TargetFollow", meta = (ClampMin = "0.0", ClampMax = "2000.0"))
    float CameraVerticalOffsetBase = 260.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Camera|TargetFollow", meta = (ClampMin = "0.1", ClampMax = "30.0"))
    float CameraPositionSmoothingSpeed = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Camera|TargetFollow", meta = (ClampMin = "0.1", ClampMax = "30.0"))
    float CameraRotationSmoothingSpeed = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Camera|TargetFollow", meta = (ClampMin = "0.1", ClampMax = "30.0"))
    float CameraStableInterpBaseLateral = 2.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Camera|TargetFollow", meta = (ClampMin = "0.1", ClampMax = "30.0"))
    float CameraStableInterpBaseForward = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Camera|TargetFollow", meta = (ClampMin = "10.0", ClampMax = "2000.0"))
    float CameraTurnaroundSpeedThreshold = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Camera|TargetFollow", meta = (ClampMin = "10.0", ClampMax = "2000.0"))
    float CameraMaxVerticalStepPerSecond = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Camera|Collision", meta = (ClampMin = "0.0", ClampMax = "500.0"))
    float CameraCollisionLiftMax = 140.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Camera|Collision", meta = (ClampMin = "0.1", ClampMax = "30.0"))
    float CameraCollisionLiftInterpSpeed = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Camera|Collision", meta = (ClampMin = "0.0", ClampMax = "20.0"))
    float CameraCollisionFovBoost = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Camera|Collision", meta = (ClampMin = "0.1", ClampMax = "30.0"))
    float CameraCollisionFovInterpSpeed = 8.0f;
};

USTRUCT(BlueprintType)
struct FZorbEnergyTuning
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Energy", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float MaxEnergy = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Energy", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float MaxHeat = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Energy", meta = (ClampMin = "0.0"))
    float EnergyGainRate = 14.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Energy", meta = (ClampMin = "0.0"))
    float EnergyDrainRate = 5.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Energy", meta = (ClampMin = "0.0"))
    float HeatGainRate = 12.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Energy", meta = (ClampMin = "0.0"))
    float HeatCooldownRate = 22.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Energy", meta = (ClampMin = "0.1"))
    float CriticalOverheatLockDuration = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Energy|Feedback", meta = (ClampMin = "0.05", ClampMax = "5.0"))
    float CriticalFlashDuration = 0.45f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Energy|Boost", meta = (ClampMin = "0.0"))
    float BoostEnergyDrainRate = 28.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Energy|Boost", meta = (ClampMin = "0.0"))
    float BoostHeatGainRate = 20.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Energy|Boost", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float MinEnergyToBoost = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Energy", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float PostCriticalHeat = 28.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Energy", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float PostCriticalEnergy = 12.f;

    // Seuils de chaleur (en %) pour passer au niveau suivant (0 → 1 → 2 → 3)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Energy|Overheat Levels", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float OverheatLevel1Threshold = 33.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Energy|Overheat Levels", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float OverheatLevel2Threshold = 66.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Energy|Overheat Levels", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float OverheatLevel3Threshold = 95.f;
};

USTRUCT(BlueprintType)
struct FZorbRespawnTuning
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn", meta = (ClampMin = "-1000000.0", ClampMax = "10000.0"))
    float KillZ = -1000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn", meta = (ClampMin = "0.0", ClampMax = "5000.0"))
    float RespawnHeightOffset = 100.f;
};

USTRUCT(BlueprintType)
struct FZorbFeedbackTuning
{
    GENERATED_BODY()

    // Vitesse de transition entre les niveaux de chaleur du halo (lerp par frame)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback|Halo", meta = (ClampMin = "0.5", ClampMax = "30.0"))
    float HeatHaloBlendSpeed = 7.0f;

    // Niveau 1 — Jaune
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback|Halo")
    FLinearColor HaloLevel1Color = FLinearColor(1.0f, 0.95f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback|Halo", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HaloLevel1Opacity = 0.18f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback|Halo", meta = (ClampMin = "1.0", ClampMax = "3.0"))
    float HaloLevel1Scale = 1.10f;

    // Niveau 2 — Orange
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback|Halo")
    FLinearColor HaloLevel2Color = FLinearColor(1.0f, 0.45f, 0.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback|Halo", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HaloLevel2Opacity = 0.22f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback|Halo", meta = (ClampMin = "1.0", ClampMax = "3.0"))
    float HaloLevel2Scale = 1.14f;

    // Niveau 3 — Rouge critique
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback|Halo")
    FLinearColor HaloLevel3Color = FLinearColor(1.0f, 0.1f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback|Halo", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HaloLevel3Opacity = 0.26f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback|Halo", meta = (ClampMin = "1.0", ClampMax = "3.0"))
    float HaloLevel3Scale = 1.18f;
};

USTRUCT(BlueprintType)
struct FZorbTelemetryTuning
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
    bool bEnableTelemetry = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
    bool bAutoStartOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry", meta = (ClampMin = "1.0", ClampMax = "240.0"))
    float SampleRateHz = 20.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
    FString OutputFilePrefix = TEXT("zorb_run");
};

USTRUCT(BlueprintType)
struct FZorbScenarioInputKeyframe
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario", meta = (ClampMin = "0.0"))
    float TimeSeconds = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
    float ForwardInput = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
    float RightInput = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario")
    bool bBoostActive = false;
};

USTRUCT(BlueprintType)
struct FZorbScenarioDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario")
    FName ScenarioName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario")
    FVector StartLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario")
    FRotator StartRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario")
    FVector InitialLinearVelocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario", meta = (ClampMin = "0.1"))
    float DurationSeconds = 3.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario")
    TArray<FZorbScenarioInputKeyframe> InputTimeline;
};

USTRUCT(BlueprintType)
struct FZorbAutomationTuning
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Automation")
    bool bEnableScenarioRunner = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Automation")
    FName ScenarioName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Automation")
    bool bRestartTelemetryOnScenarioStart = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Automation", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float StartupDelaySeconds = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Automation")
    TArray<FZorbScenarioDefinition> Scenarios;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Automation|Ghost")
    bool bEnableGhostReplay = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Automation|Ghost")
    bool bUseLatestTelemetryGhost = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Automation|Ghost", meta = (EditCondition = "!bUseLatestTelemetryGhost"))
    FString GhostTelemetryFile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Automation|Ghost", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float GhostVisualScale = 1.0f;
};
