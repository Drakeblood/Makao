// ColorTerritoryBettingComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ColorTerritoryBettingComponent.generated.h"

UENUM(BlueprintType)
enum class EBetColor : uint8
{
    Blue    UMETA(DisplayName = "Blue"),
    Orange  UMETA(DisplayName = "Orange"),
    Green   UMETA(DisplayName = "Green"),
    Purple  UMETA(DisplayName = "Purple")
};

USTRUCT(BlueprintType)
struct FColorBetInfo
{
    GENERATED_BODY()

public:

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Betting")
    int32 BlockCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Betting")
    float Share = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Betting")
    float DecimalOdds = 0.0f;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MAKAO_API UColorTerritoryBettingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UColorTerritoryBettingComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Betting|Config")
    float BaseOdds = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Betting|Config")
    float Margin = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Betting|Config")
    float MinOdds = 1.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Betting|Config")
    float MaxOdds = 100.0f;

    UFUNCTION(BlueprintCallable, Category = "Betting")
    void SetAllBlockCounts(int32 BlueBlocks, int32 OrangeBlocks, int32 GreenBlocks, int32 PurpleBlocks);

    UFUNCTION(BlueprintCallable, Category = "Betting")
    void SetBlockCountForColor(EBetColor Color, int32 BlockCount, bool bRecalculateImmediately);

    UFUNCTION(BlueprintCallable, Category = "Betting")
    void RecalculateSharesAndOdds();

    UFUNCTION(BlueprintPure, Category = "Betting")
    float GetOdds(EBetColor Color) const;

    UFUNCTION(BlueprintPure, Category = "Betting")
    FColorBetInfo GetBetInfo(EBetColor Color) const;

    UFUNCTION(BlueprintPure, Category = "Betting")
    float GetShare(EBetColor Color) const;

    UFUNCTION(BlueprintPure, Category = "Betting")
    int32 GetBlockCount(EBetColor Color) const;

    UFUNCTION(BlueprintPure, Category = "Betting")
    float GetExpectedValueForColor(EBetColor Color, float Stake) const;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, Category = "Betting")
    TMap<EBetColor, FColorBetInfo> ColorInfo;

    void InitializeColorInfoIfNeeded();

    void InternalSetBlockCount(EBetColor Color, int32 BlockCount);
};
