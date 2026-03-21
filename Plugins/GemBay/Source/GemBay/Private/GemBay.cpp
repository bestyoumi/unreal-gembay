// Copyright Epic Games, Inc. All Rights Reserved.

#include "GemBay.h"
#include "GemBayStyle.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "HAL/PlatformApplicationMisc.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformProcess.h"
#include "IPythonScriptPlugin.h"
#include "ILiveCodingModule.h"
#include "Misc/CoreDelegates.h"
#include "HAL/PlatformTime.h"
#include "Framework/Text/SyntaxHighlighterTextLayoutMarshaller.h"
#include "Framework/Text/SyntaxTokenizer.h"
#include "Framework/Text/SlateTextRun.h"

class FPythonSyntaxHighlighter : public FSyntaxHighlighterTextLayoutMarshaller
{
public:
	static TSharedRef<FPythonSyntaxHighlighter> Create()
	{
		TArray<FSyntaxTokenizer::FRule> TokenizerRules;

		// Basic Python Keywords
		TArray<FString> Keywords = {
			TEXT("and"), TEXT("as"), TEXT("assert"), TEXT("break"), TEXT("class"), TEXT("continue"), TEXT("def"),
			TEXT("del"), TEXT("elif"), TEXT("else"), TEXT("except"), TEXT("False"), TEXT("finally"), TEXT("for"),
			TEXT("from"), TEXT("global"), TEXT("if"), TEXT("import"), TEXT("in"), TEXT("is"), TEXT("lambda"),
			TEXT("None"), TEXT("nonlocal"), TEXT("not"), TEXT("or"), TEXT("pass"), TEXT("raise"), TEXT("return"),
			TEXT("True"), TEXT("try"), TEXT("while"), TEXT("with"), TEXT("yield")
		};

		for (const FString& Keyword : Keywords)
		{
			TokenizerRules.Emplace(FSyntaxTokenizer::FRule(Keyword));
		}

		return MakeShareable(new FPythonSyntaxHighlighter(FSyntaxTokenizer::Create(TokenizerRules)));
	}

protected:
	FPythonSyntaxHighlighter(TSharedPtr<FSyntaxTokenizer> InTokenizer)
		: FSyntaxHighlighterTextLayoutMarshaller(InTokenizer)
	{
	}

	virtual void ParseTokens(const FString& SourceString, FTextLayout& TargetTextLayout, TArray<ISyntaxTokenizer::FTokenizedLine> TokenizedLines) override
	{
		TArray<FTextLayout::FNewLineData> LinesToAdd;
		LinesToAdd.Reserve(TokenizedLines.Num());

		FTextBlockStyle NormalStyle = FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("NormalText");
		NormalStyle.SetFont(FCoreStyle::GetDefaultFontStyle("Mono", 10));
		NormalStyle.SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.9f, 0.9f))); 

		FTextBlockStyle KeywordStyle = NormalStyle;
		KeywordStyle.SetColorAndOpacity(FSlateColor(FLinearColor(0.2f, 0.6f, 0.9f))); 

		FTextBlockStyle StringStyle = NormalStyle;
		StringStyle.SetColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.6f, 0.2f))); 

		FTextBlockStyle CommentStyle = NormalStyle;
		CommentStyle.SetColorAndOpacity(FSlateColor(FLinearColor(0.4f, 0.7f, 0.4f))); 

		for (const ISyntaxTokenizer::FTokenizedLine& TokenizedLine : TokenizedLines)
		{
			TSharedRef<FString> ModelString = MakeShareable(new FString());
			TArray<TSharedRef<IRun>> Runs;

			for (const ISyntaxTokenizer::FToken& Token : TokenizedLine.Tokens)
			{
				FTextBlockStyle TokenStyle = NormalStyle;
				FString TokenString = SourceString.Mid(Token.Range.BeginIndex, Token.Range.Len());

				const FTextRange ModelRange(ModelString->Len(), ModelString->Len() + TokenString.Len());
				ModelString->Append(TokenString);

				if (Token.Type == ISyntaxTokenizer::ETokenType::Syntax)
				{
					TokenStyle = KeywordStyle;
				}
				else if (TokenString.StartsWith(TEXT("\"")) || TokenString.StartsWith(TEXT("\'")))
				{
					TokenStyle = StringStyle;
				}
				else if (TokenString.StartsWith(TEXT("#")))
				{
					TokenStyle = CommentStyle;
				}

				Runs.Add(FSlateTextRun::Create(FRunInfo(), ModelString, TokenStyle, ModelRange));
			}

			LinesToAdd.Emplace(FTextLayout::FNewLineData(MoveTemp(ModelString), MoveTemp(Runs)));
		}

		TargetTextLayout.AddLines(LinesToAdd);
	}
};

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
}

void FGemBayModule::ShutdownModule()
{
	if (FSlateApplication::IsInitialized())
	{
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(GemBayTabName);
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

	// Extend Play ToolBar as specified in GEMINI.md
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

	MenuBuilder.AddMenuEntry(
		LOCTEXT("RefreshSkillsLabel", "Refresh Skills List"),
		LOCTEXT("RefreshSkillsTooltip", "Scans the project for newly installed agent skills."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FGemBayModule::RefreshSkills))
	);

	MenuBuilder.AddMenuSeparator();

	MenuBuilder.BeginSection("Skills", LOCTEXT("SkillsSection", "Agent Skills List"));
	
	RefreshSkills();
	for (const FGemBaySkillInfo& Skill : AvailableSkills)
	{
		MenuBuilder.AddMenuEntry(
			FText::FromString(Skill.Name),
			FText::FromString(Skill.Description),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([Skill]() {
				// Launch Gemini with a prompt for this skill
				FString Command = FString::Printf(TEXT("/k gemini \"I want to use the %s skill. What can you do?\""), *Skill.Name);
				FPlatformProcess::LaunchFileInDefaultExternalApplication(TEXT("cmd.exe"), *Command);
			}))
		);
	}

	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

void FGemBayModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(GemBayTabName);
}

void FGemBayModule::RefreshSkills()
{
	AvailableSkills.Empty();
	FString SkillsDir = FPaths::ProjectDir() / TEXT(".gemini/skills/");
	
	TArray<FString> SkillFolders;
	IFileManager::Get().FindFiles(SkillFolders, *(SkillsDir / TEXT("*")), false, true);

	for (const FString& Folder : SkillFolders)
	{
		FGemBaySkillInfo Info;
		Info.Name = Folder;
		
		FString SkillPath = SkillsDir / Folder / TEXT("SKILL.md");
		FString Content;
		if (FFileHelper::LoadFileToString(Content, *SkillPath))
		{
			int32 DescStart = Content.Find(TEXT("description:"));
			if (DescStart != INDEX_NONE)
			{
				DescStart += 12;
				int32 LineEnd = Content.Find(TEXT("\n"), ESearchCase::IgnoreCase, ESearchDir::FromStart, DescStart);
				if (LineEnd != INDEX_NONE)
				{
					Info.Description = Content.Mid(DescStart, LineEnd - DescStart).TrimStartAndEnd();
				}
			}
		}
		AvailableSkills.Add(Info);
	}
}

void FGemBayModule::GenerateMaterial(const FString& Name, const FString& JsonParams)
{
	if (IPythonScriptPlugin::Get()->IsPythonAvailable())
	{
		FString PythonCmd = FString::Printf(TEXT("import material_gen; import importlib; importlib.reload(material_gen); mgr = material_gen.MaterialManager(); mgr.create_base_material('%s', %s)"), *Name, *JsonParams);
		IPythonScriptPlugin::Get()->ExecPythonCommand(*PythonCmd);
	}
}

TSharedRef<SDockTab> FGemBayModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	RefreshSkills();

	TSharedRef<SVerticalBox> SkillListWidget = SNew(SVerticalBox);

	SkillListWidget->AddSlot()
		.AutoHeight()
		.Padding(10)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("SkillsHeader", "Installed Agent Skills"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
		];

	for (const FGemBaySkillInfo& Skill : AvailableSkills)
	{
		SkillListWidget->AddSlot()
			.AutoHeight()
			.Padding(5, 5)
			[
				SNew(SBorder)
				.Padding(10)
				.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(FText::FromString(Skill.Name))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 5, 0, 0)
					[
						SNew(STextBlock)
						.Text(FText::FromString(Skill.Description))
						.AutoWrapText(true)
					]
				]
			];
	}

	TSharedRef<SVerticalBox> MainWidget = SNew(SVerticalBox);

	MainWidget->AddSlot()
		.AutoHeight()
		[
			SkillListWidget
		];

	MainWidget->AddSlot()
		.AutoHeight()
		.Padding(0, 20, 0, 10)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("AIHeader", "Smart Material AI"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
		];

	TSharedPtr<SEditableTextBox> MaterialDescBox;

	MainWidget->AddSlot()
		.AutoHeight()
		.Padding(5, 5)
		[
			SNew(SBorder)
			.Padding(10)
			.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("AIInputLabel", "Describe the material you want to create:"))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 5, 0, 10)
				[
					SAssignNew(MaterialDescBox, SEditableTextBox)
					.HintText(LOCTEXT("AIHint", "e.g., a glowing translucent blue glass with high refraction"))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Right)
				[
					SNew(SButton)
					.Text(LOCTEXT("AIGenBtn", "Generate with AI"))
					.OnClicked_Lambda([MaterialDescBox]()
					{
						FString Description = MaterialDescBox->GetText().ToString();
						if (Description.IsEmpty())
						{
							FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("EmptyDesc", "Please provide a description first."));
							return FReply::Handled();
						}

						if (IPythonScriptPlugin::Get()->IsPythonAvailable())
						{
							FString PythonCmd = FString::Printf(TEXT("import ue_bridge; ue_bridge.run_command('I want to generate a material based on this description: \"%s\". Please use the ue-material-generator skill to create it.')"), *Description);
							IPythonScriptPlugin::Get()->ExecPythonCommand(*PythonCmd);
						}
						
						return FReply::Handled();
					})
				]
			]
		];

	MainWidget->AddSlot()
		.AutoHeight()
		.Padding(0, 20, 0, 10)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("InteropsHeader", "Materials System Interops"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
		];

	MainWidget->AddSlot()
		.AutoHeight()
		.Padding(5, 5)
		[
			SNew(SBorder)
			.Padding(10)
			.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("MaterialDesc", "Generate a sample material asset using the Unreal Python API."))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(10, 0, 0, 0)
				[
					SNew(SButton)
					.Text(LOCTEXT("CreateMaterialBtn", "Create Sample Material"))
					.OnClicked_Lambda([]()
					{
						if (IPythonScriptPlugin::Get()->IsPythonAvailable())
						{
							FString PythonCmd = TEXT("import create_material; import importlib; importlib.reload(create_material); create_material.create_sample_material()");
							IPythonScriptPlugin::Get()->ExecPythonCommand(*PythonCmd);
						}
						else
						{
							FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("PythonMissing", "Python is not available in this editor instance."));
						}
						return FReply::Handled();
					})
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(10, 0, 0, 0)
				[
					SNew(SButton)
					.Text(LOCTEXT("CreateMaterialInstanceBtn", "Create Material Instance"))
					.OnClicked_Lambda([]()
					{
						if (IPythonScriptPlugin::Get()->IsPythonAvailable())
						{
							FString PythonCmd = TEXT("import create_material; import importlib; importlib.reload(create_material); create_material.create_sample_material_instance()");
							IPythonScriptPlugin::Get()->ExecPythonCommand(*PythonCmd);
						}
						else
						{
							FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("PythonMissing", "Python is not available in this editor instance."));
						}
						return FReply::Handled();
					})
				]
			]
		];

	MainWidget->AddSlot()
		.AutoHeight()
		.Padding(0, 20, 0, 10)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("PythonBridgeHeader", "Python Console Bridge ⚡"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
		];

	TSharedPtr<SMultiLineEditableTextBox> PythonScriptBox;

	MainWidget->AddSlot()
		.AutoHeight()
		.Padding(5, 5)
		[
			SNew(SBorder)
			.Padding(10)
			.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 0, 0, 5)
				[
					SAssignNew(PythonScriptBox, SMultiLineEditableTextBox)
					.HintText(LOCTEXT("PythonHint", "Enter Python script here..."))
					.Marshaller(FPythonSyntaxHighlighter::Create())
					.AutoWrapText(true)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("BridgeDesc", "Run scripts rapidly in the current process."))
						.Font(FCoreStyle::GetDefaultFontStyle("Italic", 8))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(5, 0)
					[
						SNew(SButton)
						.Text(LOCTEXT("ClearBtn", "Clear"))
						.OnClicked_Lambda([PythonScriptBox]()
						{
							PythonScriptBox->SetText(FText::GetEmpty());
							return FReply::Handled();
						})
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(5, 0)
					[
						SNew(SButton)
						.Text(LOCTEXT("CopyBtn", "Copy"))
						.OnClicked_Lambda([PythonScriptBox]()
						{
							FPlatformApplicationMisc::ClipboardCopy(*PythonScriptBox->GetText().ToString());
							return FReply::Handled();
						})
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(5, 0)
					[
						SNew(SButton)
						.Text(LOCTEXT("RunBtn", "Execute"))
						.OnClicked_Lambda([PythonScriptBox]()
						{
							FString Script = PythonScriptBox->GetText().ToString();
							if (!Script.IsEmpty() && IPythonScriptPlugin::Get()->IsPythonAvailable())
							{
								IPythonScriptPlugin::Get()->ExecPythonCommand(*Script);
							}
							return FReply::Handled();
						})
					]
				]
			]
		];

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SBox)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Padding(10)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					MainWidget
				]
			]
		];
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGemBayModule, GemBay)
