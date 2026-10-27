#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Animation/AnimMontage.h"
#include "Engine/Texture2D.h"
#include "Sound/SoundBase.h"
#include "ChickenGamesCustomSymptomPack.generated.h"

UENUM(BlueprintType)
enum class EChickenGamesSymptomBehavior : uint8
{
    Invalid = 0 UMETA(Hidden),
    Singing = 1 UMETA(DisplayName="Singing"),
    Dancing = 2 UMETA(DisplayName="Dancing"),
    Rap = 3 UMETA(DisplayName="Rap"),
    Basketball = 4 UMETA(DisplayName="Basketball")
};

UENUM(BlueprintType)
enum class EChickenGamesSymptomSeverity : uint8
{
    Normal = 0 UMETA(DisplayName="Normal"),
    Suspected = 1 UMETA(DisplayName="Suspected"),
    Infection = 2 UMETA(DisplayName="Infected")
};

USTRUCT(BlueprintType)
struct STEAMMOD_API FChickenGamesCustomSymptom
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Symptom") FName SymptomId;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Symptom") FText Name;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Symptom", meta=(MultiLine=true)) FText Description;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Symptom") TSoftObjectPtr<UTexture2D> Icon;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Symptom") EChickenGamesSymptomBehavior Behavior = EChickenGamesSymptomBehavior::Singing;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Symptom") EChickenGamesSymptomSeverity Severity = EChickenGamesSymptomSeverity::Normal;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Symptom") TArray<TSoftObjectPtr<UAnimMontage>> Montages;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Symptom") TSoftObjectPtr<USoundBase> Sound;
};

UCLASS(BlueprintType, meta=(DisplayName="ChickenGames Custom Symptom Pack"))
class STEAMMOD_API UChickenGamesCustomSymptomPack : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Custom Symptoms")
    TArray<FChickenGamesCustomSymptom> Symptoms;
};
