// Copyright Epic Games, Inc. All Rights Reserved.

#include "Widgets/SGemBayDashboard.h"
#include "GemBaySubsystem.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Input/SButton.h"
#include "Framework/Docking/TabManager.h"
#include "Editor.h"

void SGemBayDashboard::Construct(const FArguments& InArgs)
{
	UGemBaySubsystem* Subsystem = GEditor->GetEditorSubsystem<UGemBaySubsystem>();
	if (!Subsystem)
	{
		ChildSlot [ SNew(STextBlock).Text(FText::FromString("GemBay Subsystem not found.")) ];
		return;
	}

	TSharedRef<SVerticalBox> MainWidget = SNew(SVerticalBox);
	
	MainWidget->AddSlot()
		.AutoHeight()
		.Padding(0, 0, 0, 20)
		[
			SNew(STextBlock)
			.Text(FText::FromString("GemBay Dashboard"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
		];

	const auto& Actions = Subsystem->GetRegisteredActions();

	for (const auto& Action : Actions)
	{
		MainWidget->AddSlot()
			.AutoHeight()
			.Padding(0, 0, 0, 10)
			[
				SNew(SButton)
				.OnClicked_Lambda([Action]() {
					FGlobalTabmanager::Get()->TryInvokeTab(Action->GetActionName());
					return FReply::Handled();
				})
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
							.Text(Action->GetDisplayName())
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0, 5, 0, 0)
						[
							SNew(STextBlock)
							.Text(Action->GetDescription())
							.AutoWrapText(true)
						]
					]
				]
			];
	}

	ChildSlot
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
