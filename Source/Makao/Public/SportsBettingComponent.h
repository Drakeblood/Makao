// SportsBettingComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SportsBettingComponent.generated.h"

USTRUCT(BlueprintType)
struct FBetOutcomeOption
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SportsBetting")
    FName OutcomeId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SportsBetting")
    float TrueProbabilityWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SportsBetting")
    float DecimalOdds = 2.0f;
};

USTRUCT(BlueprintType)
struct FSportsEventConfig
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SportsBetting")
    FName EventId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SportsBetting")
    TArray<FBetOutcomeOption> OutcomeOptions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SportsBetting")
    float OverroundMargin = 0.0f;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MAKAO_API USportsBettingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USportsBettingComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SportsBetting|Config")
    float DefaultStake = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SportsBetting|Config")
    TArray<FSportsEventConfig> Events;

    UFUNCTION(BlueprintCallable, Category = "SportsBetting")
    float SimulateEventAndSettleBet(
        FName EventId,
        FName ChosenOutcomeId,
        float Stake,
        FName& OutWinningOutcomeId,
        bool& bOutPlayerWon
    );

    UFUNCTION(BlueprintCallable, Category = "SportsBetting")
    bool ComputeBetExpectedValue(
        FName EventId,
        FName OutcomeId,
        float Stake,
        float& OutEV
    ) const;

    UFUNCTION(BlueprintCallable, Category = "SportsBetting")
    void RecalculateDecimalOddsForEvent(FName EventId);

    UFUNCTION(BlueprintCallable, Category = "SportsBetting")
    void RecalculateDecimalOddsForAllEvents();

protected:
    virtual void BeginPlay() override;

private:
    const FSportsEventConfig* FindEvent(FName EventId) const;

    FSportsEventConfig* FindEventMutable(FName EventId);

    const FBetOutcomeOption* FindOutcome(const FSportsEventConfig& Event, FName OutcomeId) const;

    float GetTotalTrueProbabilityWeight(const FSportsEventConfig& Event) const;

    const FBetOutcomeOption* SimulateTrueOutcome(const FSportsEventConfig& Event) const;

    void RecalculateOddsInternal(FSportsEventConfig& Event);
};
