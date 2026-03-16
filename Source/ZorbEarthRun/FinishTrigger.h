#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FinishTrigger.generated.h"

UCLASS()
class ZORBEARTHRUN_API AFinishTrigger : public AActor
{
    GENERATED_BODY()

public:
    AFinishTrigger();

protected:
    virtual void BeginPlay() override;

private:
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UPROPERTY(VisibleAnywhere)
    class UBoxComponent* BoxComponent;

    UPROPERTY(VisibleAnywhere)
    class UStaticMeshComponent* VisualMesh;
};