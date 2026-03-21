// Copyright Epic Games, Inc. All Rights Reserved.

#include "GemBayStyle.h"
#include "GemBay.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"
#include "Styling/AppStyle.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FGemBayStyle::StyleInstance = nullptr;

void FGemBayStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FGemBayStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FGemBayStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("GemBayStyle"));
	return StyleSetName;
}

TSharedRef< FSlateStyleSet > FGemBayStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("GemBayStyle"));
	return Style;
}

void FGemBayStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FGemBayStyle::Get()
{
	return *StyleInstance;
}
