// Copyright Alex Quevillon. All Rights Reserved.

#include "UtuPlugin/Core/Public/UtuPluginCommands.h"

#define LOCTEXT_NAMESPACE "FUtuPluginModule"

void FUtuPluginCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "Utu Plugin", "Opens Utu Plugin Project Convertor's Window", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
