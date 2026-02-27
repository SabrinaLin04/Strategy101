// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
public class Strategy101 : ModuleRules
{
	public Strategy101(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

			PublicIncludePaths.AddRange(new string[] {
				"Strategy101/Grid",
				"Strategy101/Units",
				"Strategy101/AI",
				"Strategy101/GameLogic",
				"Strategy101/UI"
				});
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}

    
}

