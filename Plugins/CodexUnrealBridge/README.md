# Codex Unreal Bridge

这是一个仅在 Unreal Editor 中加载的本地桥接插件。它通过 UE 自带的 Python Remote Execution 接收命令，并用带撤销事务的 C++ 接口读取或编辑关卡、Actor、组件和蓝图资产。

## 启动

1. 打开 `ChickenPlaguePolice.uproject`，等待编辑器完成模块加载。
2. 保持 **Project Settings > Plugins > Python > Enable Remote Execution** 开启。
3. 在 PowerShell 中测试连接：

```powershell
python F:\UE_Project\ChickenPlaguel\Plugins\CodexUnrealBridge\Python\codex_unreal_client.py status
```

读取当前关卡：

```powershell
python F:\UE_Project\ChickenPlaguel\Plugins\CodexUnrealBridge\Python\codex_unreal_client.py scene
```

读取蓝图结构：

```powershell
python F:\UE_Project\ChickenPlaguel\Plugins\CodexUnrealBridge\Python\codex_unreal_client.py blueprint /Game/BP/MapItems/BP_KunKunManage
```

## 能力

- 获取当前关卡 Actor、组件和变换快照
- 获取蓝图变量、组件和函数图快照
- 创建、复制、移动和删除 Actor
- 设置 Actor 或组件反射属性
- 设置灯光、静态网格和材质
- 创建蓝图，添加组件、变量和空函数，编译并保存

所有写操作都进入 Unreal 的撤销记录。调用结果会追加到 `Saved/CodexUnrealBridge/Reports/operations.jsonl`。
