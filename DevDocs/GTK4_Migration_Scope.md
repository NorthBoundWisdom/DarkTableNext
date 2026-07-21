# GTK 4 迁移范围清单

> 状态：2026-07-21 基于当前 CMake 注册表的冻结快照。它界定 GTK 4 迁移的工作集，不改变
> `TODO_CORE_REDUCTION.md` 所定义的产品范围或 IOP 兼容性策略。源代码行为和可达性以
> `src/` 为准。

## 判定规则

- **保留并移植**：模块当前由 CMake 构建且属于既定的本地照片工作流。它必须随 GTK 4
  重编译，不能保留 GTK 3 ABI 或旧控件 shim。
- **暂缓决定**：模块当前仍构建，但 `TODO_CORE_REDUCTION.md` 已要求先完成 history、styles
  和 presets 的兼容性盘点。GTK 4 实现和产品删除都不得先于该决定。
- **先删除**：只用于已经在核心删减计划中明确排除、且完成全仓消费者清理的模块。本快照中
  **没有**这类仍可达模块；已删除视图和 Lighttable modules 不在当前 CMake 注册表中。

这不是按文件名猜测的清单：`src/CMakeLists.txt` 依次加入 `views/`、`libs/` 和 `iop/`，而
三处 CMake 文件分别注册动态模块。`watermark` 仍取决于 `Rsvg2_FOUND`，但一旦该条件满足，
同样适用本清单。

更新此文档时必须同时运行：

```sh
cmake --build --preset mac_clang_debug
scripts/gtk4_audit.sh --format markdown
```

若 CMake 注册表发生增删，必须在同一变更中更新本文件、`TODO_GTK4_MIGRATION.md` 和相应的
history/style/preset 兼容性结论；不得把“未列出”默认为可以删除。

## 应用与自定义控件

| CMake/源码所有者 | 可达对象 | 判定 | GTK 4 工作重点 |
| --- | --- | --- | --- |
| `src/main.c`、`src/common/`、`src/gui/` | GUI 入口、主/第二窗口、启动与退出、偏好、导入、导出、Action、对话框和菜单 | 保留并移植 | 单一 `GtkApplication` 生命周期、异步对话框、`GAction`/`GMenuModel`、无嵌套 main loop |
| `src/dtgtk/` | button、drawingarea、expander、gradientslider、icon、paint cell、range、resetlabel、sidepanel、thumbnail button、togglebutton | 保留并移植 | 组合控件优先；自定义 widget 迁移到 `measure`/`snapshot` 和控制器 |
| `src/bauhaus/` | Bauhaus 控件、popup、文本输入与交互参数编辑 | 保留并移植 | 父控件管理的 popover、控制器/gesture、无 grab 或全局窗口坐标 |
| `src/views/CMakeLists.txt` | `darkroom`、`lighttable` | 保留并移植 | 画布输入、Grid/Loupe/filmstrip、窗口与渲染生命周期 |

`GtkDarktablePaintCell` 虽包含在保留源码中，但它不是保留 GTK 3 cell-renderer ABI 的例外：
迁移时应与其消费者一起替换为 GTK 4 list item factory 中的绘制路径。

## Lighttable Lib 和工具模块

下列 30 个目标均由 `src/libs/CMakeLists.txt` 注册，并安装为当前应用可扫描的 Lighttable
插件；它们全都属于保留并移植范围。

| 类别 | 目标 |
| --- | --- |
| 工作流 | `import`, `export`, `copy_history`, `styles`, `image`, `tagging`, `select`, `recentcollect`, `filtering`, `metadata`, `metadata_view` |
| Darkroom/集合辅助 | `navigation`, `histogram`, `history`, `snapshots`, `modulegroups`, `backgroundjobs`, `masks`, `ioporder` |
| 固定工作区工具 | `viewswitcher`, `darktable_label`, `flags`, `colorlabels`, `ratings`, `lighttable_mode`, `view_toolbox`, `module_toolbox`, `filmstrip`, `hinter`, `image_infos` |

产品已经删除的全局顶部工具箱、时间线、日志历史、快捷筛选和 duplicate 等旧入口不在这个
注册表中；它们不应因 GTK 4 迁移重新引入。保留列表中的 `view_toolbox`、`module_toolbox` 和
`darktable_label` 名称不表示恢复已删除的顶部工具箱：实现迁移前先以其当前调用点和布局位置
为准。

## IOP UI

`src/iop/CMakeLists.txt` 当前注册 76 个 IOP 动态模块。除了下节的 14 个候选外，以下 62 个
模块为保留并移植范围；它们的参数 GUI、吸管、曲线、蒙版、预设与 Action 入口都必须在 GTK 4
下工作，但迁移不得改变 CPU/OpenCL 图像结果。

| 组别 | 保留并移植的 IOP |
| --- | --- |
| 基础 RAW、色彩与曝光 | `colorharmonizer`, `rawprepare`, `highpass`, `lowpass`, `shadhi`, `colorreconstruct`, `tonecurve`, `gamma`, `temperature`, `colorcorrection`, `exposure`, `rgbcurve`, `colorbalance`, `colorin`, `colorout`, `colorchecker`, `basecurve`, `colorzones`, `highlights`, `rawoverexposed`, `bilateral`, `profile_gamma`, `colormapping`, `rgblevels`, `colorize`, `colorcontrast`, `lut3d`, `toneequal`, `filmicrgb`, `channelmixerrgb`, `colorbalancergb`, `sigmoid`, `primaries`, `colorequal` |
| 几何、修复与像素准备 | `enlargecanvas`, `crop`, `sharpen`, `dither`, `monochrome`, `graduatednd`, `demosaic`, `rotatepixels`, `scalepixels`, `atrous`, `cacorrect`, `overexposed`, `hotpixels`, `lowlight`, `rawdenoise`, `nlmeans`, `flip`, `finalscale`, `bilat`, `denoiseprofile`, `ashift`, `hazeremoval`, `cacorrectrgb`, `diffuse`, `blurs`, `lens` |
| 工作流与显示辅助 | `mask_manager`, `rasterfile` |

以下 14 个是 `TODO_CORE_REDUCTION.md` 明确列出的创意或专用模块候选。其状态为**暂缓决定**，
不是确认删除：在每个模块完成历史记录、styles 和 presets 的使用面盘点，并写出 0.9 的迁移
或显式不兼容策略前，既不得启动 GTK 4 专项移植，也不得从 CMake 移除。

| 暂缓决定的 IOP | 所需决定 |
| --- | --- |
| `bloom`, `soften`, `overlay`, `velvia`, `vignette`, `splittoning`, `grain` | 确定创意效果是否属于冻结后的产品范围，并记录已有编辑和样式的处置 |
| `borders`, `liquify`, `retouch`, `watermark`, `censorize`, `negadoctor`, `agx` | 确定专用工作流的产品边界，并对 history/style/preset 引用给出迁移或不兼容策略 |

## 已排除的历史 UI

`TODO_CORE_REDUCTION.md` 已确认删除的 slideshow、print、map、tethering、MIDI、gamepad、mail、
Piwigo、Lighttable timeline、log history、collect、color picker 和 duplicate 等历史 UI/插件不在
当前构建图中。它们的源文件、资源或配置若被后续发现仍有可达消费者，应视为删减回归而非 GTK
4 移植任务，并按核心删减计划清理。

## 阶段 0 结论

- 当前 GTK 4 必迁工作集是应用/自定义控件、2 个 view、30 个 Lib/工具模块和 62 个 IOP UI。
- 14 个 IOP 的产品与兼容性决定仍阻塞其 GTK 4 工作；这也是阶段 0 尚未结束的原因之一。
- 不存在“先删除”但仍在 CMake 注册表中的 UI 目标。后续删减应先修改产品范围与兼容性清单，
  再从该清单和 CMake 中同步删除，不能以减少 GTK 4 编译错误为理由跳过兼容性工作。
