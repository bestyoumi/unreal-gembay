// Copyright Epic Games, Inc. All Rights Reserved.

#include "GemBay.h"
#include "GemBayStyle.h"
#include "GemBaySubsystem.h"
#include "GemBayDefaultActions.h"
#include "Widgets/SGemBayDashboard.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Docking/TabManager.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "HAL/PlatformProcess.h"
#include "Misc/CoreDelegates.h"
#include "Editor.h"

static const FName GemBayTabName("GemBayAgentSkills");

#define LOCTEXT_NAMESPACE "FGemBayModule"

void FGemBayModule::StartupModule()
{
	FGemBayStyle::Initialize();

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(GemBayTabName, FOnSpawnTab::CreateRaw(this, &FGemBayModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FGemBayTabTitle", "GemBay - Agent Skills"))
		.SetTooltipText(LOCTEXT("FGemBayTabTooltip", "Open the GemBay Agent Skills dashboard."))
		.SetMenuType(ETabSpawnerMenuType::Enabled);

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FGemBayModule::RegisterMenus));

	// Safely register actions after the engine and GEditor are fully ready
	FCoreDelegates::OnPostEngineInit.AddRaw(this, &FGemBayModule::OnPostEngineInit);
}

void FGemBayModule::OnPostEngineInit()
{
	if (UGemBaySubsystem* Subsystem = GEditor->GetEditorSubsystem<UGemBaySubsystem>())
	{
		Subsystem->RegisterAction(MakeShared<FGemBaySkillsAction>());
		Subsystem->RegisterAction(MakeShared<FGemBayMaterialAIAction>());
		Subsystem->RegisterAction(MakeShared<FGemBayPythonAction>());
	}
}

void FGemBayModule::ShutdownModule()
{
	FGemBayStyle::Shutdown();
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(GemBayTabName);
}

void FGemBayModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window"))
	{
		FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
		Section.AddMenuEntry(
			"OpenGemBay",
			LOCTEXT("OpenGemBayLabel", "GemBay - Agent Skills"),
			LOCTEXT("OpenGemBayTooltip", "Open the GemBay dashboard."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateRaw(this, &FGemBayModule::PluginButtonClicked))
		);
	}

	if (UToolMenu* PlayToolBar = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar"))
	{
		FToolMenuSection& Section = PlayToolBar->FindOrAddSection("Play");
		Section.AddEntry(FToolMenuEntry::InitComboButton(
			"GeminiAgentCombo",
			FUIAction(),
			FOnGetContent::CreateRaw(this, &FGemBayModule::GenerateGeminiMenu),
			LOCTEXT("GeminiAgentLabel", "Gemini Agent"),
			LOCTEXT("GeminiAgentTooltip", "Access Gemini Agent tools and skills."),
			FSlateIcon(FGemBayStyle::GetStyleSetName(), "GemBay.AgentIcon")
		));
	}
}

TSharedRef<SWidget> FGemBayModule::GenerateGeminiMenu()
{
	FMenuBuilder MenuBuilder(true, nullptr);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("OpenConsoleLabel", "Open Interactive Console"),
		LOCTEXT("OpenConsoleTooltip", "Launches a full gemini session."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([]() {
			FPlatformProcess::LaunchFileInDefaultExternalApplication(TEXT("cmd.exe"), TEXT("/k gemini"));
		}))
	);

	MenuBuilder.AddMenuSeparator();

	// Note: Skill listing in menu could also be refactored to use Subsystem actions if needed
	MenuBuilder.BeginSection("Skills", LOCTEXT("SkillsSection", "Agent Skills List"));
	MenuBuilder.AddMenuEntry(
		LOCTEXT("RefreshSkillsLabel", "Refresh Skills List"),
		FText::GetEmpty(),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([]() { /* Logic handled by dashboard reload */ }))
	);
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

void FGemBayModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(GemBayTabName);
}

TSharedRef<SDockTab> FGemBayModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SGemBayDashboard)
		];
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGemBayModule, GemBay)
