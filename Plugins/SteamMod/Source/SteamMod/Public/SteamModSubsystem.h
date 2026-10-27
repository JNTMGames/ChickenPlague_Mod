#pragma once
#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SteamModSubsystem.generated.h"

class UDataTable;
class UChickenGamesCustomSymptomPack;

USTRUCT(BlueprintType)
struct STEAMMOD_API FChickenGamesModMapInfo
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly) FString ModId;
    UPROPERTY(BlueprintReadOnly) FString MapPath;
    UPROPERTY(BlueprintReadOnly) FText DisplayName;
    UPROPERTY(BlueprintReadOnly) FString Author;
    UPROPERTY(BlueprintReadOnly) FText Description;
    UPROPERTY(BlueprintReadOnly) FString Version;
    UPROPERTY(BlueprintReadOnly) FString PreviewImagePath;
};

UCLASS(BlueprintType, meta=(DisplayName="ChickenGames Mod Subsystem"))
class STEAMMOD_API USteamModSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintPure, Category="ChickenGames|Mods")
    bool AreModsReady() const { return bModsReady; }

    UFUNCTION(BlueprintPure, Category="ChickenGames|Mods")
    FString GetModLoadReport() const { return ModLoadReport; }

    // The game owns the allowed slots and types, not a mod manifest.
    UFUNCTION(BlueprintCallable, Category="ChickenGames|Mods")
    bool RegisterModSlot(FName Slot, TSubclassOf<UObject> ExpectedClass);

    // Registers every developer-authored slot in a ChickenGames slot DataTable.
    UFUNCTION(BlueprintCallable, Category="ChickenGames|Mods")
    bool RegisterModSlotsFromDataTable(UDataTable* SlotTable, FString& Message);

    // Later mods may replace earlier slots ONLY when explicitly requested.
    // Manifest paths may be absolute or relative to ProjectDir (also supports Workshop paths).
    UFUNCTION(BlueprintCallable, Category="ChickenGames|Mods")
    bool LoadModManifest(const FString& ManifestPath, bool bReplaceExistingSlots, FString& Message);

    UFUNCTION(BlueprintPure, Category="ChickenGames|Mods")
    UObject* GetModResource(FName Slot) const;

    UFUNCTION(BlueprintPure, Category="ChickenGames|Mods")
    FText GetModText(FName Slot, const FText& DefaultText) const;

    UFUNCTION(BlueprintPure, Category="ChickenGames|Mods")
    FString GetModResourceOwner(FName Slot) const;

    UFUNCTION(BlueprintPure, Category="ChickenGames|Mods")
    TArray<FString> FindModManifests(const FString& Directory) const;

    // Map MODs are registered during manifest loading; the game can show these in its map menu.
    UFUNCTION(BlueprintPure, Category="ChickenGames|Mods|Maps")
    TArray<FString> GetModMapIds() const;

    UFUNCTION(BlueprintPure, Category="ChickenGames|Mods|Maps")
    bool GetModMapPath(const FString& ModId, FString& MapPath) const;

    UFUNCTION(BlueprintCallable, Category="ChickenGames|Mods|Maps")
    bool OpenModMap(const FString& ModId, FString& Message);

    UFUNCTION(BlueprintPure, Category="ChickenGames|Mods|Maps")
    TArray<FChickenGamesModMapInfo> GetModMaps() const;

    // Adds player-authored singing/dancing/rap/basketball symptoms to the in-memory table.
    UFUNCTION(BlueprintCallable, Category="ChickenGames|Mods|Symptoms")
    bool AppendCustomSymptomsToDataTable(UDataTable* DataTable, UChickenGamesCustomSymptomPack* Pack,
        const FString& ModNamespace, int32& AddedCount, FString& Message);


    // Applies loaded resources to matching UObject fields in every DataTable row.
    // Slot format: <prefix>.<row name>.<property name>, all lower-case.
    // Array fields are replaced with a one-element array containing the MOD asset.
    // This changes only the in-memory table for the current game process.
    UFUNCTION(BlueprintCallable, Category="ChickenGames|Mods",
        meta=(DisplayName="Apply Loaded Mods To Data Table"))
    bool ApplyLoadedModsToDataTable(UDataTable* DataTable, const FString& SlotPrefix,
        int32& AppliedCount, FString& Message);

private:
    bool bModsReady = false;
    FString ModLoadReport;
    UPROPERTY(Transient) TMap<FName, TSubclassOf<UObject>> SlotTypes;
    UPROPERTY(Transient) TMap<FName, TObjectPtr<UObject>> Resources;
    UPROPERTY(Transient) TMap<FName, FString> Owners;
    TMap<FString, FString> LoadedIds;
    TMap<FString, FChickenGamesModMapInfo> ModMaps;
};
