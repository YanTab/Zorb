#include "CheckpointTrigger.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "ZorbGameMode.h"

ACheckpointTrigger::ACheckpointTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CheckpointBox"));
    BoxComponent->SetBoxExtent(FVector(180.f, 180.f, 180.f));
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
        VisualMesh->SetRelativeScale3D(FVector(3.6f, 3.6f, 0.08f));
        VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
        VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        VisualMesh->SetVisibility(true);
    }

    BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ACheckpointTrigger::OnOverlapBegin);
}

void ACheckpointTrigger::BeginPlay()
{
    Super::BeginPlay();
}

void ACheckpointTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || !OtherActor->IsA(APawn::StaticClass()))
    {
        return;
    }

    AZorbGameMode* GameMode = Cast<AZorbGameMode>(UGameplayStatics::GetGameMode(this));
    if (GameMode)
    {
        GameMode->SetLastCheckpointTransform(GetActorTransform());
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Checkpoint reached"));
    }

    UE_LOG(LogTemp, Warning, TEXT("Checkpoint reached at %s"), *GetActorLocation().ToString());
}
