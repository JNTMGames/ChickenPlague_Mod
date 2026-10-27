#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/DataAsset.h"
#include "ChickenGamesModSlotTable.generated.h"

UCLASS(BlueprintType, meta=(DisplayName="ChickenGames MOD Text"))
class STEAMMOD_API UChickenGamesModTextAsset : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ChickenGames|Mods", meta=(MultiLine=true))
    FText Text;
};

USTRUCT(BlueprintType)
struct STEAMMOD_API FChickenGamesModSlotRow : public FTableRowBase
{
    GENERATED_BODY()

    // Slash-separated menu hierarchy, for example: 人物/雨姐坤/音效/唱歌音效
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ChickenGames|Mods")
    FString MenuPath;

    // Leaf text shown to the MOD author, for example: singing
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ChickenGames|Mods")
    FText EntryName;

    // Stable, globally unique runtime key. Never derive game logic from display text.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ChickenGames|Mods")
    FName SlotId;

    // Asset class accepted by this slot, such as AnimSequence, SoundBase or MaterialInterface.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ChickenGames|Mods")
    TSoftClassPtr<UObject> AllowedClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ChickenGames|Mods")
    bool bRequired = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ChickenGames|Mods")
    int32 SortOrder = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ChickenGames|Mods")
    FText Description;
};

USTRUCT(BlueprintType)
struct STEAMMOD_API FChickenGamesModCharacterRow : public FTableRowBase
{
    GENERATED_BODY()

    // Stable lowercase key used to generate SlotId, for example: yujie or laoda.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ChickenGames|Mods")
    FName CharacterId;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ChickenGames|Mods")
    FText DisplayName;

    // Parent menu before the character name, normally: 人物
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ChickenGames|Mods")
    FString MenuPath = TEXT("人物");

    // Selects all resource rows with the same TemplateId.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ChickenGames|Mods")
    FName SlotTemplate = FName(TEXT("Humanoid"));

    // ResourceId values removed from this character's selected template.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ChickenGames|Mods")
    TArray<FName> ExcludedSlots;

    // ResourceId values added even when they belong to another template.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ChickenGames|Mods")
    TArray<FName> AdditionalSlots;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ChickenGames|Mods")
    int32 SortOrder = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ChickenGames|Mods")
    bool bEnabled = true;
};

USTRUCT(BlueprintType)
struct STEAMMOD_API FChickenGamesModResourceTemplateRow : public FTableRowBase
{
    GENERATED_BODY()

    // Characters select a group through this value, for example: Humanoid.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ChickenGames|Mods")
    FName TemplateId = FName(TEXT("Humanoid"));

    // Stable suffix used to generate character.<CharacterId>.<ResourceId>.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ChickenGames|Mods")
    FName ResourceId;

    // Menu beneath the character, for example: 动画/走路动画.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ChickenGames|Mods")
    FString MenuPath;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ChickenGames|Mods")
    FText EntryName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ChickenGames|Mods")
    TSoftClassPtr<UObject> AllowedClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ChickenGames|Mods")
    bool bRequired = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ChickenGames|Mods")
    int32 SortOrder = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ChickenGames|Mods")
    FText Description;
};
