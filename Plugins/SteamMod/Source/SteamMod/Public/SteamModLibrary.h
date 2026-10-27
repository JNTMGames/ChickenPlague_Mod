#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ChickenGamesAppearanceProfile.h"
#include "ChickenGamesPlayerPropProfile.h"
#include "SteamModLibrary.generated.h"

class USoundBase;
class USceneComponent;
class USkeletalMeshComponent;
class UDecalComponent;

UCLASS()
class STEAMMOD_API USteamModLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:
    // Uses a MOD bone plus saved relative transform when valid; otherwise falls back to the game's authored socket.
    UFUNCTION(BlueprintCallable, Category="ChickenGames|Appearance",
        meta=(DisplayName="Attach Appearance Component With Mod Fallback"))
    static bool AttachAppearanceComponentWithModFallback(
        USceneComponent* TargetComponent,
        USkeletalMeshComponent* CharacterMesh,
        bool bHasModProfile,
        const FChickenGamesAttachmentPlacement& ModPlacement,
        FName DefaultSocketName,
        bool& bUsedModPlacement,
        FString& Message);

    UFUNCTION(BlueprintCallable, Category="ChickenGames|Appearance",
        meta=(DisplayName="Apply Held Prop With Mod Fallback"))
    static bool ApplyHeldPropWithModFallback(
        USceneComponent* PropMeshComponent,
        bool bHasModProfile,
        const FChickenGamesHeldPropPlacement& ModPlacement,
        bool& bUsedModProp,
        FString& Message);

    UFUNCTION(BlueprintPure, Category="ChickenGames|Appearance",
        meta=(DisplayName="Get Held Prop Sound With Mod Fallback"))
    static USoundBase* GetHeldPropSoundWithModFallback(
        USoundBase* DefaultSound,
        bool bHasModProfile,
        const FChickenGamesHeldPropPlacement& ModPlacement,
        bool& bUsedModSound);

    UFUNCTION(BlueprintCallable, Category="ChickenGames|Appearance",
        meta=(DisplayName="Attach Decal With Mod Fallback"))
    static bool AttachDecalWithModFallback(
        UDecalComponent* DecalComponent,
        USkeletalMeshComponent* CharacterMesh,
        bool bHasModProfile,
        const FChickenGamesDecalPlacement& ModPlacement,
        FName DefaultSocketName,
        bool& bUsedModPlacement,
        FString& Message);
};
