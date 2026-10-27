#include "ChickenGamesPlayerPropModPreview.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

AChickenGamesPlayerPropModPreview::AChickenGamesPlayerPropModPreview()
{
    PreviewRoot=CreateDefaultSubobject<USceneComponent>(TEXT("PreviewRoot")); RootComponent=PreviewRoot;
    PlayerCamera=CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera")); PlayerCamera->SetupAttachment(PreviewRoot); PlayerCamera->SetRelativeLocation(FVector(0,0,60));
    PropSlot=CreateDefaultSubobject<USceneComponent>(TEXT("PropSlot")); PropSlot->SetupAttachment(PlayerCamera); PropSlot->SetRelativeLocation(FVector(65,27,-29));
    auto MakeProp=[this](const TCHAR* RootName,const TCHAR* MeshName)
    {
        USceneComponent* PropRoot=CreateDefaultSubobject<USceneComponent>(FName(RootName)); PropRoot->SetupAttachment(PropSlot);
        UStaticMeshComponent* Mesh=CreateDefaultSubobject<UStaticMeshComponent>(FName(MeshName)); Mesh->SetupAttachment(PropRoot); Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); return Mesh;
    };
    SingingMesh=MakeProp(TEXT("SingingActorRoot"),TEXT("SingingMesh"));
    DanceMesh=MakeProp(TEXT("DanceActorRoot"),TEXT("DanceMesh"));
    RapMesh=MakeProp(TEXT("RapActorRoot"),TEXT("RapMesh"));
    BasketballMesh=MakeProp(TEXT("BasketballActorRoot"),TEXT("BasketballMesh"));
    MusicMesh=MakeProp(TEXT("MusicActorRoot"),TEXT("MusicMesh"));
}

FChickenGamesHeldPropPlacement AChickenGamesPlayerPropModPreview::ReadProp(UStaticMeshComponent* C) const
{
    FChickenGamesHeldPropPlacement P; if(!C||!C->GetStaticMesh()) return P; const FTransform T=C->GetRelativeTransform();
    P.bUseCustomProp=true; P.Mesh=C->GetStaticMesh(); P.MeshLocation=T.GetLocation(); P.MeshRotation=T.Rotator(); P.MeshScale=T.GetScale3D(); return P;
}

void AChickenGamesPlayerPropModPreview::WritePlayerPropProfile()
{
#if WITH_EDITOR
    if(!TargetProfile) return; TargetProfile->Modify();
    auto PreserveSound=[](FChickenGamesHeldPropPlacement NewValue,const FChickenGamesHeldPropPlacement& OldValue)
    { NewValue.Sound=OldValue.Sound; return NewValue; };
    TargetProfile->Singing=PreserveSound(ReadProp(SingingMesh),TargetProfile->Singing);
    TargetProfile->Dance=PreserveSound(ReadProp(DanceMesh),TargetProfile->Dance);
    TargetProfile->Rap=PreserveSound(ReadProp(RapMesh),TargetProfile->Rap);
    TargetProfile->Basketball=PreserveSound(ReadProp(BasketballMesh),TargetProfile->Basketball);
    TargetProfile->Music=PreserveSound(ReadProp(MusicMesh),TargetProfile->Music);
    TargetProfile->MarkPackageDirty();
#endif
}
