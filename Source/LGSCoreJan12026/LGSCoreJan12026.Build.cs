// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

//9_3 update changes for Unreal SpringBoot WebSocketsDTO 
public class LGSCoreJan12026 : ModuleRules
{
	public LGSCoreJan12026(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "Niagara" });
		
		PrivateDependencyModuleNames.AddRange(new string[] {
			"WebSockets" 
		});

		
	}
}
