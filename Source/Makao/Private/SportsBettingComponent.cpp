// SportsBettingComponent.cpp

#include "SportsBettingComponent.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

USportsBettingComponent::USportsBettingComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void USportsBettingComponent::BeginPlay()
{
    Super::BeginPlay();

    if (Events.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("SportsBettingComponent: no configured events on %s"),
            *GetOwner()->GetName());
    }
}

const FSportsEventConfig* USportsBettingComponent::FindEvent(FName EventId) const
{
    for (const FSportsEventConfig& Event : Events)
    {
        if (Event.EventId == EventId)
        {
            return &Event;
        }
    }
    return nullptr;
}

FSportsEventConfig* USportsBettingComponent::FindEventMutable(FName EventId)
{
    for (FSportsEventConfig& Event : Events)
    {
        if (Event.EventId == EventId)
        {
            return &Event;
        }
    }
    return nullptr;
}

const FBetOutcomeOption* USportsBettingComponent::FindOutcome(const FSportsEventConfig& Event, FName OutcomeId) const
{
    for (const FBetOutcomeOption& Option : Event.OutcomeOptions)
    {
        if (Option.OutcomeId == OutcomeId)
        {
            return &Option;
        }
    }
    return nullptr;
}

float USportsBettingComponent::GetTotalTrueProbabilityWeight(const FSportsEventConfig& Event) const
{
    float Total = 0.0f;
    for (const FBetOutcomeOption& Option : Event.OutcomeOptions)
    {
        if (Option.TrueProbabilityWeight > 0.0f)
        {
            Total += Option.TrueProbabilityWeight;
        }
    }
    return Total;
}

const FBetOutcomeOption* USportsBettingComponent::SimulateTrueOutcome(const FSportsEventConfig& Event) const
{
    const float TotalWeight = GetTotalTrueProbabilityWeight(Event);
    if (TotalWeight <= 0.0f || Event.OutcomeOptions.Num() == 0)
    {
        return nullptr;
    }

    const float RandomValue = FMath::FRandRange(0.0f, TotalWeight);
    float Accumulated = 0.0f;

    const FBetOutcomeOption* Selected = nullptr;

    for (const FBetOutcomeOption& Option : Event.OutcomeOptions)
    {
        if (Option.TrueProbabilityWeight <= 0.0f)
        {
            continue;
        }

        Accumulated += Option.TrueProbabilityWeight;

        if (RandomValue <= Accumulated)
        {
            Selected = &Option;
            break;
        }
    }

    if (!Selected)
    {
        for (int32 i = Event.OutcomeOptions.Num() - 1; i >= 0; --i)
        {
            if (Event.OutcomeOptions[i].TrueProbabilityWeight > 0.0f)
            {
                Selected = &Event.OutcomeOptions[i];
                break;
            }
        }
    }

    return Selected;
}

float USportsBettingComponent::SimulateEventAndSettleBet(
    FName EventId,
    FName ChosenOutcomeId,
    float Stake,
    FName& OutWinningOutcomeId,
    bool& bOutPlayerWon
)
{
    OutWinningOutcomeId = NAME_None;
    bOutPlayerWon = false;

    if (Stake <= 0.0f)
    {
        Stake = DefaultStake;
    }

    const FSportsEventConfig* Event = FindEvent(EventId);
    if (!Event)
    {
        UE_LOG(LogTemp, Warning, TEXT("SportsBettingComponent: not found event (%s) on %s"),
            *EventId.ToString(), *GetOwner()->GetName());
        return 0.0f;
    }

    const float TotalWeight = GetTotalTrueProbabilityWeight(*Event);
    if (TotalWeight <= 0.0f || Event->OutcomeOptions.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("SportsBettingComponent: propability calculation failed for event (%s)"),
            *EventId.ToString());
        return 0.0f;
    }

    const FBetOutcomeOption* WinningOption = SimulateTrueOutcome(*Event);
    if (!WinningOption)
    {
        UE_LOG(LogTemp, Warning, TEXT("SportsBettingComponent: couldn't roll outcome for event (%s)"),
            *EventId.ToString());
        return 0.0f;
    }

    OutWinningOutcomeId = WinningOption->OutcomeId;

    const FBetOutcomeOption* PlayerOption = FindOutcome(*Event, ChosenOutcomeId);
    if (!PlayerOption)
    {
        UE_LOG(LogTemp, Warning, TEXT("SportsBettingComponent: OutcomeId (%s) doesn't exist in event (%s)"),
            *ChosenOutcomeId.ToString(), *EventId.ToString());
        // Zak³ad niepoprawny – zwracamy 0.
        return 0.0f;
    }

    bOutPlayerWon = (ChosenOutcomeId == OutWinningOutcomeId);

    float NetWin = 0.0f;

    if (bOutPlayerWon)
    {
        NetWin = Stake * (PlayerOption->DecimalOdds - 1.0f);
    }
    else
    {
        NetWin = -Stake;
    }

    return NetWin;
}

bool USportsBettingComponent::ComputeBetExpectedValue(
    FName EventId,
    FName OutcomeId,
    float Stake,
    float& OutEV
) const
{
    OutEV = 0.0f;

    if (Stake <= 0.0f)
    {
        Stake = DefaultStake;
    }

    const FSportsEventConfig* Event = FindEvent(EventId);
    if (!Event)
    {
        UE_LOG(LogTemp, Warning, TEXT("SportsBettingComponent::ComputeBetExpectedValue: not found event (%s)"),
            *EventId.ToString());
        return false;
    }

    const FBetOutcomeOption* Option = FindOutcome(*Event, OutcomeId);
    if (!Option)
    {
        UE_LOG(LogTemp, Warning, TEXT("SportsBettingComponent::ComputeBetExpectedValue: OutcomeId (%s) doesn't exist in event (%s)"),
            *OutcomeId.ToString(), *EventId.ToString());
        return false;
    }

    const float TotalWeight = GetTotalTrueProbabilityWeight(*Event);
    if (TotalWeight <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("SportsBettingComponent::ComputeBetExpectedValue: weight sum <= 0 for event (%s)"),
            *EventId.ToString());
        return false;
    }

    const float pTrue = Option->TrueProbabilityWeight / TotalWeight;
    const float Odds = Option->DecimalOdds;

    const float ExpectedNet = Stake * (pTrue * (Odds - 1.0f) + (1.0f - pTrue) * (-1.0f));

    OutEV = ExpectedNet;
    return true;
}

void USportsBettingComponent::RecalculateOddsInternal(FSportsEventConfig& Event)
{
    const float TotalWeight = GetTotalTrueProbabilityWeight(Event);
    if (TotalWeight <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("SportsBettingComponent::RecalculateOddsInternal: weight sum <= 0 for event (%s)"),
            *Event.EventId.ToString());
        return;
    }

    const float Margin = Event.OverroundMargin;
    const float Factor = 1.0f / (1.0f + Margin); 

    for (FBetOutcomeOption& Option : Event.OutcomeOptions)
    {
        if (Option.TrueProbabilityWeight <= 0.0f)
        {
            continue;
        }

        const float pTrue = Option.TrueProbabilityWeight / TotalWeight;

        const float FairOdds = 1.0f / pTrue;

        Option.DecimalOdds = FairOdds * Factor;
    }
}

void USportsBettingComponent::RecalculateDecimalOddsForEvent(FName EventId)
{
    FSportsEventConfig* Event = FindEventMutable(EventId);
    if (!Event)
    {
        UE_LOG(LogTemp, Warning, TEXT("SportsBettingComponent::RecalculateDecimalOddsForEvent: not found event (%s)"),
            *EventId.ToString());
        return;
    }

    RecalculateOddsInternal(*Event);
}

void USportsBettingComponent::RecalculateDecimalOddsForAllEvents()
{
    for (FSportsEventConfig& Event : Events)
    {
        RecalculateOddsInternal(Event);
    }
}
