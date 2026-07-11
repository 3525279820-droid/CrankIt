# CrankIt

基于 Unreal Engine 5.6 的固定视角探索解谜游戏原型。玩家在封闭矿道场景中切换四向机位进行探索，通过电池管理、矿用控制台操作、电脑终端命令解谜、声音方向探测与灯光驱赶等方式推进关卡流程。

## 技术栈

| 类别 | 技术 |
|------|------|
| 引擎 | Unreal Engine 5.6 |
| 语言 | C++（核心玩法逻辑）、Blueprint（Actor 子类与 UMG 资源） |
| 输入 | Enhanced Input |
| UI | UMG |
| 叙事 | Primary Data Asset |
| 过场 | Level Sequence |
| 音频 | AudioMixer（Submix 包络采样） |

**模块依赖**（`CrankIt.Build.cs`）：`Core`、`Engine`、`EnhancedInput`、`UMG`、`AudioMixer`、`LevelSequence`、`MovieScene`、`AssetRegistry`。

## 玩法概览

- **四向固定视角探索**：玩家以 `APlayerCamera` 为 Pawn，在 North / East / South / West 四个朝向间切换观察场景。
- **交互与物品**：鼠标悬停检测充电手柄与电池，`UBatteryHoldComponent` 负责拾取、持有与挂点跟随。
- **电池与控制台**：`ABattery` 支持三档电量显示与充放电；`AMineConsole` 提供手柄旋转充电、逐灯点亮与 EMP 灯光序列。
- **电脑终端解谜**：`AComputerScreenActor` 承载 `UTerminalWidget`，玩家输入命令推进主线，触发开门、重启、扫描修复等关卡行为。
- **终端小游戏**：分类（Classification）、校准（Calibration）、BOORDLE 猜词等子玩法，由 `UTerminalMiniGameHost` 统一管理输入模式切换。
- **声音探测**：`ASoundDetectorActor` 基于玩家摄像机位置与朝向，对场景中 `UAudioComponent` 进行锥形范围检测，并将强度映射至波形 UI。
- **怪物机制**：`AMonster` 在关卡解锁后于随机方位生成，定时向玩家逼近，可通过灯光触发驱赶。
- **教程与流程**：开场过场、可跳过教程、朝向触发字幕、EMP 教程完成后解锁正式关卡，由 `UCrankItIntroFlowSubsystem` 编排。

## 架构

项目以 **WorldSubsystem** 为核心组织关卡逻辑，避免将流程、叙事与交互耦合至单一 GameMode。

```
┌─────────────────────────────────────────────────────────────┐
│                      ACrankItGameMode                       │
│          （配置 Data Asset、教程类、过场 Tag）                │
└──────────────────────────┬──────────────────────────────────┘
                           │ BeginPlay 启动
                           ▼
┌─────────────────────────────────────────────────────────────┐
│              UCrankItIntroFlowSubsystem                     │
│   开场过场 · 教程 UI · 朝向事件 · 关卡解锁                   │
└───────┬──────────────────────────────┬──────────────────────┘
        │                              │
        ▼                              ▼
┌───────────────────┐      ┌──────────────────────────────┐
│ UCrankItGameplay  │      │ UCrankItNarrativeSubsystem   │
│ Subsystem         │      │ （字幕轨 · 终端文案 · 路由配置）│
│ （玩法事件总线）   │      └──────────────────────────────┘
└───────────────────┘
        │
        ▼
┌─────────────────────────────────────────────────────────────┐
│  UCrankItActorRegistry · UCrankItInputModeService           │
│  UCrankItUIService · USubtitleSubsystem                     │
└─────────────────────────────────────────────────────────────┘
```

**终端命令处理链路**：

```
玩家输入 → FTerminalCommandRouter（阶段校验 / 文本查表）
         → UTerminalActionDispatcher（按 ActionId 执行副作用）
         → UTerminalDisplayController（输出编排）
```

## 源码结构

C++ 源码位于 `Source/CrankIt/`，约 93 个源文件、7600+ 行。按功能域划分如下：

```
Source/CrankIt/
├── Core/                    # 关卡 Actor 注册与查询（UCrankItActorRegistry）
├── Gameplay/
│   ├── GameMode/            # ACrankItGameMode
│   └── Subsystems/          # IntroFlow、Gameplay 事件子系统
├── Player/                  # APlayerCamera、交互与电池持有组件
├── Input/                   # UCrankItInputModeService
├── Interactables/
│   ├── Battery/             # ABattery、槽位触发、灯光触发
│   └── MineConsole/         # AMineConsole
├── Terminal/
│   ├── Host/                # UTerminalWidget
│   ├── Routing/             # FTerminalCommandRouter
│   ├── Actions/             # UTerminalActionDispatcher
│   ├── Display/             # UTerminalDisplayController
│   ├── Data/                # UCrankItTerminalCommandData
│   ├── MiniGames/           # 分类 / 校准 / BOORDLE
│   └── Battery/             # 终端电池消耗门槛逻辑
├── Narrative/               # 叙事 Data Asset 与子系统
├── Subtitle/                # USubtitleSubsystem、USubtitleWidget
├── Audio/                   # ASoundDetectorActor
├── Enemies/                 # AMonster
├── UI/                      # UIService、主菜单、教程、波形 Widget
└── World/
    ├── Computer/            # AComputerScreenActor
    ├── Environment/         # ATunnel
    ├── Lighting/            # AEMPLight、AFlashTopLight
    ├── Doors/               # ADoubleAutoDoor
    └── Camera/              # ABgCamera
```

`CrankIt.Build.cs` 中为上述目录配置了 `PrivateIncludePaths`，支持 `#include "ClassName.h"` 形式引用。

## 内容资源

| 目录 | 说明 |
|------|------|
| `Content/BP/` | 核心 Actor 蓝图子类（`BP_PlayerCamera`、`BP_MineConsole`、`BP_Monster` 等） |
| `Content/DataAsset/` | `DA_CrankItNarrative`、`DA_CrankItTerminalCommands` |
| `Content/Input/` | Enhanced Input 映射上下文与 Input Action |
| `Content/Map/` | `TestLevel`（默认关卡）、`MainMenu` |
| `Content/Sequence/` | 开场与 Skip 教程过场序列 |
| `Content/UI/` | 终端、字幕、教程、主菜单等 UMG 蓝图 |

默认地图配置见 `Config/DefaultEngine.ini`：`/Game/Map/TestLevel`。

## 环境要求

- **操作系统**：Windows 10/11（项目当前目标平台）
- **引擎版本**：Unreal Engine 5.6（`CrankIt.uproject` → `EngineAssociation: 5.6`）
- **IDE**：Visual Studio 2022 或 Rider（需安装 UE C++ 开发组件）

## 构建与运行

1. 克隆仓库至本地。
2. 右键 `CrankIt.uproject`，选择 **Generate Visual Studio project files**（或通过 Epic Games Launcher 关联引擎后双击打开）。
3. 编译 `CrankItEditor`（Development Editor 配置）。
4. 启动编辑器，打开 `Content/Map/TestLevel`。
5. 点击 **Play** 运行。

> 首次打开时，引擎将编译 Shader 并导入资源，耗时取决于硬件配置。

## 配置说明

### GameMode

主关卡使用蓝图 GameMode `Content/BP/BP_CrankItGamemode`（继承 `ACrankItGameMode`）。可在编辑器中配置：

- `NarrativeData` / `TerminalCommandData`：叙事与终端命令 Data Asset
- `TutorialWidgetClasses`：教程 Widget 类列表
- `IntroSequenceActorTag`：开场 Level Sequence Actor 标签
- `DesendTime`：开场下降结束后的等待秒数

### 叙事 Data Asset

`UCrankItNarrativeData` 以 `TrackId` 组织字幕轨，标识符定义于 `Narrative/CrankItNarrativeIds.h`（如 `EMP_Tutorial`、`ChargeTutorial_Level1`）。

### 终端命令 Data Asset

`UCrankItTerminalCommandData` 配置：

- `CommandSequence`：主线命令输入顺序
- `CommandTextEntries`：命令 → 纯文本响应
- `CommandActionEntries` / `SequenceActionIds`：命令 → 副作用 ActionId
- `OutputBlocks`：按 `BlockId` 索引的输出文本块

ActionId 定义于 `Terminal/Routing/CrankItTerminalActionIds.h`（如 `OpenDoor`、`Boordle`、`Calibrate`）。输出块 BlockId 定义于 `Narrative/CrankItNarrativeIds.h` 的 `Terminal` 命名空间。

### 关卡 Actor 标签

部分流程依赖关卡中 Level Sequence Actor 的标签匹配，例如：

- 开场过场：由 `ACrankItGameMode::IntroSequenceActorTag` 指定
- Skip 教程过场：标签 `SkipTutorialSequencer`（`UCrankItIntroFlowSubsystem::SkipTutorialSequenceTag`）

## 核心系统说明

### 玩家控制（`APlayerCamera`）

- 基于 Enhanced Input 处理转向、交互与退出屏幕视图。
- 通过 `OnDirectionChanged` 委托广播朝向变化，供 IntroFlow 与 LightTrigger 订阅。
- 过场期间由 `UCrankItInputModeService` 统一管理输入开关与 Mapping Context。

### 交互（`UPlayerInteractionComponent`）

- 每帧射线检测悬停目标，区分充电手柄（`AMineConsole::ChargeHandle`）与可拾取电池（`ABattery`）。
- 经 `UCrankItActorRegistry` 缓存 `AMineConsole` 引用，避免重复 `GetActorOfClass`。

### 字幕（`USubtitleSubsystem`）

- `UTickableWorldSubsystem` 实现，以音频播放进度或世界时间为时钟驱动字幕切换。
- 通过多播委托 `OnSubtitleLineChanged` 与 UMG 解耦；`UCrankItNarrativeSubsystem` 负责从 Data Asset 加载并播放字幕轨。

### 声音探测（`ASoundDetectorActor`）

- 遍历世界中正在播放的 `UAudioComponent`，以 `APlayerCamera::CameraComp` 为检测原点进行锥形判据（`DetectionRange`、`DetectionAngle`）。
- 可选绑定 `USoundSubmix` 包络回调，将 RMS 电平传递至 `USoundWaveformWidget` 实时显示。

### 怪物（`AMonster`）

- 关卡解锁后于东 / 西 / 北三个方位随机生成，播放出现音效（经 `UAudioComponent`，可被声音探测器检测）。
- 定时向玩家位置步进，达到最大次数后停止；`Repel()` 由 EMP 灯光触发，使怪物退离。

### 终端（`UTerminalWidget`）

- `FTerminalCommandRouter` 校验命令是否与当前阶段匹配，返回文本响应或 ActionId。
- `UTerminalActionDispatcher` 按 ActionId 注册并执行副作用（开门、清屏、启动小游戏等）。
- `UTerminalBatteryHoldController` 对部分命令施加电池电量门槛与持续消耗逻辑。

## 仓库说明

`.gitignore` 排除了 `Binaries/`、`Intermediate/`、`Saved/`、`DerivedDataCache/` 等引擎生成目录。克隆后需本地编译生成上述目录。
