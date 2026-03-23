// Copyright Epic Games, Inc. All Rights Reserved.

#include "GemBayDefaultActions.h"
#include "GemBayStyle.h"
#include "Misc/MessageDialog.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "HAL/PlatformApplicationMisc.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformProcess.h"
#include "IPythonScriptPlugin.h"
#include "Framework/Text/SyntaxHighlighterTextLayoutMarshaller.h"
#include "Framework/Text/SyntaxTokenizer.h"
#include "Framework/Text/SlateTextRun.h"

#define LOCTEXT_NAMESPACE "FGemBayModule"

/** Python Syntax Highlighter implementation (Moved from GemBay.cpp) */
class FPythonSyntaxHighlighter : public FSyntaxHighlighterTextLayoutMarshaller
{
public:
	static TSharedRef<FPythonSyntaxHighlighter> Create()
	{
		TArray<FSyntaxTokenizer::FRule> TokenizerRules;
		TArray<FString> Symbols = {
			TEXT("("), TEXT(")"), TEXT("["), TEXT("]"), TEXT("{"), TEXT("}"), TEXT(":"), TEXT(","), TEXT("."), TEXT(";"),
			TEXT("+"), TEXT("-"), TEXT("*"), TEXT("/"), TEXT("%"), TEXT("="), TEXT("<"), TEXT(">"), TEXT("!"), TEXT("&"), TEXT("|"), TEXT("^"),
			TEXT("\""), TEXT("\'"), TEXT("#"), TEXT("@"), TEXT(" ")
		};
		for (const FString& Symbol : Symbols) { TokenizerRules.Emplace(FSyntaxTokenizer::FRule(Symbol)); }
		return MakeShareable(new FPythonSyntaxHighlighter(FSyntaxTokenizer::Create(TokenizerRules)));
	}

protected:
	FPythonSyntaxHighlighter(TSharedPtr<FSyntaxTokenizer> InTokenizer) : FSyntaxHighlighterTextLayoutMarshaller(InTokenizer) {}

	virtual void ParseTokens(const FString& SourceString, FTextLayout& TargetTextLayout, TArray<ISyntaxTokenizer::FTokenizedLine> TokenizedLines) override
	{
		TArray<FTextLayout::FNewLineData> LinesToAdd;
		LinesToAdd.Reserve(TokenizedLines.Num());

		FTextBlockStyle NormalStyle = FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("NormalText");
		NormalStyle.SetFont(FCoreStyle::GetDefaultFontStyle("Mono", 10));
		NormalStyle.SetColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.85f, 0.85f))); 

		FTextBlockStyle KeywordStyle = NormalStyle; KeywordStyle.SetColorAndOpacity(FSlateColor(FLinearColor(0.2f, 0.6f, 0.9f))); 
		FTextBlockStyle StringStyle = NormalStyle; StringStyle.SetColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.6f, 0.3f))); 
		FTextBlockStyle CommentStyle = NormalStyle; CommentStyle.SetColorAndOpacity(FSlateColor(FLinearColor(0.4f, 0.7f, 0.4f))); 
		FTextBlockStyle BuiltinStyle = NormalStyle; BuiltinStyle.SetColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.4f, 0.8f))); 
		FTextBlockStyle DecoratorStyle = NormalStyle; DecoratorStyle.SetColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.8f, 0.4f))); 

		static const TSet<FString> Keywords = {
			TEXT("and"), TEXT("as"), TEXT("assert"), TEXT("async"), TEXT("await"), TEXT("break"), TEXT("class"), 
			TEXT("continue"), TEXT("def"), TEXT("del"), TEXT("elif"), TEXT("else"), TEXT("except"), TEXT("False"), 
			TEXT("finally"), TEXT("for"), TEXT("from"), TEXT("global"), TEXT("if"), TEXT("import"), TEXT("in"), 
			TEXT("is"), TEXT("lambda"), TEXT("None"), TEXT("nonlocal"), TEXT("not"), TEXT("or"), TEXT("pass"), 
			TEXT("raise"), TEXT("return"), TEXT("True"), TEXT("try"), TEXT("while"), TEXT("with"), TEXT("yield")
		};

		static const TSet<FString> Builtins = {
			TEXT("print"), TEXT("len"), TEXT("range"), TEXT("int"), TEXT("str"), TEXT("float"), TEXT("list"), 
			TEXT("dict"), TEXT("set"), TEXT("tuple"), TEXT("super"), TEXT("self"), TEXT("cls"), TEXT("unreal"),
			TEXT("type"), TEXT("dir"), TEXT("help"), TEXT("input"), TEXT("open"), TEXT("sum"), TEXT("min"), TEXT("max")
		};

		for (const ISyntaxTokenizer::FTokenizedLine& TokenizedLine : TokenizedLines)
		{
			TSharedRef<FString> ModelString = MakeShareable(new FString());
			TArray<TSharedRef<IRun>> Runs;
			bool bInsideString = false; TCHAR StringQuote = 0; bool bInsideComment = false;

			for (const ISyntaxTokenizer::FToken& Token : TokenizedLine.Tokens)
			{
				FString TokenString = SourceString.Mid(Token.Range.BeginIndex, Token.Range.Len());
				const FTextRange ModelRange(ModelString->Len(), ModelString->Len() + TokenString.Len());
				ModelString->Append(TokenString);
				FTextBlockStyle TokenStyle = NormalStyle;

				if (bInsideComment) { TokenStyle = CommentStyle; }
				else if (bInsideString) { TokenStyle = StringStyle; if (TokenString.Len() == 1 && TokenString[0] == StringQuote) { bInsideString = false; } }
				else {
					if (TokenString == TEXT("#")) { bInsideComment = true; TokenStyle = CommentStyle; }
					else if (TokenString == TEXT("\"") || TokenString == TEXT("\'")) { bInsideString = true; StringQuote = TokenString[0]; TokenStyle = StringStyle; }
					else if (TokenString == TEXT("@")) { TokenStyle = DecoratorStyle; }
					else if (Token.Type == ISyntaxTokenizer::ETokenType::Literal) {
						FString TrimmedToken = TokenString.TrimStartAndEnd();
						if (Keywords.Contains(TrimmedToken)) { TokenStyle = KeywordStyle; }
						else if (Builtins.Contains(TrimmedToken)) { TokenStyle = BuiltinStyle; }
					}
				}
				Runs.Add(FSlateTextRun::Create(FRunInfo(), ModelString, TokenStyle, ModelRange));
			}
			LinesToAdd.Emplace(FTextLayout::FNewLineData(MoveTemp(ModelString), MoveTemp(Runs)));
		}
		TargetTextLayout.AddLines(LinesToAdd);
	}
};

/** FGemBayDynamicSkillAction */
FGemBayDynamicSkillAction::FGemBayDynamicSkillAction(const FString& InSkillName, const FString& InDescription, const FString& InUsageHints)
	: SkillName(InSkillName), Description(InDescription), UsageHints(InUsageHints)
{
}

FName FGemBayDynamicSkillAction::GetActionName() const { return FName(*SkillName); }
FText FGemBayDynamicSkillAction::GetDisplayName() const { return FText::FromString(SkillName); }
FText FGemBayDynamicSkillAction::GetDescription() const { return FText::FromString(Description); }

TSharedRef<SWidget> FGemBayDynamicSkillAction::GetWidget()
{
	TSharedRef<SVerticalBox> HintsBox = SNew(SVerticalBox);
	
	TArray<FString> Lines;
	UsageHints.ParseIntoArrayLines(Lines, true);
	
	bool bInCodeBlock = false;
	FString CurrentCodeBlock;

	auto FlushCodeBlock = [&]() {
		if (!CurrentCodeBlock.IsEmpty())
		{
			HintsBox->AddSlot().AutoHeight().Padding(10, 5) [
				SNew(SBorder).Padding(10).BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.DarkGroupBorder")) [
					SNew(STextBlock).Text(FText::FromString(CurrentCodeBlock)).Font(FCoreStyle::GetDefaultFontStyle("Mono", 10)).ColorAndOpacity(FLinearColor(0.6f, 0.8f, 0.6f)).AutoWrapText(true)
				]
			];
			CurrentCodeBlock.Empty();
		}
	};

	for (const FString& Line : Lines)
	{
		FString TrimmedLine = Line.TrimStartAndEnd();

		if (TrimmedLine.StartsWith(TEXT("```")))
		{
			if (bInCodeBlock)
			{
				bInCodeBlock = false;
				FlushCodeBlock();
			}
			else
			{
				bInCodeBlock = true;
			}
			continue;
		}

		if (bInCodeBlock)
		{
			CurrentCodeBlock += Line + TEXT("\n");
			continue;
		}

		if (TrimmedLine.IsEmpty()) continue;

		if (TrimmedLine.StartsWith(TEXT("###")))
		{
			HintsBox->AddSlot().AutoHeight().Padding(0, 10, 0, 5) [
				SNew(STextBlock).Text(FText::FromString(TrimmedLine.RightChop(3).TrimStartAndEnd())).Font(FCoreStyle::GetDefaultFontStyle("Bold", 11)).ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f))
			];
		}
		else if (TrimmedLine.StartsWith(TEXT("##")))
		{
			HintsBox->AddSlot().AutoHeight().Padding(0, 15, 0, 5) [
				SNew(STextBlock).Text(FText::FromString(TrimmedLine.RightChop(2).TrimStartAndEnd())).Font(FCoreStyle::GetDefaultFontStyle("Bold", 13)).ColorAndOpacity(FSlateColor(FLinearColor::White))
			];
		}
		else if (TrimmedLine.StartsWith(TEXT("#")))
		{
			HintsBox->AddSlot().AutoHeight().Padding(0, 20, 0, 10) [
				SNew(STextBlock).Text(FText::FromString(TrimmedLine.RightChop(1).TrimStartAndEnd())).Font(FCoreStyle::GetDefaultFontStyle("Bold", 16)).ColorAndOpacity(FSlateColor(FLinearColor::White))
			];
		}
		else if (TrimmedLine.StartsWith(TEXT("- ")) || TrimmedLine.StartsWith(TEXT("* ")))
		{
			HintsBox->AddSlot().AutoHeight().Padding(15, 2, 0, 2) [
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 5, 0) [ SNew(STextBlock).Text(FText::FromString(TEXT("•"))) ]
				+ SHorizontalBox::Slot().FillWidth(1.0f) [ SNew(STextBlock).Text(FText::FromString(TrimmedLine.RightChop(2))).AutoWrapText(true) ]
			];
		}
		else
		{
			HintsBox->AddSlot().AutoHeight().Padding(0, 2, 0, 2) [
				SNew(STextBlock).Text(FText::FromString(TrimmedLine)).AutoWrapText(true)
			];
		}
	}
	if (bInCodeBlock) { FlushCodeBlock(); }

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(10) [ 
			SNew(STextBlock).Text(GetDisplayName()).Font(FCoreStyle::GetDefaultFontStyle("Bold", 16)) 
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(10, 5) [ 
			SNew(STextBlock).Text(GetDescription()).AutoWrapText(true) 
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(10, 15, 10, 5) [ 
			SNew(STextBlock).Text(LOCTEXT("UsageHintsHeader", "Usage Hints:")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 12)) 
		]
		+ SVerticalBox::Slot().FillHeight(1.0f).Padding(10, 5) [ 
			SNew(SBorder).Padding(10).BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot() [
					HintsBox
				]
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(10).HAlign(HAlign_Right) [
			SNew(SButton).Text(FText::Format(LOCTEXT("LaunchAIForSkill", "Launch AI for {0}"), GetDisplayName()))
			.OnClicked_Lambda([this]() {
				FString Command = FString::Printf(TEXT("/k gemini \"I want to use the %s skill. What can you do?\""), *SkillName);
				FPlatformProcess::LaunchFileInDefaultExternalApplication(TEXT("cmd.exe"), *Command);
				return FReply::Handled();
			})
		];
}

/** FGemBayMaterialAIAction */
FText FGemBayMaterialAIAction::GetDisplayName() const { return LOCTEXT("MaterialAIActionName", "Smart Material AI"); }
FText FGemBayMaterialAIAction::GetDescription() const { return LOCTEXT("MaterialAIActionDesc", "Generate PBR materials using AI descriptions via Remote Control."); }

TSharedRef<SWidget> FGemBayMaterialAIAction::GetWidget()
{
	TSharedPtr<SEditableTextBox> MaterialDescBox;
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(10) [ 
			SNew(STextBlock).Text(GetDisplayName()).Font(FCoreStyle::GetDefaultFontStyle("Bold", 16)) 
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(10, 5) [ 
			SNew(STextBlock).Text(GetDescription()).AutoWrapText(true) 
		]
		+ SVerticalBox::Slot().FillHeight(1.0f).Padding(10, 15, 10, 5) [
			SNew(SScrollBox)
			+ SScrollBox::Slot().Padding(0, 0, 0, 10) [
				SNew(SBorder).Padding(10).BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight() [ SNew(STextBlock).Text(LOCTEXT("AIInputLabel", "Describe the material you want to create:")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 12)) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 10) [ SAssignNew(MaterialDescBox, SEditableTextBox).HintText(LOCTEXT("AIHint", "e.g., a glowing translucent blue glass with high refraction")) ]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right) [
						SNew(SButton).Text(LOCTEXT("AIGenBtn", "Generate with AI"))
						.OnClicked_Lambda([MaterialDescBox]() {
							FString Description = MaterialDescBox->GetText().ToString();
							if (Description.IsEmpty()) { FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("EmptyDesc", "Please provide a description first.")); return FReply::Handled(); }
							if (IPythonScriptPlugin::Get()->IsPythonAvailable()) {
								FString PythonCmd = FString::Printf(TEXT("import ue_bridge; ue_bridge.run_command('I want to generate a material based on this description: \"%s\". Please use the ue-material-generator skill to create it.')"), *Description);
								IPythonScriptPlugin::Get()->ExecPythonCommand(*PythonCmd);
							}
							return FReply::Handled();
						})
					]
				]
			]
			+ SScrollBox::Slot().Padding(0, 10, 0, 5) [ SNew(STextBlock).Text(LOCTEXT("InteropsHeader", "Materials System Interops")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 12)) ]
			+ SScrollBox::Slot() [
				SNew(SBorder).Padding(10).BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center) [ SNew(STextBlock).Text(LOCTEXT("MaterialDesc", "Generate a sample material asset using the Unreal Python API.")) ]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10, 0, 0, 0) [
						SNew(SButton).Text(LOCTEXT("CreateMaterialBtn", "Create Sample Material"))
						.OnClicked_Lambda([]() {
							if (IPythonScriptPlugin::Get()->IsPythonAvailable()) { IPythonScriptPlugin::Get()->ExecPythonCommand(TEXT("import create_material; import importlib; importlib.reload(create_material); create_material.create_sample_material()")); }
							else { FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("PythonMissing", "Python is not available in this editor instance.")); }
							return FReply::Handled();
						})
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10, 0, 0, 0) [
						SNew(SButton).Text(LOCTEXT("CreateMaterialInstanceBtn", "Create Material Instance"))
						.OnClicked_Lambda([]() {
							if (IPythonScriptPlugin::Get()->IsPythonAvailable()) { IPythonScriptPlugin::Get()->ExecPythonCommand(TEXT("import create_material; import importlib; importlib.reload(create_material); create_material.create_sample_material_instance()")); }
							else { FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("PythonMissing", "Python is not available in this editor instance.")); }
							return FReply::Handled();
						})
					]
				]
			]
		];
}

/** FGemBayPythonAction */
FText FGemBayPythonAction::GetDisplayName() const { return LOCTEXT("PythonBridgeActionName", "Python Console Bridge ⚡"); }
FText FGemBayPythonAction::GetDescription() const { return LOCTEXT("PythonBridgeActionDesc", "Rapidly execute Python scripts directly in the active editor process."); }

TSharedRef<SWidget> FGemBayPythonAction::GetWidget()
{
	TSharedPtr<SMultiLineEditableTextBox> PythonScriptBox;
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(10) [ 
			SNew(STextBlock).Text(GetDisplayName()).Font(FCoreStyle::GetDefaultFontStyle("Bold", 16)) 
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(10, 5) [ 
			SNew(STextBlock).Text(GetDescription()).AutoWrapText(true) 
		]
		+ SVerticalBox::Slot().FillHeight(1.0f).Padding(10, 15, 10, 5) [
			SNew(SBorder).Padding(10).BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().FillHeight(1.0f).Padding(0, 0, 0, 10) [
					SAssignNew(PythonScriptBox, SMultiLineEditableTextBox).HintText(LOCTEXT("PythonHint", "Enter Python script here..."))
					.Marshaller(FPythonSyntaxHighlighter::Create()).AutoWrapText(true)
				]
				+ SVerticalBox::Slot().AutoHeight() [
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center) [ SNew(STextBlock).Text(LOCTEXT("BridgeDesc", "Run scripts rapidly in the current process.")).Font(FCoreStyle::GetDefaultFontStyle("Italic", 9)) ]
					+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0) [ SNew(SButton).Text(LOCTEXT("ClearBtn", "Clear")).OnClicked_Lambda([PythonScriptBox]() { PythonScriptBox->SetText(FText::GetEmpty()); return FReply::Handled(); }) ]
					+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0) [ SNew(SButton).Text(LOCTEXT("CopyBtn", "Copy")).OnClicked_Lambda([PythonScriptBox]() { FPlatformApplicationMisc::ClipboardCopy(*PythonScriptBox->GetText().ToString()); return FReply::Handled(); }) ]
					+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0) [
						SNew(SButton).Text(LOCTEXT("RunBtn", "Execute")).OnClicked_Lambda([PythonScriptBox]() {
							FString Script = PythonScriptBox->GetText().ToString();
							if (!Script.IsEmpty() && IPythonScriptPlugin::Get()->IsPythonAvailable()) { IPythonScriptPlugin::Get()->ExecPythonCommand(*Script); }
							return FReply::Handled();
						})
					]
				]
			]
		];
}

#undef LOCTEXT_NAMESPACE
