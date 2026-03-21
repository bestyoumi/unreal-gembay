// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "GemBayAction.h"
#include "GemBaySubsystem.generated.h"

/**
 * Subsystem to expose GemBay functionality to Remote Control and Blueprints
 */
UCLASS()
class GEMBAY_API UGemBaySubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	/** Triggers a Live Coding compilation */
	UFUNCTION(BlueprintCallable, Category = "GemBay")
	void TriggerLiveCoding();

	/** Generates a material from parameters via Python */
	UFUNCTION(BlueprintCallable, Category = "GemBay")
	void GenerateMaterial(const FString& Name, const FString& JsonParams);

	/** Registers a new action module for the dashboard */
	void RegisterAction(TSharedPtr<IGemBayAction> InAction);

	/** Returns all registered dashboard actions */
	const TArray<TSharedPtr<IGemBayAction>>& GetRegisteredActions() const { return RegisteredActions; }

private:
	/** Active modular actions for the dashboard */
	TArray<TSharedPtr<IGemBayAction>> RegisteredActions;
};
