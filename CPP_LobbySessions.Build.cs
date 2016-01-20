// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class CPP_LobbySessions : ModuleRules
{
	public CPP_LobbySessions(TargetInfo Target)
	{
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "OnlineSubsystem", "OnlineSubsystemUtils" });

		DynamicallyLoadedModuleNames.AddRange(new string[] { "OnlineSubsystemNull" });
	}
}
