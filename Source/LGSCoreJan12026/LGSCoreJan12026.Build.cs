// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;


//1_30 update changes
public class LGSCoreJan12026 : ModuleRules
{
	public LGSCoreJan12026(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "Niagara" });
	}
}
