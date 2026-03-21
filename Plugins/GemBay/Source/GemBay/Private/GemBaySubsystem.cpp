// Copyright Epic Games, Inc. All Rights Reserved.

#include "GemBaySubsystem.h"
#include "ILiveCodingModule.h"
#include "Modules/ModuleManager.h"
#include "IPythonScriptPlugin.h"

void UGemBaySubsystem::TriggerLiveCoding()
{
	ILiveCodingModule* LiveCoding = FModuleManager::GetModulePtr<ILiveCodingModule>(TEXT("LiveCoding"));
	if (LiveCoding && LiveCoding->IsEnabledForSession())
	{
		UE_LOG(LogTemp, Display, TEXT("GemBaySubsystem: Triggering Live Coding via Remote Control."));
		LiveCoding->Compile();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GemBaySubsystem: Failed to trigger Live Coding. Module not found or not enabled for session."));
	}
}

void UGemBaySubsystem::GenerateMaterial(const FString& Name, const FString& JsonParams)
{
	if (IPythonScriptPlugin::Get()->IsPythonAvailable())
	{
		UE_LOG(LogTemp, Display, TEXT("GemBaySubsystem: Generating material '%s' with params: %s"), *Name, *JsonParams);
		FString PythonCmd = FString::Printf(TEXT("import material_gen; import importlib; importlib.reload(material_gen); mgr = material_gen.MaterialManager(); mgr.create_base_material('%s', %s)"), *Name, *JsonParams);
		IPythonScriptPlugin::Get()->ExecPythonCommand(*PythonCmd);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GemBaySubsystem: Python is not available. Cannot generate material."));
	}
}
