// ColorTerritoryBettingComponent.cpp

#include "ColorTerritoryBettingComponent.h"
#include "Math/UnrealMathUtility.h"

UColorTerritoryBettingComponent::UColorTerritoryBettingComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UColorTerritoryBettingComponent::BeginPlay()
{
    Super::BeginPlay();

    InitializeColorInfoIfNeeded();
}

void UColorTerritoryBettingComponent::InitializeColorInfoIfNeeded()
{
    if (ColorInfo.Num() == 0)
    {
        ColorInfo.Add(EBetColor::Blue, FColorBetInfo());
        ColorInfo.Add(EBetColor::Orange, FColorBetInfo());
        ColorInfo.Add(EBetColor::Green, FColorBetInfo());
        ColorInfo.Add(EBetColor::Purple, FColorBetInfo());
    }
}

void UColorTerritoryBettingComponent::InternalSetBlockCount(EBetColor Color, int32 BlockCount)
{
    InitializeColorInfoIfNeeded();

    FColorBetInfo* Info = ColorInfo.Find(Color);
    if (Info)
    {
        Info->BlockCount = FMath::Max(0, BlockCount);
    }
}

void UColorTerritoryBettingComponent::SetAllBlockCounts(int32 BlueBlocks, int32 OrangeBlocks, int32 GreenBlocks, int32 PurpleBlocks)
{
    InternalSetBlockCount(EBetColor::Blue, BlueBlocks);
    InternalSetBlockCount(EBetColor::Orange, OrangeBlocks);
    InternalSetBlockCount(EBetColor::Green, GreenBlocks);
    InternalSetBlockCount(EBetColor::Purple, PurpleBlocks);

    RecalculateSharesAndOdds();
}

void UColorTerritoryBettingComponent::SetBlockCountForColor(EBetColor Color, int32 BlockCount, bool bRecalculateImmediately)
{
    InternalSetBlockCount(Color, BlockCount);

    if (bRecalculateImmediately)
    {
        RecalculateSharesAndOdds();
    }
}

void UColorTerritoryBettingComponent::RecalculateSharesAndOdds()
{
    InitializeColorInfoIfNeeded();

    int32 TotalBlocks = 0;
    int32 ActiveColors = 0;

    for (const TPair<EBetColor, FColorBetInfo>& Pair : ColorInfo)
    {
        int32 Clamped = FMath::Max(0, Pair.Value.BlockCount);
        if (Clamped > 0)
        {
            TotalBlocks += Clamped;
            ActiveColors++;
        }
    }

    if (ActiveColors <= 0 || TotalBlocks <= 0)
    {
        for (TPair<EBetColor, FColorBetInfo>& Pair : ColorInfo)
        {
            Pair.Value.Share = 0.0f;
            Pair.Value.DecimalOdds = 0.0f;
        }
        return;
    }

    const float TotalBlocksFloat = static_cast<float>(TotalBlocks);
    const float AvgShare = 1.0f / static_cast<float>(ActiveColors);

    float EffectiveBaseOdds = BaseOdds - Margin;
    if (EffectiveBaseOdds < MinOdds)
    {
        EffectiveBaseOdds = MinOdds;
    }

    for (TPair<EBetColor, FColorBetInfo>& Pair : ColorInfo)
    {
        FColorBetInfo& Info = Pair.Value;
        int32 Clamped = FMath::Max(0, Info.BlockCount);

        if (Clamped <= 0)
        {
            Info.Share = 0.0f;
            Info.DecimalOdds = 0.0f;
            continue;
        }

        Info.Share = static_cast<float>(Clamped) / TotalBlocksFloat;

        float Odds = EffectiveBaseOdds * (AvgShare / Info.Share);
        Odds = FMath::Clamp(Odds, MinOdds, MaxOdds);

        Info.DecimalOdds = Odds;
    }
}

float UColorTerritoryBettingComponent::GetOdds(EBetColor Color) const
{
    const FColorBetInfo* Info = ColorInfo.Find(Color);
    if (Info)
    {
        return Info->DecimalOdds;
    }
    return 0.0f;
}

FColorBetInfo UColorTerritoryBettingComponent::GetBetInfo(EBetColor Color) const
{
    const FColorBetInfo* Info = ColorInfo.Find(Color);
    if (Info)
    {
        return *Info;
    }
    return FColorBetInfo();
}

float UColorTerritoryBettingComponent::GetShare(EBetColor Color) const
{
    const FColorBetInfo* Info = ColorInfo.Find(Color);
    if (Info)
    {
        return Info->Share;
    }
    return 0.0f;
}

int32 UColorTerritoryBettingComponent::GetBlockCount(EBetColor Color) const
{
    const FColorBetInfo* Info = ColorInfo.Find(Color);
    if (Info)
    {
        return Info->BlockCount;
    }
    return 0;
}

float UColorTerritoryBettingComponent::GetExpectedValueForColor(EBetColor Color, float Stake) const
{
    if (Stake <= 0.0f)
    {
        return 0.0f;
    }

    const FColorBetInfo* Info = ColorInfo.Find(Color);
    if (!Info || Info->Share <= 0.0f || Info->DecimalOdds <= 0.0f)
    {
        return 0.0f;
    }

    const float p = Info->Share;
    const float Odds = Info->DecimalOdds;

    const float EVPerStake = p * (Odds - 1.0f) + (1.0f - p) * (-1.0f);
    return Stake * EVPerStake;
}
