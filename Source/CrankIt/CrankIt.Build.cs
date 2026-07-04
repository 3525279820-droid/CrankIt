// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.IO;

public class CrankIt : ModuleRules
{
	public CrankIt(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", 
			"EnhancedInput", "UMG", "AudioMixer", "LevelSequence", "MovieScene" });

		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore", "AssetRegistry" }); // ClassificationImageLibrary 扫描 Content 纹理

		// 模块根 + 特性目录：支持 #include "ClassName.h" 而不写完整相对路径
		PrivateIncludePaths.Add(ModuleDirectory);

		string[] FeatureIncludeDirs = new string[]
		{
			"Core",
			"Gameplay/GameMode",
			"Gameplay/Subsystems",
			"Player",
			"Input",
			"Narrative",
			"Subtitle",
			"Audio",
			"Enemies",
			"UI",
			"UI/MainMenu",
			"UI/Tutorial",
			"UI/Sound",
			"World/Computer",
			"World/Environment",
			"World/Camera",
			"World/Doors",
			"World/Lighting",
			"Interactables/MineConsole",
			"Interactables/Battery",
			"Interactables/Battery/Components",
			"Terminal",
			"Terminal/Host",
			"Terminal/Display",
			"Terminal/Routing",
			"Terminal/Actions",
			"Terminal/Data",
			"Terminal/MiniGames",
			"Terminal/Battery",
		};

		foreach (string RelPath in FeatureIncludeDirs)
		{
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, RelPath));
		}

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
