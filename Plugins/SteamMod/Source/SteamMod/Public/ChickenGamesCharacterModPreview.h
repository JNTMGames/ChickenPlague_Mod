#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChickenGamesAppearanceProfile.h"
#include "ChickenGamesCharacterModPreview.generated.h"

class USkeletalMeshComponent;
class UStaticMeshComponent;
class UDecalComponent;
class USceneComponent;

UCLASS(Blueprintable, PrioritizeCategories=("MOD Tools"), meta=(DisplayName="ChickenGames Character MOD Preview"))
class STEAMMOD_API AChickenGamesCharacterModPreview : public AActor
{
    GENERATED_BODY()
public:
    AChickenGamesCharacterModPreview();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Preview") TObjectPtr<USkeletalMeshComponent> CharacterMesh;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Preview") TObjectPtr<UStaticMeshComponent> MediterraneanHair;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Preview") TObjectPtr<UStaticMeshComponent> Cockscomb;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Preview") TObjectPtr<UStaticMeshComponent> BasketballHead;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Preview") TObjectPtr<UStaticMeshComponent> Microphone;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Preview") TObjectPtr<UStaticMeshComponent> RapSunglasses;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Preview") TObjectPtr<UStaticMeshComponent> RapGoldNecklace;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Preview") TObjectPtr<UStaticMeshComponent> RapHat;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Preview") TObjectPtr<UStaticMeshComponent> Basketball;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Preview") TObjectPtr<UStaticMeshComponent> GuardGun;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Preview") TObjectPtr<UStaticMeshComponent> LayEgg;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Preview") TArray<TObjectPtr<UStaticMeshComponent>> FeatherComponents;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Preview") TArray<TObjectPtr<UDecalComponent>> PawPrintDecals;


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MOD Tools", meta=(DisplayName="Target Profile（写入目标）")) TObjectPtr<UChickenGamesCharacterAppearanceProfile> TargetProfile;
    UFUNCTION(CallInEditor, BlueprintCallable, Category="MOD Tools", meta=(DisplayName="写入 Appearance Profile"))
    void WriteAppearanceProfile();

private:
    FChickenGamesAttachmentPlacement ReadAttachment(USceneComponent* Component) const;
    FChickenGamesDecalPlacement ReadDecal(UDecalComponent* Component) const;
};

