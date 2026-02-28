// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Strategy101 : ModuleRules
{
    public Strategy101(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore"
        });

        PrivateDependencyModuleNames.AddRange(new string[] { });

        PublicIncludePaths.AddRange(new string[] {
            "Strategy101/Public",
            "Strategy101/Public/Grid",
            "Strategy101/Public/Units",
            "Strategy101/Public/AI",
            "Strategy101/Public/GameLogic",
            "Strategy101/Public/UI"
        });
    }
}