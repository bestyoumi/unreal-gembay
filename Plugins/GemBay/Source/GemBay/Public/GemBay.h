// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

struct FGemBaySkillInfo
{
	FString Name;
	FString Description;
};

class FGemBayModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void RefreshSkills();
	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);

	/** Helper for generating materials via Python commands from external agents */
	void GenerateMaterial(const FString& Name, const FString& JsonParams);

private:
	void RegisterMenus();
	void PluginButtonClicked();
	TSharedRef<SWidget> GenerateGeminiMenu();
	void OnBeginFrame();

private:
	TArray<FGemBaySkillInfo> AvailableSkills;
};
