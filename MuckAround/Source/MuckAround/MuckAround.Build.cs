// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MuckAround : ModuleRules
{
	public MuckAround(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", 
			"CoreUObject",
			"Engine",
			"GameplayTags",
			"InputCore", 
			"HeadMountedDisplay", 
			"EnhancedInput",
			"Niagara",
			"UMG"
		});
	}
}
