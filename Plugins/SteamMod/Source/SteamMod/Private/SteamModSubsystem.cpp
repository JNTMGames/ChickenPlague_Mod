#include "SteamModSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "ChickenGamesModSlotTable.h"
#include "ChickenGamesCustomSymptomPack.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFileManager.h"
#include "IPlatformFilePak.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/PackageName.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UnrealType.h"

namespace
{
    // Mounts and loaded packages live for the process lifetime. Do not unmount beneath live UObjects.
    TMap<FString, FString> MountedRoots;
    bool IsId(const FString& S)
    {
        if (S.IsEmpty() || S.Len() > 96) return false;
        for (TCHAR C : S) if (!((C >= 'a' && C <= 'z') || (C >= '0' && C <= '9') || C == '_')) return false;
        return true;
    }
    bool IsRelativeFile(const FString& P)
    {
        return !P.IsEmpty() && FPaths::IsRelative(P) && !P.Contains(TEXT("..")) && !P.Contains(TEXT(":")) && !P.Contains(TEXT("\\"));
    }
    FString FullPath(const FString& P)
    {
        FString R = FPaths::ConvertRelativePathToFull(FPaths::IsRelative(P) ? FPaths::Combine(FPaths::ProjectDir(), P) : P);
        FPaths::NormalizeFilename(R);
        FPaths::CollapseRelativeDirectories(R);
        return R;
    }

    FString SlotPart(FString Value)
    {
        Value.TrimStartAndEndInline();
        Value.ToLowerInline();
        for (TCHAR& C : Value)
        {
            const bool bAllowed = (C >= 'a' && C <= 'z') || (C >= '0' && C <= '9') || C == '_';
            if (!bAllowed) C = '_';
        }
        while (Value.Contains(TEXT("__"))) Value.ReplaceInline(TEXT("__"), TEXT("_"));
        Value.RemoveFromStart(TEXT("_"));
        Value.RemoveFromEnd(TEXT("_"));
        return Value;
    }

    bool SetObjectProperty(FProperty* Property, void* Container, UObject* Resource)
    {
        if (FTextProperty* TextProperty = CastField<FTextProperty>(Property))
        {
            const UChickenGamesModTextAsset* TextAsset = Cast<UChickenGamesModTextAsset>(Resource);
            if (!TextAsset) return false;
            TextProperty->SetPropertyValue_InContainer(Container, TextAsset->Text);
            return true;
        }
        if (FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(Property))
        {
            if (!Resource->IsA(ObjectProperty->PropertyClass)) return false;
            ObjectProperty->SetObjectPropertyValue_InContainer(Container, Resource);
            return true;
        }
        if (FSoftObjectProperty* SoftProperty = CastField<FSoftObjectProperty>(Property))
        {
            if (!Resource->IsA(SoftProperty->PropertyClass)) return false;
            *SoftProperty->GetPropertyValuePtr_InContainer(Container) = FSoftObjectPtr(Resource);
            return true;
        }
        FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property);
        if (!ArrayProperty) return false;
        FScriptArrayHelper Helper(ArrayProperty, ArrayProperty->ContainerPtrToValuePtr<void>(Container));
        if (FObjectPropertyBase* InnerObject = CastField<FObjectPropertyBase>(ArrayProperty->Inner))
        {
            if (!Resource->IsA(InnerObject->PropertyClass)) return false;
            Helper.EmptyValues();
            const int32 Index = Helper.AddValue();
            InnerObject->SetObjectPropertyValue(Helper.GetRawPtr(Index), Resource);
            return true;
        }
        if (FSoftObjectProperty* InnerSoft = CastField<FSoftObjectProperty>(ArrayProperty->Inner))
        {
            if (!Resource->IsA(InnerSoft->PropertyClass)) return false;
            Helper.EmptyValues();
            const int32 Index = Helper.AddValue();
            *InnerSoft->GetPropertyValuePtr(Helper.GetRawPtr(Index)) = FSoftObjectPtr(Resource);
            return true;
        }
        return false;
    }

    FProperty* FindAuthoredProperty(const UScriptStruct* Struct, const TCHAR* Name)
    {
        for(TFieldIterator<FProperty> It(Struct);It;++It)
        {
            FString Candidate=It->GetAuthoredName();
            if(Candidate.IsEmpty()) Candidate=It->GetName();
            if(Candidate.Equals(Name,ESearchCase::IgnoreCase)) return *It;
        }
        return nullptr;
    }

    bool SetEnumNumeric(FProperty* Property,void* Container,int64 Value)
    {
        if(FEnumProperty* Enum=CastField<FEnumProperty>(Property))
        {
            void* Ptr=Enum->ContainerPtrToValuePtr<void>(Container);
            Enum->GetUnderlyingProperty()->SetIntPropertyValue(Ptr,Value); return true;
        }
        if(FByteProperty* Byte=CastField<FByteProperty>(Property))
        { Byte->SetPropertyValue_InContainer(Container,(uint8)Value); return true; }
        return false;
    }
}

void USteamModSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    bModsReady = false;
    ModLoadReport.Empty();
#if WITH_EDITOR
    ModLoadReport = TEXT("External cooked MODs are not mounted in editor processes.");
    bModsReady = true;
#else
    int32 LoadedCount = 0;
    int32 FailedCount = 0;
    FString SlotMessage;
    UDataTable* SlotTable = LoadObject<UDataTable>(nullptr,
        TEXT("/Game/ModKitReference/DT_ModSlots.DT_ModSlots"));
    if (!SlotTable || !RegisterModSlotsFromDataTable(SlotTable, SlotMessage))
    {
        UE_LOG(LogTemp, Warning, TEXT("ChickenGamesMods: automatic slot registration failed: %s"), *SlotMessage);
    }
    else
    {
        UE_LOG(LogTemp, Display, TEXT("ChickenGamesMods: %s"), *SlotMessage);
    }

    // ProjectDir/Mods is stable in packaged builds. Each child directory may
    // contain one manifest.json and resources.pak pair.
    const TArray<FString> Manifests = FindModManifests(TEXT("Mods"));
    for (const FString& Manifest : Manifests)
    {
        FString LoadMessage;
        if (LoadModManifest(Manifest, true, LoadMessage)) ++LoadedCount;
        else ++FailedCount;
    }
    ModLoadReport = FString::Printf(TEXT("Automatic MOD loading finished: %d loaded, %d failed, %d maps available."),
        LoadedCount, FailedCount, ModMaps.Num());
    UE_LOG(LogTemp, Display, TEXT("ChickenGamesMods: %s"), *ModLoadReport);
    bModsReady = true;
#endif
}

void USteamModSubsystem::Deinitialize()
{
    bModsReady = false;
    SlotTypes.Empty();
    Resources.Empty();
    Owners.Empty();
    LoadedIds.Empty();
    ModMaps.Empty();
    Super::Deinitialize();
}

bool USteamModSubsystem::RegisterModSlot(FName Slot, TSubclassOf<UObject> ExpectedClass)
{
    if (!IsInGameThread() || Slot.IsNone() || !ExpectedClass) return false;
    if (const auto* Existing = SlotTypes.Find(Slot)) return Existing->Get() == ExpectedClass.Get();
    SlotTypes.Add(Slot, ExpectedClass);
    return true;
}

bool USteamModSubsystem::RegisterModSlotsFromDataTable(UDataTable* SlotTable, FString& Message)
{
    if (!IsInGameThread() || !SlotTable)
    {
        Message = TEXT("Slot DataTable is missing.");
        return false;
    }
    if (SlotTable->GetRowStruct() != FChickenGamesModSlotRow::StaticStruct())
    {
        Message = TEXT("DataTable must use ChickenGamesModSlotRow.");
        return false;
    }

    TArray<FChickenGamesModSlotRow*> Rows;
    SlotTable->GetAllRows(TEXT("RegisterModSlotsFromDataTable"), Rows);
    TSet<FName> Seen;
    int32 Registered = 0;
    for (const FChickenGamesModSlotRow* Row : Rows)
    {
        if (!Row || Row->SlotId.IsNone() || Seen.Contains(Row->SlotId))
        {
            Message = TEXT("Slot DataTable contains an empty or duplicate SlotId.");
            return false;
        }
        Seen.Add(Row->SlotId);
        UClass* ExpectedClass = Row->AllowedClass.LoadSynchronous();
        if (!ExpectedClass || !ExpectedClass->IsChildOf(UObject::StaticClass()) || !RegisterModSlot(Row->SlotId, ExpectedClass))
        {
            Message = TEXT("Invalid AllowedClass or conflicting SlotId: ") + Row->SlotId.ToString();
            return false;
        }
        ++Registered;
    }
    Message = FString::Printf(TEXT("Registered %d MOD slots."), Registered);
    return Registered > 0;
}

UObject* USteamModSubsystem::GetModResource(FName Slot) const
{
    const auto* Found = Resources.Find(Slot);
    return Found ? Found->Get() : nullptr;
}

FText USteamModSubsystem::GetModText(FName Slot, const FText& DefaultText) const
{
    const UChickenGamesModTextAsset* TextAsset = Cast<UChickenGamesModTextAsset>(GetModResource(Slot));
    return TextAsset ? TextAsset->Text : DefaultText;
}

FString USteamModSubsystem::GetModResourceOwner(FName Slot) const
{
    const auto* Found = Owners.Find(Slot);
    return Found ? *Found : FString();
}

TArray<FString> USteamModSubsystem::FindModManifests(const FString& Directory) const
{
    TArray<FString> Result;
    IFileManager::Get().FindFilesRecursive(Result, *FullPath(Directory), TEXT("manifest.json"), true, false);
    Result.Sort();
    return Result;
}

bool USteamModSubsystem::ApplyLoadedModsToDataTable(UDataTable* DataTable, const FString& SlotPrefix,
    int32& AppliedCount, FString& Message)
{
    AppliedCount = 0;
    if (!IsInGameThread() || !DataTable || !DataTable->GetRowStruct())
    {
        Message = TEXT("DataTable is missing or invalid.");
        return false;
    }
    const FString Prefix = SlotPart(SlotPrefix);
    if (Prefix.IsEmpty())
    {
        Message = TEXT("SlotPrefix is empty.");
        return false;
    }

    const UScriptStruct* RowStruct = DataTable->GetRowStruct();
    for (const TPair<FName, uint8*>& RowPair : DataTable->GetRowMap())
    {
        if (!RowPair.Value) continue;
        const FString RowPart = SlotPart(RowPair.Key.ToString());
        for (TFieldIterator<FProperty> It(RowStruct); It; ++It)
        {
            FProperty* Property = *It;
            FString AuthoredPropertyName = Property->GetAuthoredName();
            if (AuthoredPropertyName.IsEmpty()) AuthoredPropertyName = Property->GetName();
            const FString PropertyPart = SlotPart(AuthoredPropertyName);
            if (RowPart.IsEmpty() || PropertyPart.IsEmpty()) continue;
            const FName Slot(*(Prefix + TEXT(".") + RowPart + TEXT(".") + PropertyPart));
            const TObjectPtr<UObject>* Found = Resources.Find(Slot);
            if (!Found || !Found->Get()) continue;
            if (SetObjectProperty(Property, RowPair.Value, Found->Get()))
            {
                ++AppliedCount;
                UE_LOG(LogTemp, Display, TEXT("ChickenGamesMods: applied slot %s to row %s field %s (internal: %s)."),
                    *Slot.ToString(), *RowPair.Key.ToString(), *AuthoredPropertyName, *Property->GetName());
            }
            else UE_LOG(LogTemp, Warning, TEXT("ChickenGamesMods: slot %s cannot replace field %s (internal: %s) due to type mismatch."),
                *Slot.ToString(), *AuthoredPropertyName, *Property->GetName());
        }
    }
    Message = FString::Printf(TEXT("Applied %d MOD DataTable field overrides using prefix '%s'."), AppliedCount, *Prefix);
    UE_LOG(LogTemp, Display, TEXT("ChickenGamesMods: %s"), *Message);
    return true;
}

TArray<FString> USteamModSubsystem::GetModMapIds() const
{
    TArray<FString> Result;
    ModMaps.GetKeys(Result);
    Result.Sort();
    return Result;
}

bool USteamModSubsystem::GetModMapPath(const FString& ModId, FString& MapPath) const
{
    if (const FChickenGamesModMapInfo* Found = ModMaps.Find(ModId))
    {
        MapPath = Found->MapPath;
        return true;
    }
    MapPath.Empty();
    return false;
}

TArray<FChickenGamesModMapInfo> USteamModSubsystem::GetModMaps() const
{
    TArray<FChickenGamesModMapInfo> Result;
    ModMaps.GenerateValueArray(Result);
    Result.Sort([](const FChickenGamesModMapInfo& A,const FChickenGamesModMapInfo& B)
    { return A.DisplayName.ToString()==B.DisplayName.ToString()?A.ModId<B.ModId:A.DisplayName.ToString()<B.DisplayName.ToString(); });
    return Result;
}

bool USteamModSubsystem::AppendCustomSymptomsToDataTable(UDataTable* DataTable,UChickenGamesCustomSymptomPack* Pack,
    const FString& ModNamespace,int32& AddedCount,FString& Message)
{
    AddedCount=0;
    if(!DataTable||!DataTable->GetRowStruct()||!Pack){Message=TEXT("DataTable or symptom pack is missing.");return false;}
    const FString SafeNamespace=SlotPart(ModNamespace);
    if(SafeNamespace.IsEmpty()){Message=TEXT("MOD namespace is invalid.");return false;}
    const UScriptStruct* Struct=DataTable->GetRowStruct();
    FProperty* NameP=FindAuthoredProperty(Struct,TEXT("Name"));
    FProperty* DescriptionP=FindAuthoredProperty(Struct,TEXT("Description"));
    FProperty* ImageP=FindAuthoredProperty(Struct,TEXT("Image"));
    FProperty* ClassP=FindAuthoredProperty(Struct,TEXT("SymptomClass"));
    FProperty* TypeP=FindAuthoredProperty(Struct,TEXT("SymptomType"));
    FProperty* MontageP=FindAuthoredProperty(Struct,TEXT("Montage"));
    FProperty* SoundP=FindAuthoredProperty(Struct,TEXT("Sound"));
    if(!NameP||!DescriptionP||!ClassP||!TypeP||!MontageP||!SoundP)
    {Message=TEXT("Symptom DataTable row structure does not contain the required fields.");return false;}
    TSet<FName> LocalIds;
    for(const FChickenGamesCustomSymptom& Entry:Pack->Symptoms)
    {
        const FString Local=SlotPart(Entry.SymptomId.ToString());
        if(Local.IsEmpty()||Entry.Name.IsEmpty()||Entry.Montages.IsEmpty()||LocalIds.Contains(FName(*Local))) continue;
        LocalIds.Add(FName(*Local));
        const FName RowName(*(TEXT("mod_")+SafeNamespace+TEXT("_")+Local));
        if(DataTable->GetRowMap().Contains(RowName)) continue;
        uint8* Memory=(uint8*)FMemory::Malloc(Struct->GetStructureSize(),Struct->GetMinAlignment());
        Struct->InitializeStruct(Memory);
        if(FTextProperty* P=CastField<FTextProperty>(NameP)) P->SetPropertyValue_InContainer(Memory,Entry.Name);
        if(FTextProperty* P=CastField<FTextProperty>(DescriptionP)) P->SetPropertyValue_InContainer(Memory,Entry.Description);
        if(ImageP&&!Entry.Icon.IsNull()) if(UTexture2D* Icon=Entry.Icon.LoadSynchronous()) SetObjectProperty(ImageP,Memory,Icon);
        SetEnumNumeric(ClassP,Memory,(int64)Entry.Behavior);
        SetEnumNumeric(TypeP,Memory,(int64)Entry.Severity);
        if(FArrayProperty* Array=CastField<FArrayProperty>(MontageP))
        {
            FScriptArrayHelper Helper(Array,Array->ContainerPtrToValuePtr<void>(Memory));
            for(const TSoftObjectPtr<UAnimMontage>& Soft:Entry.Montages) if(UAnimMontage* Montage=Soft.LoadSynchronous())
            {
                const int32 Index=Helper.AddValue();
                if(FObjectPropertyBase* Inner=CastField<FObjectPropertyBase>(Array->Inner)) Inner->SetObjectPropertyValue(Helper.GetRawPtr(Index),Montage);
                else if(FSoftObjectProperty* InnerSoft=CastField<FSoftObjectProperty>(Array->Inner)) *InnerSoft->GetPropertyValuePtr(Helper.GetRawPtr(Index))=FSoftObjectPtr(Montage);
            }
        }
        if(!Entry.Sound.IsNull()) if(USoundBase* Sound=Entry.Sound.LoadSynchronous()) SetObjectProperty(SoundP,Memory,Sound);
        DataTable->AddRow(RowName,*reinterpret_cast<FTableRowBase*>(Memory));
        Struct->DestroyStruct(Memory); FMemory::Free(Memory); ++AddedCount;
    }
    Message=FString::Printf(TEXT("Added %d custom symptoms from namespace '%s'."),AddedCount,*SafeNamespace);
    return true;
}

bool USteamModSubsystem::OpenModMap(const FString& ModId, FString& Message)
{
    FString MapPath;
    if (!GetModMapPath(ModId, MapPath))
    {
        Message = TEXT("Map MOD is not loaded: ") + ModId;
        return false;
    }
    if (!GetWorld())
    {
        Message = TEXT("No game world is available.");
        return false;
    }
    Message = TEXT("Opening map: ") + MapPath;
    UGameplayStatics::OpenLevel(GetWorld(), FName(*MapPath));
    return true;
}

bool USteamModSubsystem::LoadModManifest(const FString& ManifestPath, bool bReplaceExistingSlots, FString& Message)
{
    const auto Fail = [&Message](const FString& Reason) { Message = Reason; UE_LOG(LogTemp, Warning, TEXT("ChickenGamesMods: %s"), *Reason); return false; };
#if WITH_EDITOR
    return Fail(TEXT("Use a packaged Pak-only build; PIE cannot validate cooked external assets."));
#else
    if (!IsInGameThread()) return Fail(TEXT("Load mods on the game thread."));
    const FString Path = FullPath(ManifestPath);
    const int64 Size = IFileManager::Get().FileSize(*Path);
    if (Size <= 0 || Size > 1024 * 1024) return Fail(TEXT("Manifest missing, empty or larger than 1 MiB: ") + Path);
    FString Text;
    TSharedPtr<FJsonObject> Json;
    if (!FFileHelper::LoadFileToString(Text, *Path) || !FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text), Json) || !Json.IsValid()) return Fail(TEXT("Invalid manifest JSON."));
    double Schema = 0; FString Id, Kind, PakName, ContentRoot, Platform;
    if (!Json->TryGetNumberField(TEXT("schemaVersion"), Schema) || Schema != 1 ||
        !Json->TryGetStringField(TEXT("id"), Id) || !IsId(Id) ||
        !Json->TryGetStringField(TEXT("packageKind"), Kind) || Kind != TEXT("cooked-pak") ||
        !Json->TryGetStringField(TEXT("platform"), Platform) || Platform != TEXT("Windows") ||
        !Json->TryGetStringField(TEXT("pak"), PakName) || !IsRelativeFile(PakName) || !PakName.EndsWith(TEXT(".pak")) ||
        !Json->TryGetStringField(TEXT("contentRoot"), ContentRoot)) return Fail(TEXT("Invalid or unsupported manifest fields."));
    // Flat, mod-owned namespace; shared base game assets may be referenced but never repackaged here.
    const FString RootPrefix = TEXT("/Game/Mods/");
    if (!ContentRoot.StartsWith(RootPrefix) || !ContentRoot.EndsWith(TEXT("/"))) return Fail(TEXT("contentRoot must be /Game/Mods/<namespace>/."));
    const FString Namespace = ContentRoot.Mid(RootPrefix.Len()).LeftChop(1);
    // Case is allowed in UE folders (legacy test folder Sound), but IDs remain lowercase.
    if (!IsId(Namespace.ToLower())) return Fail(TEXT("Invalid MOD namespace."));
    if (LoadedIds.Contains(Id)) return Fail(TEXT("MOD ID already loaded; restart to reload: ") + Id);
    const FString PakPath = FullPath(FPaths::Combine(FPaths::GetPath(Path), PakName));
    if (!PakPath.StartsWith(FPaths::GetPath(Path) + TEXT("/"))) return Fail(TEXT("Pak must be inside the manifest directory."));
    if (MountedRoots.Contains(ContentRoot.ToLower())) return Fail(TEXT("Resource namespace already reserved; restart to change MODs: ") + ContentRoot);
    FString ModType;
    Json->TryGetStringField(TEXT("modType"), ModType);
    const bool bMapMod = ModType == TEXT("map");
    if (!ModType.IsEmpty() && !bMapMod && ModType != TEXT("resource")) return Fail(TEXT("Unsupported MOD type."));
    FString MapPackage;
    if (bMapMod)
    {
        if (!Json->TryGetStringField(TEXT("map"), MapPackage) ||
            !MapPackage.StartsWith(ContentRoot) || !FPackageName::IsValidLongPackageName(MapPackage) ||
            !MapPackage.EndsWith(TEXT(".umap")) && MapPackage.Contains(TEXT(".")))
            return Fail(TEXT("Invalid map package path."));
    }
    const TArray<TSharedPtr<FJsonValue>>* Items = nullptr;
    if (!Json->TryGetArrayField(TEXT("replacements"), Items) || Items->Num() > 512 ||
        (!bMapMod && Items->IsEmpty()) || (bMapMod && !Items->IsEmpty()))
        return Fail(TEXT("Invalid replacements for MOD type."));
    TMap<FName, FString> Requests;
    for (const auto& Item : *Items)
    {
        const TSharedPtr<FJsonObject>* Entry = nullptr; FString SlotText, Asset;
        if (!Item->TryGetObject(Entry) || !(*Entry)->TryGetStringField(TEXT("slot"), SlotText) ||
            !(*Entry)->TryGetStringField(TEXT("asset"), Asset)) return Fail(TEXT("Invalid replacement entry."));
        const FName Slot(*SlotText);
        if (!SlotTypes.Contains(Slot)) return Fail(TEXT("Slot not registered by game: ") + SlotText);
        if (Requests.Contains(Slot)) return Fail(TEXT("Duplicate slot in manifest: ") + SlotText);
        if (Resources.Contains(Slot) && !bReplaceExistingSlots) return Fail(TEXT("Slot conflict: ") + SlotText);
        if (!Asset.StartsWith(ContentRoot) || Asset.Contains(TEXT(":")) || !FPackageName::IsValidObjectPath(Asset)) return Fail(TEXT("Invalid or out-of-namespace object path: ") + Asset);
        Requests.Add(Slot, Asset);
    }
    auto* Pak = static_cast<FPakPlatformFile*>(FPlatformFileManager::Get().FindPlatformFile(TEXT("PakFile")));
    if (!Pak || !FPaths::FileExists(PakPath)) return Fail(TEXT("Pak platform unavailable or pak file missing: ") + PakPath);
    TRefCountPtr<FPakFile> Inspection = new FPakFile(Pak->GetLowerLevel(), *PakPath, false);
    if (!Inspection->IsValid()) return Fail(TEXT("Invalid pak archive."));
    FString ArchiveRoot = Inspection->GetMountPoint(); ArchiveRoot.ReplaceInline(TEXT("\\"), TEXT("/"));
    const int32 ContentIndex = ArchiveRoot.Find(TEXT("/Content/"));
    if (!ArchiveRoot.StartsWith(TEXT("../../../")) || ContentIndex == INDEX_NONE) return Fail(TEXT("Unsupported pak mount point: ") + ArchiveRoot);
    const FString RelativeRoot = ArchiveRoot.Mid(ContentIndex + 9);
    if (RelativeRoot.Contains(TEXT(".."))) return Fail(TEXT("Invalid archive root."));
    const FString Allowed = ContentRoot.Mid(6); // /Game/ -> path below Content/
    TArray<FString> ConfiguredSharedRoots;
    TArray<FString> AllowedSharedRoots;
    GConfig->GetArray(TEXT("ChickenGamesModLoader"), TEXT("AllowedSharedContentRoots"), ConfiguredSharedRoots, GGameIni);
    for (FString SharedRoot : ConfiguredSharedRoots)
    {
        SharedRoot.TrimStartAndEndInline();
        SharedRoot.ReplaceInline(TEXT("\\"), TEXT("/"));
        if (SharedRoot.StartsWith(TEXT("/Game/")) && SharedRoot.EndsWith(TEXT("/")) && !SharedRoot.Contains(TEXT("..")))
        {
            AllowedSharedRoots.Add(SharedRoot.Mid(6));
        }
    }
    int32 Count = 0;
    TSet<FString> ArchiveAssets;
    for (FPakFile::FFilenameIterator It(*Inspection); It; ++It)
    {
        const FString Entry = It.Filename();
        const FString Relative = RelativeRoot + Entry;
        bool bAllowedPath = Relative.StartsWith(Allowed);
        if (!bAllowedPath)
        {
            for (const FString& SharedRoot : AllowedSharedRoots)
            {
                if (Relative.StartsWith(SharedRoot)) { bAllowedPath = true; break; }
            }
        }
        if (++Count > 20000 || !IsRelativeFile(Entry) || !bAllowedPath) return Fail(TEXT("Pak contains files outside its declared namespace or configured shared roots: ") + Relative);
        const FString Ext = FPaths::GetExtension(Entry).ToLower();
        if (Ext == TEXT("uasset") || Ext == TEXT("umap")) ArchiveAssets.Add(Relative);
        if (Ext != TEXT("uasset") && Ext != TEXT("umap") && Ext != TEXT("uexp") && Ext != TEXT("ubulk") && Ext != TEXT("uptnl")) return Fail(TEXT("Unsupported file in resource pak: ") + Entry);
    }
    if (Count == 0) return Fail(TEXT("Empty pak."));
    for (const auto& Request : Requests)
    {
        const FString PackageFile = FPackageName::ObjectPathToPackageName(Request.Value).Mid(6) + TEXT(".uasset");
        if (!ArchiveAssets.Contains(PackageFile)) return Fail(TEXT("Requested asset is not in this pak: ") + Request.Value);
    }
    if (bMapMod && !ArchiveAssets.Contains(MapPackage.Mid(6) + TEXT(".umap")))
        return Fail(TEXT("Declared map is not in the pak: ") + MapPackage);
    const FString MountPath = FPaths::ProjectContentDir() + RelativeRoot;
    if (!Pak->Mount(*PakPath, 0, *MountPath)) return Fail(TEXT("Pak mount failed."));
    // Reserve even on load failure: partially loaded UObjects cannot safely be hot-replaced.
    MountedRoots.Add(ContentRoot.ToLower(), PakPath);
    TMap<FName, TObjectPtr<UObject>> Pending;
    TArray<TStrongObjectPtr<UObject>> KeepAlive;
    for (const auto& Request : Requests)
    {
        UObject* Object = StaticLoadObject(SlotTypes[Request.Key].Get(), nullptr, *Request.Value);
        if (!Object) return Fail(TEXT("Asset/type/dependency load failed; slots unchanged. Restart before retry: ") + Request.Value);
        KeepAlive.Emplace(Object);
        Pending.Add(Request.Key, Object);
    }
    // Commit only after all requested top-level resources passed type/loading checks.
    for (const auto& Pair : Pending) { Resources.Add(Pair.Key, Pair.Value); Owners.Add(Pair.Key, Id); }
    LoadedIds.Add(Id, Path);
    if (bMapMod)
    {
        FChickenGamesModMapInfo Info; Info.ModId=Id; Info.MapPath=MapPackage;
        FString Value;
        Info.DisplayName=FText::FromString(Json->TryGetStringField(TEXT("displayName"),Value)&&!Value.IsEmpty()?Value:Id);
        Value.Empty(); Json->TryGetStringField(TEXT("author"),Value); Info.Author=Value;
        Value.Empty(); Json->TryGetStringField(TEXT("description"),Value); Info.Description=FText::FromString(Value);
        Value.Empty(); Json->TryGetStringField(TEXT("version"),Value); Info.Version=Value;
        Value.Empty();
        if(Json->TryGetStringField(TEXT("preview"),Value)&&IsRelativeFile(Value)) Info.PreviewImagePath=FullPath(FPaths::Combine(FPaths::GetPath(Path),Value));
        ModMaps.Add(Id,Info);
    }
    Message = bMapMod ? FString::Printf(TEXT("Loaded map MOD %s: %s."), *Id, *MapPackage) :
        FString::Printf(TEXT("Loaded MOD %s: %d slots."), *Id, Pending.Num());
    UE_LOG(LogTemp, Display, TEXT("ChickenGamesMods: %s"), *Message);
    return true;
#endif
}
