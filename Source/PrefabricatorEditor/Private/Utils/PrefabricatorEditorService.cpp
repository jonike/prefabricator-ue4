//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Utils/PrefabricatorEditorService.h"

#include "Asset/PrefabricatorAsset.h"
#include "Asset/PrefabricatorAsset.h"
#include "Prefab/PrefabActor.h"
#include "Prefab/PrefabComponent.h"

#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "Editor.h"
#include "Editor/EditorEngine.h"
#include "EditorViewportClient.h"
#include "Engine/Selection.h"
#include "IContentBrowserSingleton.h"
#include "LevelEditor.h"
#include "LevelEditorViewport.h"
#include "ThumbnailRendering/SceneThumbnailInfo.h"

void FPrefabricatorEditorService::ParentActors(AActor* ParentActor, AActor* ChildActor)
{
	if (GEditor) {
		GEditor->ParentActors(ParentActor, ChildActor, NAME_None);
	}
}

void FPrefabricatorEditorService::SelectPrefabActor(AActor* PrefabActor)
{
	if (GEditor) {
		GEditor->SelectNone(true, true);
		GEditor->SelectActor(PrefabActor, true, true);
	}
}

void FPrefabricatorEditorService::GetSelectedActors(TArray<AActor*>& OutActors)
{
	if (GEditor) {
		USelection* SelectedActors = GEditor->GetSelectedActors();
		for (FSelectionIterator Iter(*SelectedActors); Iter; ++Iter)
		{
			// We only care about actors that are referenced in the world for literals, and also in the same level as this blueprint
			AActor* Actor = Cast<AActor>(*Iter);
			if (Actor)
			{
				OutActors.Add(Actor);
			}
		}
	}
}

int FPrefabricatorEditorService::GetNumSelectedActors()
{
	return GEditor ? GEditor->GetSelectedActorCount() : 0;
}

UPrefabricatorAsset* FPrefabricatorEditorService::CreatePrefabAsset()
{
	IContentBrowserSingleton& ContentBrowserSingleton = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser").Get();
	TArray<FString> SelectedFolders;
	ContentBrowserSingleton.GetSelectedPathViewFolders(SelectedFolders);
	FString PrefabFolder = SelectedFolders.Num() > 0 ? SelectedFolders[0] : "/Game";
	FString PrefabPath = PrefabFolder + "/Prefab";

	FString PackageName, AssetName;
	IAssetTools& AssetTools = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	AssetTools.CreateUniqueAssetName(*PrefabPath, TEXT(""), PackageName, AssetName);
	UPrefabricatorAsset* PrefabAsset = Cast<UPrefabricatorAsset>(AssetTools.CreateAsset(AssetName, PrefabFolder, UPrefabricatorAsset::StaticClass(), nullptr));

	UpdateThumbnail(PrefabAsset);

	ContentBrowserSingleton.SyncBrowserToAssets(TArray<UObject*>({ PrefabAsset }));

	return PrefabAsset;
}

void FPrefabricatorEditorService::UpdateThumbnail(UPrefabricatorAsset* PrefabAsset)
{
	if (GCurrentLevelEditingViewportClient) {
		IContentBrowserSingleton& ContentBrowserSingleton = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser").Get();
		TArray<FAssetData> AssetList({ FAssetData(PrefabAsset) });
		ContentBrowserSingleton.CaptureThumbnailFromViewport(GCurrentLevelEditingViewportClient->Viewport, AssetList);
	}
}

FVector FPrefabricatorEditorService::SnapToGrid(const FVector& InLocation)
{
	auto& Settings = GetDefault<ULevelEditorViewportSettings>()->SnapToSurface;
	if (GEditor) {
		float GridSize = GEditor->GetGridSize();
#define SNAP_TO_GRID(X) FMath::RoundToInt((X) / GridSize) * GridSize
		FVector SnappedLocation(
			SNAP_TO_GRID(InLocation.X),
			SNAP_TO_GRID(InLocation.Y),
			SNAP_TO_GRID(InLocation.Z));
#undef SNAP_TO_GRID
		return SnappedLocation;
	}
	else {
		return InLocation;
	}
}
