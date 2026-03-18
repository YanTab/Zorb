#include "ZorbPawn.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/Engine.h"
#include "HAL/FileManager.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/ConstructorHelpers.h"
#include "ZorbGameMode.h"
#include "ZorbTuningSettings.h"

namespace
{
struct FZorbTypeBehavior
{
    float ShellMassMultiplier = 1.f;
    float MaxSpeedMultiplier = 1.f;
    float TurnRateMultiplier = 1.f;
    float GripMultiplier = 1.f;
    float BrakeMultiplier = 1.f;
    float InternalSpringMultiplier = 1.f;
    float InternalDampingMultiplier = 1.f;
    float InternalNoiseMultiplier = 1.f;
    float InternalReactionMultiplier = 1.f;
};

FZorbTypeBehavior GetZorbTypeBehavior(EZorbTypePreset TypePreset)
{
    FZorbTypeBehavior Behavior;

    switch (TypePreset)
    {
    case EZorbTypePreset::Agile:
        Behavior.ShellMassMultiplier = 0.9f;
        Behavior.MaxSpeedMultiplier = 1.05f;
        Behavior.TurnRateMultiplier = 1.25f;
        Behavior.GripMultiplier = 1.15f;
        Behavior.BrakeMultiplier = 1.05f;
        Behavior.InternalSpringMultiplier = 1.2f;
        Behavior.InternalDampingMultiplier = 1.1f;
        Behavior.InternalNoiseMultiplier = 0.8f;
        Behavior.InternalReactionMultiplier = 0.95f;
        break;
    case EZorbTypePreset::Heavy:
        Behavior.ShellMassMultiplier = 1.25f;
        Behavior.MaxSpeedMultiplier = 0.95f;
        Behavior.TurnRateMultiplier = 0.82f;
        Behavior.GripMultiplier = 0.9f;
        Behavior.BrakeMultiplier = 1.2f;
        Behavior.InternalSpringMultiplier = 0.8f;
        Behavior.InternalDampingMultiplier = 1.3f;
        Behavior.InternalNoiseMultiplier = 0.55f;
        Behavior.InternalReactionMultiplier = 0.75f;
        break;
    case EZorbTypePreset::Wild:
        Behavior.ShellMassMultiplier = 1.f;
        Behavior.MaxSpeedMultiplier = 1.02f;
        Behavior.TurnRateMultiplier = 1.08f;
        Behavior.GripMultiplier = 0.92f;
        Behavior.BrakeMultiplier = 0.9f;
        Behavior.InternalSpringMultiplier = 0.9f;
        Behavior.InternalDampingMultiplier = 0.75f;
        Behavior.InternalNoiseMultiplier = 1.45f;
        Behavior.InternalReactionMultiplier = 1.28f;
        break;
    case EZorbTypePreset::Classic:
    default:
        break;
    }

    return Behavior;
}
}

AZorbPawn::AZorbPawn()
{
    PrimaryActorTick.bCanEverTick = true;
    bUseProjectTuningSettings = true;
    ZorbTypePreset = EZorbTypePreset::Classic;
    CurrentEnergy = 0.f;
    CurrentHeat = 0.f;
    ForwardInputValue = 0.f;
    RightInputValue = 0.f;
    SmoothedForwardInputValue = 0.f;
    SmoothedRightInputValue = 0.f;
    SteeringRightInputValue = 0.f;
    OverheatLockRemaining = 0.f;
    CriticalFlashRemaining = 0.f;
    HeatHaloCurrentOpacity = 0.f;
    HeatHaloCurrentScale = 1.06f;
    HeatHaloCurrentColor = FLinearColor(1.0f, 0.95f, 0.2f, 1.0f);
    bAutomationInputOverrideActive = false;
    bBoostRequested = false;
    bBoostActive = false;
    DesiredCameraRotation = FRotator(-20.f, 0.f, 0.f);
    CameraGroundOffsetZ = 0.f;
    FrozenAirbornePitch = -20.f;
    bHasFrozenAirbornePitch = false;
    CameraPlanarForward = FVector(1.f, 0.f, 0.f);
    CameraStableTravelDir = FVector(1.f, 0.f, 0.f);
    CameraReverseIntentHoldTime = 0.f;
    CameraLookAheadDistance = 0.f;
    CameraCollisionLiftZ = 0.f;
    CameraBaseFov = 90.f;
    CameraTargetFov = 90.f;
    InternalMassOffsetLocal = FVector::ZeroVector;
    InternalMassVelocityLocal = FVector::ZeroVector;
    PreviousLinearVelocity = FVector::ZeroVector;
    bHasPreviousLinearVelocity = false;
    TelemetrySampleAccumulator = 0.f;
    bTelemetrySessionStarted = false;

    AutoPossessPlayer = EAutoReceiveInput::Player0;

    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
    CollisionComponent->SetSphereRadius(50.f);
    CollisionComponent->SetCollisionProfileName(TEXT("Pawn"));
    CollisionComponent->SetSimulatePhysics(true);
    CollisionComponent->SetEnableGravity(true);
    CollisionComponent->SetUseCCD(true);
    CollisionComponent->SetLinearDamping(MovementTuning.DriveLinearDamping);
    CollisionComponent->SetAngularDamping(0.2f);
    CollisionComponent->SetMassOverrideInKg(NAME_None, 80.f);
    RootComponent = CollisionComponent;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    MeshComponent->SetupAttachment(RootComponent);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereMesh.Succeeded())
    {
        MeshComponent->SetStaticMesh(SphereMesh.Object);
    }
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArmComponent->SetupAttachment(RootComponent);
    SpringArmComponent->TargetArmLength = 400.f;
    SpringArmComponent->bEnableCameraLag = true;
    SpringArmComponent->CameraLagSpeed = 3.f;
    SpringArmComponent->bDoCollisionTest = true;
    SpringArmComponent->ProbeSize = 12.f;
    SpringArmComponent->SetUsingAbsoluteRotation(true);
    SpringArmComponent->bInheritPitch = false;
    SpringArmComponent->bInheritYaw = false;
    SpringArmComponent->bInheritRoll = false;
    SpringArmComponent->SetRelativeRotation(FRotator(-20.f, 0.f, 0.f));

    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);

    HeatHaloComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeatHalo"));
    HeatHaloComponent->SetupAttachment(RootComponent);
    HeatHaloComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    HeatHaloComponent->SetCastShadow(false);
    HeatHaloComponent->SetReceivesDecals(false);
    HeatHaloComponent->SetRelativeScale3D(FVector(1.12f));
    HeatHaloComponent->SetVisibility(false);

    if (SphereMesh.Succeeded())
    {
        HeatHaloComponent->SetStaticMesh(SphereMesh.Object);
    }

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> HaloMaterial(
        TEXT("/Engine/EngineMaterials/Widget3DPassThrough_Translucent.Widget3DPassThrough_Translucent"));
    if (HaloMaterial.Succeeded())
    {
        HeatHaloComponent->SetMaterial(0, HaloMaterial.Object);
    }
}

void AZorbPawn::BeginPlay()
{
    Super::BeginPlay();

    ApplyProjectTuningIfEnabled();
    CurrentEnergy = 0.f;
    CurrentHeat = 0.f;

    if (CollisionComponent)
    {
        CollisionComponent->SetLinearDamping(MovementTuning.DriveLinearDamping);
        const FZorbTypeBehavior TypeBehavior = GetZorbTypeBehavior(ZorbTypePreset);
        CollisionComponent->SetMassOverrideInKg(NAME_None, 80.f * TypeBehavior.ShellMassMultiplier);
    }

    ResetInternalMassState();

    DesiredCameraRotation = FRotator(MovementTuning.CameraPitchAngle, 0.f, 0.f);
    FrozenAirbornePitch = MovementTuning.CameraPitchAngle;
    bHasFrozenAirbornePitch = false;
    CameraGroundOffsetZ = 0.f;
    CameraCollisionLiftZ = 0.f;
    CameraLookAheadDistance = MovementTuning.CameraLookAheadMin;
    CameraPlanarForward = GetActorForwardVector();
    CameraPlanarForward.Z = 0.f;
    if (!CameraPlanarForward.Normalize())
    {
        CameraPlanarForward = FVector(1.f, 0.f, 0.f);
    }
    CameraStableTravelDir = CameraPlanarForward;
    CameraReverseIntentHoldTime = 0.f;

    if (CameraComponent)
    {
        CameraBaseFov = CameraComponent->FieldOfView;
        CameraTargetFov = CameraBaseFov;
    }
    if (SpringArmComponent)
    {
        SpringArmComponent->SocketOffset = FVector::ZeroVector;
    }

    if (HeatHaloComponent)
    {
        UMaterialInterface* BaseMaterial = HeatHaloComponent->GetMaterial(0);
        if (BaseMaterial)
        {
            HeatHaloMID = UMaterialInstanceDynamic::Create(BaseMaterial, this);
            HeatHaloComponent->SetMaterial(0, HeatHaloMID);
            HeatHaloMID->SetVectorParameterValue(TEXT("TintColorAndOpacity"), FLinearColor(1.f, 0.95f, 0.2f, 0.0f));
        }
    }

    BeginTelemetrySession(false);
}

void AZorbPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    EndTelemetrySession();
    Super::EndPlay(EndPlayReason);
}

void AZorbPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    CriticalFlashRemaining = FMath::Max(0.f, CriticalFlashRemaining - DeltaTime);

    if (CollisionComponent)
    {
        SmoothedForwardInputValue = FMath::FInterpTo(SmoothedForwardInputValue, ForwardInputValue, DeltaTime, MovementTuning.DirectionResponse);
        SmoothedRightInputValue = FMath::FInterpTo(SmoothedRightInputValue, RightInputValue, DeltaTime, MovementTuning.DirectionResponse);

        float SteeringInterpSpeed = MovementTuning.SteeringInputReleaseSpeed;
        const bool bSteeringSignChanged =
            !FMath::IsNearlyZero(SteeringRightInputValue, KINDA_SMALL_NUMBER) &&
            !FMath::IsNearlyZero(RightInputValue, KINDA_SMALL_NUMBER) &&
            FMath::Sign(SteeringRightInputValue) != FMath::Sign(RightInputValue);
        if (bSteeringSignChanged)
        {
            SteeringInterpSpeed = MovementTuning.SteeringInputReverseSpeed;
        }
        else if (FMath::Abs(RightInputValue) > FMath::Abs(SteeringRightInputValue))
        {
            SteeringInterpSpeed = MovementTuning.SteeringInputRiseSpeed;
        }
        SteeringRightInputValue = FMath::FInterpTo(SteeringRightInputValue, RightInputValue, DeltaTime, SteeringInterpSpeed);

        const float RawInputMagnitude = FMath::Clamp(
            FMath::Sqrt((SmoothedForwardInputValue * SmoothedForwardInputValue) + (SmoothedRightInputValue * SmoothedRightInputValue)),
            0.f,
            1.f);

        const float ImmediateInputMagnitude = FMath::Clamp(
            FMath::Sqrt((ForwardInputValue * ForwardInputValue) + (RightInputValue * RightInputValue)),
            0.f,
            1.f);

        const float InputMagnitude = (OverheatLockRemaining > 0.f) ? 0.f : RawInputMagnitude;

        bool bGroundedForMovement = false;
        FVector GroundNormal = FVector::UpVector;
        if (UWorld* World = GetWorld())
        {
            FHitResult GroundHit;
            FCollisionQueryParams QueryParams;
            QueryParams.AddIgnoredActor(this);

            const FVector BallPos = CollisionComponent->GetComponentLocation();
            const float GroundProbeLength = CollisionComponent->GetScaledSphereRadius() + 35.f;
            const FVector GroundStart = BallPos;
            const FVector GroundEnd = GroundStart - FVector(0.f, 0.f, GroundProbeLength);
            bGroundedForMovement = World->LineTraceSingleByChannel(GroundHit, GroundStart, GroundEnd, ECC_WorldStatic, QueryParams) && GroundHit.bBlockingHit;
            if (bGroundedForMovement)
            {
                GroundNormal = GroundHit.ImpactNormal.GetSafeNormal();
            }
        }

        const float MovementInputMagnitude = bGroundedForMovement ? InputMagnitude : 0.f;
        const float ImmediateMovementInputMagnitude = bGroundedForMovement ? ImmediateInputMagnitude : 0.f;
        bBoostActive = bGroundedForMovement && bBoostRequested && OverheatLockRemaining <= 0.f && CurrentEnergy >= EnergyTuning.MinEnergyToBoost;
        const FZorbTypeBehavior TypeBehavior = GetZorbTypeBehavior(ZorbTypePreset);
        const float EffectiveMaxSpeed = MovementTuning.MaxSpeed * TypeBehavior.MaxSpeedMultiplier;

        float TargetLinearDamping = bGroundedForMovement
            ? FMath::Lerp(MovementTuning.IdleLinearDamping, MovementTuning.DriveLinearDamping, MovementInputMagnitude)
            : MovementTuning.AirLinearDamping;
        const float SpeedForFreeRoll = CollisionComponent->GetPhysicsLinearVelocity().Size();
        if (ImmediateMovementInputMagnitude <= 0.05f && bGroundedForMovement && SpeedForFreeRoll > 150.f)
        {
            TargetLinearDamping = FMath::Min(TargetLinearDamping, MovementTuning.FreeRollLinearDamping);
        }
        CollisionComponent->SetLinearDamping(TargetLinearDamping);

        FVector Velocity = CollisionComponent->GetPhysicsLinearVelocity();
        if (!bHasPreviousLinearVelocity)
        {
            PreviousLinearVelocity = Velocity;
            bHasPreviousLinearVelocity = true;
        }
        const FVector WorldLinearAcceleration = (Velocity - PreviousLinearVelocity) / FMath::Max(DeltaTime, KINDA_SMALL_NUMBER);
        PreviousLinearVelocity = Velocity;
        UpdateInternalMassModel(DeltaTime, WorldLinearAcceleration, bGroundedForMovement);

        const float Speed = Velocity.Size();
        if (bGroundedForMovement && Speed > EffectiveMaxSpeed)
        {
            const FVector OverSpeedBrake = -Velocity.GetSafeNormal() * ((Speed - EffectiveMaxSpeed) * 1200.f);
            CollisionComponent->AddForce(OverSpeedBrake, NAME_None, false);
        }

        if (!bGroundedForMovement && MovementTuning.AirHorizontalDeceleration > 0.f)
        {
            FVector HorizontalVelocity = Velocity;
            HorizontalVelocity.Z = 0.f;
            const float HorizontalSpeed = HorizontalVelocity.Size();
            if (HorizontalSpeed > 1.f)
            {
                const FVector AirDragAccel = -(HorizontalVelocity / HorizontalSpeed) * MovementTuning.AirHorizontalDeceleration;
                CollisionComponent->AddForce(AirDragAccel * CollisionComponent->GetMass(), NAME_None, false);
            }
        }

        if (MovementInputMagnitude <= 0.05f && bGroundedForMovement && MovementTuning.DownhillGravityAssist > 0.f)
        {
            const FVector DownhillVector = FVector::VectorPlaneProject(FVector::DownVector, GroundNormal);
            const float SlopeAmount = DownhillVector.Size();
            if (SlopeAmount > MovementTuning.DownhillAssistMinSlope)
            {
                const FVector DownhillDir = DownhillVector / SlopeAmount;
                const float AssistAccel = MovementTuning.DownhillGravityAssist * SlopeAmount;
                CollisionComponent->AddForce(DownhillDir * (AssistAccel * CollisionComponent->GetMass()), NAME_None, false);
            }
        }

        if (MovementInputMagnitude > 0.05f)
        {
            FVector CamForward = SpringArmComponent ? SpringArmComponent->GetForwardVector() : GetActorForwardVector();
            FVector CamRight = SpringArmComponent ? SpringArmComponent->GetRightVector() : GetActorRightVector();
            CamForward.Z = 0.f;
            CamRight.Z = 0.f;
            CamForward.Normalize();
            CamRight.Normalize();

            FVector VelXYForInput = Velocity;
            VelXYForInput.Z = 0.f;
            const float PlanarSpeedForInput = VelXYForInput.Size();
            const bool bAllowDirectionalReverse = PlanarSpeedForInput <= MovementTuning.ReverseDirectionalSpeedThreshold;

            const float DriveForwardInput = bAllowDirectionalReverse ? SmoothedForwardInputValue : FMath::Max(0.f, SmoothedForwardInputValue);
            const float BrakeInput = bAllowDirectionalReverse ? 0.f : FMath::Max(0.f, -SmoothedForwardInputValue);
            FVector DesiredDir = (CamForward * DriveForwardInput) + (CamRight * SteeringRightInputValue);
            if (!DesiredDir.IsNearlyZero())
            {
                DesiredDir.Normalize();

                FVector VelXY = Velocity;
                VelXY.Z = 0.f;
                const float CurrentPlanarSpeed = VelXY.Size();
                const FVector CurrentDir = (CurrentPlanarSpeed > 10.f) ? (VelXY / CurrentPlanarSpeed) : DesiredDir;
                const float DotFwd = FVector::DotProduct(CurrentDir, DesiredDir);

                // Longitudinal: build speed along current heading. Never brakes, so speed is preserved through turns.
                FVector LongitudinalAccel = FVector::ZeroVector;
                const float TargetSpeed = InputMagnitude * EffectiveMaxSpeed;
                if (CurrentPlanarSpeed < TargetSpeed)
                {
                    const float Gain = FMath::Min((TargetSpeed - CurrentPlanarSpeed) * MovementTuning.VelocityResponse,
                                                   MovementTuning.MaxDriveAcceleration);
                    LongitudinalAccel = CurrentDir * Gain;
                }

                // Centripetal + lateral grip: only when grounded (no air-steering).
                const FVector CentripetalRaw = DesiredDir - CurrentDir * DotFwd;
                const float SinAngle = CentripetalRaw.Size();
                FVector CentripetalAccel = FVector::ZeroVector;
                FVector LateralGripAccel = FVector::ZeroVector;
                if (bGroundedForMovement)
                {
                    // Centripetal: rotate velocity toward DesiredDir without shedding speed.
                    // Force scales with current speed → constant turn radius regardless of speed.
                    if (SinAngle > 0.001f && CurrentPlanarSpeed > 10.f)
                    {
                        const float CentripetalMag = FMath::Min(
                            CurrentPlanarSpeed * MovementTuning.TurnRateScale * TypeBehavior.TurnRateMultiplier,
                            MovementTuning.MaxSteeringAcceleration);
                        CentripetalAccel = (CentripetalRaw / SinAngle) * CentripetalMag;
                    }

                    // Lateral grip: drift correction only when heading is close to desired dir (< ~45°).
                    // Disabled during active turning to avoid cancelling the centripetal force.
                    if (DotFwd > 0.7f && MovementTuning.LateralGripForce > 0.f)
                    {
                        const float ZorbMass = FMath::Max(CollisionComponent->GetMass(), 1.f);
                        const FVector ForwardVel = DesiredDir * FVector::DotProduct(VelXY, DesiredDir);
                        const FVector LateralVel = VelXY - ForwardVel;
                        FVector RawGrip = (-LateralVel * (MovementTuning.LateralGripForce * TypeBehavior.GripMultiplier)) / ZorbMass;
                        LateralGripAccel = RawGrip.GetClampedToMaxSize(MovementTuning.MaxDriveAcceleration * 1.5f);
                    }
                }

                FVector TotalAccel = LongitudinalAccel + CentripetalAccel + LateralGripAccel;
                if (bBoostActive)
                {
                    TotalAccel *= MovementTuning.BoostAccelerationMultiplier;
                }
                CollisionComponent->AddForce(TotalAccel * CollisionComponent->GetMass(), NAME_None, false);
            }

            if (bGroundedForMovement && BrakeInput > 0.05f)
            {
                FVector HorizontalVelocity = Velocity;
                HorizontalVelocity.Z = 0.f;
                const float HorizontalSpeed = HorizontalVelocity.Size();
                if (HorizontalSpeed > 1.f)
                {
                    const FVector BrakeAccel = -(HorizontalVelocity / HorizontalSpeed) * (MovementTuning.BrakeDeceleration * TypeBehavior.BrakeMultiplier * BrakeInput);
                    CollisionComponent->AddForce(BrakeAccel * CollisionComponent->GetMass(), NAME_None, false);
                }
            }
        }

        const UZorbTuningSettings* ProjectSettings = UZorbTuningSettings::Get();
        if (bTelemetrySessionStarted && ProjectSettings && ProjectSettings->Telemetry.bEnableTelemetry && GetWorld())
        {
            const float SampleInterval = (ProjectSettings->Telemetry.SampleRateHz > KINDA_SMALL_NUMBER)
                ? (1.f / ProjectSettings->Telemetry.SampleRateHz)
                : 0.f;
            TelemetrySampleAccumulator += DeltaTime;

            if (SampleInterval <= 0.f)
            {
                RecordTelemetrySample(GetWorld()->GetTimeSeconds(), GetActorLocation(), CollisionComponent->GetPhysicsLinearVelocity(), bGroundedForMovement, GroundNormal);
            }
            else
            {
                while (TelemetrySampleAccumulator >= SampleInterval)
                {
                    TelemetrySampleAccumulator -= SampleInterval;
                    RecordTelemetrySample(GetWorld()->GetTimeSeconds(), GetActorLocation(), CollisionComponent->GetPhysicsLinearVelocity(), bGroundedForMovement, GroundNormal);
                }
            }
        }
    }

    UpdateEnergyAndHeat(DeltaTime);

    UpdateCameraRotation(DeltaTime);

    if (GetActorLocation().Z < RespawnTuning.KillZ)
    {
        RespawnToCheckpoint();
    }

    if (HeatHaloComponent)
    {
        const int32 OverheatLevel = GetOverheatLevel();
        FLinearColor TargetColor = FeedbackTuning.HaloLevel1Color;
        float TargetOpacity = 0.f;
        float TargetScale = 1.06f;

        if (OverheatLevel <= 0)
        {
            TargetOpacity = 0.f;
            TargetScale = 1.06f;
        }
        else if (OverheatLevel == 1)
        {
            TargetColor = FeedbackTuning.HaloLevel1Color;
            TargetOpacity = FeedbackTuning.HaloLevel1Opacity;
            TargetScale = FeedbackTuning.HaloLevel1Scale;
        }
        else if (OverheatLevel == 2)
        {
            TargetColor = FeedbackTuning.HaloLevel2Color;
            TargetOpacity = FeedbackTuning.HaloLevel2Opacity;
            TargetScale = FeedbackTuning.HaloLevel2Scale;
        }
        else
        {
            TargetColor = FeedbackTuning.HaloLevel3Color;
            TargetOpacity = FeedbackTuning.HaloLevel3Opacity;
            TargetScale = FeedbackTuning.HaloLevel3Scale;
        }

        const float BlendAlpha = FMath::Clamp(DeltaTime * FeedbackTuning.HeatHaloBlendSpeed, 0.f, 1.f);
        HeatHaloCurrentColor = FMath::Lerp(HeatHaloCurrentColor, TargetColor, BlendAlpha);
        HeatHaloCurrentOpacity = FMath::Lerp(HeatHaloCurrentOpacity, TargetOpacity, BlendAlpha);
        HeatHaloCurrentScale = FMath::Lerp(HeatHaloCurrentScale, TargetScale, BlendAlpha);

        HeatHaloComponent->SetVisibility(HeatHaloCurrentOpacity > 0.01f);
        HeatHaloComponent->SetRelativeScale3D(FVector(HeatHaloCurrentScale));

        if (HeatHaloMID)
        {
            HeatHaloMID->SetVectorParameterValue(
                TEXT("TintColorAndOpacity"),
                FLinearColor(HeatHaloCurrentColor.R, HeatHaloCurrentColor.G, HeatHaloCurrentColor.B, HeatHaloCurrentOpacity));
        }
    }
}

void AZorbPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    PlayerInputComponent->BindAxis("MoveForward", this, &AZorbPawn::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AZorbPawn::MoveRight);
    PlayerInputComponent->BindAction("Boost", IE_Pressed, this, &AZorbPawn::StartBoost);
    PlayerInputComponent->BindAction("Boost", IE_Released, this, &AZorbPawn::StopBoost);
}

void AZorbPawn::MoveForward(float Value)
{
    if (bAutomationInputOverrideActive)
    {
        return;
    }

    ForwardInputValue = Value;
}

void AZorbPawn::MoveRight(float Value)
{
    if (bAutomationInputOverrideActive)
    {
        return;
    }

    RightInputValue = Value;
}

void AZorbPawn::PrepareForAutomatedScenario(const FTransform& StartTransform, const FVector& InitialLinearVelocity)
{
    SetActorLocationAndRotation(
        StartTransform.GetLocation(),
        StartTransform.GetRotation().Rotator(),
        false,
        nullptr,
        ETeleportType::TeleportPhysics);

    if (CollisionComponent)
    {
        CollisionComponent->SetPhysicsLinearVelocity(InitialLinearVelocity);
        CollisionComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
    }

    ForwardInputValue = 0.f;
    RightInputValue = 0.f;
    SmoothedForwardInputValue = 0.f;
    SmoothedRightInputValue = 0.f;
    SteeringRightInputValue = 0.f;
    bBoostRequested = false;
    bBoostActive = false;
    bAutomationInputOverrideActive = false;
    ResetInternalMassState();
}

void AZorbPawn::ApplyAutomatedScenarioInput(float ForwardValue, float RightValue, bool bBoostEnabled)
{
    bAutomationInputOverrideActive = true;
    ForwardInputValue = ForwardValue;
    RightInputValue = RightValue;
    bBoostRequested = bBoostEnabled;
}

void AZorbPawn::ResetAutomatedScenarioInput()
{
    bAutomationInputOverrideActive = false;
    ForwardInputValue = 0.f;
    RightInputValue = 0.f;
    bBoostRequested = false;
    bBoostActive = false;
}

void AZorbPawn::RestartTelemetrySession()
{
    EndTelemetrySession();
    BeginTelemetrySession(true);
}

void AZorbPawn::StartBoost()
{
    if (bAutomationInputOverrideActive)
    {
        return;
    }

    bBoostRequested = true;
}

void AZorbPawn::StopBoost()
{
    if (bAutomationInputOverrideActive)
    {
        return;
    }

    bBoostRequested = false;
    bBoostActive = false;
}

void AZorbPawn::RespawnToCheckpoint()
{
    AZorbGameMode* GameMode = Cast<AZorbGameMode>(UGameplayStatics::GetGameMode(this));
    if (!GameMode)
    {
        return;
    }

    const FTransform CheckpointTransform = GameMode->GetLastCheckpointTransform();
    SetActorLocationAndRotation(
        CheckpointTransform.GetLocation() + FVector(0.f, 0.f, RespawnTuning.RespawnHeightOffset),
        CheckpointTransform.GetRotation().Rotator(),
        false,
        nullptr,
        ETeleportType::TeleportPhysics);

    CollisionComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
    CollisionComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
    SmoothedForwardInputValue = 0.f;
    SmoothedRightInputValue = 0.f;
    SteeringRightInputValue = 0.f;
    bBoostRequested = false;
    bBoostActive = false;
    ResetInternalMassState();

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("Respawned at checkpoint"));
    }
}

void AZorbPawn::UpdateEnergyAndHeat(float DeltaTime)
{
    if (!CollisionComponent)
    {
        return;
    }

    OverheatLockRemaining = FMath::Max(0.f, OverheatLockRemaining - DeltaTime);

    bool bIsGrounded = false;
    if (UWorld* World = GetWorld())
    {
        FHitResult GroundHit;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);

        const FVector BallPos = CollisionComponent->GetComponentLocation();
        const float GroundProbeLength = CollisionComponent->GetScaledSphereRadius() + 25.f;
        const FVector GroundStart = BallPos;
        const FVector GroundEnd = GroundStart - FVector(0.f, 0.f, GroundProbeLength);
        bIsGrounded = World->LineTraceSingleByChannel(GroundHit, GroundStart, GroundEnd, ECC_WorldStatic, QueryParams) && GroundHit.bBlockingHit;
    }

    const float Speed = CollisionComponent->GetPhysicsLinearVelocity().Size();
    const float SpeedFactor = FMath::Clamp(Speed / 2500.f, 0.f, 1.f);
    const float InputFactor = FMath::Clamp(
        FMath::Sqrt((SmoothedForwardInputValue * SmoothedForwardInputValue) + (SmoothedRightInputValue * SmoothedRightInputValue)),
        0.f,
        1.f);

    // Gameplay rule: gain energy in free roll (no input), spend energy while actively steering.
    if (bIsGrounded)
    {
        const float FreeRollFactor = 1.f - InputFactor;
        const float EnergyGain = EnergyTuning.EnergyGainRate * FreeRollFactor * SpeedFactor;
        const float EnergyDrain = EnergyTuning.EnergyDrainRate * InputFactor;
        const float EnergyDelta = (EnergyGain - EnergyDrain) * DeltaTime;
        CurrentEnergy = FMath::Clamp(CurrentEnergy + EnergyDelta, 0.f, EnergyTuning.MaxEnergy);
    }

    const bool bActivelyDriving = InputFactor > 0.1f;
    const float HeatDelta = bActivelyDriving
        ? ((EnergyTuning.HeatGainRate * (0.4f + 0.6f * InputFactor)) + (3.f * SpeedFactor)) * DeltaTime
        : -(EnergyTuning.HeatCooldownRate * DeltaTime);
    CurrentHeat = FMath::Clamp(CurrentHeat + HeatDelta, 0.f, EnergyTuning.MaxHeat);

    if (bBoostActive && bIsGrounded)
    {
        const float BoostDrain = EnergyTuning.BoostEnergyDrainRate * DeltaTime;
        const float BoostHeat = EnergyTuning.BoostHeatGainRate * DeltaTime;
        CurrentEnergy = FMath::Clamp(CurrentEnergy - BoostDrain, 0.f, EnergyTuning.MaxEnergy);
        CurrentHeat = FMath::Clamp(CurrentHeat + BoostHeat, 0.f, EnergyTuning.MaxHeat);

        if (CurrentEnergy <= EnergyTuning.MinEnergyToBoost)
        {
            bBoostActive = false;
            bBoostRequested = false;
        }
    }

    if (CurrentHeat >= EnergyTuning.MaxHeat)
    {
        TriggerCriticalOverheat();
    }
}

void AZorbPawn::TriggerCriticalOverheat()
{
    CurrentHeat = EnergyTuning.PostCriticalHeat;
    CurrentEnergy = EnergyTuning.PostCriticalEnergy;
    OverheatLockRemaining = EnergyTuning.CriticalOverheatLockDuration;
    CriticalFlashRemaining = EnergyTuning.CriticalFlashDuration;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Red, TEXT("CRITICAL OVERHEAT! Respawn triggered"));
    }

    UE_LOG(LogTemp, Warning, TEXT("Critical overheat reached. Triggering respawn."));
    RespawnToCheckpoint();
}

float AZorbPawn::GetEnergyPercent() const
{
    return EnergyTuning.MaxEnergy > 0.f ? CurrentEnergy / EnergyTuning.MaxEnergy : 0.f;
}

float AZorbPawn::GetHeatPercent() const
{
    return EnergyTuning.MaxHeat > 0.f ? CurrentHeat / EnergyTuning.MaxHeat : 0.f;
}

void AZorbPawn::ApplyProjectTuningIfEnabled()
{
    if (!bUseProjectTuningSettings)
    {
        return;
    }

    if (const UZorbTuningSettings* Settings = UZorbTuningSettings::Get())
    {
        MovementTuning = Settings->Movement;
        EnergyTuning = Settings->Energy;
        RespawnTuning = Settings->Respawn;
        FeedbackTuning = Settings->Feedback;
        ZorbTypePreset = static_cast<EZorbTypePreset>(Settings->Movement.ZorbTypePreset);
    }
}

void AZorbPawn::ResetInternalMassState()
{
    InternalMassOffsetLocal = FVector::ZeroVector;
    InternalMassVelocityLocal = FVector::ZeroVector;
    PreviousLinearVelocity = FVector::ZeroVector;
    bHasPreviousLinearVelocity = false;
}

void AZorbPawn::UpdateInternalMassModel(float DeltaTime, const FVector& WorldLinearAcceleration, bool bGroundedForMovement)
{
    if (!CollisionComponent || DeltaTime <= KINDA_SMALL_NUMBER)
    {
        return;
    }

    // Simplified double-shell model: inner mass as isotropic spring-damper point.
    constexpr float SpringStiffness = 12.0f;
    constexpr float DampingGrounded = 5.5f;
    constexpr float DampingAirborne = 3.0f;
    constexpr float InertialCoupling = 0.24f;
    constexpr float InputBiasForward = 160.0f;
    constexpr float InputBiasRight = 120.0f;
    constexpr float NoiseStrengthGrounded = 28.0f;
    constexpr float NoiseStrengthAirborne = 12.0f;
    constexpr float ReactionCoupling = 0.22f;
    constexpr float MaxOffsetRadiusRatio = 0.26f;
    const FZorbTypeBehavior TypeBehavior = GetZorbTypeBehavior(ZorbTypePreset);

    const FTransform BodyTransform = CollisionComponent->GetComponentTransform();
    const FVector AccelLocal = BodyTransform.InverseTransformVectorNoScale(WorldLinearAcceleration);

    const FVector InputBiasLocal(
        -SmoothedForwardInputValue * InputBiasForward,
        -SteeringRightInputValue * InputBiasRight,
        0.f);

    const float TimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
    const float NoiseStrength = (bGroundedForMovement ? NoiseStrengthGrounded : NoiseStrengthAirborne) * TypeBehavior.InternalNoiseMultiplier;
    const FVector NoiseLocal(
        FMath::PerlinNoise1D(TimeSeconds * 1.7f),
        FMath::PerlinNoise1D(TimeSeconds * 2.3f + 37.1f),
        FMath::PerlinNoise1D(TimeSeconds * 1.2f + 83.5f));

    const float Damping = (bGroundedForMovement ? DampingGrounded : DampingAirborne) * TypeBehavior.InternalDampingMultiplier;
    const FVector LocalRelativeAccel =
        (-InternalMassOffsetLocal * (SpringStiffness * TypeBehavior.InternalSpringMultiplier)) -
        (InternalMassVelocityLocal * Damping) -
        (AccelLocal * InertialCoupling) +
        InputBiasLocal +
        (NoiseLocal * NoiseStrength);

    InternalMassVelocityLocal += LocalRelativeAccel * DeltaTime;
    InternalMassOffsetLocal += InternalMassVelocityLocal * DeltaTime;

    const float MaxOffsetRadius = CollisionComponent->GetScaledSphereRadius() * MaxOffsetRadiusRatio;
    InternalMassOffsetLocal = InternalMassOffsetLocal.GetClampedToMaxSize(MaxOffsetRadius);

    const float ShellMass = FMath::Max(1.f, CollisionComponent->GetMass());
    const FVector ReactionForceWorld = BodyTransform.TransformVectorNoScale(-LocalRelativeAccel * (ShellMass * ReactionCoupling * TypeBehavior.InternalReactionMultiplier));
    const FVector OffsetWorld = BodyTransform.TransformVectorNoScale(InternalMassOffsetLocal);
    CollisionComponent->AddForceAtLocation(
        ReactionForceWorld,
        CollisionComponent->GetComponentLocation() + OffsetWorld,
        NAME_None);
}

void AZorbPawn::UpdateCameraRotation(float DeltaTime)
{
    if (!SpringArmComponent || !CollisionComponent || !CameraComponent)
    {
        return;
    }

    const FVector BallPos = CollisionComponent->GetComponentLocation();
    const FVector Velocity = CollisionComponent->GetPhysicsLinearVelocity();
    FVector VelocityXY = Velocity;
    VelocityXY.Z = 0.f;

    // Update pivot and height (updates CameraStableTravelDir and CameraGroundOffsetZ)
    UpdateCameraPivotAndHeight(DeltaTime, BallPos, VelocityXY);

    // Compute look-ahead based on planar speed
    const float VelMag = VelocityXY.Size();
    const float SpeedAlpha = FMath::Clamp(VelMag / FMath::Max(1.f, MovementTuning.CameraLookAheadSpeedRef), 0.f, 1.f);
    const float TargetLookAheadDist = FMath::Lerp(MovementTuning.CameraLookAheadMin, MovementTuning.CameraLookAheadMax, SpeedAlpha);
    CameraLookAheadDistance = FMath::FInterpTo(CameraLookAheadDistance, TargetLookAheadDist, DeltaTime, MovementTuning.CameraLookAheadInterpSpeed);

    // Focus point is the ball plus lookahead in travel direction
    const FVector FocusPoint = BallPos + (CameraStableTravelDir * CameraLookAheadDistance);

    // Compute desired rotation to look at focus point from CameraPivotWorld
    const FVector ToFocus = FocusPoint - CameraPivotWorld;
    FRotator DesiredRot = ToFocus.Rotation();

    // Maintain pitch freeze when airborne
    bool bIsGrounded = false;
    if (UWorld* World = GetWorld())
    {
        FHitResult GroundHit;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);
        const float GroundProbeLength = CollisionComponent->GetScaledSphereRadius() + 25.f;
        const FVector GroundStart = BallPos;
        const FVector GroundEnd = GroundStart - FVector(0.f, 0.f, GroundProbeLength);
        bIsGrounded = World->LineTraceSingleByChannel(GroundHit, GroundStart, GroundEnd, ECC_WorldStatic, QueryParams) && GroundHit.bBlockingHit;
    }

    if (!bIsGrounded)
    {
        if (!bHasFrozenAirbornePitch)
        {
            FrozenAirbornePitch = DesiredCameraRotation.Pitch;
            bHasFrozenAirbornePitch = true;
        }
        DesiredRot.Pitch = FrozenAirbornePitch;
    }
    else
    {
        FrozenAirbornePitch = DesiredRot.Pitch;
        bHasFrozenAirbornePitch = false;
    }

    // Smooth rotation using RInterpTo (keeps shortest path)
    DesiredCameraRotation = FMath::RInterpTo(DesiredCameraRotation, DesiredRot, DeltaTime, MovementTuning.CameraRotationSpeed);

    // Apply rotation to spring arm
    SpringArmComponent->SetUsingAbsoluteRotation(true);
    SpringArmComponent->SetWorldRotation(DesiredCameraRotation);

    // Enforce constant horizontal distance via TargetArmLength and let spring arm handle collision.
    SpringArmComponent->TargetArmLength = MovementTuning.CameraHorizontalDistance;

    // Handle collision compression -> lift + FOV compensation (existing behavior)
    float CollisionCompression = 0.f;
    if (MovementTuning.CameraHorizontalDistance > KINDA_SMALL_NUMBER)
    {
        const float CurrentArmLength = (CameraComponent->GetComponentLocation() - SpringArmComponent->GetComponentLocation()).Size();
        CollisionCompression = 1.f - FMath::Clamp(CurrentArmLength / MovementTuning.CameraHorizontalDistance, 0.f, 1.f);
    }

    const float TargetCollisionLift = CollisionCompression * MovementTuning.CameraCollisionLiftMax;
    CameraCollisionLiftZ = FMath::FInterpTo(CameraCollisionLiftZ, TargetCollisionLift, DeltaTime, MovementTuning.CameraCollisionLiftInterpSpeed);
    SpringArmComponent->SocketOffset = FVector(0.f, 0.f, CameraGroundOffsetZ + CameraCollisionLiftZ);

    const float DesiredFov = CameraBaseFov + (CollisionCompression * MovementTuning.CameraCollisionFovBoost);
    CameraTargetFov = FMath::FInterpTo(CameraTargetFov, DesiredFov, DeltaTime, MovementTuning.CameraCollisionFovInterpSpeed);
    CameraComponent->SetFieldOfView(CameraTargetFov);
}

void AZorbPawn::UpdateCameraPivotAndHeight(float DeltaTime, const FVector& BallPos, const FVector& VelocityXY)
{
    // Compute safe planar travel direction (with fallback)
    const FVector TravelDir = ComputeSafePlanarDirection(VelocityXY);

    // Smooth travel direction to avoid jitter
    const bool bPureLateralInput = FMath::Abs(SmoothedRightInputValue) > MovementTuning.CameraInputDirectionThreshold && FMath::Abs(SmoothedForwardInputValue) <= MovementTuning.CameraInputDirectionThreshold;
    const float StableDirInterpBase = bPureLateralInput ? MovementTuning.CameraStableInterpBaseLateral : MovementTuning.CameraStableInterpBaseForward;
    CameraStableTravelDir = FMath::VInterpNormalRotationTo(CameraStableTravelDir, TravelDir, DeltaTime, StableDirInterpBase * MovementTuning.CameraRotationSpeed);
    if (!CameraStableTravelDir.Normalize())
    {
        CameraStableTravelDir = TravelDir;
    }

    // Desired pivot position (behind the ball) and base vertical offset
    const FVector DesiredPivot = BallPos - (CameraStableTravelDir * MovementTuning.CameraHorizontalDistance) + FVector(0.f, 0.f, MovementTuning.CameraVerticalOffsetBase + CameraGroundOffsetZ);

    // Smooth pivot world position
    CameraPivotWorld = FMath::VInterpTo(CameraPivotWorld, DesiredPivot, DeltaTime, MovementTuning.CameraPositionSmoothingSpeed);

    // Ground probe under the pivot to maintain clearance
    if (UWorld* World = GetWorld())
    {
        FHitResult Hit;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);

        const FVector TraceStart = CameraPivotWorld + FVector(0.f, 0.f, 60.f);
        const FVector TraceEnd = CameraPivotWorld - FVector(0.f, 0.f, MovementTuning.CameraGroundTraceLength);

        if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams) && Hit.bBlockingHit)
        {
            const float CurrentClearance = CameraPivotWorld.Z - Hit.ImpactPoint.Z;
            const float ClearanceError = MovementTuning.CameraGroundClearance - CurrentClearance;
            const float TargetOffset = FMath::Clamp(ClearanceError, -MovementTuning.CameraMaxGroundOffset, MovementTuning.CameraMaxGroundOffset);

            const float SmoothedOffset = FMath::FInterpTo(CameraGroundOffsetZ, TargetOffset, DeltaTime, MovementTuning.CameraHeightAdjustSpeed);

            const float MaxStep = MovementTuning.CameraMaxVerticalStepPerSecond * DeltaTime;
            const float DeltaOffset = SmoothedOffset - CameraGroundOffsetZ;
            CameraGroundOffsetZ += FMath::Clamp(DeltaOffset, -MaxStep, MaxStep);
        }
    }
}

FVector AZorbPawn::ComputeSafePlanarDirection(const FVector& VelocityXY) const
{
    FVector Dir = VelocityXY;
    Dir.Z = 0.f;
    if (Dir.SizeSquared() < KINDA_SMALL_NUMBER)
    {
        FVector Fwd = GetActorForwardVector();
        Fwd.Z = 0.f;
        if (!Fwd.Normalize())
        {
            return FVector(1.f, 0.f, 0.f);
        }
        return Fwd;
    }

    Dir.Normalize();
    return Dir;
}

int32 AZorbPawn::GetOverheatLevel() const
{
    const float HeatPct = GetHeatPercent() * 100.f;
    if (HeatPct >= EnergyTuning.OverheatLevel3Threshold)
    {
        return 3;
    }
    if (HeatPct >= EnergyTuning.OverheatLevel2Threshold)
    {
        return 2;
    }
    if (HeatPct >= EnergyTuning.OverheatLevel1Threshold)
    {
        return 1;
    }

    return 0;
}

bool AZorbPawn::IsBoosting() const
{
    return bBoostActive;
}

float AZorbPawn::GetCriticalFlashAlpha() const
{
    if (EnergyTuning.CriticalFlashDuration <= 0.f)
    {
        return 0.f;
    }

    return FMath::Clamp(CriticalFlashRemaining / EnergyTuning.CriticalFlashDuration, 0.f, 1.f);
}

void AZorbPawn::BeginTelemetrySession(bool bIgnoreAutoStart)
{
    const UZorbTuningSettings* ProjectSettings = UZorbTuningSettings::Get();
    if (!ProjectSettings || !ProjectSettings->Telemetry.bEnableTelemetry)
    {
        return;
    }

    if (!bIgnoreAutoStart && !ProjectSettings->Telemetry.bAutoStartOnBeginPlay)
    {
        return;
    }

    const FString OutputDir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Telemetry"));
    IFileManager::Get().MakeDirectory(*OutputDir, true);

    const FString FilePrefix = ProjectSettings->Telemetry.OutputFilePrefix.IsEmpty()
        ? TEXT("zorb_run")
        : ProjectSettings->Telemetry.OutputFilePrefix;
    const FString Timestamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
    ActiveTelemetryFilePath = FPaths::Combine(OutputDir, FString::Printf(TEXT("%s_%s.csv"), *FilePrefix, *Timestamp));

    const FString Header = TEXT("time_seconds,pos_x,pos_y,pos_z,vel_x,vel_y,vel_z,planar_speed,vertical_speed,grounded,slope_percent,forward_input,right_input,smoothed_forward,smoothed_right,steering_right,boost_active,energy,heat,max_speed,turn_rate_scale,brake_deceleration,air_horizontal_deceleration\n");
    if (FFileHelper::SaveStringToFile(Header, *ActiveTelemetryFilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
    {
        bTelemetrySessionStarted = true;
        TelemetrySampleAccumulator = 0.f;
        UE_LOG(LogTemp, Log, TEXT("Telemetry started: %s"), *ActiveTelemetryFilePath);
    }
}

void AZorbPawn::EndTelemetrySession()
{
    if (bTelemetrySessionStarted)
    {
        UE_LOG(LogTemp, Log, TEXT("Telemetry ended: %s"), *ActiveTelemetryFilePath);
    }

    bTelemetrySessionStarted = false;
    TelemetrySampleAccumulator = 0.f;
    ActiveTelemetryFilePath.Empty();
}

void AZorbPawn::RecordTelemetrySample(float TimeSeconds, const FVector& Position, const FVector& Velocity, bool bGrounded, const FVector& GroundNormal)
{
    if (!bTelemetrySessionStarted || ActiveTelemetryFilePath.IsEmpty())
    {
        return;
    }

    FVector PlanarVelocity = Velocity;
    PlanarVelocity.Z = 0.f;
    const float SlopePercent = ComputeGroundSlopePercent(GroundNormal, bGrounded);

    const FString Row = FString::Printf(
        TEXT("%.4f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\n"),
        TimeSeconds,
        Position.X,
        Position.Y,
        Position.Z,
        Velocity.X,
        Velocity.Y,
        Velocity.Z,
        PlanarVelocity.Size(),
        Velocity.Z,
        bGrounded ? 1 : 0,
        SlopePercent,
        ForwardInputValue,
        RightInputValue,
        SmoothedForwardInputValue,
        SmoothedRightInputValue,
        SteeringRightInputValue,
        bBoostActive ? 1 : 0,
        CurrentEnergy,
        CurrentHeat,
        MovementTuning.MaxSpeed,
        MovementTuning.TurnRateScale,
        MovementTuning.BrakeDeceleration,
        MovementTuning.AirHorizontalDeceleration);

    FFileHelper::SaveStringToFile(Row, *ActiveTelemetryFilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM, &IFileManager::Get(), FILEWRITE_Append);
}

float AZorbPawn::ComputeGroundSlopePercent(const FVector& GroundNormal, bool bGrounded) const
{
    if (!bGrounded)
    {
        return 0.f;
    }

    const float SafeZ = FMath::Max(FMath::Abs(GroundNormal.Z), 0.001f);
    const float HorizontalComponent = FVector(GroundNormal.X, GroundNormal.Y, 0.f).Size();
    return (HorizontalComponent / SafeZ) * 100.f;
}

bool AZorbPawn::IsGrounded() const
{
    if (!CollisionComponent)
    {
        return false;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    FHitResult GroundHit;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    const FVector BallPos = CollisionComponent->GetComponentLocation();
    const float GroundProbeLength = CollisionComponent->GetScaledSphereRadius() + 25.f;
    const FVector GroundStart = BallPos;
    const FVector GroundEnd = GroundStart - FVector(0.f, 0.f, GroundProbeLength);
    return World->LineTraceSingleByChannel(GroundHit, GroundStart, GroundEnd, ECC_WorldStatic, QueryParams) && GroundHit.bBlockingHit;
}

float AZorbPawn::GetGroundSlopePercent() const
{
    if (!CollisionComponent)
    {
        return 0.f;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return 0.f;
    }

    FHitResult GroundHit;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    const FVector BallPos = CollisionComponent->GetComponentLocation();
    const float GroundProbeLength = CollisionComponent->GetScaledSphereRadius() + 25.f;
    const FVector GroundStart = BallPos;
    const FVector GroundEnd = GroundStart - FVector(0.f, 0.f, GroundProbeLength);

    if (!(World->LineTraceSingleByChannel(GroundHit, GroundStart, GroundEnd, ECC_WorldStatic, QueryParams) && GroundHit.bBlockingHit))
    {
        return 0.f;
    }

    return ComputeGroundSlopePercent(GroundHit.ImpactNormal.GetSafeNormal(), true);
}
