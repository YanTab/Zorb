#include "ZorbGameMode.h"
#include "ZorbPawn.h"
#include "ZorbHUD.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

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

    RaceStartTime = 0.f;
    bRaceFinished = false;
    LastCheckpointTransform = FTransform::Identity;
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
