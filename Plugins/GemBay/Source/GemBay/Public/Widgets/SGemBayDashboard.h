// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

/**
 * The main dashboard widget that dynamically displays all registered GemBay actions.
 */
class SGemBayDashboard : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SGemBayDashboard) {}
	SLATE_END_ARGS()

	/** Constructs the widget */
	void Construct(const FArguments& InArgs);
};
