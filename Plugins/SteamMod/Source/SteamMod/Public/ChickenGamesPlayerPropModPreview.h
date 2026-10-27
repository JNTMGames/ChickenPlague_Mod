#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChickenGamesPlayerPropProfile.h"
#include "ChickenGamesPlayerPropModPreview.generated.h"

class UCameraComponent;
class USceneComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable, PrioritizeCategories=("MOD Tools"), meta=(DisplayName="ChickenGames Player Prop MOD Preview"))
class STEAMMOD_API AChickenGamesPlayerPropModPreview : public AActor
{
    GENERATED_BODY()
public:
    AChickenGamesPlayerPropModPreview();
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Preview") TObjectPtr<USceneComponent> PreviewRoot;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Preview") TObjectPtr<UCameraComponent> PlayerCamera;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Preview") TObjectPtr<USceneComponent> PropSlot;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Player Props") TObjectPtr<UStaticMeshComponent> SingingMesh;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Player Props") TObjectPtr<UStaticMeshComponent> DanceMesh;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Player Props") TObjectPtr<UStaticMeshComponent> RapMesh;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Player Props") TObjectPtr<UStaticMeshComponent> BasketballMesh;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Player Props") TObjectPtr<UStaticMeshComponent> MusicMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MOD Tools", meta=(DisplayName="Player Prop Profile（写入目标）"))
    TObjectPtr<UChickenGamesPlayerPropProfile> TargetProfile;
    UFUNCTION(CallInEditor, BlueprintCallable, Category="MOD Tools", meta=(DisplayName="写入 Player Prop Profile"))
    void WritePlayerPropProfile();
private:
    FChickenGamesHeldPropPlacement ReadProp(UStaticMeshComponent* Component) const;
};
