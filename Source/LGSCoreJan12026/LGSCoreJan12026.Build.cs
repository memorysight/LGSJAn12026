using UnrealBuildTool;

//9_3_Updates for Unreal To WebSockets->SpringBoot Communication!
public class LGSCoreJan12026 : ModuleRules
{
	public LGSCoreJan12026(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore", 
			"WebSockets",    
			"Json",          
			"JsonUtilities", 
			"Niagara",       
			"EnhancedInput"  
		});
		
	}
}
