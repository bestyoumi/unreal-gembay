// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWidget.h"

/**
 * Interface for modular GemBay actions that can be plugged into the dashboard.
 */
class IGemBayAction : public TSharedFromThis<IGemBayAction>
{
public:
	virtual ~IGemBayAction() {}

	/** Returns the unique name of this action */
	virtual FName GetActionName() const = 0;

	/** Returns the display name of this action for the UI */
	virtual FText GetDisplayName() const = 0;

	/** Returns the widget to be displayed in the dashboard for this action */
	virtual TSharedRef<SWidget> GetWidget() = 0;

	/** Optional initialization logic */
	virtual void Initialize() {}
};
