#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "CodexUnrealBridgeSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FCodexBridgeResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="Codex Unreal Bridge")
    bool bSuccess = false;

    UPROPERTY(BlueprintReadOnly, Category="Codex Unreal Bridge")
    FString Message;

    UPROPERTY(BlueprintReadOnly, Category="Codex Unreal Bridge")
    FString ObjectPath;

    UPROPERTY(BlueprintReadOnly, Category="Codex Unreal Bridge")
    FString Json;
};

/** Editor-only local automation API. Every mutating operation is transactional and audited. */
UCLASS()
class CODEXUNREALBRIDGE_API UCodexUnrealBridgeSubsystem : public UEditorSubsystem
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category="Codex Unreal Bridge")
    FString GetBridgeVersion() const { return TEXT("1.0.0"); }

    UFUNCTION(BlueprintCallable, Category="Codex Unreal Bridge|Inspect")
    FCodexBridgeResult GetCurrentLevelSnapshot() const;

    UFUNCTION(BlueprintCallable, Category="Codex Unreal Bridge|Inspect")
    FCodexBridgeResult GetBlueprintSnapshot(const FString& BlueprintAssetPath) const;

    UFUNCTION(BlueprintCallable, Category="Codex Unreal Bridge|Scene")
    FCodexBridgeResult SpawnActorFromClass(const FString& ActorClassPath, const FString& ActorLabel,
        FVector Location, FRotator Rotation, FVector Scale);

    UFUNCTION(BlueprintCallable, Category="Codex Unreal Bridge|Scene")
    FCodexBridgeResult SpawnActorFromAsset(const FString& AssetPath, const FString& ActorLabel,
        FVector Location, FRotator Rotation, FVector Scale);

    UFUNCTION(BlueprintCallable, Category="Codex Unreal Bridge|Scene")
    FCodexBridgeResult DuplicateActor(const FString& SourceActorLabel, const FString& NewActorLabel, FVector Offset);

    UFUNCTION(BlueprintCallable, Category="Codex Unreal Bridge|Scene")
    FCodexBridgeResult SetActorTransform(const FString& ActorLabel, FVector Location, FRotator Rotation, FVector Scale);

    UFUNCTION(BlueprintCallable, Category="Codex Unreal Bridge|Scene")
    FCodexBridgeResult DeleteActor(const FString& ActorLabel);

    UFUNCTION(BlueprintCallable, Category="Codex Unreal Bridge|Properties")
    FCodexBridgeResult SetActorProperty(const FString& ActorLabel, const FString& PropertyName, const FString& ValueText);

    UFUNCTION(BlueprintCallable, Category="Codex Unreal Bridge|Properties")
    FCodexBridgeResult SetComponentProperty(const FString& ActorLabel, const FString& ComponentName,
        const FString& PropertyName, const FString& ValueText);

    UFUNCTION(BlueprintCallable, Category="Codex Unreal Bridge|Scene")
    FCodexBridgeResult SetLight(const FString& ActorLabel, const FString& ComponentName,
        float Intensity, FLinearColor Color, bool bVisible);

    UFUNCTION(BlueprintCallable, Category="Codex Unreal Bridge|Scene")
    FCodexBridgeResult SetStaticMesh(const FString& ActorLabel, const FString& ComponentName, const FString& MeshAssetPath);

    UFUNCTION(BlueprintCallable, Category="Codex Unreal Bridge|Scene")
    FCodexBridgeResult SetMaterial(const FString& ActorLabel, const FString& ComponentName,
        int32 MaterialIndex, const FString& MaterialAssetPath);

    UFUNCTION(BlueprintCallable, Category="Codex Unreal Bridge|Assets")
    FCodexBridgeResult CreateBlueprint(const FString& BlueprintAssetPath, const FString& ParentClassPath);

    UFUNCTION(BlueprintCallable, Category="Codex Unreal Bridge|Blueprint")
    FCodexBridgeResult AddBlueprintComponent(const FString& BlueprintAssetPath, const FString& ComponentClassPath,
        const FString& ComponentName);

    UFUNCTION(BlueprintCallable, Category="Codex Unreal Bridge|Blueprint")
    FCodexBridgeResult AddBlueprintVariable(const FString& BlueprintAssetPath, const FString& VariableName,
        const FString& VariableType);

    UFUNCTION(BlueprintCallable, Category="Codex Unreal Bridge|Blueprint")
    FCodexBridgeResult AddBlueprintFunction(const FString& BlueprintAssetPath, const FString& FunctionName);

    UFUNCTION(BlueprintCallable, Category="Codex Unreal Bridge|Blueprint")
    FCodexBridgeResult CompileBlueprint(const FString& BlueprintAssetPath, bool bSaveOnSuccess = true);

    UFUNCTION(BlueprintCallable, Category="Codex Unreal Bridge|Assets")
    FCodexBridgeResult SaveAsset(const FString& AssetPath);

    UFUNCTION(BlueprintCallable, Category="Codex Unreal Bridge|Scene")
    FCodexBridgeResult SaveCurrentLevel();

    // Public so the module's small internal helpers can return the same audited result type.
    static FCodexBridgeResult Result(bool bSuccess, const FString& Message,
        const FString& ObjectPath = FString(), const FString& Json = FString());
};
