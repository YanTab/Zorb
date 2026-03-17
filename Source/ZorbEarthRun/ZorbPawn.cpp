#include "ZorbPawn.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "ZorbGameMode.h"
#include "ZorbTuningSettings.h"

AZorbPawn::AZorbPawn()
{
    PrimaryActorTick.bCanEverTick = true;
    bUseProjectTuningSettings = true;
    CurrentEnergy = 0.f;
    CurrentHeat = 0.f;
    ForwardInputValue = 0.f;
    RightInputValue = 0.f;
    SmoothedForwardInputValue = 0.f;
    SmoothedRightInputValue = 0.f;
    OverheatLockRemaining = 0.f;
    CriticalFlashRemaining = 0.f;
    HeatHaloCurrentOpacity = 0.f;
    HeatHaloCurrentScale = 1.06f;
    HeatHaloCurrentColor = FLinearColor(1.0f, 0.95f, 0.2f, 1.0f);
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
    }

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
}

void AZorbPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    CriticalFlashRemaining = FMath::Max(0.f, CriticalFlashRemaining - DeltaTime);

    if (CollisionComponent)
    {
        SmoothedForwardInputValue = FMath::FInterpTo(SmoothedForwardInputValue, ForwardInputValue, DeltaTime, MovementTuning.DirectionResponse);
        SmoothedRightInputValue = FMath::FInterpTo(SmoothedRightInputValue, RightInputValue, DeltaTime, MovementTuning.DirectionResponse);

        const float RawInputMagnitude = FMath::Clamp(
            FMath::Sqrt((SmoothedForwardInputValue * SmoothedForwardInputValue) + (SmoothedRightInputValue * SmoothedRightInputValue)),
            0.f,
            1.f);

        const float ImmediateInputMagnitude = FMath::Clamp(
            FMath::Sqrt((ForwardInputValue * ForwardInputValue) + (RightInputValue * RightInputValue)),
            0.f,
            1.f);

        const float InputMagnitude = (OverheatLockRemaining > 0.f) ? 0.f : RawInputMagnitude;
        bBoostActive = bBoostRequested && OverheatLockRemaining <= 0.f && CurrentEnergy >= EnergyTuning.MinEnergyToBoost;

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

        float TargetLinearDamping = FMath::Lerp(MovementTuning.IdleLinearDamping, MovementTuning.DriveLinearDamping, InputMagnitude);
        const float SpeedForFreeRoll = CollisionComponent->GetPhysicsLinearVelocity().Size();
        if (ImmediateInputMagnitude <= 0.05f && bGroundedForMovement && SpeedForFreeRoll > 150.f)
        {
            TargetLinearDamping = FMath::Min(TargetLinearDamping, MovementTuning.FreeRollLinearDamping);
        }
        CollisionComponent->SetLinearDamping(TargetLinearDamping);

        FVector Velocity = CollisionComponent->GetPhysicsLinearVelocity();
        const float Speed = Velocity.Size();
        if (Speed > MovementTuning.MaxSpeed)
        {
            const FVector OverSpeedBrake = -Velocity.GetSafeNormal() * ((Speed - MovementTuning.MaxSpeed) * 1200.f);
            CollisionComponent->AddForce(OverSpeedBrake, NAME_None, false);
        }

        if (InputMagnitude <= 0.05f && bGroundedForMovement && MovementTuning.DownhillGravityAssist > 0.f)
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

        if (InputMagnitude > 0.05f)
        {
            FVector CamForward = SpringArmComponent ? SpringArmComponent->GetForwardVector() : GetActorForwardVector();
            FVector CamRight = SpringArmComponent ? SpringArmComponent->GetRightVector() : GetActorRightVector();
            CamForward.Z = 0.f;
            CamRight.Z = 0.f;
            CamForward.Normalize();
            CamRight.Normalize();

            FVector DesiredDir = (CamForward * SmoothedForwardInputValue) + (CamRight * SmoothedRightInputValue);
            if (!DesiredDir.IsNearlyZero())
            {
                DesiredDir.Normalize();

                FVector VelocityXY = CollisionComponent->GetPhysicsLinearVelocity();
                VelocityXY.Z = 0.f;

                const float CurrentPlanarSpeed = VelocityXY.Size();
                const float InputTargetSpeed = InputMagnitude * MovementTuning.MaxSpeed;
                const float DesiredPlanarSpeed = FMath::Max(CurrentPlanarSpeed, InputTargetSpeed);
                const FVector DesiredVelocity = DesiredDir * DesiredPlanarSpeed;
                FVector DesiredAcceleration = (DesiredVelocity - VelocityXY) * MovementTuning.VelocityResponse;

                if (bBoostActive)
                {
                    DesiredAcceleration *= MovementTuning.BoostAccelerationMultiplier;
                }

                const float MaxAccel = bBoostActive
                    ? (MovementTuning.MaxDriveAcceleration * MovementTuning.BoostAccelerationMultiplier)
                    : MovementTuning.MaxDriveAcceleration;
                DesiredAcceleration = DesiredAcceleration.GetClampedToMaxSize(MaxAccel);
                CollisionComponent->AddForce(DesiredAcceleration * CollisionComponent->GetMass(), NAME_None, false);
            }
        }

        if (InputMagnitude > 0.05f && MovementTuning.LateralGripForce > 0.f)
        {
            FVector CamForward = SpringArmComponent ? SpringArmComponent->GetForwardVector() : GetActorForwardVector();
            FVector CamRight = SpringArmComponent ? SpringArmComponent->GetRightVector() : GetActorRightVector();
            CamForward.Z = 0.f;
            CamRight.Z = 0.f;
            CamForward.Normalize();
            CamRight.Normalize();

            FVector DesiredDir = (CamForward * SmoothedForwardInputValue) + (CamRight * SmoothedRightInputValue);
            if (!DesiredDir.IsNearlyZero())
            {
                DesiredDir.Normalize();
                FVector VelXY = Velocity;
                VelXY.Z = 0.f;
                const FVector ForwardVel = DesiredDir * FVector::DotProduct(VelXY, DesiredDir);
                const FVector LateralVel = VelXY - ForwardVel;

                // Clamp lateral correction so repeated left/right inputs do not inject unstable forces.
                const float ZorbMass = FMath::Max(CollisionComponent->GetMass(), 1.f);
                FVector LateralCorrectionAccel = (-LateralVel * MovementTuning.LateralGripForce) / ZorbMass;
                const float MaxGripAccel = MovementTuning.MaxDriveAcceleration * 1.5f;
                LateralCorrectionAccel = LateralCorrectionAccel.GetClampedToMaxSize(MaxGripAccel);
                CollisionComponent->AddForce(LateralCorrectionAccel * ZorbMass, NAME_None, false);
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
    ForwardInputValue = Value;
}

void AZorbPawn::MoveRight(float Value)
{
    RightInputValue = Value;
}

void AZorbPawn::StartBoost()
{
    bBoostRequested = true;
}

void AZorbPawn::StopBoost()
{
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
    bBoostRequested = false;
    bBoostActive = false;

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
    }
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

    const FVector GroundNormal = GroundHit.ImpactNormal.GetSafeNormal();
    const float UpDot = FMath::Clamp(FVector::DotProduct(GroundNormal, FVector::UpVector), 0.01f, 1.f);
    const float TangentMag = FMath::Sqrt(FMath::Max(0.f, 1.f - (UpDot * UpDot)));
    return (TangentMag / UpDot) * 100.f;
}
