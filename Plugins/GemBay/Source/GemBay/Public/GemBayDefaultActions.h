// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GemBayAction.h"

/**
 * Action for managing and displaying installed Gemini skills.
 */
class FGemBaySkillsAction : public IGemBayAction
{
public:
	virtual FName GetActionName() const override { return "Skills"; }
	virtual FText GetDisplayName() const override;
	virtual TSharedRef<SWidget> GetWidget() override;
	virtual void Initialize() override;

private:
	struct FGemBaySkillInfo
	{
		FString Name;
		FString Description;
	};

	void RefreshSkills();
	TArray<FGemBaySkillInfo> AvailableSkills;
};

/**
 * Action for AI-driven material generation and interops.
 */
class FGemBayMaterialAIAction : public IGemBayAction
{
public:
	virtual FName GetActionName() const override { return "MaterialAI"; }
	virtual FText GetDisplayName() const override;
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
	virtual TSharedRef<SWidget> GetWidget() override;
};
