# GeoControls 联调与代码边界

本文定义 DarkTableNext 接入 GeoControls 时的职责边界、依赖方式和双仓开发流程。
当前主构建可显式启用一个最小 Qt/QML 壳；它不连接图像核心，也不表示产品迁移已经完成。

参考的 GeoControls checkout 当前要求 Qt 6.8+、C++20，并提供 `GeoControls 1.0`
和 `GeoControls.AppShell 1.0` 两个 QML 模块。接入时应以实际拉取版本的 CMake 与
公开 API 为准。

## 一句话判定规则

一个组件如果在不知道 DarkTableNext、照片、IOP、pixelpipe、历史、蒙版和色彩空间
含义的情况下仍可完整复用，就放入 GeoControls；只要它理解上述任一产品语义，就
留在 DarkTableNext。

不要用“以后可能复用”作为下沉理由。先让边界保持单向：

```text
GeoControls
    ↑
DarkTableNext.Controls
    ↑
DarkTableNext.App
```

GeoControls 不得依赖 DarkTableNext 的头文件、类型、资源或运行时。领域层可以依赖
GeoControls，应用层可以依赖前两者。

## 在 GeoControls 中扩展

### 设计系统和基础控件

- 主题、字号、间距、圆角、颜色和交互状态 token；
- 通用按钮、标签、文本输入、复选框、开关、单选框、ComboBox、ToolTip；
- Popup、Menu、Dialog、MessageDialog 和可复用的弹窗/页面外壳；
- 通用 `Expander`、卡片、标题栏、分组与可折叠容器原语；
- 键盘导航、焦点、鼠标/触控行为和可访问性状态；
- 不携带产品语义的图标加载与稳定 `qrc` 路径规则。

### 通用数值与选择控件

- Slider、RangeSlider、SpinBox 的输入、拖动、步进和键盘行为；
- hard/soft range、默认值、步长、精度、单位、factor/offset 格式化；
- 线性、对数和由调用方提供的自定义数值映射接口；
- preview/commit 或 changing/changed 这类通用信号时序；
- 通用渐变轨道与由调用方提供的 stops；
- model-backed list、tree、grid、selection 和 pagination。

GeoControls 可以定义“延迟提交”机制，但不能决定一次提交是否创建 DarkTableNext
历史项，也不能读取或修改 IOP 参数结构。

### 通用绘制原语

- 不带照片语义的曲线、折线、散点、网格、刻度和控制点；
- 必要时使用 C++ `QQuickItem` / scene graph 实现高性能绘制和命中测试；
- 数据通过稳定、通用的点列、范围和样式接口提供；
- 通用控件的示例、QML lint、单元/交互测试和视觉回归基线。

曲线控件可以知道“点、切线、范围”，但不能知道 tone curve、Lab、直方图通道或
曝光 EV 的产品含义。

### 可复用应用外壳

- 与业务无关的窗口 chrome、可停靠/可折叠布局原语和状态栏骨架；
- 由属性、model 和 signal 驱动的导航外壳；
- 不引用 DarkTableNext 路由名、动作名和资源的通用 AppShell 部件。

GeoControls 当前的 `AppShell` 只能按这一标准复用。任何 GeoDebugger 专属命令、
日志、服务器、地图或几何语义都不能进入 DarkTableNext，也不应继续沉淀为通用 API。

## 在 DarkTableNext 中实现

### C/C++ 核心到 Qt 的适配层

- 保持现有 C ABI 与图像核心所有权，在边界上封装为 `QObject`、
  `QAbstractItemModel` 和明确生命周期的值类型；
- 从现有 introspection 生成或填充 `ParameterDescriptor`，暴露名称、范围、默认值、
  单位、枚举项和可见性等元数据；
- IOP/module、history、undo/redo、preset、mask、catalog、selection、filmstrip 和
  pixelpipe 状态模型；
- 主线程与 worker/pixelpipe 线程之间的快照、队列和取消协议；QML 不接触裸 C
  指针，也不直接操作数据库或处理线程。

### 照片领域控件

- `ParameterSlider`、`ParameterComboBox`、`ColorParameterControl` 等对 GeoControls
  通用原语的领域包装；
- `ModulePanel`：启用、收藏、预设、重置、多实例、蒙版、错误状态与帮助；
- 编辑过程的 preview/commit 合并、单次历史事务、撤销/重做和参数重置语义；
- RGB、Lab、JzCzhz 等色彩空间转换和颜色管理后的显示值；
- histogram、waveform、vectorscope 的数据生产、通道语义和产品交互；
- DarkTableNext 专属动作、快捷键、文案、图标和资源。

`Expander`、Slider 轨道等机械部分可以来自 GeoControls，但组合成 `ModulePanel` 后
属于 DarkTableNext。不要把 IOP module 指针、历史栈或色彩配置塞进通用控件属性。

### 产品画布与工作流

- Darkroom `ImageCanvas`，包括缩放、平移、色彩管理显示和 before/after；
- crop、perspective、mask、retouch 等图像 overlay、坐标变换和产品级命中测试；
- Lighttable 网格、缩略图调度、filmstrip、集合筛选和批量选择；
- 导入、编辑、磁盘导出、偏好设置与产品导航；
- 端到端性能、内存、撤销/重做和 CPU/GPU 一致性测试。

通用 scene graph 绘制原语可以下沉，照片数据准备、坐标语义、算法和最终交互必须
留在本仓库。

## 建议的目标目录与 QML 模块

具体目录在第一个垂直切片中确定，但模块边界应稳定为：

```text
GeoControls repository
├── Controls/                    # import GeoControls 1.0
└── AppShell/                    # import GeoControls.AppShell 1.0

DarkTableNext repository
├── src/ui/qt/                   # C/C++ Qt adapters and application bootstrap
├── src/ui/qml/Controls/         # import DarkTableNext.Controls
└── src/ui/qml/App/              # product composition and screens
```

第一个切片应选择一个真实但边界较小的功能，例如“单个 IOP 的参数面板 + 历史提交 +
图像预览”。不要先复制完整 GTK 壳，也不要在同一批改动中同时替换 pixelpipe。

## 首次接入 GeoControls

当前工作树的 `source_roots.lock.jsonc.in` 和 `configs/source_roots.py` 已登记
GeoControls 候选 commit 与 FreeCM 源码根。`DARKTABLENEXT_BUILD_QML_SHELL=ON`
会构建 `DarkTableNext/` 的独立欢迎页→主界面壳，并链接 GeoControls；它不连接图像
核心，也不替代默认 GTK 路径。仅能启动壳不构成产品集成；后续垂直切片仍应完成：

1. 审核 `source_roots.lock.jsonc.in` 中的 remote、`depsManualPath` 和 commit，确认
   固定的是已推送且许可证可接受的版本；若条目不存在再补齐；
2. 保持 `configs/source_roots.py` 的 `DependencyRootSpec` 与锁模板同步，并验证
   `CMakeLists.txt`、`Controls/CMakeLists.txt` 和 `AppShell/CMakeLists.txt`；
3. 让主 CMake 显式解析 Qt 6.8+ 并添加 GeoControls 源码根；应用作为子项目使用时
   关闭 `GEOCONTROLS_BUILD_DEMO`；
4. 保持最小 Qt/QML 应用目标、QML 模块和欢迎页→主界面过渡可运行；
5. 以一个真实的领域垂直切片替换占位工作区，并同步更新构建文档与测试入口。

不要在 CMake 配置期使用 `FetchContent` 下载 GeoControls，不要复制它的源码到
`src/`，也不要把一个无版本记录的目录加入 include/QML import path。

## 双仓联调

### 正常固定模式

日常构建保持活动锁中的：

```jsonc
"depsMode": "pinned"
```

运行：

```sh
python3 configs/source_root_workflow.py --update
```

DarkTableNext 将使用 FreeCM 根据固定 commit 物化的 GeoControls 源码根。

### 本地联调模式

需要同时修改 GeoControls 时，让活动的 `source_roots.lock.jsonc` 使用 `manual`，并
把 `depsManualPath.GeoControls` 指向一个真实、独立的 GeoControls checkout：

```jsonc
"depsMode": "manual",
"depsManualPath": {
  "GeoControls": "/absolute/path/to/GeoControls"
}
```

这个 checkout 可以位于 DarkTableNext 目录外，也可以在未来约定的本地依赖目录中，
但必须保留自己的 `.git`、远端与分支，并且不能是以下目录：

- `build/dependency_seed_repos/GeoControls`
- `build/dependency_source_roots/GeoControls`
- 任意 preset 的 `dependency_installs` 目录

修改活动锁后运行：

```sh
python3 configs/source_root_workflow.py --update
cmake --preset mac_clang_debug
cmake --build --preset mac_clang_debug
```

`source_roots.lock.jsonc` 和 `CMakePresets.json` 都是本地生成/联调状态，不提交。
永久的依赖版本、路径字段和 CMake 图改动只进入受控模板与源码。

### 双仓提交顺序

一次跨仓改动按以下顺序收口：

1. 在 GeoControls 中完成通用能力、demo/测试、lint 和兼容性检查；
2. 提交并推送 GeoControls，确认 DarkTableNext 的远端能够取得该 commit；
3. 在 DarkTableNext 固定模式下更新 `source_roots.lock.jsonc.in` 的 GeoControls
   commit；
4. 运行本仓库 `--init`（需要获取新 commit 时）与 `--update`，重新配置并验证
   DarkTableNext；
5. 最后提交 DarkTableNext 的锁模板、适配层、QML 和文档改动。

不要让宿主仓库引用只存在于某台机器、未推送或来自 dirty worktree 的依赖状态。

## 验收要求

### GeoControls 侧

- 独立配置与构建成功，`GEOCONTROLS_BUILD_DEMO=ON` 的示例可运行；
- 新 QML 文件通过 lint，公共控件有交互/视觉或最小行为测试；
- 公共 API 不出现 DarkTableNext、darktable、IOP、pixelpipe 等领域类型；
- 新图标、字体和第三方实现记录来源与许可证。

### DarkTableNext 侧

- Qt adapter 的生命周期、线程和错误路径有测试；
- 参数拖动只产生预期的 preview，commit 只生成一次正确历史事务；
- QML 销毁、切图、撤销/重做和 pixelpipe 取消不留下悬空引用；
- Debug 构建和相关 CTest 通过；交互切片有可复现的手工验收步骤；
- 不因 UI 迁移改变 CPU 图像结果，涉及 GPU 路径时继续遵守
  [GPU Phase 0 准入规范](DevDocs/GPU_Baseline.md)。

## 接入前阻断项

当前参考的 GeoControls checkout 根目录未见明确的 `LICENSE`/`COPYING` 文件，也未见
成熟的自动测试入口。在它成为 DarkTableNext 的固定依赖前，至少需要：

- 添加明确的 GPLv3 兼容许可证和必要的 SPDX/版权信息；
- 审核已有图标、字体、QML/C++ 代码的来源与再分发条件；
- 建立最小的构建、QML lint 和公共控件行为测试；
- 冻结首批公共 API，避免 DarkTableNext 直接依赖 demo 或内部文件路径。

这些是正式集成和分发前置条件，不应通过“仅作为源码依赖”绕过。
