// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class CrankIt : ModuleRules
{
	public CrankIt(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", 
			"EnhancedInput", "UMG", "AudioMixer", "LevelSequence", "MovieScene" });

		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// 模块根 + 已重构子目录：本模块 .cpp 编译时的 #include 搜索路径
		PrivateIncludePaths.Add(ModuleDirectory);
		PrivateIncludePaths.AddRange(new string[]
		{
			System.IO.Path.Combine(ModuleDirectory, "Narrative"),
			System.IO.Path.Combine(ModuleDirectory, "Subtitle"),
			System.IO.Path.Combine(ModuleDirectory, "Gameplay"),
			System.IO.Path.Combine(ModuleDirectory, "World", "Computer"),
			System.IO.Path.Combine(ModuleDirectory, "Terminal"),
			System.IO.Path.Combine(ModuleDirectory, "Terminal", "Host"),
			System.IO.Path.Combine(ModuleDirectory, "Terminal", "Display"),
			System.IO.Path.Combine(ModuleDirectory, "Terminal", "Routing"),
			System.IO.Path.Combine(ModuleDirectory, "Terminal", "Actions"),
			System.IO.Path.Combine(ModuleDirectory, "Terminal", "Data"),
			System.IO.Path.Combine(ModuleDirectory, "Terminal", "MiniGames"),
			System.IO.Path.Combine(ModuleDirectory, "Terminal", "Battery"),
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
