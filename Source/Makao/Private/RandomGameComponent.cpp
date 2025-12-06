// RandomGameComponent.cpp

#include "RandomGameComponent.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

URandomGameComponent::URandomGameComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void URandomGameComponent::BeginPlay()
{
    Super::BeginPlay();

    if (Outcomes.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("RandomGameComponent: missing configured outcomes na %s"),
            *GetOwner()->GetName());
    }
}

float URandomGameComponent::GetTotalWeight() const
{
    float TotalWeight = 0.0f;

    for (const FRandomGameOutcome& Outcome : Outcomes)
    {
        if (Outcome.ProbabilityWeight > 0.0f)
        {
            TotalWeight += Outcome.ProbabilityWeight;
        }
    }

    return TotalWeight;
}

float URandomGameComponent::PlayRound(float Stake, FRandomGameOutcome& OutChosenOutcome)
{
    if (Stake <= 0.0f)
    {
        Stake = DefaultStake;
    }

    const float TotalWeight = GetTotalWeight();

    if (TotalWeight <= 0.0f || Outcomes.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("RandomGameComponent: weight sum <= 0 or no outcomes found."));
        OutChosenOutcome = FRandomGameOutcome();
        return 0.0f;
    }

    const float RandomValue = FMath::FRandRange(0.0f, TotalWeight);
    float Accumulated = 0.0f;

    const FRandomGameOutcome* SelectedOutcome = nullptr;

    for (const FRandomGameOutcome& Outcome : Outcomes)
    {
        if (Outcome.ProbabilityWeight <= 0.0f)
        {
            continue;
        }

        Accumulated += Outcome.ProbabilityWeight;

        if (RandomValue <= Accumulated)
        {
            SelectedOutcome = &Outcome;
            break;
        }
    }

    if (!SelectedOutcome)
    {
        for (int32 i = Outcomes.Num() - 1; i >= 0; --i)
        {
            if (Outcomes[i].ProbabilityWeight > 0.0f)
            {
                SelectedOutcome = &Outcomes[i];
                break;
            }
        }
    }

    if (!SelectedOutcome)
    {
        UE_LOG(LogTemp, Warning, TEXT("RandomGameComponent: unable to choose outcome."));
        OutChosenOutcome = FRandomGameOutcome();
        return 0.0f;
    }

    OutChosenOutcome = *SelectedOutcome;

    const float NetWin = Stake * SelectedOutcome->PayoutMultiplier;

    return NetWin;
}

float URandomGameComponent::ComputeExpectedValue(float Stake) const
{
    if (Stake <= 0.0f)
    {
        Stake = DefaultStake;
    }

    const float TotalWeight = GetTotalWeight();

    if (TotalWeight <= 0.0f || Outcomes.Num() == 0)
    {
        return 0.0f;
    }

    float EVMultiplier = 0.0f;

    for (const FRandomGameOutcome& Outcome : Outcomes)
    {
        if (Outcome.ProbabilityWeight <= 0.0f)
        {
            continue;
        }

        const float Pi = Outcome.ProbabilityWeight / TotalWeight;
        EVMultiplier += Pi * Outcome.PayoutMultiplier;
    }

    const float EV = Stake * EVMultiplier;
    return EV;
}
