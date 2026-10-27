#include "SteamModLibrary.h"
#include "SteamModSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "Sound/SoundBase.h"
#include "Modules/ModuleManager.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DecalComponent.h"
IMPLEMENT_MODULE(FDefaultModuleImpl, SteamMod)

bool USteamModLibrary::AttachAppearanceComponentWithModFallback(
    USceneComponent* TargetComponent,
    USkeletalMeshComponent* CharacterMesh,
    bool bHasModProfile,
    const FChickenGamesAttachmentPlacement& ModPlacement,
    FName DefaultSocketName,
    bool& bUsedModPlacement,
    FString& Message)
{
    bUsedModPlacement = false;
    if (!TargetComponent || !CharacterMesh)
    {
        Message = TEXT("Target component or character mesh is missing.");
        return false;
    }

    const bool bCanUseMod = bHasModProfile && ModPlacement.bUseCustomPlacement &&
        !ModPlacement.AttachBone.IsNone() && CharacterMesh->DoesSocketExist(ModPlacement.AttachBone);
    if (bCanUseMod)
    {
        const FAttachmentTransformRules Rules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, false);
        if (!TargetComponent->AttachToComponent(CharacterMesh, Rules, ModPlacement.AttachBone))
        {
            Message = TEXT("Failed to attach component to MOD bone: ") + ModPlacement.AttachBone.ToString();
            return false;
        }
        TargetComponent->SetRelativeTransform(FTransform(ModPlacement.Rotation, ModPlacement.Location, ModPlacement.Scale));
        if (!ModPlacement.Mesh.IsNull())
        {
            if (UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(TargetComponent))
            {
                if (UStaticMesh* Mesh = ModPlacement.Mesh.LoadSynchronous()) StaticMeshComponent->SetStaticMesh(Mesh);
            }
        }
        bUsedModPlacement = true;
        Message = TEXT("Attached using MOD bone and relative transform.");
        return true;
    }

    if (DefaultSocketName.IsNone() || !CharacterMesh->DoesSocketExist(DefaultSocketName))
    {
        Message = TEXT("MOD placement is unavailable and default socket does not exist: ") + DefaultSocketName.ToString();
        return false;
    }
    const FAttachmentTransformRules Rules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false);
    if (!TargetComponent->AttachToComponent(CharacterMesh, Rules, DefaultSocketName))
    {
        Message = TEXT("Failed to attach component to default socket: ") + DefaultSocketName.ToString();
        return false;
    }
    Message = bHasModProfile ? TEXT("Invalid or disabled MOD placement; used default socket.") : TEXT("No MOD profile; used default socket.");
    return true;
}


bool USteamModLibrary::ApplyHeldPropWithModFallback(
    USceneComponent* PropMeshComponent,
    bool bHasModProfile,
    const FChickenGamesHeldPropPlacement& ModPlacement,
    bool& bUsedModProp,
    FString& Message)
{
    bUsedModProp = false;
    UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(PropMeshComponent);
    if (!StaticMeshComponent)
    {
        Message = TEXT("Held prop component must be a Static Mesh Component.");
        return false;
    }
    if (!bHasModProfile || !ModPlacement.bUseCustomProp || ModPlacement.Mesh.IsNull())
    {
        Message = TEXT("No custom held prop in MOD profile; kept the game's authored mesh and transform.");
        return true;
    }
    UStaticMesh* Mesh = ModPlacement.Mesh.LoadSynchronous();
    if (!Mesh)
    {
        Message = TEXT("Unable to load held prop mesh from MOD profile.");
        return false;
    }
    StaticMeshComponent->SetStaticMesh(Mesh);
    StaticMeshComponent->SetRelativeTransform(FTransform(ModPlacement.MeshRotation, ModPlacement.MeshLocation, ModPlacement.MeshScale));
    bUsedModProp = true;
    Message = TEXT("Applied MOD held prop mesh and internal relative transform.");
    return true;
}

USoundBase* USteamModLibrary::GetHeldPropSoundWithModFallback(
    USoundBase* DefaultSound,
    bool bHasModProfile,
    const FChickenGamesHeldPropPlacement& ModPlacement,
    bool& bUsedModSound)
{
    bUsedModSound=false;
    if(bHasModProfile && !ModPlacement.Sound.IsNull())
    {
        if(USoundBase* Sound=ModPlacement.Sound.LoadSynchronous())
        {
            bUsedModSound=true;
            return Sound;
        }
    }
    return DefaultSound;
}

bool USteamModLibrary::AttachDecalWithModFallback(
    UDecalComponent* DecalComponent,
    USkeletalMeshComponent* CharacterMesh,
    bool bHasModProfile,
    const FChickenGamesDecalPlacement& ModPlacement,
    FName DefaultSocketName,
    bool& bUsedModPlacement,
    FString& Message)
{
    bUsedModPlacement = false;
    if (!DecalComponent || !CharacterMesh)
    {
        Message = TEXT("Decal component or character mesh is missing.");
        return false;
    }
    const bool bCanUseMod = bHasModProfile && ModPlacement.bUseCustomPlacement &&
        !ModPlacement.AttachBone.IsNone() && CharacterMesh->DoesSocketExist(ModPlacement.AttachBone);
    if (bCanUseMod)
    {
        const FAttachmentTransformRules Rules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, false);
        if (!DecalComponent->AttachToComponent(CharacterMesh, Rules, ModPlacement.AttachBone)) return false;
        DecalComponent->SetRelativeLocation(ModPlacement.Location);
        DecalComponent->SetRelativeRotation(ModPlacement.Rotation);
        DecalComponent->DecalSize = ModPlacement.DecalSize;
        bUsedModPlacement = true;
        Message = TEXT("Attached decal using MOD bone, transform and size.");
        return true;
    }
    if (DefaultSocketName.IsNone() || !CharacterMesh->DoesSocketExist(DefaultSocketName))
    {
        Message = TEXT("MOD decal placement is unavailable and default socket does not exist: ") + DefaultSocketName.ToString();
        return false;
    }
    const FAttachmentTransformRules Rules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false);
    if (!DecalComponent->AttachToComponent(CharacterMesh, Rules, DefaultSocketName)) return false;
    Message = bHasModProfile ? TEXT("Invalid or disabled MOD decal placement; used default socket.") : TEXT("No MOD profile; used default decal socket.");
    return true;
}

