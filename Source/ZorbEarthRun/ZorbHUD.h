#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ZorbHUD.generated.h"

UCLASS()
class ZORBEARTHRUN_API AZorbHUD : public AHUD
{
    GENERATED_BODY()

public:
    virtual void DrawHUD() override;

private:
    FString FormatRaceTime(float Seconds) const;
};
