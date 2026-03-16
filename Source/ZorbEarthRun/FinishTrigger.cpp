#include "FinishTrigger.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "ZorbGameMode.h"

AFinishTrigger::AFinishTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("FinishBox"));
    BoxComponent->SetBoxExtent(FVector(200.f, 200.f, 200.f));
    BoxComponent->SetCollisionProfileName(TEXT("Trigger"));
    BoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    BoxComponent->SetGenerateOverlapEvents(true);
    BoxComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
    RootComponent = BoxComponent;

    VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
    VisualMesh->SetupAttachment(RootComponent);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded())
    {
        VisualMesh->SetStaticMesh(CubeMesh.Object);
        VisualMesh->SetRelativeScale3D(FVector(4.f, 4.f, 0.1f));
        VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, -95.f));
        VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        VisualMesh->SetMobility(EComponentMobility::Static);
        VisualMesh->SetVisibility(true);
    }

    BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AFinishTrigger::OnOverlapBegin);
    UE_LOG(LogTemp, Warning, TEXT("FinishTrigger created: %s"), *GetName());
}

void AFinishTrigger::BeginPlay()
{
    Super::BeginPlay();
}

void AFinishTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("FinishTrigger overlap with %s (%s)"),
        OtherActor ? *OtherActor->GetName() : TEXT("None"),
        OtherActor ? *OtherActor->GetClass()->GetName() : TEXT("None"));

    if (OtherActor && OtherActor->IsA(APawn::StaticClass()))
    {
        UE_LOG(LogTemp, Warning, TEXT("Finish line reached by pawn!"));

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                3.0f,
                FColor::Cyan,
                TEXT("You reached the finish!"));
        }

        APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
        if (PC)
        {
            PC->ClientMessage(TEXT("You reached the finish!"));
        }

        AZorbGameMode* GameMode = Cast<AZorbGameMode>(UGameplayStatics::GetGameMode(this));
        if (GameMode)
        {
            GameMode->OnRaceFinished();
        }

        BoxComponent->SetGenerateOverlapEvents(false);
    }
}
