// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GemBayAction.h"

/**
 * Action for dynamically loaded Gemini skills.
 */
class FGemBayDynamicSkillAction : public IGemBayAction
{
public:
	FGemBayDynamicSkillAction(const FString& InSkillName, const FString& InDescription, const FString& InUsageHints);

	virtual FName GetActionName() const override;
	virtual FText GetDisplayName() const override;
	virtual FText GetDescription() const override;
	virtual TSharedRef<SWidget> GetWidget() override;

private:
	FString SkillName;
	FString Description;
	FString UsageHints;
};

/**
 * Action for AI-driven material generation and interops.
 */
class FGemBayMaterialAIAction : public IGemBayAction
{
public:
	virtual FName GetActionName() const override { return "MaterialAI"; }
	virtual FText GetDisplayName() const override;
	virtual FText GetDescription() const override;
	virtual TSharedRef<SWidget> GetWidget() override;
};

/**
 * Action for the Python Console Bridge.
 */
class FGemBayPythonAction : public IGemBayAction
{
public:
	virtual FName GetActionName() const override { return "PythonBridge"; }
	virtual FText GetDisplayName() const override;
	virtual FText GetDescription() const override;
	virtual TSharedRef<SWidget> GetWidget() override;
};
