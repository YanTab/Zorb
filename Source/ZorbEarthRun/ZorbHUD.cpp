#include "ZorbHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "ZorbGameMode.h"
#include "ZorbPawn.h"

void AZorbHUD::DrawHUD()
{
    Super::DrawHUD();

    const APlayerController* PC = GetOwningPlayerController();
    const APawn* Pawn = PC ? PC->GetPawn() : nullptr;
    const AZorbPawn* ZorbPawn = Cast<AZorbPawn>(Pawn);
    const AZorbGameMode* GameMode = GetWorld() ? Cast<AZorbGameMode>(GetWorld()->GetAuthGameMode()) : nullptr;

    const float ElapsedTime = GameMode ? GameMode->GetElapsedRaceTime() : 0.f;
    const float SpeedKmh = Pawn ? Pawn->GetVelocity().Size() * 0.036f : 0.f;
    const FString RaceState = (GameMode && GameMode->IsRaceFinished()) ? TEXT("FINISHED") : TEXT("RUNNING");

    const FString TimerText = FString::Printf(TEXT("Time: %s"), *FormatRaceTime(ElapsedTime));
    const FString StateText = FString::Printf(TEXT("State: %s"), *RaceState);
    const FString SpeedText = FString::Printf(TEXT("Speed: %.1f km/h"), SpeedKmh);
    const float EnergyPct = ZorbPawn ? ZorbPawn->GetEnergyPercent() : 0.f;
    const float HeatPct = ZorbPawn ? ZorbPawn->GetHeatPercent() : 0.f;
    const int32 OverheatLevel = ZorbPawn ? ZorbPawn->GetOverheatLevel() : 0;
    const bool bBoosting = ZorbPawn ? ZorbPawn->IsBoosting() : false;
    const float CriticalFlashAlpha = ZorbPawn ? ZorbPawn->GetCriticalFlashAlpha() : 0.f;
    const bool bGrounded = ZorbPawn ? ZorbPawn->IsGrounded() : false;
    const float SlopePct = ZorbPawn ? ZorbPawn->GetGroundSlopePercent() : 0.f;
    const FString EnergyText = FString::Printf(TEXT("Energy: %3.0f%%"), EnergyPct * 100.f);
    const FString HeatText = FString::Printf(TEXT("Heat: %3.0f%%"), HeatPct * 100.f);
    const FString LevelText = FString::Printf(TEXT("Overheat Level: %d/3"), OverheatLevel);
    const FString BoostText = FString::Printf(TEXT("Boost: %s"), bBoosting ? TEXT("ACTIVE") : TEXT("OFF"));
    const FString SlopeText = bGrounded
        ? FString::Printf(TEXT("Slope: %.1f%%"), SlopePct)
        : FString(TEXT("Slope: AIR"));

    const FVector2D Origin(50.f, 50.f);
    UFont* Font = GEngine ? GEngine->GetLargeFont() : nullptr;

    DrawText(TimerText, FLinearColor::White, Origin.X, Origin.Y, Font, 1.0f, false);
    DrawText(StateText, FLinearColor::Yellow, Origin.X, Origin.Y + 35.f, Font, 0.85f, false);
    DrawText(SpeedText, FLinearColor(0.0f, 1.0f, 1.0f, 1.0f), Origin.X, Origin.Y + 65.f, Font, 0.85f, false);
    DrawText(EnergyText, FLinearColor::Green, Origin.X, Origin.Y + 95.f, Font, 0.85f, false);
    DrawText(HeatText, FLinearColor(1.0f, 0.5f, 0.0f, 1.0f), Origin.X, Origin.Y + 125.f, Font, 0.85f, false);
    DrawText(LevelText, OverheatLevel >= 2 ? FLinearColor::Red : FLinearColor::Yellow, Origin.X, Origin.Y + 155.f, Font, 0.85f, false);
    DrawText(BoostText, bBoosting ? FLinearColor::Green : FLinearColor(0.7f, 0.7f, 0.7f, 1.0f), Origin.X, Origin.Y + 185.f, Font, 0.85f, false);
    DrawText(SlopeText, bGrounded ? FLinearColor(0.6f, 0.9f, 1.0f, 1.0f) : FLinearColor(0.6f, 0.6f, 0.6f, 1.0f), Origin.X, Origin.Y + 215.f, Font, 0.85f, false);

    if (CriticalFlashAlpha > 0.f)
    {
        const float OverlayAlpha = 0.55f * CriticalFlashAlpha;
        DrawRect(FLinearColor(1.0f, 0.05f, 0.05f, OverlayAlpha), 0.f, 0.f, Canvas->SizeX, Canvas->SizeY);
        DrawText(TEXT("OVERHEAT CRITIQUE!"), FLinearColor::White, Origin.X, Origin.Y + 255.f, Font, 1.0f, false);
    }
}

FString AZorbHUD::FormatRaceTime(float Seconds) const
{
    const int32 TotalCentiseconds = FMath::Max(0, FMath::RoundToInt(Seconds * 100.f));
    const int32 Minutes = TotalCentiseconds / 6000;
    const int32 SecondsPart = (TotalCentiseconds / 100) % 60;
    const int32 Centiseconds = TotalCentiseconds % 100;

    return FString::Printf(TEXT("%02d:%02d.%02d"), Minutes, SecondsPart, Centiseconds);
}
