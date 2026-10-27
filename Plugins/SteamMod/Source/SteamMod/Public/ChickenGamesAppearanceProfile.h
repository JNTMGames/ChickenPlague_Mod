#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "ChickenGamesAppearanceProfile.generated.h"

USTRUCT(BlueprintType)
struct STEAMMOD_API FChickenGamesAttachmentPlacement
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName SourceSocket = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<UStaticMesh> Mesh;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) bool bUseCustomPlacement = false;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FName AttachBone = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FVector Location = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FRotator Rotation = FRotator::ZeroRotator;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FVector Scale = FVector::OneVector;
};

USTRUCT(BlueprintType)
struct STEAMMOD_API FChickenGamesDecalPlacement
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName SourceSocket = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) bool bUseCustomPlacement = false;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FName AttachBone = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FVector Location = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FRotator Rotation = FRotator::ZeroRotator;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.01")) FVector DecalSize = FVector(32.0, 64.0, 64.0);
};



UCLASS(BlueprintType, meta=(DisplayName="ChickenGames Character Appearance Profile"))
class STEAMMOD_API UChickenGamesCharacterAppearanceProfile : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UChickenGamesCharacterAppearanceProfile()
    {
        MediterraneanHair.SourceSocket = TEXT("MediterraneanSeaHairSlot");
        Cockscomb.SourceSocket = TEXT("HavingCockscombOnTheHeadSlot");
        BasketballHead.SourceSocket = TEXT("BasketballHeadSlot");
        Microphone.SourceSocket = TEXT("MicrophoneSlot");
        RapSunglasses.SourceSocket = TEXT("Rap_SunglassSlot");
        RapGoldNecklace.SourceSocket = TEXT("Rap_NickSlot");
        RapHat.SourceSocket = TEXT("Rap_HatSlot");
        Basketball.SourceSocket = TEXT("BasketballSlot");
        GuardGun.SourceSocket = TEXT("GunSlot");
        LayEgg.SourceSocket = TEXT("LayEggsSlot");
        for (int32 Index = 0; Index < 5; ++Index) { FChickenGamesDecalPlacement P; P.SourceSocket = FName(*FString::Printf(TEXT("ChickenPawPrintsSlot_%d"), Index)); ChickenPawPrintDecals.Add(P); }
        for (int32 Index = 0; Index < 3; ++Index) { FChickenGamesAttachmentPlacement P; P.SourceSocket = FName(*FString::Printf(TEXT("FeatherOnBodySlot_%d"), Index)); FeatherOnBody.Add(P); }
    }
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character") TSoftObjectPtr<USkeletalMesh> CharacterMesh;
    // Serialized compatibility only; hidden from Profile authoring and preview tools.
    UPROPERTY(BlueprintReadOnly, Category="Deprecated", meta=(DeprecatedProperty, DeprecationMessage="Face material slots are no longer supported."))
    TArray<int32> FaceMaterialSlotIndices;
    // Material element index arrays on the replacement Skeletal Mesh. Empty means not configured.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Material Slot Indices", meta=(ClampMin="-1")) TArray<int32> HeadMaterialSlotIndices;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Material Slot Indices", meta=(ClampMin="-1")) TArray<int32> BodyMaterialSlotIndices;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Material Slot Indices", meta=(ClampMin="-1")) TArray<int32> HairMaterialSlotIndices;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Symptom Placement") FChickenGamesAttachmentPlacement MediterraneanHair;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Symptom Placement") TArray<FChickenGamesDecalPlacement> ChickenPawPrintDecals;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Symptom Placement") FChickenGamesAttachmentPlacement Cockscomb;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Symptom Placement") TArray<FChickenGamesAttachmentPlacement> FeatherOnBody;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Symptom Placement") FChickenGamesAttachmentPlacement BasketballHead;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Prop Placement") FChickenGamesAttachmentPlacement Microphone;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Prop Placement") FChickenGamesAttachmentPlacement RapSunglasses;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Prop Placement") FChickenGamesAttachmentPlacement RapGoldNecklace;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Prop Placement") FChickenGamesAttachmentPlacement RapHat;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Prop Placement") FChickenGamesAttachmentPlacement Basketball;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Prop Placement") FChickenGamesAttachmentPlacement GuardGun;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Prop Placement") FChickenGamesAttachmentPlacement LayEgg;

};
