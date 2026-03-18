#include "ZorbGameMode.h"
#include "ZorbPawn.h"
#include "ZorbHUD.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/PlayerStart.h"
#include "HAL/FileManager.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/ConstructorHelpers.h"
#include "ZorbTuningSettings.h"

AZorbGameMode::AZorbGameMode()
{
    static ConstructorHelpers::FClassFinder<APawn> BPZorbPawnClass(TEXT("/Game/Blueprints/BP_ZorbPawn"));
    if (BPZorbPawnClass.Succeeded())
    {
        DefaultPawnClass = BPZorbPawnClass.Class;
    }
    else
    {
        DefaultPawnClass = AZorbPawn::StaticClass();
    }

    HUDClass = AZorbHUD::StaticClass();
    PrimaryActorTick.bCanEverTick = true;

    RaceStartTime = 0.f;
    bRaceFinished = false;
    LastCheckpointTransform = FTransform::Identity;
    bAutomationScenarioStarted = false;
    bAutomationScenarioCompleted = false;
    AutomationScenarioElapsed = 0.f;
    GhostSampleCursor = 0;
    GhostActor = nullptr;
    GhostVerticalOffset = 25.f;
}

void AZorbGameMode::BeginPlay()
{
    Super::BeginPlay();

    RaceStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
    bRaceFinished = false;

    if (AActor* Start = UGameplayStatics::GetActorOfClass(this, APlayerStart::StaticClass()))
    {
        LastCheckpointTransform = Start->GetActorTransform();
    }

    UE_LOG(LogTemp, Warning, TEXT("Race started at %.2f"), RaceStartTime);

    TryLoadGhostReplay();
    TryStartAutomationScenario();
}

void AZorbGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!bAutomationScenarioStarted)
    {
        TryStartAutomationScenario();
        return;
    }

    UpdateAutomationScenario(DeltaSeconds);
}

void AZorbGameMode::OnRaceFinished()
{
    if (bRaceFinished)
    {
        return;
    }

    bRaceFinished = true;

    const float ElapsedTime = GetElapsedRaceTime();
    UE_LOG(LogTemp, Warning, TEXT("Race finished in %.2f seconds"), ElapsedTime);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            4.0f,
            FColor::Green,
            FString::Printf(TEXT("Finish time: %.2fs"), ElapsedTime));
    }
}

float AZorbGameMode::GetElapsedRaceTime() const
{
    if (!GetWorld())
    {
        return 0.f;
    }

    return GetWorld()->GetTimeSeconds() - RaceStartTime;
}

bool AZorbGameMode::IsRaceFinished() const
{
    return bRaceFinished;
}

void AZorbGameMode::SetLastCheckpointTransform(const FTransform& NewCheckpointTransform)
{
    LastCheckpointTransform = NewCheckpointTransform;
}

FTransform AZorbGameMode::GetLastCheckpointTransform() const
{
    return LastCheckpointTransform;
}

void AZorbGameMode::TryStartAutomationScenario()
{
    const UZorbTuningSettings* ProjectSettings = UZorbTuningSettings::Get();
    if (!ProjectSettings || !ProjectSettings->Automation.bEnableScenarioRunner || !GetWorld() || bAutomationScenarioCompleted)
    {
        return;
    }

    if ((GetWorld()->GetTimeSeconds() - RaceStartTime) < ProjectSettings->Automation.StartupDelaySeconds)
    {
        return;
    }

    FZorbScenarioDefinition Scenario;
    if (!ResolveAutomationScenario(Scenario))
    {
        return;
    }

    AZorbPawn* ZorbPawn = Cast<AZorbPawn>(UGameplayStatics::GetPlayerPawn(this, 0));
    if (!ZorbPawn)
    {
        return;
    }

    ActiveAutomationScenario = Scenario;
    ZorbPawn->PrepareForAutomatedScenario(
        FTransform(ActiveAutomationScenario.StartRotation, ActiveAutomationScenario.StartLocation),
        ActiveAutomationScenario.InitialLinearVelocity);

    if (ProjectSettings->Automation.bRestartTelemetryOnScenarioStart)
    {
        ZorbPawn->RestartTelemetrySession();
    }

    bAutomationScenarioStarted = true;
    AutomationScenarioElapsed = 0.f;

    UE_LOG(LogTemp, Warning, TEXT("Automation scenario started: %s"), *ActiveAutomationScenario.ScenarioName.ToString());
}

void AZorbGameMode::UpdateAutomationScenario(float DeltaSeconds)
{
    AZorbPawn* ZorbPawn = Cast<AZorbPawn>(UGameplayStatics::GetPlayerPawn(this, 0));
    if (!ZorbPawn)
    {
        return;
    }

    AutomationScenarioElapsed += DeltaSeconds;

    float ForwardInput = 0.f;
    float RightInput = 0.f;
    bool bBoostEnabled = false;
    for (const FZorbScenarioInputKeyframe& Keyframe : ActiveAutomationScenario.InputTimeline)
    {
        if (Keyframe.TimeSeconds <= AutomationScenarioElapsed)
        {
            ForwardInput = Keyframe.ForwardInput;
            RightInput = Keyframe.RightInput;
            bBoostEnabled = Keyframe.bBoostActive;
        }
        else
        {
            break;
        }
    }

    ZorbPawn->ApplyAutomatedScenarioInput(ForwardInput, RightInput, bBoostEnabled);
    UpdateGhostReplay(AutomationScenarioElapsed);

    if (AutomationScenarioElapsed >= ActiveAutomationScenario.DurationSeconds)
    {
        FinishAutomationScenario();
    }
}

void AZorbGameMode::FinishAutomationScenario()
{
    if (AZorbPawn* ZorbPawn = Cast<AZorbPawn>(UGameplayStatics::GetPlayerPawn(this, 0)))
    {
        ZorbPawn->ResetAutomatedScenarioInput();
        ZorbPawn->EndTelemetrySession();
    }

    UE_LOG(LogTemp, Warning, TEXT("Automation scenario finished: %s"), *ActiveAutomationScenario.ScenarioName.ToString());
    ResetGhostReplay();
    bAutomationScenarioStarted = false;
    bAutomationScenarioCompleted = true;
}

bool AZorbGameMode::ResolveAutomationScenario(FZorbScenarioDefinition& OutScenario) const
{
    const UZorbTuningSettings* ProjectSettings = UZorbTuningSettings::Get();
    if (!ProjectSettings)
    {
        return false;
    }

    const FName RequestedScenario = ProjectSettings->Automation.ScenarioName;
    for (const FZorbScenarioDefinition& Scenario : ProjectSettings->Automation.Scenarios)
    {
        if (!RequestedScenario.IsNone())
        {
            if (Scenario.ScenarioName == RequestedScenario)
            {
                OutScenario = Scenario;
                return true;
            }
        }
        else if (!Scenario.ScenarioName.IsNone())
        {
            OutScenario = Scenario;
            return true;
        }
    }

    return false;
}

void AZorbGameMode::TryLoadGhostReplay()
{
    const UZorbTuningSettings* ProjectSettings = UZorbTuningSettings::Get();
    if (!ProjectSettings || !ProjectSettings->Automation.bEnableScenarioRunner || !ProjectSettings->Automation.bEnableGhostReplay)
    {
        return;
    }

    FString CsvPath;
    if (!ResolveGhostCsvPath(CsvPath))
    {
        UE_LOG(LogTemp, Warning, TEXT("Ghost replay: no telemetry CSV found."));
        return;
    }

    if (!LoadGhostSamplesFromCsv(CsvPath))
    {
        UE_LOG(LogTemp, Warning, TEXT("Ghost replay: failed to parse CSV %s"), *CsvPath);
        return;
    }

    if (!GhostActor && GetWorld())
    {
        GhostActor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), FTransform::Identity);
        if (GhostActor)
        {
            UStaticMeshComponent* GhostMesh = GhostActor->GetStaticMeshComponent();
            if (GhostMesh)
            {
                UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
                UMaterialInterface* GhostMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineMaterials/Widget3DPassThrough_Translucent.Widget3DPassThrough_Translucent"));
                GhostMesh->SetMobility(EComponentMobility::Movable);
                if (SphereMesh)
                {
                    GhostMesh->SetStaticMesh(SphereMesh);
                }
                if (GhostMaterial)
                {
                    UMaterialInstanceDynamic* GhostMID = UMaterialInstanceDynamic::Create(GhostMaterial, this);
                    if (GhostMID)
                    {
                        GhostMID->SetVectorParameterValue(TEXT("TintColorAndOpacity"), FLinearColor(0.05f, 0.95f, 1.0f, 0.32f));
                        GhostMesh->SetMaterial(0, GhostMID);
                    }
                    else
                    {
                        GhostMesh->SetMaterial(0, GhostMaterial);
                    }
                }
                GhostMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                GhostMesh->SetCastShadow(false);
                GhostMesh->SetRenderCustomDepth(false);
                GhostMesh->SetVisibility(true);
                GhostMesh->SetWorldScale3D(FVector(ProjectSettings->Automation.GhostVisualScale * 1.05f));
            }
            GhostActor->SetActorEnableCollision(false);
            GhostActor->SetActorTickEnabled(false);
        }
    }

    GhostSampleCursor = 0;
    if (GhostActor && GhostSamples.Num() > 0)
    {
        GhostActor->SetActorLocation(GhostSamples[0].Position + FVector(0.f, 0.f, GhostVerticalOffset));
    }

    UE_LOG(LogTemp, Log, TEXT("Ghost replay loaded: %s (%d samples)"), *CsvPath, GhostSamples.Num());
}

bool AZorbGameMode::ResolveGhostCsvPath(FString& OutCsvPath) const
{
    OutCsvPath.Empty();

    const UZorbTuningSettings* ProjectSettings = UZorbTuningSettings::Get();
    if (!ProjectSettings || !ProjectSettings->Automation.bEnableGhostReplay)
    {
        return false;
    }

    if (!ProjectSettings->Automation.bUseLatestTelemetryGhost && !ProjectSettings->Automation.GhostTelemetryFile.IsEmpty())
    {
        const FString Candidate = FPaths::ConvertRelativePathToFull(ProjectSettings->Automation.GhostTelemetryFile);
        if (FPaths::FileExists(Candidate))
        {
            OutCsvPath = Candidate;
            return true;
        }
        return false;
    }

    const FString TelemetryDir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Telemetry"));
    TArray<FString> FoundFiles;
    IFileManager::Get().FindFiles(FoundFiles, *FPaths::Combine(TelemetryDir, TEXT("*.csv")), true, false);
    if (FoundFiles.Num() == 0)
    {
        return false;
    }

    FoundFiles.Sort([&TelemetryDir](const FString& A, const FString& B)
    {
        const FDateTime TimeA = IFileManager::Get().GetTimeStamp(*FPaths::Combine(TelemetryDir, A));
        const FDateTime TimeB = IFileManager::Get().GetTimeStamp(*FPaths::Combine(TelemetryDir, B));
        return TimeA > TimeB;
    });

    OutCsvPath = FPaths::Combine(TelemetryDir, FoundFiles[0]);
    return true;
}

bool AZorbGameMode::LoadGhostSamplesFromCsv(const FString& CsvPath)
{
    FString CsvContent;
    if (!FFileHelper::LoadFileToString(CsvContent, *CsvPath))
    {
        return false;
    }

    TArray<FString> Lines;
    CsvContent.ParseIntoArrayLines(Lines, true);
    if (Lines.Num() <= 1)
    {
        return false;
    }

    GhostSamples.Reset();
    float FirstTime = 0.f;
    bool bHasFirstTime = false;

    for (int32 LineIndex = 1; LineIndex < Lines.Num(); ++LineIndex)
    {
        TArray<FString> Columns;
        Lines[LineIndex].ParseIntoArray(Columns, TEXT(","), false);
        if (Columns.Num() < 4)
        {
            continue;
        }

        const float TimeSec = FCString::Atof(*Columns[0]);
        const float PosX = FCString::Atof(*Columns[1]);
        const float PosY = FCString::Atof(*Columns[2]);
        const float PosZ = FCString::Atof(*Columns[3]);

        if (!bHasFirstTime)
        {
            FirstTime = TimeSec;
            bHasFirstTime = true;
        }

        FGhostSample Sample;
        Sample.TimeSeconds = FMath::Max(0.f, TimeSec - FirstTime);
        Sample.Position = FVector(PosX, PosY, PosZ);
        GhostSamples.Add(Sample);
    }

    GhostSampleCursor = 0;
    return GhostSamples.Num() >= 2;
}

void AZorbGameMode::UpdateGhostReplay(float ScenarioTimeSeconds)
{
    if (!GhostActor || GhostSamples.Num() < 2)
    {
        return;
    }

    const FVector GhostPosition = EvaluateGhostPosition(ScenarioTimeSeconds);
    GhostActor->SetActorLocation(GhostPosition + FVector(0.f, 0.f, GhostVerticalOffset));
}

FVector AZorbGameMode::EvaluateGhostPosition(float ScenarioTimeSeconds) const
{
    if (GhostSamples.Num() == 0)
    {
        return FVector::ZeroVector;
    }

    if (ScenarioTimeSeconds <= GhostSamples[0].TimeSeconds)
    {
        return GhostSamples[0].Position;
    }

    if (ScenarioTimeSeconds >= GhostSamples.Last().TimeSeconds)
    {
        return GhostSamples.Last().Position;
    }

    for (int32 Index = 1; Index < GhostSamples.Num(); ++Index)
    {
        if (ScenarioTimeSeconds <= GhostSamples[Index].TimeSeconds)
        {
            const FGhostSample& Prev = GhostSamples[Index - 1];
            const FGhostSample& Next = GhostSamples[Index];
            const float Denominator = FMath::Max(KINDA_SMALL_NUMBER, Next.TimeSeconds - Prev.TimeSeconds);
            const float Alpha = FMath::Clamp((ScenarioTimeSeconds - Prev.TimeSeconds) / Denominator, 0.f, 1.f);
            return FMath::Lerp(Prev.Position, Next.Position, Alpha);
        }
    }

    return GhostSamples.Last().Position;
}

void AZorbGameMode::ResetGhostReplay()
{
    GhostSampleCursor = 0;
    if (GhostActor)
    {
        GhostActor->Destroy();
        GhostActor = nullptr;
    }
}
