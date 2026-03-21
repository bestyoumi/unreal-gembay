// Copyright Epic Games, Inc. All Rights Reserved.

#include "Widgets/SGemBayDashboard.h"
#include "GemBaySubsystem.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
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
	const auto& Actions = Subsystem->GetRegisteredActions();

	for (const auto& Action : Actions)
	{
		MainWidget->AddSlot()
			.AutoHeight()
			.Padding(0, 0, 0, 20)
			[
				Action->GetWidget()
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
