# ChickenPlague ModKit

[中文](#中文说明) | [English](#english)

## 中文说明

这是《鸡瘟检察官：最后的 IKUN》的官方 Mod 制作工程。仓库使用 Git LFS 保存 Unreal 资源和较大的编译文件，作者可通过 Git 持续获取 ModKit 更新。

## 支持的 Mod 类型

- 创建资源类 Mod 和地图 Mod
- 替换角色骨骼网格体、材质、动画和音效
- 替换症状表现与玩家道具
- 追加自定义症状
- 配置 Mod 缩略图与资源插槽
- 检查、打包并在游戏中测试 Mod

## 使用要求

- Unreal Engine 5.7.4（安装核心组件）
- Windows 64 位
- Git 与 Git LFS
- 制作 Mod 前请阅读 [中文制作手册](ChickenPlaguePoliceModDocument_ZH.docx)
- English users can read the [English Mod Creation Manual](ChickenPlaguePoliceModDocument_EN.docx)

所有 Mod 资源必须放在 `Content/Mods/ModID/` 对应目录内。除 Mod 目录和制作地图所需资源外，请勿修改工程提供的蓝图与参考资源。

## 第一次下载

先安装 Git 与 Git LFS，然后运行：

```powershell
git lfs install
git clone https://github.com/JNTMGames/ChickenPlague_Mod.git
cd ChickenPlague_Mod
git lfs pull
```

下载完成后，打开 `ChickenPlague_Mod.uproject`。首次下载体积较大，请等待 Git LFS 完成资源下载后再启动工程。

## 获取后续更新

进入工程目录后运行：

```powershell
git pull
git lfs pull
```

如果你自己修改过同一批文件，请先提交或备份修改，再拉取更新，以免产生冲突。

## 主要目录

- `Binaries/`：随 ModKit 提供的编译文件
- `Config/`：工程配置
- `Content/`：Mod 模板、示例和参考资源
- `ModKitConfig/`：Mod 槽位配置表及说明
- `Plugins/`：ModKit 所需插件源码
- `Source/`：工程 C++ 源码

详细的角色、动画、症状、道具、地图制作以及打包测试步骤，请查看[中文制作手册](ChickenPlaguePoliceModDocument_ZH.docx)。

## 维护者发布更新

修改完成后，在工程目录依次运行：

```powershell
git add -A
git commit -m "说明本次更新内容"
git push
```

大文件会自动通过 Git LFS 上传。

## English

This is the official ModKit project for *Chicken Plague Police: The Last IKUN*. The repository uses Git LFS for Unreal Engine assets and large binaries, allowing mod authors to receive ongoing ModKit updates through Git.

### Supported Mod Types

- Create asset mods and map mods
- Replace character Skeletal Meshes, materials, animations, and sound effects
- Replace symptom effects and player props
- Add custom symptoms
- Configure mod thumbnails and asset slots
- Validate, package, and test mods in the game

### Requirements

- Unreal Engine 5.7.4 with the core components installed
- Windows 64-bit
- Git and Git LFS
- Read the [English Mod Creation Manual](ChickenPlaguePoliceModDocument_EN.docx) before creating a mod

Every asset used by a mod must be stored under `Content/Mods/ModID/`. Do not modify the Blueprints or reference assets supplied with the project, except for assets inside your own mod folder and assets required by a map mod.

### First-Time Setup

Install Git and Git LFS, then run:

```powershell
git lfs install
git clone https://github.com/JNTMGames/ChickenPlague_Mod.git
cd ChickenPlague_Mod
git lfs pull
```

After the download finishes, open `ChickenPlague_Mod.uproject`. The first download is large, so wait for Git LFS to finish before opening the project.

### Get Future Updates

Run these commands inside the project folder:

```powershell
git pull
git lfs pull
```

If you have changed the same files locally, commit or back up your work before pulling to avoid conflicts.

### Main Folders

- `Binaries/`: compiled files included with the ModKit
- `Config/`: project configuration
- `Content/`: mod templates, examples, and reference assets
- `ModKitConfig/`: mod-slot configuration and documentation
- `Plugins/`: source code for required ModKit plugins
- `Source/`: project C++ source code

The English manual covers character, animation, symptom, prop, and map creation, as well as packaging and testing.
