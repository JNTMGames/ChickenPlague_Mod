# ChickenPlague Mod 工程

这是供 Mod 作者使用的 Unreal Engine 工程。仓库使用 Git LFS 保存 Unreal 资源和较大的编译文件。

## 第一次下载

先安装 Git 与 Git LFS，然后运行：

```powershell
git lfs install
git clone https://github.com/y2260104520-debug/ChickenPlague_Mod.git
cd ChickenPlague_Mod
git lfs pull
```

下载完成后，打开 `ChickenPlague_Mod.uproject`。

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

详细使用方式请查看仓库内的 `ChickenPlaguePoliceModDocment.docx`。

## 维护者发布更新

修改完成后，在工程目录依次运行：

```powershell
git add -A
git commit -m "说明本次更新内容"
git push
```

大文件会自动通过 Git LFS 上传。
