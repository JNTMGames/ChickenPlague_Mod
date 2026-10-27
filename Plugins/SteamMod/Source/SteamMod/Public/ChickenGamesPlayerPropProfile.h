#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/StaticMesh.h"
#include "Sound/SoundBase.h"
#include "ChickenGamesPlayerPropProfile.generated.h"

USTRUCT(BlueprintType)
struct STEAMMOD_API FChickenGamesHeldPropPlacement
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadOnly) bool bUseCustomProp = false;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<UStaticMesh> Mesh;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<USoundBase> Sound;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FVector MeshLocation = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FRotator MeshRotation = FRotator::ZeroRotator;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FVector MeshScale = FVector::OneVector;
};

UCLASS(BlueprintType, meta=(DisplayName="ChickenGames Player Prop Profile"))
class STEAMMOD_API UChickenGamesPlayerPropProfile : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Player Props") FChickenGamesHeldPropPlacement Singing;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Player Props") FChickenGamesHeldPropPlacement Dance;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Player Props") FChickenGamesHeldPropPlacement Rap;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Player Props") FChickenGamesHeldPropPlacement Basketball;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Player Props") FChickenGamesHeldPropPlacement Music;
};
