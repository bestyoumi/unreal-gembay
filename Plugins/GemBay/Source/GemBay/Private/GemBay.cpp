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

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

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
		auto RegisterSkill = [this, Subsystem](TSharedRef<IGemBayAction> Action)
		{
			Subsystem->RegisterAction(Action);
			
			FGlobalTabmanager::Get()->RegisterNomadTabSpawner(Action->GetActionName(), FOnSpawnTab::CreateLambda([Action](const FSpawnTabArgs& SpawnTabArgs) {
				return SNew(SDockTab)
					.TabRole(ETabRole::NomadTab)
					[
						Action->GetWidget()
					];
			}))
			.SetDisplayName(Action->GetDisplayName())
			.SetMenuType(ETabSpawnerMenuType::Hidden); // Don't show in the standard Window menu directly
		};

		// 1. Register Hardcoded Native Actions
		RegisterSkill(MakeShared<FGemBayMaterialAIAction>());
		RegisterSkill(MakeShared<FGemBayPythonAction>());

		// 2. Dynamically Parse and Register Local Gemini Skills
		FString SkillsDir = FPaths::ProjectDir() / TEXT(".gemini/skills/");
		TArray<FString> SkillFolders;
		IFileManager::Get().FindFiles(SkillFolders, *(SkillsDir / TEXT("*")), false, true);

		for (const FString& Folder : SkillFolders)
		{
			FString SkillPath = SkillsDir / Folder / TEXT("SKILL.md");
			FString Content;
			if (FFileHelper::LoadFileToString(Content, *SkillPath))
			{
				FString SkillName = Folder;
				FString Description;
				FString UsageHints;

				// Parse Description from YAML frontmatter
				int32 DescStart = Content.Find(TEXT("description:"));
				if (DescStart != INDEX_NONE)
				{
					DescStart += 12;
					int32 LineEnd = Content.Find(TEXT("\n"), ESearchCase::IgnoreCase, ESearchDir::FromStart, DescStart);
					if (LineEnd != INDEX_NONE) 
					{ 
						Description = Content.Mid(DescStart, LineEnd - DescStart).TrimStartAndEnd(); 
					}
				}

				// The rest of the file is UsageHints (strip the frontmatter if present)
				int32 ContentStart = Content.Find(TEXT("---"), ESearchCase::IgnoreCase, ESearchDir::FromStart, 3);
				if (ContentStart != INDEX_NONE)
				{
					UsageHints = Content.Mid(ContentStart + 3).TrimStartAndEnd();
				}
				else
				{
					UsageHints = Content;
				}

				RegisterSkill(MakeShared<FGemBayDynamicSkillAction>(SkillName, Description, UsageHints));
			}
		}
	}
}

void FGemBayModule::ShutdownModule()
{
	if (FSlateApplication::IsInitialized())
	{
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(GemBayTabName);
		
		if (GEditor)
		{
			if (UGemBaySubsystem* Subsystem = GEditor->GetEditorSubsystem<UGemBaySubsystem>())
			{
				for (const auto& Action : Subsystem->GetRegisteredActions())
				{
					FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(Action->GetActionName());
				}
			}
		}
	}

	if (UObjectInitialized() && !IsEngineExitRequested())
	{
		UToolMenus::UnRegisterStartupCallback(this);
		UToolMenus::UnregisterOwner(this);
	}

	FGemBayStyle::Shutdown();
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
