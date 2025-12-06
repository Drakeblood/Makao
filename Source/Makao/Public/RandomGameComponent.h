// RandomGameComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RandomGameComponent.generated.h"

USTRUCT(BlueprintType)
struct FRandomGameOutcome
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomGame")
    FName OutcomeId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomGame")
    float ProbabilityWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomGame")
    float PayoutMultiplier = -1.0f;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MAKAO_API URandomGameComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URandomGameComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomGame|Config")
    float DefaultStake = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomGame|Config")
    TArray<FRandomGameOutcome> Outcomes;

    UFUNCTION(BlueprintCallable, Category = "RandomGame")
    float PlayRound(float Stake, FRandomGameOutcome& OutChosenOutcome);

    UFUNCTION(BlueprintCallable, Category = "RandomGame")
    float ComputeExpectedValue(float Stake) const;

protected:
    virtual void BeginPlay() override;

private:
    float GetTotalWeight() const;
};
