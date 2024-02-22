// Copyright Alex Quevillon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "UtuPlugin/Core/Public/UtuPluginStyle.h"

class FUtuPluginCommands : public TCommands<FUtuPluginCommands>
{
public:

	FUtuPluginCommands()
		: TCommands<FUtuPluginCommands>(TEXT("UtuPlugin"), NSLOCTEXT("Contexts", "UtuPlugin", "UtuPlugin Plugin"), NAME_None, FUtuPluginStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
