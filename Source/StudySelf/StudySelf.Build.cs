// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class StudySelf : ModuleRules
{
	public StudySelf(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"Paper2D",
			"HTTP",     
			"Json",     
			"JsonUtilities"
		});
		
		if (Target.Type == TargetType.Editor)
		{
			// 에디터에서만 사용하는 모듈들
			PublicDependencyModuleNames.AddRange(new string[] { "UnrealEd", "AssetTools" });
		}

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"StudySelf",
			"StudySelf/Variant_Horror",
			"StudySelf/Variant_Horror/UI",
			"StudySelf/Variant_Shooter",
			"StudySelf/Variant_Shooter/AI",
			"StudySelf/Variant_Shooter/UI",
			"StudySelf/Variant_Shooter/Weapons"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
