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

    AutoPossessPlayer = EAutoReceiveInput::Player0;

    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
    CollisionComponent->SetSphereRadius(50.f);
    CollisionComponent->SetCollisionProfileName(TEXT("Pawn"));
    CollisionComponent->SetSimulatePhysics(true);
    CollisionComponent->SetEnableGravity(true);
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

        const float InputMagnitude = (OverheatLockRemaining > 0.f) ? 0.f : RawInputMagnitude;
        bBoostActive = bBoostRequested && OverheatLockRemaining <= 0.f && CurrentEnergy >= EnergyTuning.MinEnergyToBoost;

        CollisionComponent->SetLinearDamping(FMath::Lerp(MovementTuning.IdleLinearDamping, MovementTuning.DriveLinearDamping, InputMagnitude));

        FVector Velocity = CollisionComponent->GetPhysicsLinearVelocity();
        const float Speed = Velocity.Size();
        if (Speed > MovementTuning.MaxSpeed)
        {
            const FVector OverSpeedBrake = -Velocity.GetSafeNormal() * ((Speed - MovementTuning.MaxSpeed) * 1200.f);
            CollisionComponent->AddForce(OverSpeedBrake, NAME_None, false);
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

                const FVector DesiredVelocity = DesiredDir * (InputMagnitude * MovementTuning.MaxSpeed);
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
                CollisionComponent->AddForce(-LateralVel * MovementTuning.LateralGripForce, NAME_None, false);
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

    const float Speed = CollisionComponent->GetPhysicsLinearVelocity().Size();
    const float SpeedFactor = FMath::Clamp(Speed / 2500.f, 0.f, 1.f);
    const float InputFactor = FMath::Clamp(
        FMath::Sqrt((SmoothedForwardInputValue * SmoothedForwardInputValue) + (SmoothedRightInputValue * SmoothedRightInputValue)),
        0.f,
        1.f);

    const float EnergyDelta = ((EnergyTuning.EnergyGainRate * InputFactor) + (4.f * SpeedFactor) - EnergyTuning.EnergyDrainRate) * DeltaTime;
    CurrentEnergy = FMath::Clamp(CurrentEnergy + EnergyDelta, 0.f, EnergyTuning.MaxEnergy);

    const bool bActivelyDriving = InputFactor > 0.1f;
    const float HeatDelta = bActivelyDriving
        ? ((EnergyTuning.HeatGainRate * (0.4f + 0.6f * InputFactor)) + (3.f * SpeedFactor)) * DeltaTime
        : -(EnergyTuning.HeatCooldownRate * DeltaTime);
    CurrentHeat = FMath::Clamp(CurrentHeat + HeatDelta, 0.f, EnergyTuning.MaxHeat);

    if (bBoostActive)
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
    if (!SpringArmComponent || !CollisionComponent)
    {
        return;
    }

    FVector Velocity = CollisionComponent->GetPhysicsLinearVelocity();
    FVector VelocityXY = Velocity;
    VelocityXY.Z = 0.f;

    // Calculate desired camera yaw based on ball direction.
    FRotator TargetRotation = DesiredCameraRotation;
    
    const float VelMag = VelocityXY.Length();
    if (VelMag > 50.f) // Only update if moving significantly
    {
        FVector ForwardDir = VelocityXY.GetSafeNormal();
        TargetRotation = ForwardDir.Rotation();
        TargetRotation.Pitch = MovementTuning.CameraPitchAngle;
        TargetRotation.Roll = 0.f;
    }

    // Smooth interpolation to new rotation.
    const float BlendAlpha = FMath::Clamp(DeltaTime * MovementTuning.CameraRotationSpeed, 0.f, 1.f);
    DesiredCameraRotation = FMath::Lerp(DesiredCameraRotation, TargetRotation, BlendAlpha);

    // Apply to spring arm.
    SpringArmComponent->SetUsingAbsoluteRotation(true);
    SpringArmComponent->SetRelativeRotation(DesiredCameraRotation);
    SpringArmComponent->TargetArmLength = MovementTuning.CameraDistance;
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
