// Copyright Alex Quevillon. All Rights Reserved.

#include "UtuPlugin/Core/Public/UtuPluginModule.h"
#include "UtuPlugin/Core/Public/UtuPluginStyle.h"
#include "UtuPlugin/Core/Public/UtuPluginCommands.h"
#include "UtuPlugin/Scripts/Public/UtuPluginLibrary.h"
#include "UtuPlugin/Scripts/Public/UtuPluginPaths.h"

#include "Misc/MessageDialog.h"
#include "ToolMenus.h"

#include "LevelEditor.h"
#include "Editor/UMGEditor/Public/WidgetBlueprint.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

static const FName UtuPluginTabName("UtuPlugin");

#define LOCTEXT_NAMESPACE "FUtuPluginModule"

void FUtuPluginModule::StartupModule()
{
	UtuPluginPaths::ConstructUtuPluginPaths();

	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FUtuPluginStyle::Initialize();
	FUtuPluginStyle::ReloadTextures();

	FUtuPluginCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FUtuPluginCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FUtuPluginModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FUtuPluginModule::RegisterMenus));
}

void FUtuPluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FUtuPluginStyle::Shutdown();

	FUtuPluginCommands::Unregister();
}

void FUtuPluginModule::PluginButtonClicked()
{
	FString Path = "/UtuPlugin/W_UtuPlugin";
	UWidgetBlueprint* Widget = Cast<UWidgetBlueprint>(StaticLoadObject(UWidgetBlueprint::StaticClass(), nullptr, *Path));
	if (Widget != nullptr) {
		UUtuPluginLibrary::OpenWidget(Widget, LOCTEXT("UtuPlugin_DisplayName", "Utu Plugin"));
	}
}

void FUtuPluginModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FUtuPluginCommands::Get().PluginAction, PluginCommands);
		}
	}

#if ENGINE_MAJOR_VERSION >= 5
	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FUtuPluginCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
#endif

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FUtuPluginCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUtuPluginModule, UtuPlugin)