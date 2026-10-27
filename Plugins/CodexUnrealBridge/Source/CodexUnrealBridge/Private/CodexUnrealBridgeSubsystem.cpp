#include "CodexUnrealBridgeSubsystem.h"

#include "BlueprintEditorLibrary.h"
#include "Components/LightComponent.h"
#include "Components/MeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Dom/JsonObject.h"
#include "EdGraph/EdGraph.h"
#include "EdGraphSchema_K2.h"
#include "Editor.h"
#include "Engine/Blueprint.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "FileHelpers.h"
#include "HAL/FileManager.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "KismetCompiler.h"
#include "Materials/MaterialInterface.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "ScopedTransaction.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Subsystems/EditorActorSubsystem.h"
#include "Subsystems/EditorAssetSubsystem.h"
#include "UObject/UnrealType.h"

#define LOCTEXT_NAMESPACE "CodexUnrealBridge"

namespace CodexBridge
{
    UWorld* World()
    {
        return GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    }

    AActor* ActorByLabel(const FString& Label)
    {
        UWorld* CurrentWorld = World();
        if (!CurrentWorld) return nullptr;
        for (TActorIterator<AActor> It(CurrentWorld); It; ++It)
            if (It->GetActorLabel().Equals(Label, ESearchCase::CaseSensitive)) return *It;
        return nullptr;
    }

    UActorComponent* ComponentByName(AActor* Actor, const FString& Name)
    {
        if (!Actor) return nullptr;
        TInlineComponentArray<UActorComponent*> Components(Actor);
        for (UActorComponent* Component : Components)
            if (Component && (Component->GetName()==Name || Component->GetReadableName()==Name)) return Component;
        return nullptr;
    }

    UBlueprint* Blueprint(const FString& Path)
    {
        return Cast<UBlueprint>(UEditorAssetSubsystem::StaticClass()
            ? GEditor->GetEditorSubsystem<UEditorAssetSubsystem>()->LoadAsset(Path) : nullptr);
    }

    bool SetProperty(UObject* Object, const FString& Name, const FString& Value, FString& Error)
    {
        if (!Object) { Error=TEXT("Target object is null."); return false; }
        FProperty* Property=Object->GetClass()->FindPropertyByName(*Name);
        if (!Property) { Error=FString::Printf(TEXT("Property '%s' was not found on %s."),*Name,*Object->GetClass()->GetName()); return false; }
        Object->Modify();
        Object->PreEditChange(Property);
        const TCHAR* End=Property->ImportText_Direct(*Value,Property->ContainerPtrToValuePtr<void>(Object),Object,PPF_None);
        if (!End)
        {
            Error=FString::Printf(TEXT("Value '%s' could not be imported into property '%s'."),*Value,*Name);
            return false;
        }
        FPropertyChangedEvent Event(Property,EPropertyChangeType::ValueSet);
        Object->PostEditChangeProperty(Event);
        Object->MarkPackageDirty();
        return true;
    }

    UClass* ClassFromPath(const FString& Path)
    {
        if (UClass* Class=LoadObject<UClass>(nullptr,*Path)) return Class;
        if (UBlueprint* BP=LoadObject<UBlueprint>(nullptr,*Path)) return BP->GeneratedClass;
        return nullptr;
    }

    FString JsonString(const TSharedRef<FJsonObject>& Object)
    {
        FString Output;
        const TSharedRef<TJsonWriter<>> Writer=TJsonWriterFactory<>::Create(&Output);
        FJsonSerializer::Serialize(Object,Writer);
        return Output;
    }

    void Audit(bool Success, const FString& Message, const FString& ObjectPath)
    {
        const FString Directory=FPaths::Combine(FPaths::ProjectSavedDir(),TEXT("CodexUnrealBridge"),TEXT("Reports"));
        IFileManager::Get().MakeDirectory(*Directory,true);
        const TSharedRef<FJsonObject> Entry=MakeShared<FJsonObject>();
        Entry->SetStringField(TEXT("time_utc"),FDateTime::UtcNow().ToIso8601());
        Entry->SetBoolField(TEXT("success"),Success);
        Entry->SetStringField(TEXT("message"),Message);
        Entry->SetStringField(TEXT("object_path"),ObjectPath);
        FString Line=JsonString(Entry)+LINE_TERMINATOR;
        FFileHelper::SaveStringToFile(Line,*FPaths::Combine(Directory,TEXT("operations.jsonl")),FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM,&IFileManager::Get(),FILEWRITE_Append);
    }

    FCodexBridgeResult BlueprintCompile(UBlueprint* BP, bool bSave)
    {
        if (!BP) return UCodexUnrealBridgeSubsystem::Result(false,TEXT("Blueprint could not be loaded."));
        FCompilerResultsLog Log;
        FKismetEditorUtilities::CompileBlueprint(BP,EBlueprintCompileOptions::None,&Log);
        if (Log.NumErrors || BP->Status==BS_Error)
            return UCodexUnrealBridgeSubsystem::Result(false,FString::Printf(TEXT("Blueprint compilation failed with %d error(s)."),Log.NumErrors),BP->GetPathName());
        if (bSave && !GEditor->GetEditorSubsystem<UEditorAssetSubsystem>()->SaveLoadedAsset(BP,false))
            return UCodexUnrealBridgeSubsystem::Result(false,TEXT("Blueprint compiled but could not be saved."),BP->GetPathName());
        return UCodexUnrealBridgeSubsystem::Result(true,TEXT("Blueprint compiled successfully."),BP->GetPathName());
    }
}

FCodexBridgeResult UCodexUnrealBridgeSubsystem::Result(bool bSuccess,const FString& Message,const FString& ObjectPath,const FString& Json)
{
    FCodexBridgeResult Out;
    Out.bSuccess=bSuccess; Out.Message=Message; Out.ObjectPath=ObjectPath; Out.Json=Json;
    CodexBridge::Audit(bSuccess,Message,ObjectPath);
    return Out;
}

FCodexBridgeResult UCodexUnrealBridgeSubsystem::GetCurrentLevelSnapshot() const
{
    UWorld* World=CodexBridge::World();
    if (!World) return Result(false,TEXT("No editor world is open."));
    TArray<TSharedPtr<FJsonValue>> Actors;
    for (TActorIterator<AActor> It(World);It;++It)
    {
        AActor* Actor=*It;
        const TSharedRef<FJsonObject> Item=MakeShared<FJsonObject>();
        Item->SetStringField(TEXT("label"),Actor->GetActorLabel());
        Item->SetStringField(TEXT("path"),Actor->GetPathName());
        Item->SetStringField(TEXT("class"),Actor->GetClass()->GetPathName());
        const FTransform T=Actor->GetActorTransform();
        Item->SetStringField(TEXT("location"),T.GetLocation().ToString());
        Item->SetStringField(TEXT("rotation"),T.Rotator().ToString());
        Item->SetStringField(TEXT("scale"),T.GetScale3D().ToString());
        TArray<TSharedPtr<FJsonValue>> Components;
        TInlineComponentArray<UActorComponent*> ActorComponents(Actor);
        for (UActorComponent* Component:ActorComponents)
        {
            const TSharedRef<FJsonObject> ComponentItem=MakeShared<FJsonObject>();
            ComponentItem->SetStringField(TEXT("name"),Component->GetName());
            ComponentItem->SetStringField(TEXT("class"),Component->GetClass()->GetPathName());
            Components.Add(MakeShared<FJsonValueObject>(ComponentItem));
        }
        Item->SetArrayField(TEXT("components"),Components);
        Actors.Add(MakeShared<FJsonValueObject>(Item));
    }
    const TSharedRef<FJsonObject> Root=MakeShared<FJsonObject>();
    Root->SetStringField(TEXT("world"),World->GetPathName());
    Root->SetArrayField(TEXT("actors"),Actors);
    return Result(true,FString::Printf(TEXT("Read %d actor(s)."),Actors.Num()),World->GetPathName(),CodexBridge::JsonString(Root));
}

FCodexBridgeResult UCodexUnrealBridgeSubsystem::GetBlueprintSnapshot(const FString& BlueprintAssetPath) const
{
    UBlueprint* BP=CodexBridge::Blueprint(BlueprintAssetPath);
    if (!BP) return Result(false,TEXT("Blueprint could not be loaded."),BlueprintAssetPath);
    const TSharedRef<FJsonObject> Root=MakeShared<FJsonObject>();
    Root->SetStringField(TEXT("path"),BP->GetPathName());
    Root->SetStringField(TEXT("parent_class"),BP->ParentClass ? BP->ParentClass->GetPathName() : TEXT("None"));
    TArray<TSharedPtr<FJsonValue>> Graphs;
    TArray<UEdGraph*> AllGraphs;
    BP->GetAllGraphs(AllGraphs);
    for (UEdGraph* Graph:AllGraphs)
    {
        const TSharedRef<FJsonObject> Entry=MakeShared<FJsonObject>();
        Entry->SetStringField(TEXT("name"),Graph->GetName());
        Entry->SetNumberField(TEXT("nodes"),Graph->Nodes.Num());
        Graphs.Add(MakeShared<FJsonValueObject>(Entry));
    }
    Root->SetArrayField(TEXT("graphs"),Graphs);
    TArray<TSharedPtr<FJsonValue>> Variables;
    for (const FBPVariableDescription& Variable:BP->NewVariables)
        Variables.Add(MakeShared<FJsonValueString>(Variable.VarName.ToString()));
    Root->SetArrayField(TEXT("variables"),Variables);
    return Result(true,TEXT("Blueprint snapshot created."),BP->GetPathName(),CodexBridge::JsonString(Root));
}

FCodexBridgeResult UCodexUnrealBridgeSubsystem::SpawnActorFromClass(const FString& ActorClassPath,const FString& ActorLabel,FVector Location,FRotator Rotation,FVector Scale)
{
    UClass* Class=CodexBridge::ClassFromPath(ActorClassPath);
    if (!Class || !Class->IsChildOf(AActor::StaticClass())) return Result(false,TEXT("Actor class could not be loaded."),ActorClassPath);
    const FScopedTransaction Transaction(LOCTEXT("SpawnActor","Codex Bridge: Spawn Actor"));
    AActor* Actor=GEditor->GetEditorSubsystem<UEditorActorSubsystem>()->SpawnActorFromClass(Class,Location,Rotation,false);
    if (!Actor) return Result(false,TEXT("Actor could not be spawned."),ActorClassPath);
    Actor->Modify(); Actor->SetActorLabel(ActorLabel.IsEmpty()?Class->GetName():ActorLabel,true); Actor->SetActorScale3D(Scale);
    return Result(true,TEXT("Actor spawned."),Actor->GetPathName());
}

FCodexBridgeResult UCodexUnrealBridgeSubsystem::SpawnActorFromAsset(const FString& AssetPath,const FString& ActorLabel,FVector Location,FRotator Rotation,FVector Scale)
{
    UObject* Asset=GEditor->GetEditorSubsystem<UEditorAssetSubsystem>()->LoadAsset(AssetPath);
    if (!Asset) return Result(false,TEXT("Asset could not be loaded."),AssetPath);
    const FScopedTransaction Transaction(LOCTEXT("SpawnAssetActor","Codex Bridge: Spawn Asset Actor"));
    AActor* Actor=GEditor->GetEditorSubsystem<UEditorActorSubsystem>()->SpawnActorFromObject(Asset,Location,Rotation,false);
    if (!Actor) return Result(false,TEXT("Asset cannot be placed as an actor."),AssetPath);
    Actor->Modify(); Actor->SetActorLabel(ActorLabel.IsEmpty()?Asset->GetName():ActorLabel,true); Actor->SetActorScale3D(Scale);
    return Result(true,TEXT("Asset actor spawned."),Actor->GetPathName());
}

FCodexBridgeResult UCodexUnrealBridgeSubsystem::DuplicateActor(const FString& SourceActorLabel,const FString& NewActorLabel,FVector Offset)
{
    AActor* Source=CodexBridge::ActorByLabel(SourceActorLabel);
    if (!Source) return Result(false,TEXT("Source actor was not found."),SourceActorLabel);
    const FScopedTransaction Transaction(LOCTEXT("DuplicateActor","Codex Bridge: Duplicate Actor"));
    AActor* NewActor=GEditor->GetEditorSubsystem<UEditorActorSubsystem>()->DuplicateActor(Source,nullptr,Offset);
    if (!NewActor) return Result(false,TEXT("Actor could not be duplicated."),Source->GetPathName());
    NewActor->SetActorLabel(NewActorLabel.IsEmpty()?SourceActorLabel+TEXT("_Copy"):NewActorLabel,true);
    return Result(true,TEXT("Actor duplicated."),NewActor->GetPathName());
}

FCodexBridgeResult UCodexUnrealBridgeSubsystem::SetActorTransform(const FString& ActorLabel,FVector Location,FRotator Rotation,FVector Scale)
{
    AActor* Actor=CodexBridge::ActorByLabel(ActorLabel);
    if (!Actor) return Result(false,TEXT("Actor was not found."),ActorLabel);
    const FScopedTransaction Transaction(LOCTEXT("TransformActor","Codex Bridge: Transform Actor"));
    Actor->Modify(); Actor->SetActorTransform(FTransform(Rotation,Location,Scale),false,nullptr,ETeleportType::TeleportPhysics);
    return Result(true,TEXT("Actor transform updated."),Actor->GetPathName());
}

FCodexBridgeResult UCodexUnrealBridgeSubsystem::DeleteActor(const FString& ActorLabel)
{
    AActor* Actor=CodexBridge::ActorByLabel(ActorLabel);
    if (!Actor) return Result(false,TEXT("Actor was not found."),ActorLabel);
    const FString Path=Actor->GetPathName();
    const FScopedTransaction Transaction(LOCTEXT("DeleteActor","Codex Bridge: Delete Actor"));
    return Result(GEditor->GetEditorSubsystem<UEditorActorSubsystem>()->DestroyActor(Actor),TEXT("Actor delete requested."),Path);
}

FCodexBridgeResult UCodexUnrealBridgeSubsystem::SetActorProperty(const FString& ActorLabel,const FString& PropertyName,const FString& ValueText)
{
    AActor* Actor=CodexBridge::ActorByLabel(ActorLabel);
    FString Error;
    const FScopedTransaction Transaction(LOCTEXT("SetActorProperty","Codex Bridge: Set Actor Property"));
    const bool Success=CodexBridge::SetProperty(Actor,PropertyName,ValueText,Error);
    return Result(Success,Success?TEXT("Actor property updated."):Error,Actor?Actor->GetPathName():ActorLabel);
}

FCodexBridgeResult UCodexUnrealBridgeSubsystem::SetComponentProperty(const FString& ActorLabel,const FString& ComponentName,const FString& PropertyName,const FString& ValueText)
{
    UActorComponent* Component=CodexBridge::ComponentByName(CodexBridge::ActorByLabel(ActorLabel),ComponentName);
    FString Error;
    const FScopedTransaction Transaction(LOCTEXT("SetComponentProperty","Codex Bridge: Set Component Property"));
    const bool Success=CodexBridge::SetProperty(Component,PropertyName,ValueText,Error);
    return Result(Success,Success?TEXT("Component property updated."):Error,Component?Component->GetPathName():ActorLabel+TEXT(".")+ComponentName);
}

FCodexBridgeResult UCodexUnrealBridgeSubsystem::SetLight(const FString& ActorLabel,const FString& ComponentName,float Intensity,FLinearColor Color,bool bVisible)
{
    ULightComponent* Light=Cast<ULightComponent>(CodexBridge::ComponentByName(CodexBridge::ActorByLabel(ActorLabel),ComponentName));
    if (!Light) return Result(false,TEXT("Light component was not found."),ActorLabel+TEXT(".")+ComponentName);
    const FScopedTransaction Transaction(LOCTEXT("SetLight","Codex Bridge: Set Light"));
    Light->Modify(); Light->SetIntensity(FMath::Max(0.0f,Intensity)); Light->SetLightColor(Color); Light->SetVisibility(bVisible); Light->MarkPackageDirty();
    return Result(true,TEXT("Light updated."),Light->GetPathName());
}

FCodexBridgeResult UCodexUnrealBridgeSubsystem::SetStaticMesh(const FString& ActorLabel,const FString& ComponentName,const FString& MeshAssetPath)
{
    UStaticMeshComponent* Component=Cast<UStaticMeshComponent>(CodexBridge::ComponentByName(CodexBridge::ActorByLabel(ActorLabel),ComponentName));
    UStaticMesh* Mesh=LoadObject<UStaticMesh>(nullptr,*MeshAssetPath);
    if (!Component || !Mesh) return Result(false,TEXT("Static mesh component or mesh asset was not found."),MeshAssetPath);
    const FScopedTransaction Transaction(LOCTEXT("SetMesh","Codex Bridge: Set Static Mesh"));
    Component->Modify(); Component->SetStaticMesh(Mesh); Component->MarkPackageDirty();
    return Result(true,TEXT("Static mesh assigned."),Component->GetPathName());
}

FCodexBridgeResult UCodexUnrealBridgeSubsystem::SetMaterial(const FString& ActorLabel,const FString& ComponentName,int32 MaterialIndex,const FString& MaterialAssetPath)
{
    UMeshComponent* Component=Cast<UMeshComponent>(CodexBridge::ComponentByName(CodexBridge::ActorByLabel(ActorLabel),ComponentName));
    UMaterialInterface* Material=LoadObject<UMaterialInterface>(nullptr,*MaterialAssetPath);
    if (!Component || !Material || MaterialIndex<0) return Result(false,TEXT("Mesh component, material, or material index is invalid."),MaterialAssetPath);
    const FScopedTransaction Transaction(LOCTEXT("SetMaterial","Codex Bridge: Set Material"));
    Component->Modify(); Component->SetMaterial(MaterialIndex,Material); Component->MarkPackageDirty();
    return Result(true,TEXT("Material assigned."),Component->GetPathName());
}

FCodexBridgeResult UCodexUnrealBridgeSubsystem::CreateBlueprint(const FString& BlueprintAssetPath,const FString& ParentClassPath)
{
    UClass* Parent=CodexBridge::ClassFromPath(ParentClassPath);
    if (!Parent) return Result(false,TEXT("Parent class could not be loaded."),ParentClassPath);
    const FScopedTransaction Transaction(LOCTEXT("CreateBlueprint","Codex Bridge: Create Blueprint"));
    UBlueprint* BP=UBlueprintEditorLibrary::CreateBlueprintAssetWithParent(BlueprintAssetPath,Parent);
    if (!BP) return Result(false,TEXT("Blueprint could not be created; check that the path does not exist."),BlueprintAssetPath);
    return Result(true,TEXT("Blueprint created."),BP->GetPathName());
}

FCodexBridgeResult UCodexUnrealBridgeSubsystem::AddBlueprintComponent(const FString& BlueprintAssetPath,const FString& ComponentClassPath,const FString& ComponentName)
{
    UBlueprint* BP=CodexBridge::Blueprint(BlueprintAssetPath);
    UClass* ComponentClass=CodexBridge::ClassFromPath(ComponentClassPath);
    if (!BP || !ComponentClass || !ComponentClass->IsChildOf(UActorComponent::StaticClass()))
        return Result(false,TEXT("Blueprint or component class is invalid."),BlueprintAssetPath);
    const FScopedTransaction Transaction(LOCTEXT("AddComponent","Codex Bridge: Add Blueprint Component"));
    BP->Modify();
    USCS_Node* Node=BP->SimpleConstructionScript->CreateNode(ComponentClass,*ComponentName);
    BP->SimpleConstructionScript->AddNode(Node);
    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BP);
    return CodexBridge::BlueprintCompile(BP,false);
}

FCodexBridgeResult UCodexUnrealBridgeSubsystem::AddBlueprintVariable(const FString& BlueprintAssetPath,const FString& VariableName,const FString& VariableType)
{
    UBlueprint* BP=CodexBridge::Blueprint(BlueprintAssetPath);
    if (!BP) return Result(false,TEXT("Blueprint could not be loaded."),BlueprintAssetPath);
    FEdGraphPinType Pin;
    const FString Type=VariableType.ToLower();
    if (Type==TEXT("bool")) Pin.PinCategory=UEdGraphSchema_K2::PC_Boolean;
    else if (Type==TEXT("int") || Type==TEXT("integer")) Pin.PinCategory=UEdGraphSchema_K2::PC_Int;
    else if (Type==TEXT("float") || Type==TEXT("real")) Pin.PinCategory=UEdGraphSchema_K2::PC_Real;
    else if (Type==TEXT("string")) Pin.PinCategory=UEdGraphSchema_K2::PC_String;
    else if (Type==TEXT("name")) Pin.PinCategory=UEdGraphSchema_K2::PC_Name;
    else if (Type==TEXT("text")) Pin.PinCategory=UEdGraphSchema_K2::PC_Text;
    else if (Type==TEXT("vector")) { Pin.PinCategory=UEdGraphSchema_K2::PC_Struct; Pin.PinSubCategoryObject=TBaseStructure<FVector>::Get(); }
    else if (Type==TEXT("rotator")) { Pin.PinCategory=UEdGraphSchema_K2::PC_Struct; Pin.PinSubCategoryObject=TBaseStructure<FRotator>::Get(); }
    else return Result(false,TEXT("Unsupported variable type. Use bool, int, float, string, name, text, vector, or rotator."),BlueprintAssetPath);
    const FScopedTransaction Transaction(LOCTEXT("AddVariable","Codex Bridge: Add Blueprint Variable"));
    if (!FBlueprintEditorUtils::AddMemberVariable(BP,*VariableName,Pin)) return Result(false,TEXT("Variable could not be added; it may already exist."),BP->GetPathName());
    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BP);
    return CodexBridge::BlueprintCompile(BP,false);
}

FCodexBridgeResult UCodexUnrealBridgeSubsystem::AddBlueprintFunction(const FString& BlueprintAssetPath,const FString& FunctionName)
{
    UBlueprint* BP=CodexBridge::Blueprint(BlueprintAssetPath);
    if (!BP) return Result(false,TEXT("Blueprint could not be loaded."),BlueprintAssetPath);
    TArray<UEdGraph*> ExistingGraphs;
    BP->GetAllGraphs(ExistingGraphs);
    if (ExistingGraphs.ContainsByPredicate([&FunctionName](const UEdGraph* Graph)
        { return Graph && Graph->GetName().Equals(FunctionName,ESearchCase::IgnoreCase); }))
        return Result(false,TEXT("A graph with that name already exists."),BP->GetPathName());
    const FScopedTransaction Transaction(LOCTEXT("AddFunction","Codex Bridge: Add Blueprint Function"));
    UEdGraph* Graph=UBlueprintEditorLibrary::AddFunctionGraph(BP,FunctionName);
    if (!Graph) return Result(false,TEXT("Function graph could not be created."),BP->GetPathName());
    return CodexBridge::BlueprintCompile(BP,false);
}

FCodexBridgeResult UCodexUnrealBridgeSubsystem::CompileBlueprint(const FString& BlueprintAssetPath,bool bSaveOnSuccess)
{
    return CodexBridge::BlueprintCompile(CodexBridge::Blueprint(BlueprintAssetPath),bSaveOnSuccess);
}

FCodexBridgeResult UCodexUnrealBridgeSubsystem::SaveAsset(const FString& AssetPath)
{
    UEditorAssetSubsystem* Assets=GEditor->GetEditorSubsystem<UEditorAssetSubsystem>();
    UObject* Asset=Assets->LoadAsset(AssetPath);
    if (!Asset) return Result(false,TEXT("Asset could not be loaded."),AssetPath);
    return Result(Assets->SaveLoadedAsset(Asset,false),TEXT("Asset save requested."),Asset->GetPathName());
}

FCodexBridgeResult UCodexUnrealBridgeSubsystem::SaveCurrentLevel()
{
    UWorld* World=CodexBridge::World();
    if (!World) return Result(false,TEXT("No editor world is open."));
    return Result(FEditorFileUtils::SaveCurrentLevel(),TEXT("Current level save requested."),World->GetPathName());
}

#undef LOCTEXT_NAMESPACE
