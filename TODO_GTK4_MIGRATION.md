# GTK 4 最新稳定版迁移计划

## 目标、版本与当前状态

将 DarkTableNext 的整个保留 GTK 前端从 GTK 3.24 迁移到 GTK 4 最新稳定系列，同时保持
macOS、Windows 与 Linux 为正式目标。迁移完成后，应用、所有内置动态模块、安装包和开发文档
只使用 GTK 4；不保留 GTK 3 UI、并行工具包、运行时选择开关或旧 UI ABI shim。

截至 2026-07-21，[GTK 官方首页](https://www.gtk.org/)和本机 `homebrew/core/gtk4` formula
提供的最新稳定版本都是 **4.22.4**，最新开发版本 4.23.x 不属于本计划目标。因此本文当前以
**GTK 4.22.x** 描述具体 API；正式开工时则按下节的 Homebrew 规则重新确定目标：

- 源码和 CMake 以正式开工时 Homebrew `gtk4` formula 提供的稳定 minor 为最低基线，使用
  `gtk4 >= <该 minor>` 的 pkg-config 包；
- 开发、打包和发布候选统一使用该稳定系列当时最新的 Homebrew bottle 补丁版；
- 不跟随 Homebrew `--HEAD` 或 GTK 奇数开发系列；
- 启用所选版本的 `GTK_VERSION_MIN_REQUIRED`、`GDK_VERSION_MIN_REQUIRED` 和禁止 deprecated
  API 的编译检查。目标不是“能链接 GTK 4”，而是没有继续依赖所选稳定版已弃用的旧控件。

当前状态：**已完成 macOS Homebrew GTK 4 bottle、静态 API 基线和 CMake 可达 UI 范围冻结，
并正在以 GTK 3 可构建的适配层推进准备阶段**；尚未切换项目的 GTK 主依赖。GTK 3.24.15 仍是
`src/CMakeLists.txt` 的构建要求；Windows/Linux 依赖闭包、GTK 3 运行时诊断和 UI/数值参考基线
仍未完成。

GTK 4 与 GTK 3 ABI/API 不兼容，实施时以官方
[GTK 3 到 GTK 4 迁移指南](https://docs.gtk.org/gtk4/migrating-3to4.html)和
[GTK 4 API 文档](https://docs.gtk.org/gtk4/)为上游依据；本文负责记录 DarkTableNext 自身的
范围、顺序和验收标准。

## 正式开工的 Homebrew 版本规则

用户已明确要求：**正式开始 GTK 4 实现时，直接使用 Homebrew 当时能安装的最新稳定版本，
不得为了追赶 GTK 官网版本而从源码编译 GTK。** 这条规则覆盖本文写作时的 4.22.4 快照。

macOS 开工准备固定为：

```sh
brew update
brew info --json=v2 gtk4
brew install --force-bottle gtk4
# 已安装旧版时使用：brew upgrade --force-bottle gtk4
pkg-config --modversion gtk4
```

- 以 `pkg-config --modversion gtk4` 的实际结果更新本文目标 minor、CMake 最低版本和三平台依赖
  矩阵，然后才开始源码迁移。
- `--force-bottle` 是硬约束；不得使用 `--build-from-source`、`--HEAD`、GTK 源码 tarball、GTK
  Git checkout、CMake 下载或 FreeCM source root 自行编译 GTK 及其“最新版”闭包。
- 执行安装前确认当前 macOS/CPU 有对应 bottle；若 GTK 或必要依赖只能从源码构建，立即停止并
  记录平台阻塞，等待 Homebrew bottle，不自行降级为源码编译。
- 如果 Homebrew 稳定 formula 暂时落后于 GTK 官网，以 Homebrew 可安装的最新 bottle 为准；
  不为追平官网版本绕过 Homebrew。后续常规 patch 升级仍走 `brew upgrade --force-bottle gtk4`。
- 这是用户对 GTK 4 主机包准备的显式联网例外，不扩展到其他依赖。FreeCM 管理的 source roots
  仍只通过仓库规定的 `source_root_workflow.py --init/--update` 获取和物化。
- Windows vcpkg 与 Linux 包源应验证能提供与该 Homebrew 目标 API 相容的 GTK 4；若不能，先
  记录并解决平台版本差异，不以 macOS 源码自编 GTK 作为替代方案。

## 不变量与非目标

- 不改变 pixelpipe、IOP 参数语义、CPU/OpenCL 图像算法、RAW 解码或导出像素结果。GTK 4 的
  GSK 渲染器只负责界面合成，不是 Metal/OpenCL pixelpipe 后端，也不进入图像处理 API。
- 不借迁移重写应用、引入第二套运行时或替换 GTK/Bauhaus/dtgtk 的产品边界；现有 C/C++
  核心和 GTK 前端仍是唯一应用。
- 不保留 GTK 3 动态模块 ABI。所有保留的 IOP、Lib、格式和存储模块与主程序一起重编译；
  不加载 GTK 3 构建的历史插件。
- 不删除 OpenCL。它只能在 `TODO_CORE_REDUCTION.md` 所定义的 Metal 验收完成后删除，GTK 4
  迁移不能改变这一门槛。
- 不改数据库 schema、history/XMP、样式或预设格式，除非某个 GTK 绑定状态确实进入了这些
  格式；若发现这种情况，先单独记录设计、兼容边界和迁移测试。
- 不把 GTK 3 的窗口定位、菜单抓取或事件细节伪装成跨平台承诺。GTK 4/Wayland 不再支持的
  全局坐标和窗口控制行为应删除或改成平台中立语义，并在发布说明中说明。
- 不改 `FreeCM/` 或 `build/dependency_*` 中的外部源码，不手工编辑 `source_roots.lock.jsonc`、
  `CMakePresets.json` 或 `.freecm/`。永久依赖变化只进入锁模板、生成逻辑、CMake 和打包源码。
- 不批量格式化未触及的遗留 C/C++，不在 UI 迁移中顺手改变产品功能或图像结果。

## 与现有维护计划的顺序

1. 先完成 `TODO_CORE_REDUCTION.md` 中 Metal 前第二轮清理，并冻结最终保留的视图、Lib 和 IOP
   UI；明确将删除的代码不进入 GTK 4 移植范围。
2. 在 GTK 3.24 上完成能提前落地的准备工作，每一批仍可正常构建和运行 GTK 3 应用。
3. 准备阶段完成后，在专用集成分支一次切换构建依赖。主分支不合入“GTK 3 已失效、GTK 4
   也未闭环”的中间状态。
4. 切换期间不使用遍布源码的 `GTK_MAJOR_VERSION` 双实现。确有必要的短期适配只存在于单一
   项目封装中，并在 GTK 4 集成完成前删除。
5. GTK 4 三平台发布闭环完成后，再恢复被它阻塞的 Metal 阶段；两项工作分别维护结果基线。

## 2026-07-21 初步源码基线

以下数字由对 `src/` 中 C/C++ 源码的 `rg` 静态搜索得到，匹配项之间可能重叠；它们用于估算和
发现残留，不代表编译器诊断的最终全集。开始实施、产品范围冻结和最终切换前各重跑一次。

- 129 处直接 GTK/GDK 头包含分布在 115 个源码文件；dtgtk 与 Bauhaus 中有 12 个自定义 GTK 子类。
- `GtkTreeView`/`GtkTreeModel`/store/path/selection 代表性匹配 2,331 处，分布在 28 个文件；
  最大集中区为 `src/libs/tagging.c`、`src/libs/masks.c`、`src/gui/accelerators.c`、
  `src/libs/import.c`、`src/gui/preferences.c` 和样式/元数据对话框。
- 旧 `GdkEvent*` 结构或事件回调代表性匹配 187 处、40 个文件；显式事件 mask 22 处、13 个
  文件。Darkroom 画布、Lighttable 缩略图、蒙版、曲线和 Bauhaus 控件都是高风险输入路径。
- `GtkMenu`/menu shell 代表性匹配 395 处、36 个文件；`gtk_dialog_run()` 44 处、21 个文件；
  `GtkFileChooser*` 90 处、13 个文件。
- `gtk_box_pack_*` 290 处、36 个文件；通用 `gtk_container_*` 73 处、22 个文件；
  `show_all`/widget destroy 227 处、53 个文件。
- `GdkWindow`/`gtk_widget_get_window()` 代表性匹配 130 处、17 个文件；拖放和 selection data
  78 处、12 个文件；旧 main-loop API 10 处、4 个文件。
- allocation/旧尺寸接口代表性匹配 97 处、27 个文件；style context 147 处、25 个文件。
- `src/` 与 `data/` 中没有 GtkBuilder `.ui` 资源，界面主要由 C 代码构造；官方
  `gtk4-builder-tool --3to4` 不能承担本仓库的主体迁移。
- 主题由 `data/themes/` 的 4 个 CSS 文件构成。选择器依赖 EventBox、TreeView、MenuItem 等
  GTK 3 节点名，并通过 style context 向自绘控件提供颜色，必须与控件移植同步处理。

`scripts/gtk4_audit.sh --format markdown` 是可重复的静态盘点入口，统计本节类别并限定检查
`src/` 自有源码；它不扫描 FreeCM、构建树或依赖 checkout。开始实施、产品范围冻结和最终切换
前都必须重跑，并将差异更新到本节。

### 阶段 0 已完成的 macOS 基线（2026-07-21）

- [x] 通过 `brew update` 后以 `brew install --force-bottle gtk4` 安装 Homebrew `gtk4` 4.22.4；
      `brew info --json=v2 gtk4` 确认 `poured_from_bottle: true`，没有从 GTK 源码构建。
- [x] `pkg-config --modversion gtk4` 返回 4.22.4，`brew linkage --test gtk4` 与
      `brew missing gtk4` 均无错误输出；实际版本和最小 API 目标已记录在本文开头。
- [x] 增加 `scripts/gtk4_audit.sh`，其当前输出复现上述 GTK/GDK 头、类型、事件、菜单、树、
      对话框、容器、窗口、拖放、尺寸、样式和 main-loop 静态基线。

脚本采用其固定的合并正则表达式，当前可重复输出如下；它是后续各批次应比较的版本化数值
快照（匹配数与文件数可能包含同一文件中的多个 API）：

| 类别 | 匹配数 | 文件数 |
| --- | ---: | ---: |
| 直接 GTK/GDK 头包含 | 129 | 115 |
| 自定义 GTK 类型定义 | 12 | 12 |
| TreeView/TreeModel/cell renderer | 2334 | 28 |
| 旧 GdkEvent 类型或回调 | 104 | 32 |
| 显式 widget 事件 mask | 14 | 9 |
| GtkMenu/MenuShell | 395 | 36 |
| 同步 gtk_dialog_run | 44 | 21 |
| GtkFileChooser | 90 | 13 |
| GtkContainer 通用 API | 73 | 22 |
| GtkBox pack API | 290 | 36 |
| show_all/widget destroy | 227 | 53 |
| GdkWindow/widget_get_window | 111 | 15 |
| 旧拖放/selection data | 78 | 12 |
| allocation/旧尺寸 API | 97 | 27 |
| GtkStyleContext 旧 getter | 147 | 25 |
| 旧 GTK main loop API | 10 | 4 |

## 所有权、生命周期与线程边界

迁移前先固定以下设计，避免把 GTK 3 的隐含引用和同步 main loop 直接搬到 GTK 4：

- `GtkApplication` 成为 GUI 进程生命周期所有者，负责 `startup`、`activate`/`open`、窗口和
  `shutdown`。当前 `src/gui/system_commands.c` 中后期创建的 `GtkApplication` 命令投影合并到
  这一实例，不再另建投影对象。
- `src/main.c`、`src/common/darktable.c` 的参数解析和核心初始化仍负责非 UI 域状态；
  `darktable-cli` 保持无 GTK 初始化。GUI 的主窗口改为 `GtkApplicationWindow`。
- 现有生产实例锁、数据库锁、D-Bus 名称和远程打开语义按 `DevDocs/Runtime_Persistence.md`
  保持不变。GTK 应用生命周期重构不能顺手改变单实例策略或应用 ID。
- GTK 对象只在主上下文创建、读取和销毁。工作线程继续通过项目已有任务队列和主上下文投递
  结果；投递数据必须拥有独立引用，回调通过弱引用或取消令牌重新验证接收者。
- GTK 4 父控件持有子控件引用。自定义控件在 `GObject::dispose` 中断开信号、取消异步任务、
  `unparent` 子控件并打破引用环；`finalize` 只释放自身内存，不用 `destroy` 信号管理域状态。
- 对话框、文件选择器、剪贴板和拖放全部按异步完成设计。回调不得保留裸模块、图片或 tree row
  指针；需要稳定域 ID、`GWeakRef` 和可取消上下文。
- GTK 4 的 `GdkSurface`/`GdkToplevel` 只表示窗口系统表面；pixelpipe 缓冲、Cairo 图像表面和
  GSK texture 的所有权必须分开命名和记录，不能因同为“surface”混用生命周期。

## 阶段 0：冻结范围、基线与平台依赖证明

- [x] 完成当前 CMake 注册表中的 UI/IOP 保留清单；
      [`DevDocs/GTK4_Migration_Scope.md`](DevDocs/GTK4_Migration_Scope.md) 为每个可达 UI 模块
      标记“保留并移植 / 先删除 / 暂缓决定”。14 个候选 IOP 必须先完成 history/style/preset
      兼容性决定，禁止将确认删除的模块纳入移植。
- [x] 把本节静态盘点固化为 `scripts/gtk4_audit.sh` 和版本化摘要；后续每个迁移批次补齐被宏、
      函数指针、C++ 包装或运行时 signal 名称隐藏的 GTK 使用。
- [ ] 以当前 GTK 3.24 构建运行 `G_ENABLE_DIAGNOSTIC=1`，记录属性、signal、CSS 和对象生命周期
      警告；先消除基线自身的 critical/warning，避免切换后无法区分新旧问题。
  - [x] 2026-07-21 隔离 config/cache/tmp、内存数据库、关闭 OpenCL 的 macOS Debug 启动/正常
        退出路径首次发现 `GtkCellRendererToggle:indicator-size` 已弃用；已从
        `src/libs/tagging.c` 移除两处属性设置，并在后续启动中确认该 warning 消失。
  - [x] `src/gui/accelerators.c` 的 tooltip 位置修正现在在找不到关联 `GdkMonitor` 时安全返回，
        不再把空 monitor 传给 workarea API。
  - [ ] 当前机器的内建显示器处于休眠状态，重复启动仍由 Homebrew GTK 3 Cocoa 后端报告
        `Failed to initialize CVDisplayLink` 和两条 `gdk_monitor_get_workarea` critical；项目内仅有的
        workarea 调用均已审查并防御空 monitor。待显示器唤醒后在真实交互会话重跑，再决定是否有
        可归因于项目的剩余诊断。
- [ ] 固化 UI 行为清单和参考截图：首次启动/数据库错误、Lighttable Grid/Loupe/filmstrip、
      Darkroom、偏好、导入、导出、history、masks、tags、styles、presets、快捷键、右键菜单、
      第二窗口、后台任务与取消。
- [ ] 为 Lighttable/Darkroom 的代表性图片记录缩放、滚动、选择、颜色配置、峰值内存、首帧和
      交互延迟基线；使用现有 CPU 金样确认 UI 迁移前后的导出结果一致。
- [ ] 已完成 macOS Homebrew `gtk4` 4.22.4 bottle、pkg-config 和 linkage 证明；继续在 Windows
      vcpkg 和 Linux 目标包源上证明相容 GTK 4、GLib、Pango、Cairo、GdkPixbuf、Graphene/GSK
      及运行时工具可获取。记录实际版本、架构、license、debug/release 产物和部署闭包。
- [ ] 明确最低 macOS、Windows 和 Linux 发行版/运行库版本。若最新 GTK 稳定版迫使提高系统
      基线，先更新产品和发布文档，不允许某个平台静默停留 GTK 3。
- [ ] 决定 Linux 是否要求系统提供所选 GTK 4 minor，还是由项目打包完整运行时；不得通过
      CMake `FetchContent` 或未锁定网络下载补依赖。

当前 GTK 3 回归基线（2026-07-21）：`cmake --preset mac_clang_debug -DBUILD_TESTING=ON`、
`cmake --build --preset mac_clang_debug` 和 `ctest --test-dir build/mac_clang_debug --output-on-failure -L unit`
均通过（3/3）。使用 `G_ENABLE_DIAGNOSTIC=1`、隔离的 `--configdir`/`--cachedir`/`--tmpdir`、
`--library :memory:` 与 `--disable-opencl` 启动 Debug app 的首次结果已清除项目属性警告；当前
显示器休眠会触发 Homebrew GTK 3 Cocoa 后端的 `CVDisplayLink`/monitor diagnostic，故完整运行时
基线仍待屏幕唤醒后重跑。`darktable --version` 能打印 0.9.0 及编译选项，但现有 CLI 以退出码 1
返回；该行为在 GTK 3 基线中记录为待单独诊断，不能作为 GTK 4 成功标准。

阶段出口：范围、交互基线、数值基线和三平台依赖闭包均有可重复记录；无法得到所选 GTK 4
运行时的平台必须在继续源码切换前解决，而不是在发布阶段才发现。

## 阶段 1：在 GTK 3.24 上完成可前置准备

- [ ] 将准备环境更新到官方最新 GTK 3.24.x 补丁版，并保持 `GDK_DISABLE_DEPRECATED`、
      `GTK_DISABLE_DEPRECATED`；这只用于降低迁移噪音，不形成新的长期 GTK 3 基线。
- [ ] 将可在 GTK 3 使用的旧 widget 事件 signal 改为 `GtkEventController`/`GtkGesture`，封装
      click、motion、scroll、key、focus、stylus 和 drag 手势的项目级连接/清理模式。
  - [x] `dt_gui_connect_click()`、`dt_gui_connect_drag()`、`dt_gui_connect_motion()`、
        `dt_gui_connect_key()`、`dt_gui_connect_scroll()` 和 `dt_gui_connect_scroll_handled()` 已成为
        唯一的 controller/gesture 构造适配边界：GTK 3 保留 widget 绑定与弱引用清理；GTK 4 分支
        创建 controller 后由 `GtkWidget` 接管所有权。
        2026-07-21 已分别用 Homebrew GTK 3.24.52 与 GTK 4.22.4 头文件完成 API 编译探针；
        仍未迁移的旧 signal 不能因此勾选父项。
  - [x] 滚动快照现复制设备、方向、位置、修饰键、平滑 delta、stop 与 pointer-emulated 标志；
        `dt_gui_connect_scroll()` 的 GTK 3 路径继续把侧栏事件显式交给 ScrolledWindow，GTK 4 路径
        对侧栏返回未处理以由框架继续传播，其他目标使用 GTK 4 布尔 `scroll` handler。中心画布及
        snapshot 的旧 `scroll-event`，以及 Toast 的 EventBox/button/mask/scroll 路径，现分别改为
        该 controller 和横向 GtkBox。旧事件从 403/67 降至 401/67，event mask 从 49/26 降至
        48/26，通用-container 从 74/22 降至 73/22。已完成 GTK 3 Debug/unit、Homebrew GTK 4.22.4
        scroll event/controller API 编译探针；显示器唤醒后须手工检查中心及 snapshot 的滚轮、
        Ctrl+滚轮、触控板平移/平滑累积、Toast 任意按钮隐藏及其上滚动传给画布，并覆盖侧栏的
        Bauhaus、histogram、modulegroups 与 navigation 滚动传播。
  - [x] 侧栏 ScrolledWindow 的 capture controller 现以同一滚动快照计算固定滚轮步长与平滑
        触控板 delta，GTK 4 使用无参 `gtk_event_controller_scroll_new()`、widget 所有权和布尔
        已处理返回，GTK 3 保留双参数构造与弱引用。原 `GTK_PHASE_CAPTURE`、vertical-only、
        adjustment 边界及“控件上滚轮仍滚动侧栏”语义不变；不触及 border 事件转发或空侧栏的
        DnD/Action。旧事件从 401/67 降至 400/67。已完成 GTK 3 Debug/unit、Homebrew GTK 4.22.4
        capture-controller API 编译探针；显示器唤醒后须手工覆盖左右侧栏、模块滑块/combobox、
        histogram 与触控板平滑滚动的 capture/边界行为。
  - [x] 折叠及 overlay 边框不再把 `GdkEvent` 再注入目标 ScrolledWindow；二者均经
        `dt_gui_connect_scroll()` 调用与 capture controller 共用的 adjustment 路径，保留固定
        滚轮步长、平滑 delta、上下边界和左右匹配关系。GTK 3 的通用 adapter 内部仍为其他
        side-panel controller 保留显式转发，不能与此已移除的跨 widget 注入混淆。旧事件从
        400/67 降至 398/67。已完成 GTK 3 Debug/unit，复用 Homebrew GTK 4.22.4 scroll
        controller API 编译探针；显示器唤醒后须手工检查左右面板折叠/展开时两个边框的鼠标滚轮、
        触控板、滚动边界、模块控件 capture 和边框按钮点击。
  - [x] `dt_action_define()` 对 Action target 的 element-reset 不再连接旧
        `leave-notify-event`，而是用 `dt_gui_connect_motion()` 的 leave controller 将当前
        element 复位；tooltip、Action target 的弱引用和既有 context-menu attachment 不变。
        此准备没有迁移 context-menu 的 button/key 信号、GtkMenu 或 provider 的点击位置数据，
        它们仍属于阶段 5。旧事件从 398/67 降至 397/67。已完成 GTK 3 Debug/unit、Homebrew
        GTK 4.22.4 motion-controller API 编译探针；显示器唤醒后须手工检查鼠标离开 Action widget
        后的快捷键映射 element 重置、tooltip 与右键菜单。
  - [x] `dt_gui_connect_key()` 现在统一创建 target-phase 的 `GtkEventControllerKey`，并由同一
        适配层处理 GTK 3 的 widget 绑定/弱引用与 GTK 4 的 widget 所有权。`dt_gui_context_menu_attach()`
        的 Menu 键和 Shift+F10 已改接该 controller，仍向各 provider 传递空位置数据并保留 generic
        Action fallback；右键 button signal、点击位置 provider 和 GtkMenu 弹出尚未触及，继续归入
        阶段 5。项目的 `g_signal_connect` 返回值校验亦登记 `key-pressed` 为布尔信号。旧事件从
        397/67 降至 396/67，event mask 不变。已完成 GTK 3 Debug/unit、Homebrew GTK 4.22.4
        key-controller API 编译探针；显示器唤醒后须手工检查有焦点的普通 Action widget 及安全的
        provider widget 中 Menu/Shift+F10 的菜单、快捷键不重复执行和焦点保持。
  - [x] 左、右和底部面板的 `GtkDrawingArea` 缩放把手已移除 button/motion/crossing signal 与
        event mask，改为限定 primary button 的 click、drag、motion controller。双击仍收起对应
        面板；drag begin 记录既有尺寸，局部 drag offset 继续驱动左右宽度或底部高度的原约束，
        drag end/cancel 均复位状态，enter/leave 仍切换正确的 resize cursor。旧事件从 396/67 降至
        393/67，event mask 从 48/26 降至 45/26。已完成 GTK 3 Debug/unit、Homebrew GTK 4.22.4
        click/drag/motion-controller API 编译探针；显示器唤醒后须手工检查三条把手的 hover cursor、
        primary 拖动到最小/最大边界、释放/取消、双击收起，以及次要按钮不开始拖动。
  - [x] 所有 outer border 与侧栏 overlay toggle 的 `GtkDrawingArea` 现通过
        `dt_gui_drawing_area_set_draw_func()` 绘制、`dt_gui_connect_click_all()` 接收任意按钮；现有
        几何、CSS name、面板切换和先前附着的滚动 controller 不变。GTK 4 由 DrawingArea 的 CSS
        snapshot 绘制背景，避免 `gtk_widget_get_style_context()`/`gtk_render_background()` 的 4.22.4
        弃用调用；GTK 3 保留原 app-paintable/background 路径。旧事件从 393/67 降至 392/67，event
        mask 从 45/26 降至 44/26，allocation/旧尺寸从 207/47 降至 205/47。已完成 GTK 3
        Debug/unit、Homebrew GTK 4.22.4 DrawingArea/gesture API 编译探针；显示器唤醒后须手工检查
        四条边和两个 overlay toggle 的绘制/normal-hover 主题、任意按钮切换、滚轮与面板状态同步。
  - [x] filtering 的 misc 文本筛选项由旧 button-press signal 改为任意按钮 click gesture：secondary
        sequence 显式 claim 后仍打开同一候选值 popover 并同步 tree selection，primary 的第二击仍清空
        条件并走既有 changed 路径；普通单击不 claim，保留 Entry 默认编辑/焦点行为。该批不迁移
        focus-out、GtkPopover 或 TreeView。旧事件从 392/67 降至 391/66。已完成 GTK 3 Debug/unit、
        Homebrew GTK 4.22.4 click/sequence-claim API 编译探针；显示器唤醒后须手工检查相机、镜头、
        ISO 等 misc 条件的输入、右键候选值/选择同步、左键双击清空与焦点提交。
  - [x] 导出元数据编辑对话框的 tag list Delete 键现经 `dt_gui_connect_key()` 的 target-phase
        `GtkEventControllerKey` 删除当前选中行；仅无修饰的 Delete 被处理，其余键继续传播。该批不
        迁移 TreeView/list store 或对话框同步流程。旧事件从 391/66 降至 390/65。已完成 GTK 3
        Debug/unit、Homebrew GTK 4.22.4 key-controller API 编译探针；显示器唤醒后须手工检查选中/
        未选中行、Delete、带修饰的 Delete、编辑 formula cell 时的键盘行为以及保存/取消。
  - [x] `dt_gui_connect_key_bubble()` 复用 key controller 的 GTK 3 widget 绑定/弱引用和 GTK 4
        widget 所有权，但在 bubble phase 观察子控件键盘事件。metadata、styles、import、
        metadata view、recent collections 和 tagging 的同步 GtkDialog 已用它调用共享的
        `dt_handle_dialog_enter()`；Return/KP Enter 仍发出 `GTK_RESPONSE_ACCEPT`，其余键继续传播。
        该批不把 GtkDialog/`gtk_dialog_run()` 改为异步。旧事件从 390/65 降至 388/64。已完成 GTK 3
        Debug/unit、Homebrew GTK 4.22.4 bubble key-controller API 编译探针；显示器唤醒后须逐项检查
        六个对话框的按钮/文本/列表子控件焦点下 Return、KP Enter、其他键、保存和取消。
  - [x] `dt_gui_connect_scroll_handled()` 使 callback 明确决定是否消费滚轮：GTK 4 直接返回
        `GtkEventControllerScroll::scroll` 的布尔结果；GTK 3 在 callback 未处理时向侧栏转发，若
        controller 本身挂在嵌套 ScrolledWindow，则从其父级继续传播以避免再注入自身。`dt_ui_resize_wrap()`
        的 DrawingArea 高度调整和通用列表 ScrolledWindow 现均使用该边界：Shift+Alt 仍调整高度，
        普通滚轮仍调整列表，到内部 adjustment 边界才继续向父侧栏传播；不再为 scroll 设置 widget
        event mask。EventBox、draw/motion/button/size 语义尚未迁移。旧事件从 388/64 降至 386/64。
        已完成 GTK 3 Debug/unit、Homebrew GTK 4.22.4 boolean scroll-controller API 编译探针；显示器
        唤醒后须手工检查所有 graphheight 和列表高度的 Shift+Alt+滚轮、普通滚轮、触控板平滑输入、
        上下边界到父侧栏的传播及 resize handle。
  - [x] `dt_ui_resize_wrap()` 的 resize handle 已移除 button/motion/crossing signal 与对应的
        widget event mask，改用 target-phase motion controller 和 primary-button click gesture。
        仅主键单击位于底部 handle 才 claim sequence 并开始拖动；release 或 gesture cancel 均清除
        dragging/cursor 状态。拖动期间仍按原公式更新 DrawingArea 或列表高度；离开子 widget 时继续
        用 crossing snapshot 保留 `GDK_NOTIFY_INFERIOR`，并保留 GTK ungrab 的清理行为。通用列表的
        重绘查询现经已知 ScrolledWindow/Viewport child helper，不再依赖固定的 GtkBin 嵌套层级。
        外层 `GtkEventBox`、旧 draw callback 和 allocation/尺寸 API 尚未迁移，不能据此删除该
        wrapper。旧事件从 386/64 降至 383/64，event mask 从 44/26 降至 43/26。已完成 GTK 3
        Debug/unit、Homebrew GTK 4.22.4 click/motion/sequence-claim API 编译探针；显示器唤醒后须
        手工检查 DrawingArea 和列表两类 wrapper 的 handle hover cursor、主键拖动、释放/取消、
        双击、次要按钮、列表滚动与 Shift+Alt+滚轮，尤其检查内层列表上下边界向父侧栏传播。
  - [x] `GtkDarktableDrawingArea` 现为同一 height-for-width 约定提供 GTK 4 `GtkWidgetClass::measure`
        实现：水平尺寸请求继续 chain 至 `GtkDrawingArea` 父类，垂直请求按现有 `height == 0`、固定
        height 与 aspect-ratio 三种模式返回 min/natural height；未知宽度安全归零，baseline 固定为
        `-1`。GTK 3 分支保留原 `get_preferred_height_for_width` vfunc。此基础覆盖所有 graphheight
        和 aspect-ratio 实例的 GTK 4 布局需求，但不迁移各调用点的 `draw` signal、输入或 EventBox
        wrapper。静态脚本未单列该 vfunc，计数不变。已完成 GTK 3 Debug/unit，以及真实
        `src/dtgtk/drawingarea.c` 对 Homebrew GTK 4.22.4 的 `-Werror` API 编译；显示器唤醒后须手工
        检查 graphheight 的初始高度、拖动/Shift+Alt 调整、侧栏宽度变化和所有 aspect-ratio 预览。
- [ ] 停止直接读取或构造 `GdkEvent` 字段，统一使用访问器和局部不可变事件快照。
  - [x] 新增 `dt_gui_controller_get_current_event_time()` 和
        `dt_gui_controller_get_current_event_state()`：GTK 3 在单一适配层取当前事件并立即释放，
        GTK 4 使用 controller-local API。Bauhaus popup/slider 拖动及 histogram 拖动已迁移到
        这两个快照；其余 `gtk_get_current_event()` 调用仍逐个保留在审计清单中。
  - [x] 新增一次性 `dt_gui_controller_get_current_scroll_event()` 快照，并将 Darkroom navigation
        的 controller 滚动回调迁入：设备、方向、位置和修饰键不再由模块读取 `GdkEvent` 字段。
        GTK 3 临时事件副本只在适配层创建并立即释放；GTK 4 使用 controller-local event。静态
        `GdkEvent` 匹配从 424 变为 425，是该单一 GTK 3/GTK 4 适配实现的两条分支各被脚本计入、
        同时替换掉 navigation 原有的一处，净增一处；并非导航模块残留的字段访问。
  - [x] 新增 `dt_gui_controller_get_current_crossing_event()` 快照；histogram 的 motion-controller
        leave 回调不再读取或释放 `GdkEvent`，仍保留“进入按钮子控件产生的 ungrab leave 不隐藏
        按钮”的原有条件。GTK 3/GTK 4 头文件探针、Debug 构建和 unit 测试已通过；显示器唤醒后
        仍须手工覆盖按钮之间快速移动的路径。
- [ ] 用容器专用 add/remove 和子控件 expand/align/margin 替换通用 `gtk_container_*` 与
      `GtkBox` pack child properties；记录必须在 GTK 4 切换时再改名的少量调用。
  - [x] `dt_gui_box_add()` 是现有 hbox/vbox/dialog-content 的单一 child-add 封装；其 C 和 C++
        实现都已在 GTK 4 分支使用 `gtk_box_append()`，GTK 3 分支保持 `gtk_container_add()`。
        调用方、顺序和 GTK 3 expand/fill 行为未改；静态通用-container 匹配从 170 降到 168。
        已完成 GTK 3/GTK 4 C/C++ 头文件探针与 GTK 3 Debug/unit 验证，GTK 4 集成构建时仍须检查
        代表性 hbox/vbox 的实际 expand/align 布局。
  - [x] `dt_gui_container_has_children()`、`num_children()`、`first_child()` 与 `nth_child()`
        现接收 `GtkWidget*`；GTK 4 通过 first-child/next-sibling 查询，GTK 3 保持受
        `GtkContainer` 类型检查保护的列表实现。所有现有查询调用者已移除 `GTK_CONTAINER()`
        依赖；`remove_children()`/`destroy_children()` 不在本批次，因为它们必须按实际 Box、Grid、
        Stack 等所有权关系分别转换，不能用通用 unparent 替代。
  - [x] `dt_gui_container_remove_children()` 与 `destroy_children()` 现接收 `GtkWidget*`，并仅为
        已审计的实际父容器实现 GTK 4 路径：`GtkBox` 用 `gtk_box_remove()`，`GtkGrid` 用
        `gtk_grid_remove()`，`GtkStack` 用 `gtk_stack_remove()`，`GtkFlowBox`/`GtkListBox` 用各自
        的 `remove()`，`GtkNotebook` 逐页 `gtk_notebook_remove_page()`。不使用通用 unparent；未知
        容器类型会保留并给出警告，须先补充明确的所有权语义。GTK 3 保持原有 remove/destroy 行为；GTK 4 中
        `destroy_children()` 表示通过对应 remove API 释放父容器所有权，不替调用方管理其自行
        持有的引用。此前的 GTK 4 头文件探针及 GTK 3 Debug/unit 验证已通过；新增 FlowBox/ListBox
        分支的验证随本批调用点执行。
  - [x] `dt_gui_window_set_child()`、`dt_gui_overlay_set_child()` 与
        `dt_gui_scrolled_window_set_child()` 集中三个单 child 容器的 GTK 3/GTK 4 差异；主窗口
        根、主工作区和三侧栏 overlay、以及侧栏 scrolled window 已接入。`dt_gui_scroll_wrap()`
        同时使用 GTK 4 无参构造和 `set_child()`；侧栏保留现有 vertical adjustment。GTK 3 路径
        仍用原有 `gtk_container_add()`。已完成 GTK 3 Debug/unit、GTK 4 C/C++ API 探针；静态
        通用-container 匹配从 168 降到 164。EventBox、Grid、Notebook 等尚未混入此封装。
  - [x] 新增 `dt_gui_popover_set_child()`，并将已确认类型的单 child 容器调用扩展至 Darkroom
        第二窗口及四个工具 popover、log history、快捷键窗口、scope/混合/评级 overlay、guides、
        modulegroups、文件名/元数据筛选 popover，以及 Wayland color-label popover。GTK 4 均走
        对应 `set_child()`，GTK 3 保留 `gtk_container_add()`；不处理同时承载 GtkWindow 与
        GtkPopover 的运行时分支，也不把 EventBox、Grid、Notebook 等冒充为单 child 容器。已完成
        GTK 3 Debug/unit、GTK 4 C/C++ API 探针；静态通用-container 匹配从 164 降到 148（文件数
        52 降到 49）。显示器唤醒后仍须手工检查上述 popover、Darkroom 第二窗口和 log history。
  - [x] `GtkDarktableRangeSelect` 的当前值与日期选择 popover 也接入
        `dt_gui_popover_set_child()`，通用-container 匹配从 148 降到 146。此批次只替换已确定的
        单 child 附着操作；range 的 destroy/dispose、计时器、日期模型及回调生命周期没有改动，
        仍属于后续需要专门验证的高风险路径。GTK 3 Debug/unit 验证已通过。
  - [x] Export 的 `storage_extra_container` 与 `format_extra_container` 均已确认是 `GtkStack`；
        新增 `dt_gui_stack_add_child()`/`remove_child()`，使存储和格式插件的初始填充、热更新和
        cleanup 在 GTK 4 分别使用 `gtk_stack_add_child()`/`gtk_stack_remove()`，GTK 3 保持原有
        container API。与已迁移的“清空 storage stack”路径拥有一致的父容器语义。已完成 GTK 3
        Debug/unit、GTK 4 C/C++ API 探针；通用-container 匹配从 146 降到 143。显示器唤醒后
        仍须手工检查更换存储/格式后控件显示和退出清理。
  - [x] Export 的 print-size 与 pixel-size 均已确认是 `GtkFlowBox`；新增
        `dt_gui_flow_box_append()` 和 `dt_gui_flow_box_set_children_can_focus()`，GTK 4 使用
        `gtk_flow_box_append()` 并逐一设置 FlowBox child wrapper 的 focusability，GTK 3 保持
        `gtk_container_add()`/`foreach`。不改变输出尺寸控件次序或可聚焦性；其中 dpi/px 内部
        GtkBox 的显式 expand/fill packing 仍留待布局阶段。通用-container 匹配从 137 降至 129
        （文件数 43 降至 42）。已完成 GTK 3 Debug/unit、GTK 4 C/C++ API 探针；显示器唤醒后仍须
        手工检查 cm/inch/pixel 三种尺寸模式下的键盘焦点顺序和控件换行。
  - [x] 快捷键窗口的临时表格容器是 `GtkFlowBox`，log history 的行容器是 `GtkListBox`；两者
        刷新时现调用已审计的 `dt_gui_container_destroy_children()`，避免手写 child-list 和
        `gtk_widget_destroy()` 循环。GTK 4 分别用 `gtk_flow_box_remove()`、`gtk_list_box_remove()`，
        GTK 3 保留原销毁语义。通用-container 匹配从 129 降至 127（文件数 42 降至 41），
        show_all/destroy 匹配从 231 降至 229。已完成 GTK 3 Debug/unit、GTK 4 C/C++ API 探针；
        显示器唤醒后仍须手工检查连续打开/关闭快捷键窗口和连续刷新 log history。
  - [x] 新增 Button、CheckButton、Frame、Revealer 的专用 child setter，并将自定义图标
        Button/ToggleButton、IOP check button、blend revealer 和 dtgtk Expander 已确定的 child
        连接接入；Expander 查询 Revealer child 不再依赖 `GtkBin`。GTK 4 使用各类型 `set_child()`
        /`get_child()`，GTK 3 保持原 container/bin 路径。没有移除 Expander 或 blend 的 EventBox，
        也没有改动自绘、旧事件 signal 或生命周期。已完成 GTK 3 Debug/unit、GTK 4 C/C++ API
        探针；通用-container 匹配从 143 降到 141（文件数 49 降到 46）。显示器唤醒后仍须手工
        检查 blend 展开收起、图标按钮和 IOP 参数切换。
  - [x] 快捷键临时窗口切换为常驻窗口时，`dt_gui_window_remove_child()` 在 GTK 4 先确认当前
        child 后以 `gtk_window_set_child(NULL)` 释放旧窗口所有权，再由现有 setter 交给新窗口；
        保留原有的显式引用，GTK 3 仍使用 remove/add。已完成 GTK 3 Debug/unit、GTK 4 C/C++ API
        探针；通用-container 匹配从 141 降到 140。显示器唤醒后仍须手工检查快捷键窗口的 sticky
        切换和销毁路径。
  - [x] Import 命名规则中，base-directory 输入框从 `GtkGrid` 转移到附带目录按钮的 hbox 前，
        现经 `dt_gui_grid_remove_child()` 脱离；GTK 4 使用 `gtk_grid_remove()`，GTK 3 保持
        container remove，原有的显式引用保护不变。已完成 GTK 3 Debug/unit、GTK 4 C/C++ API
        探针；通用-container 匹配数量不变（GTK 3 回退移入适配层），文件数从 46 降到 45。
  - [x] 所有项目级 ScrolledWindow 构造现经 `dt_gui_scrolled_window_new()`：GTK 4 无参创建，
        仅在调用方显式提供时设置 horizontal/vertical adjustment；GTK 3 保持原构造签名。
        `dt_gui_scroll_wrap()`、侧栏和 log history 均已接入，源码中不再有调用方直接使用 GTK 3
        的双参数构造。已完成 GTK 3 Debug/unit、GTK 4 C/C++ API 探针。
  - [x] `dt_ui_resize_wrap()` 的 ScrolledWindow/自动 Viewport 子项查询现分别经
        `dt_gui_scrolled_window_get_child()` 和 `dt_gui_viewport_get_child()`；GTK 4 使用各类型
        `get_child()`，GTK 3 保持 `GtkBin` fallback。此批只处理该已知双层查询，外层 EventBox、
        旧 scroll event、draw 与尺寸 API 仍待输入和自绘阶段整体迁移。已完成 GTK 3 Debug/unit、
        GTK 4 C/C++ API 探针；显示器唤醒后仍须手工检查 Alt+Shift 滚轮调整高度与普通滚动。
  - [x] 新增 `dt_gui_viewport_set_child()`，vectorscope 的色彩和谐按钮 box 现以该专用 API
        附着到已知 `GtkViewport`；GTK 4 使用 `gtk_viewport_set_child()`，GTK 3 保持 container
        fallback。同时将 slider context dialog 的 content-area 附着及 copy 格式页的已知 GtkBox
        附着接入既有 dialog/box helper。未改动同步 dialog 生命周期、viewport 的旧尺寸/事件逻辑
        或任何 EventBox。通用-container 匹配从 127 降至 125（文件数 41 降至 39）。已完成 GTK 3
        Debug/unit、GTK 4 C/C++ API 探针；显示器唤醒后仍须手工检查 vectorscope 色彩和谐按钮、
        slider 的 set-value dialog 以及 copy 格式页。
  - [x] 新增 `dt_gui_box_remove()`，background-jobs 根 GtkBox 的完成移除、split scope 在左右
        button box 之间的重父化、以及 AGX 在两个 GtkBox 页面之间的 basic-controls 转移均经该
        专用 API。现有的 `g_object_ref()` 保护、child 顺序和后续 `gtk_box_reorder_child()` 未改；
        不处理 EventBox 内容或 Notebook page API。通用-container 匹配从 125 降至 123（文件数
        39 降至 37）。已完成 GTK 3 Debug/unit、GTK 4 C/C++ API 探针；显示器唤醒后仍须手工检查
        background job 取消/完成、split scope 进出以及 AGX 页面切换。
  - [x] Darkroom 模块 header 已确认是 `GtkBox`；其首 child 尺寸查询及从末端开始的连续 Button
        遍历现使用 `dt_gui_container_first_child()`、`num_children()`、`nth_child()`，保留原有
        show/hide、opacity 与动态 drawing-area trigger 行为。mask-indicator 的 child-property 和
        reorder 路径、header focus 查询及旧事件/尺寸 API 未在此批迁移。通用-container 匹配从 123
        降至 121（文件数不变）。已完成 GTK 3 Debug/unit；显示器唤醒后仍须手工覆盖所有
        hide-header-buttons 选项、模块重命名和 mask-indicator 插入/移除。
  - [x] background-jobs 的每项进度 UI 原 EventBox 没有事件处理器，仅承载垂直内容 box；现直接
        使用垂直 `GtkBox`，保留 `background-job-eventbox` widget ID、`dt_big_btn_canvas` class、
        CSS 选择器、主线程添加/移除和取消按钮逻辑。通用-container 匹配从 121 降至 120（文件数
        37 降至 36）。已完成 GTK 3 Debug/unit；显示器唤醒后仍须手工检查任务背景、进度、取消和
        最后一项完成后的容器收起。
  - [x] image-info 与 hinter 的 EventBox 均没有事件、类型转换或额外布局语义；模块根 widget
        现直接使用各自既有 Label。`#image-info` 继续命中同一 Label，hinter 不新增
        `#top-hinter box` 的 CSS 匹配。通用-container 匹配从 120 降至 118（文件数 36 降至 34）；
        已完成 GTK 3 Debug/unit；显示器唤醒后仍须手工检查 Darkroom footer 的 image-info 截断与
        top hinter 的提示文字、边距和清空行为。
  - [x] temperature 的“scene illuminant temp”标题原 EventBox 只处理 button-release，回调不读取
        旧 `GdkEventButton`；现以单 child 横向 GtkBox 承载同一 Label，并通过
        `dt_gui_connect_click_all()` 接收任意按钮。每次释放（包括多击的每一次）仍循环 slider
        颜色模式；Tooltip、配置键和三组 slider 重绘不变。通用-container 匹配从 118 降至 117
        （文件数 34 降至 33）。已完成 GTK 3 Debug/unit；显示器唤醒后仍须手工检查标题点击的
        三态循环、右键行为、Tooltip 与标题对齐。
  - [x] guides popover 的空白“额外设置”容器没有输入、CSS 标识或特殊绘制，仅承载当前 guide
        的一个专属 widget；现改为纵向 GtkBox。替换 guide 时经已限定 GtkBox 的子项查询、销毁和
        添加适配器，原有初始 `no-show-all` 与新 child 的 `show_all` 语义不变。通用-container
        匹配从 117 降至 116（文件数 33 降至 32）。已完成 GTK 3 Debug/unit；显示器唤醒后仍须
        手工检查各 guide 切换、带专属选项的 guide 显隐和 popover 的首次打开。
  - [x] viewswitcher 的每个视图标签原 EventBox 现为单 child 横向 GtkBox；标签的
        `#view-label` CSS name、快捷键 action 注册和 `d->labels` 仍指向同一 Label。左键
        press 现经 click controller 切换视图，多击保持逐次切换；motion controller 在 enter/leave
        保留未选中时的 prelight 与已选中时的显式清除。旧事件回调匹配从 424 降至 421（文件数
        72 降至 71）、显式 event mask 从 62/31 降至 61/30、通用-container 从 116/32 降至
        115/31。已完成 GTK 3 Debug/unit；显示器唤醒后仍须手工检查 Lighttable/Darkroom 标签的
        左键、多击、右键不切换、hover/selected CSS 状态及快捷键提示。
  - [x] metadata 面板每一行的双击重置标签原 EventBox 现为单 child 横向 GtkBox；Tooltip、grid
        位置、`label` object data 和 TextBuffer 的清空/空 buffer 时显式 `changed` 信号保持。
        `dt_gui_connect_click_all()` 只在 `n_press == 2` 时执行，保留原任意按钮双击与三击中的
        第二次点击重置。旧事件匹配从 421 降至 420、event mask 从 61 降至 60、通用-container
        从 115 降至 114（这三类文件数不变）。已完成 GTK 3 Debug/unit；显示器唤醒后仍须手工
        检查单/双/三击、右键双击、不同 metadata 行和“leave unchanged”状态。
  - [x] import metadata 的共用标签包装与三个双击重置连接现使用横向 GtkBox 和任意按钮的 click
        controller；metadata 行、`metadata presets` 全部重置和 tags 行均仅在 `n_press == 2` 执行，
        保留原 grid/可见性/object data、entry changed 链与 `#import-presets` CSS。旧事件从 420/71
        降至 418/70、event mask 从 60/30 降至 59/29、通用-container 从 114/31 降至 113/30。
        已完成 GTK 3 Debug/unit；显示器唤醒后仍须手工检查每种标签的单/双/三击、右键双击、
        preset 的全量清空、tags 以及 XMP/import toggle 状态。
  - [x] preferences 的八个标签包装统一为横向 GtkBox：theme、font size 与 DPI 标签保持无交互布局；
        use-system-font、user CSS 和通用 bool 标签的任意按钮首击仍切换、双击仍重置默认、三击不再
        额外动作；int/string 标签只在双击重置。所有标签/控件的 grid 位置、Tooltip 和已有配置
        changed/toggled 链不变。旧事件从 418/70 降至 414/70、event mask 从 59/29 降至 51/28、
        通用-container 从 113/30 降至 105/29。已完成 GTK 3 Debug/unit；显示器唤醒后仍须手工检查
        General tab 的单/双/三击和右键、theme/font/DPI 排版、泛用 bool/int/string 偏好项及 CSS
        编辑器开关。
  - [x] colorbalance 的布局标题原 EventBox 现为横向 GtkBox；Tooltip 与每次任意按钮释放的
        list/columns/tabs 循环保持，回调不读取旧事件内容。旧事件从 414/70 降至 413/69、
        通用-container 从 105 降至 104（文件数不变）。已完成 GTK 3 Debug/unit；显示器唤醒后仍须
        手工检查三种布局、每种的 sliders/picker、任意按钮、多击和标题对齐。
  - [x] colorbalance 动态主区域现为 GtkBox；重配前仍先持有三组 slider block 的引用，再经已审计
        的 Box/Grid/Notebook child-remove 路径脱离旧容器并移除旧根。list 使用 Box 适配器，columns
        明确为三列两行 Grid，tabs 保持 Notebook page 附着；配置变更后的 child 顺序和首次/后续
        显示语义不变。通用-container 从 104/29 降至 99/28，show/destroy 从 228/54 降至 227/53。
        已完成 GTK 3 Debug/unit、Homebrew GTK 4 C/C++ API 探针；显示器唤醒后仍须手工覆盖三种
        布局的多次切换、mode/偏好更新、slider/picker 状态、首次打开和销毁路径。
  - [x] modulegroups 编辑器的九个图标选择行原 EventBox 现由统一横向 GtkBox choice-row 构造；
        保留每个图标/文本、`ic_name` object data、图标按钮的 CSS descendant、相对 group 按钮更新
        和选择后 popover 立即关闭。任意按钮的每次 press 仍会选择对应图标；popover 改由祖先查询
        获取，避免依赖 EventBox 的固定父级层数。旧事件从 413 降至 412、通用-container 从 99 降至
        90、GtkBox pack 从 312 降至 288（文件数均不变）。已完成 GTK 3 Debug/unit；显示器唤醒后
        仍须手工检查全部九项选择、图标实时更新、单/多击与右键、popover 关闭和编辑器保存/取消。
  - [x] modulegroups quick-access 的模块名原 EventBox 现为横向 GtkBox；关联 enable ToggleButton、
        disabled 多实例路径、`#basics-iop_name` CSS 和行内排列均保持。任意按钮 press 仍直接翻转
        ToggleButton，不触及相邻的右键模块菜单、group scroll 或搜索快捷键包装。旧事件从 412
        降至 411、通用-container 从 90 降至 89（文件数不变）。已完成 GTK 3 Debug/unit；显示器
        唤醒后仍须手工检查 quick-access 的启用/禁用、单/多击/右键、多实例禁用及模块可见性同步。
  - [x] modulegroups quick-access 标题原 EventBox 现为横向 GtkBox；仅 secondary 且 `n_press == 1`
        时打开原有 module action menu，保留 `module` object data、header CSS/class、首次显示和
        菜单锚点。主按钮、双击和其他模块组右键路径不触及。旧事件从 411 降至 409、通用-container
        从 89 降至 88（文件数不变）。已完成 GTK 3 Debug/unit；显示器唤醒后仍须手工检查标题右键
        菜单、主/中键和双击不弹出、菜单项执行及多项 quick-access 模块的创建/销毁。
  - [x] modulegroups 搜索输入的被动 visibility wrapper 原 EventBox 现为横向 GtkBox；继续保留
        搜索 entry 的独立父层，避免隐藏搜索区域时禁用其快捷键，同时不触及输入自身、group scroll
        或右键菜单。通用-container 从 88 降至 87（文件数不变）。已完成 GTK 3 Debug/unit；显示器
        唤醒后仍须手工检查显示/隐藏搜索、快捷键唤起、焦点进入/退出和过滤结果。
  - [x] 主窗口日志消息覆盖层原 EventBox 现为单 child 横向 GtkBox，并使用
        `dt_gui_connect_click_all()`；原回调不读取 `GdkEvent`，任意按钮按下后仍只隐藏同一条
        log label，保留 ellipsize、`.dt_messages` CSS、overlay 对齐和 `no_show_all` 语义。Toast
        覆盖层因仍有旧 scroll-event/mask 路径而未改动。旧事件从 409 降至 408、通用-container
        从 87 降至 86（文件数不变）。已完成 GTK 3 Debug/unit；显示器唤醒后仍须手工检查点击日志
        消息隐藏、下一条日志重新显示，以及 Toast 点击隐藏和滚动不受影响。
  - [x] modulegroups 组标签区的滚动包装原 EventBox 现为横向 GtkBox，并经
        `dt_gui_connect_scroll()` 使用双轴离散 controller；该适配器继续过滤模拟指针事件、累计
        平滑触控板滚动，回调仍以 `dx + dy` 的单位步长选择相邻 group button。basic/active 边界、
        自定义组循环、按钮点击和搜索/右键路径未改动。旧事件从 408 降至 407、event mask 从 51/28
        降至 50/27、通用-container 从 86 降至 85（文件数分别不变、减少 1、未变）。已完成 GTK 3
        Debug/unit；显示器唤醒后仍须手工检查鼠标滚轮与触控板的横/纵滚动、basic/active 边界、
        自定义组切换及按钮点击不受影响。
  - [x] `GtkDarktableResetLabel` 自定义控件的父类由 EventBox 改为横向 GtkBox；其单一 label、
        tooltip、ellipsize 和所有 8 个 IOP 调用点保持不变。任意按钮的双击仍复制对应 default
        params、更新 GUI 并追加 history；三击不再额外重置。该控件无 EventBox 类型消费者。
        旧事件从 407/69 降至 406/68、event mask 从 50/27 降至 49/26、通用-container 从 85/28
        降至 84/27。已完成 GTK 3 Debug/unit；显示器唤醒后仍须手工检查 borders、flip、overlay 和
        watermark 的单/双/三击及右键、参数重置、history/undo 与 label 文本更新。
  - [x] 通用 collapsible section 标题原 EventBox 现为单 child 横向 GtkBox；仅 primary click
        仍翻转同一 ToggleButton，进而保留 focus、expander 展开状态、箭头图标和配置写入。secondary
        与其他按钮仍不切换，连续多击仍逐次切换。旧事件从 406 降至 405、通用-container 从 84
        降至 83（文件数不变）。已完成 GTK 3 Debug/unit；显示器唤醒后仍须手工检查 IOP/Lib 各种
        collapsible section 的主/中/右键、多击、展开状态持久化、焦点和销毁。
  - [x] histogram scope 的根 EventBox 现为单 child 横向 GtkBox；原有 capture scroll、target
        enter/leave 和 bubble motion controllers 仍附着在同一外层，overlay 的按钮 pass-through、
        scope drag/click 和响应式布局均未改动。该批不改 scope scroll 回调中仍待迁移的旧事件访问。
        通用-container 从 83/27 降至 82/26。已完成 GTK 3 Debug/unit；显示器唤醒后仍须手工检查
        histogram/vectorscope/waveform、模式和色彩按钮显示、拖动曝光/black point、滚动、tooltip
        与鼠标快速进出。
  - [x] `GtkDarktableIcon` 的父类由 EventBox 改为 GtkDrawingArea，Cairo icon paint callback
        改由 `dt_gui_drawing_area_set_draw_func()` 驱动；GTK 4 直接使用 `gtk_drawing_area_set_draw_func()`
        和 `gtk_widget_get_color()`，GTK 3 的单一封装保留同一 callback、尺寸和 destroy 语义。无
        in-tree EventBox 类型消费者、子控件或输入路径，`dtgtk_icon_set_paint()` 仍只替换 paint data
        并 queue redraw。allocation/旧尺寸文件从 49 降至 48、StyleContext getter 文件从 28 降至 27
        （匹配数不变，封装仍提供 GTK 3 fallback）。已完成 GTK 3 Debug/unit、Homebrew GTK 4 C API
        探针；显示器唤醒后仍须手工检查所有出现该控件的图标尺寸、主题前景色和动态 repaint。
  - [x] 所有只读取 widget 当前 state 前景色的绘制路径现统一调用
        `dt_gui_widget_get_color()`：dtgtk button/toggle button/gradient slider/paint cell、缩略图
        alpha、评分星标、通用 paint-to-pixbuf helper 与主窗口边框。GTK 4 走 `gtk_widget_get_color()`，
        GTK 3 保持相同 `gtk_widget_get_state_flags()` + style-context 结果；Bauhaus 的
        selected/prelight 状态和 range 的显式 state 参数未被压平，保留给各自的绘制迁移。静态
        StyleContext 旧 getter 从 158/27 降至 148/25（初始代表性盘点为 156/27 至 146/25）。
        已完成 GTK 3 Debug/unit；显示器唤醒后仍须手工检查 normal/hover/active/insensitive
        主题下的图标、slider、缩略图透明度、星级与边框。
  - [x] filmstrip 根容器由无输入 EventBox 改为纵向 GtkBox；原 draw signal 的延迟挂载改为
        `view_enter()` 中显式的单 child attach，仍会先脱离旧 thumbtable、设置 FILMSTRIP mode、显示并
        queue redraw 新表。`dt_thumbtable_set_parent()` 的 GTK 4 路径仅接受已由全部调用点证明的中心
        GtkOverlay 与 filmstrip GtkBox，并分别使用 `gtk_overlay_add/remove_overlay()`、
        `gtk_box_append/remove()`；GTK 3 保留原通用 container fallback。通用-container 从 82/26
        降至 81/25。已完成 GTK 3 Debug/unit、Homebrew GTK 4 Overlay/Box API 探针；显示器唤醒后
        须手工检查 Lighttable filemanager/非 filemanager、Darkroom、视图反复切换、filmstrip 滚动/
        选择/快捷键以及关闭时的 detach。
  - [x] 顶部 Darktable 标志由 EventBox + `draw`/button-press signal 改为 GtkDrawingArea、
        `dt_gui_drawing_area_set_draw_func()` 与任意按钮 click controller；每次 press 仍直接打开同一
        About 对话框。绘制尺寸由 draw callback 参数提供，当前主题前景色走
        `dt_gui_widget_get_color()`；GTK 4 从 widget Pango context 复制字体且由 DrawingArea 的 CSS
        snapshot 绘制背景，GTK 3 保留原 style-context font/background 路径。旧事件从 405/68 降至
        403/67、allocation/旧尺寸从 209/48 降至 207/47、StyleContext getter 从 148 降至 147
        （初始代表性盘点为 405/68、209/48、146/25 至 403/67、207/47、145/25）。已完成 GTK 3
        Debug/unit、Homebrew GTK 4 DrawingArea/gesture/Pango API 探针；显示器唤醒后须手工检查
        所有 bundled theme、SVG/PNG fallback、1x/2x 缩放、任意按钮/多击打开 About 以及背景是否与
        原面板连续。
  - [x] blending 的 parametric/drawn/raster、blend 和 refinement 五个 Revealer 包装由 EventBox
        改为单 child 纵向 GtkBox；所有调用点的外层均明确为 GtkBox，统一通过
        `dt_gui_box_add()` 附着 wrapper 与 Revealer，保留帮助 URL、`blending-box` CSS name 和
        `box → revealer` 父级关系，因此可见性过渡仍由同一 Revealer 管理。无本地 EventBox callback、
        DnD 或 Action target。通用-container 从 81/25 降至 79/24。已完成 GTK 3 Debug/unit；
        显示器唤醒后须手工检查所有三类 mask、blending/refinement 的显示隐藏和动画、帮助链接与
        主题/窄侧栏布局。
  - [x] `dt_gui_add_help_link()` 在 GTK 4 只保存 `dt-help-url` object data，不再调用已移除的
        widget event-mask API；GTK 3 保持原 `GDK_BUTTON_PRESS_MASK`。显式帮助按钮原有的
        `clicked → dt_gui_show_help()` 连接不变。此准备不等同于完成 GTK 4 的通用上下文帮助手势，
        后者仍随 Action/context-menu 输入迁移处理；静态 event-mask 数因 GTK 3 fallback 保留而不变。
        已完成 GTK 3 Debug/unit。
  - [x] 独立 yes/no 与 string 对话框的新建 top-level window 均通过
        `dt_gui_window_set_child()` 附着唯一 vbox：GTK 4 使用 `gtk_window_set_child()`，GTK 3 保持
        container add。通用-container 从 79/24 降至 77/24；`gtk_main()`、同步返回、窗口定位和
        keep-above 语义没有在此批改变，仍由异步 dialog/application 生命周期阶段统一迁移。已完成
        GTK 3 Debug/unit；显示器唤醒后须手工检查两类对话框的内容布局、transient/modal、yes/no、
        entry 提交/取消和关闭路径。
  - [x] color-label 编辑的非 Wayland 浮动 GtkWindow 与 tagging 快捷输入的 runtime Popover/Window
        分支均改为类型对应的 child setter；tagging 在 Wayland 使用
        `dt_gui_popover_set_child()`，其他后端使用 `dt_gui_window_set_child()`，不再把两种类型混作
        GtkContainer。通用-container 从 77/24 降至 75/22。既有颜色标签按键/坐标逻辑、tagging
        TreeView completion、焦点销毁与后端定位策略均未在此批改变。已完成 GTK 3 Debug/unit；显示器
        唤醒后须手工检查 color-label 右键编辑的提交/Escape/焦点退出及 Wayland/X11/macOS 路径，和
        tagging 输入的完成、提交/取消、焦点销毁与浮动定位。
  - [x] Bauhaus popup GtkWindow 的唯一 DrawingArea child 改由
        `dt_gui_window_set_child()` 附着，GTK 4 不再在该点使用 GtkContainer；通用-container 从
        75/22 降至 74/22。popup 的 `GTK_WINDOW_POPUP`、GdkWindow 定位、grab、旧 draw/input
        signals、主题查询与 Wayland 分支未在此批改变，仍须作为 Bauhaus popup/输入专项整体迁移。
        已完成 GTK 3 Debug/unit；显示器唤醒后须手工检查 slider/combobox popup 打开关闭、键盘、
        鼠标/触控板、拖动、Wayland/X11/macOS 定位和焦点恢复。
  - [ ] 2026-07-21 已完成其余 `GtkEventBox` 的所有权审查；低风险的纯布局包装已清空，以下
        控件必须随其承载的语义整体迁移，不能仅替换外层 widget：
    - `src/dtgtk/thumbnail.c` 的四个包装和 `src/dtgtk/range.c` 的自定义 EventBox 子类先迁入
      pointer/scroll/drag controller，并为选择、缩放、overlay、DnD 与销毁建立回归路径。
    - `src/dtgtk/expander.c`、`src/develop/imageop.c` 的模块 header 及 `src/libs/lib.c` 的标签先
      明确 `dt_action_define()` 所附 leave/context-menu 行为、tooltip 与模块 DnD 的新宿主，再移除
      EventBox；不得让 Action target 或 header 拖放失效。
    - `src/gui/gtk.c` 的 Toast 已随滚动 controller 改为 GtkBox，resize wrapper 的 scroll、motion 和
      primary-button handle 输入也已迁入 controller；空侧栏和 resize wrapper 仍分别有 DnD/Action
      以及 EventBox/draw/size 语义，须同旧事件、自绘和尺寸阶段一起处理。
    - `src/views/darkroom.c` 的 overlay 是窗口化的 pass-through 层，须先以 GTK 4 overlay/输入
      模型重做 tooltip 与 macOS tracking；不得用普通 Box 取代后改变画布命中测试。
        每项完成后均需比较 `scripts/gtk4_audit.sh --format markdown`、完成 GTK 3 Debug/unit，
        并在显示器唤醒后按所属输入路径手工验证；该清单不会替代阶段 4 对自绘/输入的整体验收。
  - [x] 所有原 `GTK_WINDOW_TOPLEVEL` 构造现经 `dt_gui_toplevel_window_new()`：GTK 4 使用无参
        `gtk_window_new()`，GTK 3 保持原 top-level 构造。主窗口、独立确认/输入对话框、log history、
        Darkroom 第二窗口、快捷键常驻窗口、splash 与现有 transient 顶层窗口均已接入。
        `GTK_WINDOW_POPUP`、位置/type-hint、同步 main loop 和 GtkApplication 关联没有在此批次
        改动，仍须在窗口/应用生命周期阶段整体迁移。已完成 GTK 3 Debug/unit、GTK 4 C/C++ API
        探针；显示器唤醒后须覆盖上述顶层窗口的创建、关闭和 transient 行为。
  - [x] 新增 `dt_gui_button_get_child()` 与 `dt_gui_check_button_get_child()`，将已由
        `GtkButton`/`GtkCheckButton` 构造或既有 button API 证明类型的 label 查询迁入。覆盖
        colorzones、exposure、colormapping、colorharmonizer、channelmixerrgb 等 IOP，以及
        metadata、styles、recentcollect、history、select、image 等库 UI；GTK 4 使用各类型
        `get_child()`，GTK 3 保持 `GtkBin` fallback。未处理菜单、FontButton、EventBox/Viewport
        等类型不明确或有独立 GTK 4 语义的 `GtkBin` 调用。已完成 GTK 3
        Debug/unit、GTK 4 C/C++ API 探针；显示器唤醒后仍须手工检查这些 checkbox/button 标签的
        省略和动态文本更新。
  - [x] 新增 `dt_gui_overlay_get_child()`，将导航 overlay 的基础 child 附着/重绘查询与 histogram
        已知 overlay 基础 child 查询迁入；同时迁移快捷键提示和 action button、guides checkbox、
        快照 ToggleButton 的 child 查询。快照改用 `dt_gui_container_nth_child()` 保持四个子项的
        顺序查询，并用既有 Button setter 附着其 hbox。GTK 4 走各类型 `get_child()`/`set_child()`，
        GTK 3 保持原有路径；通用-container 匹配从 140 降至 137（文件数 45 降至 43）。已完成
        静态审计、GTK 3 Debug/unit、GTK 4 C/C++ API 探针；显示器唤醒后仍须手工检查导航、快照、
        快捷键与 guides 路径。
  - [x] Lens 的 camera-model 与 lens-model 均由 `dt_iop_button_new()` 创建，动态相机/镜头名称
        现通过 `dt_gui_button_get_child()` 更新其 label；GTK 4 使用 `gtk_button_get_child()`，GTK 3
        保持 `GtkBin` fallback。已完成 GTK 3 Debug/unit、GTK 4 C/C++ API 探针；显示器唤醒后仍须
        手工检查自动检测与两个人工选择菜单后的文本和 tooltip 更新。
- [ ] 清理非顶层 `gtk_widget_destroy()`，把自定义 widget `destroy` vfunc 转成 `dispose`；增加
      销毁时仍有 idle、job、popover、dialog 和 DnD 回调的测试。
  - [x] `GtkDarktableGradientSlider` 的 timeout 和颜色列表清理由 GTK 3 `GtkWidget::destroy`
        vfunc 改为幂等的 `GObject::dispose`；`dispose` 先取消 source、释放列表并置空，再调用
        父类。range select、缩略图和异步 UI 的复杂销毁路径仍待分别验证。
- [ ] 把产品级菜单数据统一接入现有 `dt_action_t`、`GAction` 与 `GMenuModel` 投影，确保业务
      Action 是唯一真相来源；不在 GTK 4 层复制命令状态机。
- [ ] 建立异步 UI 服务边界：alert/confirm、open/save/folder、color/font 选择和进度反馈返回
      结果对象或回调，不让业务代码依赖 `gtk_dialog_run()` 的同步返回。
- [ ] 将 tree/list UI 的数据读取、排序、过滤、选择和业务操作从 `GtkTreeIter`/row path 中拆出，
      使用稳定域 ID 和独立模型，为 GTK 4 `GListModel` 适配做准备。
- [ ] 收敛 `dt_ui_notebook_*`、菜单、popover、缩略图布局、CSS palette 和文件选择为少数项目
      API；调用方不得继续扩散将被删除的 GTK 类型。
- [ ] 把当前后期创建的 `GtkApplication` 投影改造成可成为启动所有者的单一对象，同时保持
      GTK 3 构建、命令行导入、实例锁和远程打开行为不变。

阶段出口：GTK 3 应用在三平台仍完整工作；可由 GTK 3 提前消除的事件、生命周期、容器和同步
对话框耦合已清理，GTK 4 切换不再同时承担业务状态重写。

## 阶段 2：构建系统与 GTK 4 一次性切换

- [ ] 用 GTK 4 的 imported target/pkg-config 检测替换 `cmake/modules/FindGTK3.cmake` 和
      `find_package(GTK3 3.24.15)`；目标与动态模块通过 target linkage 继承 include、compile
      options 和库，不再依赖目录级全局变量。
- [ ] 将 `cmake/compiler-warnings.cmake` 的版本宏切换到 Homebrew 实际选定的 GTK/GDK minor，
      并继续禁止 deprecated 和单头误用；同步更新 GLib 最低版本为该 bottle 依赖闭包要求的
      版本。
- [ ] 检查 GTK 4 引入的 GSK/Graphene 和渲染依赖。只有源码直接使用的库才显式链接；其余
      经 GTK imported target 传递，避免手写不一致闭包。
- [ ] 更新 macOS Homebrew、Windows vcpkg、Linux 构建环境和依赖说明。若必须永久改变
      FreeCM 输入，只改 `source_roots.lock.jsonc.in`、`configs/source_roots.py` 与消费 CMake，
      再运行标准 `--update`，不提交生成状态。
- [ ] 增加 configure 失败检查：发现 GTK 3 头、`gtk+-3.0`、GTK 3 动态库或低于目标的 GTK 4
      时给出可执行错误；版本输出记录编译期和运行期 GTK/GLib 版本。
- [ ] 在专用迁移分支完成一次构建切换，不引入全仓 GTK3/GTK4 条件编译。切换提交本身只改
      构建边界和已经准备好的调用点，不混入功能删减。

阶段出口：所有保留目标只链接 GTK 4，三平台都能 configure；尚未完成的 UI 移植是明确的
编译错误清单，不以 deprecated 开关或空 shim 隐藏。

## 阶段 3：应用、窗口、容器和对象生命周期

- [ ] 让 `GtkApplication` 在任何窗口和交互 GTK 对象之前创建，由 `activate`/`open` 驱动主
      窗口；移除 `gtk_main()`、`gtk_main_quit()`、`gtk_events_pending()` 和嵌套 GTK main loop。
- [ ] 重构初始化中的用户目录、workspace、数据库锁和错误对话框为显式启动状态机；错误和
      取消能安全进入 `shutdown`，不能留下半初始化全局对象。
- [ ] 将主窗口和第二 Darkroom 窗口接入 application；用 `close-request`、窗口属性和
      `GdkToplevel` 状态替代 destroy/window-state/configure 事件。
- [ ] 用 `gtk_box_append/prepend/remove` 和各容器 `set_child`/remove API 完成剩余通用容器迁移；
      移除 `GtkEventBox`，直接给目标 widget 安装控制器或使用自有复合控件。
- [ ] 将默认可见语义纳入控件创建，删除 `gtk_widget_show_all()`/`no-show-all`；对初始隐藏、
      revealer、popover 和延迟创建内容逐一写明状态。
- [ ] 将 `get_preferred_*`、旧 allocation signal/vfunc 改为 GTK 4 `measure`、`size_allocate` 和
      `GtkLayoutManager`；尺寸缓存失效只走 GTK 允许的 queue resize/draw 路径。
  - [x] `GtkDarktableSidePanel` 的 GTK 4 路径现实现 `measure`：先 chain 至 `GtkBox`，只在水平
        请求时按原有 left/right 配置值提高 natural width，minimum 与 baseline 均保持父类结果；GTK 3
        继续使用原 `get_preferred_width`。未改变面板 resize、滚动、overlay 或持久化的所有权。静态
        脚本未单列此 vfunc，计数不变。已完成 GTK 3 Debug build 与 Homebrew GTK 4.22.4 `measure`
        `-Werror` API 探针；显示器唤醒后须手工检查左右侧栏的初始/持久化宽度、窗口缩放、最小宽度、
        handle 拖动、overlay 与模块布局。
  - [x] `GtkDarktableExpander` 的布局后滚动决策已抽出为不依赖 allocation 参数的内部函数。GTK 4
        在 `GtkWidgetClass::size_allocate(widget, width, height, baseline)` 中先 chain 至 GtkBox 父类，
        再以同一 frame-selected/last-expanded/drop-zone 判定启动既有 tick 滚动；GTK 3 保留原
        `size-allocate` signal 的薄适配层。未改模块 header/body 的 EventBox、旧 DnD、draw 或
        allocation 查询。静态计数不变。已完成 GTK 3 Debug/unit、Homebrew GTK 4.22.4
        size-allocate vfunc API 编译探针；显示器唤醒后须手工检查 IOP/Lib 展开收起、Quick Access
        跳转、模块标签切换、拖放插入前后和滚动动画。
- [ ] 用 `GdkDisplay`、`GdkMonitor`、`GdkSurface` 和 widget cursor API 替换 `GdkScreen`、
      `GdkVisual`、`GdkWindow`、全局坐标、窗口移动和底层 cursor 操作。
- [ ] 重新定义窗口持久化为宽高、最大化、全屏和产品级第二窗口状态；Wayland 不支持的 x/y
      位置不再假装可恢复。同步清理失效配置键、文档和测试。

阶段出口：应用能正常启动、开关所有窗口并退出；ASan/GLib debug 下没有窗口引用环、迟到回调
或非主线程 GTK 访问；CLI 行为不变。

## 阶段 4：dtgtk、Bauhaus、自绘和输入系统

- [ ] 逐个迁移 `src/dtgtk/` 的自定义类。能组合的 EventBox/Button 子类改成组合控件；确需
      子类化的控件直接继承允许的 GTK 4 基类并实现 `measure`/`size_allocate`/`snapshot`。
  - [x] `GtkDarktableButton` 与 `GtkDarktableToggleButton` 的 GTK 4 路径现实现 `snapshot`：先
        chain 至原生 `GtkButton`/`GtkToggleButton`，再以 `gtk_widget_compute_bounds()` 取得
        `button-canvas` 的实际布局范围，并通过 `gtk_snapshot_append_cairo()` 叠加既有 Cairo 图标。
        active/focus/prelight paint flags、图标 callback/data 与原生按钮状态保持；GTK 3 的 `draw`
        分支暂留，直到一次性主依赖切换。GTK 4 不读取已弃用的 CSS geometry API，canvas margin 由
        GTK 布局直接生效；旧实现额外把该 margin 数值解释为百分比的光学校正不在 GTK 4 分支复制。
        已完成 GTK 3 Debug/unit 和 Homebrew GTK 4.22.4 snapshot/bounds/Cairo `-Werror` API 探针；
        显示器唤醒后须手工覆盖普通/大号/模块头按钮及 toggle 的 normal/hover/disabled/active/focus
        图标大小、主题、HiDPI 与窄侧栏，确认该布局语义没有可见回归。
  - [x] `GtkDarktableGradientSlider` 已从自定义 `draw`/尺寸/输入 vfunc 与显式 event mask 改为
        `GtkDrawingArea` draw func、click/motion/key controller。主键选择并拖动 marker、双击 reset、
        次键切换多 marker 选择、hover/prelight、箭头键与 modifier 加速、延迟 `value-changed`/`value-reset`
        信号均保持；click cancel 只结束并提交当前位置，绝不伪造 `(0,0)` release。旧滚轮回调始终让
        侧栏处理，故新路径不安装 scroll controller，让事件自然传播。GTK 3 保留原 CSS geometry
        绘制/measure 分支；GTK 4 由 DrawingArea 的 CSS snapshot 和标准 measure/layout 处理背景、frame
        与尺寸。旧事件从 303/57 降至 290/56，event mask 从 35/18 降至 34/17，allocation/旧尺寸从
        107/29 降至 99/28。已完成 GTK 3 Debug/unit 与 Homebrew GTK 4.22.4 click/cancel/motion/key/
        draw-func/width-height `-Werror` API 探针；整仓 GTK4 编译仍会先被 `gui/gtk.h`、
        `develop/imageop.h` 暴露的既有 GTK3-only 类型阻塞，不能据此宣称集成可编译。显示器唤醒后须
        手工检查 retouch preview levels 的单击/拖动/双击/次键、gesture cancel、hover、键盘及 modifier、
        所有预设/history/undo、滚轮到侧栏、主题、HiDPI 与窄侧栏。
  - [x] `thumbnail.c` 的 overlay 消费端已将评分/拒绝、星级、分组、音频及各状态图标（含 zoom）
        的 raw button/crossing signal 改为 click 和 motion controller。评分主键 claim 以阻止缩略图
        下层选择；次键明确 deny，仍由既有右键菜单处理。分组与音频只监听真实 release、不主动 claim，
        保留旧事件的传播语义及 Shift/Control 整组选择。释放回调手工连接，避免通用 click helper 在
        gesture cancel 时伪造 `(0,0)` release。hover/prelight、mouse-over ID、tags tooltip、星级逐级
        prelight 与 control element 保持。旧事件从 290/56 降至 283/56；显式 mask 仍为 34/17，因为
        `GtkDarktableThumbnailBtn` 的自定义 Cairo renderer、类 crossing vfunc 和 GTK3 event mask 暂未
        迁移。该 renderer 通过 GTK3 style context 读取 background color 作为星级填色；GTK4 没有可直接
        替代的非弃用 computed-CSS accessor，故其 snapshot 与主题 palette 重构必须在阶段 7 一起完成，
        不在这里硬编码颜色。已完成 GTK 3 Debug/unit、Homebrew GTK 4.22.4 click/motion controller
        `-Werror` API 探针；整仓 GTK4 编译仍受 `gui/gtk.h`、`develop/imageop.h` 的 GTK3-only 公共类型
        阻塞。显示器唤醒后须手工检查 Lighttable 的所有 overlay 模式、reject/0--5 星评级、右键菜单、
        hover/prelight、tags tooltip、Shift/Control 分组、展开/收起/代表图、音频启停、local-copy/altered/
        color/tags/zoom 图标、主题、HiDPI、窄缩略图和取消手势，确认没有误触下层缩略图选择。
  - [x] metadata view 的 filmroll 值双击跳转现在通过每个 value `GtkLabel` 创建时安装的 all-button
        click controller 完成；grid 重排只更新 `metadata-filmroll-clickable` object data。只有第二击且
        当前行是 filmroll 时才 claim 并跳转，其他 metadata 行和首击均保持原 label selectable 行为。
        这取代了按每次重排断开/重连 raw signal 的 `filmroll_event` 指针，避免重排行对应变化时遗留过期
        handler。旧事件从 275/55 降至 274/54，其他静态类别不变。已完成 GTK 3 Debug/unit 与 Homebrew
        GTK 4.22.4 GtkLabel/click controller/object-data `-Werror` API 探针；整仓 GTK4 编译仍受
        `gui/gtk.h`、`develop/imageop.h` 的 GTK3-only 公共类型阻塞。显示器唤醒后须手工检查 metadata
        panel 的 filmroll 双击跳转、首击文本选择、非 filmroll 值双击、可见字段重排/偏好变更后的重建、
        键盘 jump action、主题、HiDPI 与窄侧栏。
  - [x] filtering 的 `misc` 文本规则在 entry 失焦时改监听 `notify::has-focus`，仅当属性变为 false
        才按原条件提交 raw text；activate、右键候选 popover、清理期抑制提交和既有 click controller 均
        不变。本机 GTK 3.24 没有 `GtkEventControllerFocus`，因此不能把 GTK4-only focus controller
        引入双版本构建；该 property-notify 路径同时适用于 GTK3/GTK4。审计脚本的 event 类别原本未统计
        `GdkEventFocus`，故静态数值不变。已完成 GTK 3 Debug/unit 与 GTK3、Homebrew GTK 4.22.4
        `notify::has-focus`/`gtk_widget_has_focus()` `-Werror` API 探针；整仓 GTK4 编译仍受
        `gui/gtk.h`、`develop/imageop.h` 的 GTK3-only 公共类型阻塞。显示器唤醒后须手工检查所有 misc
        filter 的输入、Enter、点击其他控件失焦提交、right-click 候选、多选、popover 关闭、规则清理和
        过滤结果更新。
  - [x] filtering 的 `filename` 规则两个 entry 都接入相同的 `notify::has-focus` 提交路径和 all-button
        click controller。次键仍只打开对应 filename/extension 候选 popover 并 claim；主键第二击仍清空
        entry、刷新 raw filter text，但不 claim，以保留 entry 的既有默认处理。activate、候选 selection、
        popover close 提交与 TreeView 仍未在此批改变。旧事件从 274/54 降至 273/53；focus event 不计入
        审计类别，其他静态数值不变。已完成 GTK 3 Debug/unit，及前项的 GTK3、Homebrew GTK 4.22.4
        `notify::has-focus`、click controller/current button `-Werror` API 探针；整仓 GTK4 编译仍受
        `gui/gtk.h`、`develop/imageop.h` 的 GTK3-only 公共类型阻塞。显示器唤醒后须手工检查 filename 与
        extension 的输入/Enter/失焦、右键候选、主键双击清空、单/多选、popover close、RAW/NOT RAW/LDR/HDR
        关键字、规则清理和过滤结果。
  - [x] filtering 的 color rule 已将六个 color/all 按钮的 raw press/enter signal 改为 all-button click
        和 motion controller。纯状态变换从回调抽出，鼠标 controller 传入真实 button/modifier，快捷键
        action 直接传入等效的主键/Control 状态，不再构造伪 `GdkEventButton`。双主键继续 reset 并 claim；
        其他按键仍执行旧的包含/排除/灰色全选逻辑但不 claim；operator 保留原生 `clicked` 和 hover element。
        旧事件从 273/53 降至 270/52，其他静态类别不变。已完成 GTK 3 Debug/unit 与 Homebrew GTK 4.22.4
        click/motion controller、current button/modifier `-Werror` API 探针；整仓 GTK4 编译仍受
        `gui/gtk.h`、`develop/imageop.h` 的 GTK3-only 公共类型阻塞。显示器唤醒后须手工检查所有 color
        label、all/gray、Ctrl 排除、双击 reset、intersection/union、快捷键 action、hover element、规则
        组合/清理、主题、HiDPI 与窄侧栏。
  - [x] export 的 width/height、print width/height 与 scale 五个 entry 现由 all-button click
        controller 处理。中键继续将对应设置恢复为 `0` 或 `1`，并在阻断原 changed handler 时更新文本；
        其他按键继续调用原有 size/scale 同步，且不 claim，以维持 GtkEntry 的默认文本编辑行为。移除了
        五个局部 `GDK_BUTTON_PRESS_MASK` 和 raw press signal；style menu 的独立 raw menu callback 未在
        此批改变。旧事件从 270/52 降至 267/52，event mask 从 33/16 降至 28/15；静态表的
        allocation/旧尺寸计数亦已校正为当前 97/27。已完成 GTK 3 Debug/unit 与前项的 Homebrew GTK
        4.22.4 all-button click/current-button `-Werror` API 探针；整仓 GTK4 编译仍受 `gui/gtk.h`、
        `develop/imageop.h` 的 GTK3-only 公共类型阻塞。显示器唤醒后须手工检查所有五个 entry 的键盘
        输入、左/右键默认编辑、中键恢复、像素/打印尺寸与 scale 联动、预设切换、导出开始/取消/错误，
        并以 CPU 金样确认输出像素不变。
  - [x] filtering 主界面的 collect rule 与 sort order 删除按钮改为 press click controller。删除配置、
        后续 rule/sort 的持久化键前移、GUI 重建及 collection reload 留在不依赖 GTK event 的核心函数；
        wrapper 只在该函数原本返回 `TRUE` 时 claim，因 manual-update 或最小 sort 数量而原本返回 `FALSE`
        的路径继续传播。旧事件从 267/52 降至 265/51，其他静态类别不变。已完成 GTK 3 Debug/unit 与
        前项的 Homebrew GTK 4.22.4 all-button click/current-button `-Werror` API 探针；整仓 GTK4
        编译仍受 `gui/gtk.h`、`develop/imageop.h` 的 GTK3-only 公共类型阻塞。显示器唤醒后须手工检查
        删除任意位置的 collect rule/sort order、首项/末项边界、最少一个 sort 的限制、配置重启恢复、
        collection reload、快速连击、rule/sort GUI 重建和快捷键/鼠标传播。
  - [x] `GtkDarktableRangeSelect` 的 min/max entry 已从 raw focus/button signal 改为跨版本
        `notify::has-focus` 和 all-button click controller。失焦仍提交该 bound；次键仍打开 numeric/date
        popup 并 claim，其他按键维持 entry 默认传播。随后 band 的 raw press/release/motion/leave signal
        与显式 event mask 也改为 click/motion controller：主键拖选、临近上下界调整、Shift 仅设置第二
        bound、双击全范围 reset、次键 popup、hover cursor/current-value popover 和真实 release 的
        `value-changed` 均保持。click cancel 只恢复按下前的 bounds/min/max，不伪造 `(0,0)` release 或
        发出变化信号。旧事件从 265/51 降至 260/50，event mask 从 28/15 降至 27/14。
        此准备不等同于迁移 range 主控件：它仍继承 `GtkEventBox`，band 仍有 GTK3 `draw` signal、Cairo
        cache、GTK3 CSS getter、全局 pointer/date popup 和 TreeView；这些必须同 snapshot/layout、异步
        popover 和列表模型替换一起处理。已完成 GTK 3 Debug/unit 与 GTK3、Homebrew GTK 4.22.4
        `notify::has-focus`、click/cancel/motion、`gtk_widget_get_height()` `-Werror` API 探针；整仓 GTK4
        编译仍受 `gui/gtk.h`、`develop/imageop.h` 的 GTK3-only 公共类型阻塞。显示器唤醒后须手工检查
        numeric/date range 的 min/max 输入、Enter/失焦、右键菜单、相对日期、清空/非法值、blocks/markers、
        popup close、主键拖选/边界 resize/Shift/双击、取消手势和 filter query 更新。
  - [x] modulegroups 的 basics-link、Quick Access/active tab、各 module-group tab 与 presets button
        已从 raw button-press signal 改为 all-button click controller；组名 `changed` callback 也删除了从未
        使用的伪 `GdkEventButton` 参数。basics-link 保持跳转并在 single-module 模式确保目标最终展开；
        Quick Access、active 与 group tab 的次键菜单只在可编辑目标上 claim，保留左键 toggle 和 deprecated
        group 的原传播；presets 仅在 Ctrl 时打开管理界面并 claim，其他输入仍交给原按钮默认处理。旧事件从
        260/50 降至 248/48，其他静态类别不变。已完成 GTK 3 Debug/unit 与此前 Homebrew GTK 4.22.4
        all-button click/current-button/modifier `-Werror` API 探针；整仓 GTK4 编译仍受 `gui/gtk.h`、
        `develop/imageop.h` 的 GTK3-only 公共类型阻塞。显示器唤醒后须手工检查 basics-link 跳转（含
        single-module）、每个 tab 的左键布局切换、右键添加/移除菜单、deprecated group 次键传播、组名编辑、
        presets 的普通点击与 Ctrl+click、主题、HiDPI、窄侧栏和快速重建。
  - [x] history 的压缩按钮和每行的主键选择已从 raw button-press signal 改为 all-button click controller。
        Ctrl 状态仍决定截断到当前选中 history item 或生成等价的最小 history stack；压缩 controller 始终
        claim，保持旧 handler 消费按键以避免 `clicked` action 再执行一次。行选择将真实 button/modifier
        传给原布尔核心：普通主键保留默认 toggle，Shift 只聚焦对应模块并 claim；context action 直接传入
        primary/无 modifier，不再构造伪 `GdkEventButton`。右键 context menu 的事件坐标和 keyboard popup
        路径未改，留待菜单阶段。键盘 action 的原 `clicked` 路径不变。旧事件从 247/48 降至 242/48，其他
        静态类别不变。已完成 GTK 3 Debug build/history 目标与 unit、此前 Homebrew GTK 4.22.4
        current-modifier/click `-Werror` API 探针；整仓 GTK4 编译仍受 `gui/gtk.h`、`develop/imageop.h` 的
        GTK3-only 公共类型阻塞。显示器唤醒后须手工检查普通/Ctrl/Shift 点击、行右键菜单与键盘 popup、
        键盘 action、当前 history end、无有效图片、history reload、undo/redo、主题与窄侧栏。
  - [x] temperature 的 as-shot、user-modified、camera-reference 与 as-shot-to-reference 四个预设按钮
        不再使用共享的 raw-button `dt_iop_togglebutton_new()` 通道；模块局部构造器保留同一 tooltip、初始
        inactive 状态、IOP toggle Action 与图标，改以 all-button click controller 在按下时运行原 preset
        核心并 claim。预设选择、slider/combobox 更新、chroma debug、模块 reset guard 与鼠标前的 toggle
        时序保持；旧事件从 242/48 降至 241/47。已完成 temperature 生成式 IOP 的 GTK 3 Debug build/unit
        与此前 Homebrew GTK 4.22.4 click/current-button `-Werror` API 探针；整仓 GTK4 编译仍受
        `gui/gtk.h`、`develop/imageop.h` 的 GTK3-only 公共类型阻塞。
  - [x] color-picker proxy 将普通 picker 的 raw `button-press-event` callback 改为 all-button click
        controller，并把激活、切换、主/次键及 Ctrl 的 point/area 选择收敛为显式语义 API；Bauhaus quad
        继续通过其既有 controller 触发的 `quad-pressed` 语义信号进入同一核心。Action toggle 对已注册
        picker 直接调用该 API，因此不会再为快捷键合成旧 GDK button event，且保留主键 point、右键/Ctrl
        area、on/off/toggle 和互斥 picker 状态。`DT_IOP_TEMP_SPOT` 现只在未激活时调用
        `dt_iop_color_picker_activate()`，不会误把 active picker 关闭。旧事件从 241/47 降至 240/46。
        已完成 GTK 3 Debug 完整构建、3/3 unit 以及 Homebrew GTK 4.22.4
        click/current-button/current-state API `-Werror` 探针；整仓 GTK4 编译仍受
        `gui/gtk.h`、`develop/imageop.h` 的 GTK3-only 公共类型阻塞。显示器唤醒后须手工检查四个 preset
        的主/次键、Action/键盘、reset guard、RAW/RGB/无图像状态、spot picker 互斥、point/area picker、
        slider/combobox/history、主题与窄侧栏。
  - [x] masks Lib 的 gradient/path/ellipse/circle/brush 五个创建按钮由 raw `button-press-event` 改为
        all-button click controller，共用只接收 button/state/shape 的创建核心。主键继续创建 shape，Ctrl
        主键继续进入连续创建，次键和长按 Action 仍不创建但消费输入；原 GUI-update guard 返回 false 时
        controller 不 claim，保留 GTK 默认 toggle 的既有回退。五个按钮使用局部 Action 定义直接调用该核心，
        不再经 `dt_action_def_toggle` 合成旧 GDK button event；toggle/on/off、Ctrl 和 right/long fallback
        的既有快捷键效果保持。masks TreeView 的坐标选中、右键菜单和 context provider 未在本批改动，留待
        菜单/列表阶段。旧事件从 240/46 降至 239/46。已完成 GTK 3 Debug masks 目标与完整构建、3/3 unit，
        并复用 Homebrew GTK 4.22.4 click/current-button/current-state API `-Werror` 探针；整仓 GTK4
        编译仍受 `gui/gtk.h`、`develop/imageop.h` 的 GTK3-only 公共类型阻塞。显示器唤醒后须手工检查五种
        shape 的主/次键、Ctrl 连续创建、Action/键盘、creation cancel、shape 绘制/编辑、history/undo、
        TreeView 选择与右键菜单、主题和窄侧栏。
  - [x] Lighttable 的 fixed/dynamic culling 和 restricted 三个 toggle 不再连接 raw
        `button-release-event`。它们以 capture-phase all-button gesture 的真实 `released` callback 在原
        默认 toggle 之前读取 active 状态、提取 Ctrl 限制模式、更新 layout/restriction 并 claim；不使用
        会伪造取消 release 的连接包装。所有既有显式 Action/快捷键仍直接调用相同的布局/restriction 核心，
        不依赖旧事件注入。旧事件从 239/46 降至 237/45。已完成 GTK 3 Debug Lighttable mode 目标与完整
        构建、3/3 unit，以及 Homebrew GTK 4.22.4 release/current-state/capture-phase API `-Werror` 探针；
        整仓 GTK4 编译仍受 `gui/gtk.h`、`develop/imageop.h` 的 GTK3-only 公共类型阻塞。显示器唤醒后须
        手工检查 fixed/dynamic culling 的主/次键、Ctrl collection restriction、release/cancel/drag-out、
        restricted lock、grid/loupe/compare/survey/exit 快捷键、缩略图重建、主题和窄窗口。
  - [x] filmicrgb 的 highlight-reconstruction mask 不再经共享 raw-button
        `dt_iop_togglebutton_new()` 构造器。模块局部按钮以 all-button click controller 和显式 Action 定义
        调用同一 `show_mask` 核心，保留鼠标、toggle/on/off、Ctrl/right/long Action fallback、GUI reset guard
        与每次切换的 center reprocess。焦点丢失时的程序化 mask 隐藏继续直接设置状态，不因 `toggled` signal
        产生额外 reprocess。按钮附着继续经过项目 GtkBox 适配器。图表右键 context provider 仍依赖
        `GdkEventButton` 坐标，留待菜单阶段。旧事件从 237/45 降至 236/45。已完成 GTK 3 Debug filmicrgb
        生成式 IOP 目标与完整构建、3/3 unit，并复用 Homebrew GTK 4.22.4 click/current-button/current-state
        API `-Werror` 探针；整仓 GTK4 编译仍受 `gui/gtk.h`、`develop/imageop.h` 的 GTK3-only 公共类型阻塞。
        显示器唤醒后须手工检查 mask 的主/次键、Action/键盘、焦点切出/切回、每次开关的 preview reprocess、
        highlight reconstruction 参数、history/undo、图表右键菜单、主题和窄侧栏。
  - [x] metadata 面板 TextView 的 Enter/Escape 处理已由 raw `key-press-event` 改接 target-phase
        `GtkEventControllerKey`：无 Ctrl 的 Return/KP Enter 仍应用 metadata，无修饰 Escape 仍取消；Ctrl+Enter
        和其他键返回未处理，让 TextView 的默认编辑与输入法路径继续执行。用于 tab 顺序的 `focus`、显式
        focus/enter event mask、右键的 `GdkEventButton` 坐标及 GtkMenu context provider 未在本批迁移，留待
        对应输入/菜单阶段。旧事件从 236/45 降至 235/45。已完成 GTK 3 Debug metadata 目标与完整构建、3/3
        unit，并复用 Homebrew GTK 4.22.4 key-controller API `-Werror` 探针；整仓 GTK4 编译仍受
        `gui/gtk.h`、`develop/imageop.h` 的 GTK3-only 公共类型阻塞。显示器唤醒后须手工检查普通 Enter、
        KP Enter、Ctrl+Enter 换行、Escape 取消、其他输入法/组合键、tab 顺序、右键 metadata 菜单、主题和窄侧栏。
  - [x] tagging 面板的新标签 Entry 已将 raw `key-press-event` 改接 target-phase
        `GtkEventControllerKey`，并移除没有 release callback 消费的 `GDK_KEY_RELEASE_MASK`。Return/KP Enter
        和 Escape 保持原本的未处理传播；Tab/Down 继续聚焦第一个匹配 dictionary tag，Shift+Tab 继续选择首个
        用户 attached tag，Up 保留输入框焦点。两棵 TreeView 的坐标点击/键盘、浮动 tag editor 的 focus/key
        生命周期、completion popup 和 GtkMenu 均未在本批迁移。旧事件从 235/45 降至 234/45，显式 event mask
        从 27/14 降至 26/14。已完成 GTK 3 Debug tagging 目标与完整构建、3/3 unit，并复用 Homebrew GTK 4.22.4
        key-controller API `-Werror` 探针；整仓 GTK4 编译仍受 `gui/gtk.h`、`develop/imageop.h` 的 GTK3-only
        公共类型阻塞。显示器唤醒后须手工检查输入/自动完成、Return/KP Enter、Escape、Up/Down/Tab/Shift+Tab、
        attached/dictionary TreeView 焦点、浮动 editor、主题和窄侧栏。
  - [x] colorlabels 的浮动描述 Entry 已由 raw `key-press-event` 改接 target-phase
        `GtkEventControllerKey`。Escape、Tab、Return/KP Enter 的消费和配置/tooltip 更新、焦点回主窗口行为
        保持，其他输入仍交由 Entry 默认编辑处理。右键按钮的 `GdkEventButton`、X11/Quartz 浮动窗口根坐标/
        定位、Wayland Popover 与 focus-out 销毁生命周期未在本批迁移。旧事件从 234/45 降至 233/45，其他静态
        类别不变。已完成 GTK 3 Debug colorlabels 目标与完整构建、3/3 unit，并复用 Homebrew GTK 4.22.4
        key-controller API `-Werror` 探针；整仓 GTK4 编译仍受 `gui/gtk.h`、`develop/imageop.h` 的 GTK3-only
        公共类型阻塞。显示器唤醒后须手工检查五个 label 的右键编辑、Escape/Tab/Return/KP Enter、文本输入、
        tooltip 更新、focus-out、X11/Quartz 定位、Wayland Popover、主题和窄侧栏。
  - [x] Darkroom blend-if 的两个 multivalue gradient slider 已将 raw `key-press-event` 改接
        target-phase `GtkEventControllerKey`。`a/A` 仍切换可用 slider mode，`c/C` 仍切换 transient/sticky
        channel mask，`m/M` 仍切换 mask view；被处理时仍请求 IOP focus，其他键继续传播。slider 的 enter/leave
        magic-mask 生命周期、指针/拖动与绘制未在本批迁移。旧事件从 233/45 降至 232/45，其他静态类别不变。
        已完成 GTK 3 Debug 完整构建、3/3 unit，并复用 Homebrew GTK 4.22.4 key-controller API `-Werror` 探针；
        整仓 GTK4 编译仍受 `gui/gtk.h`、`develop/imageop.h` 的 GTK3-only 公共类型阻塞。显示器唤醒后须手工检查
        两组 slider 的 `a/A`、`c/C`、`m/M`、其他键、mouse enter/leave、magic mask 显示、参数/history/undo、
        主题和窄侧栏。
  - [x] Darkroom 模块实例重命名 Entry 已用 target-phase `GtkEventControllerKey` 和
        `notify::has-focus` 取代 raw key/focus-out event 及 `GDK_FOCUS_CHANGE_MASK`。Return/KP Enter 与失焦
        仍保存名称、写 history/XMP；Escape 仍只取消。关闭 Entry 前显式断开 focus 通知，避免销毁时失焦通知
        重入；header button、标题和 masks group 更新顺序保持。header hover、按钮点击/拖动、尺寸和样式路径
        未在本批迁移。旧事件从 232/45 降至 231/45，显式 event mask 从 26/14 降至 25/14。已完成 GTK 3 Debug
        主应用与完整构建、3/3 unit，并复用 Homebrew GTK 4.22.4 key-controller/has-focus `-Werror` 探针；整仓
        GTK4 编译仍受 `gui/gtk.h`、`develop/imageop.h` 的 GTK3-only 公共类型阻塞。显示器唤醒后须手工检查
        空/非空名称的 Enter/KP Enter/失焦/Escape、history/XMP、重复 instance、header button 重显、masks group、
        多次快速重命名、主题和窄侧栏。
  - [x] tagging 的浮动 tag Entry 已将 raw `key-press-event` 改接 target-phase
        `GtkEventControllerKey`。Escape 仍取消，Tab 仍消费，Return/KP Enter 仍 attach tag、同步 XMP、刷新两棵
        tree 与焦点回主窗口；其他文本输入继续传播。其 focus-out、completion popup、Wayland Popover、其他后端的
        浮动窗口及根坐标定位仍按原生命周期保留，留待菜单/窗口阶段一并处理。旧事件从 231/45 降至 230/45，其他
        静态类别不变。已完成 GTK 3 Debug tagging 目标与完整构建、3/3 unit，并复用 Homebrew GTK 4.22.4
        key-controller API `-Werror` 探针；整仓 GTK4 编译仍受 `gui/gtk.h`、`develop/imageop.h` 的 GTK3-only
        公共类型阻塞。显示器唤醒后须手工检查 completion、Escape/Tab/Return/KP Enter、attach/XMP、focus-out、
        Wayland Popover、X11/Quartz 窗口定位、主题和窄侧栏。
  - [x] tagging 的 attached 与 dictionary TreeView 键盘导航已将 raw `key-press-event` 改接
        target-phase `GtkEventControllerKey`。attached 的 Delete、Tab/Shift+Tab 和 Menu/Shift+F10，dictionary
        的 Enter/Shift+Enter、Left/Right、Tab/Shift+Tab 和 Menu/Shift+F10 均继续调用原有核心并保留处理/传播结果。
        TreeView 本身、行坐标点击/双击/拖动、GtkMenu 弹出与列表模型仍留待列表/菜单阶段。旧事件从 230/45 降至
        228/45，其他静态类别不变。已完成 GTK 3 Debug tagging 目标与完整构建、3/3 unit，并复用 Homebrew GTK
        4.22.4 key-controller API `-Werror` 探针；整仓 GTK4 编译仍受 `gui/gtk.h`、`develop/imageop.h` 的
        GTK3-only 公共类型阻塞。显示器唤醒后须手工检查两种视图的导航、attach/detach、展开/收起、Tab 焦点、
        Menu/Shift+F10、右键/双击/拖动、主题和窄侧栏。
  - [x] preferences 的 presets TreeView 删除键已由 raw `key-press-event` 改接 target-phase
        `GtkEventControllerKey`。Delete/Backspace 在选中可编辑 leaf 时仍确认并删除、必要时移除空父节点，其他键
        继续传播。搜索输入的 `GtkSearchEntry` 事件转发、TreeView 模型/行激活、同步对话框与文件选择仍留待对应
        GTK4 列表、输入和对话框阶段。旧事件从 228/45 降至 227/45，其他静态类别不变。已完成 GTK 3 Debug
        主应用与完整构建、3/3 unit，并复用 Homebrew GTK 4.22.4 key-controller API `-Werror` 探针；整仓 GTK4
        编译仍受 `gui/gtk.h`、`develop/imageop.h` 的 GTK3-only 公共类型阻塞。显示器唤醒后须手工检查可编辑/
        受保护/父节点/未选择 preset 的 Delete/Backspace、确认取消/确认删除、搜索、行激活、导入导出与主题。
  - [x] Lighttable culling 与 thumbtable 的图像 context-menu 键已由 raw `key-press-event` 改接
        target-phase `GtkEventControllerKey`。Menu 和 Shift+F10 仍选用同一 selected/mouse-over image 并调用
        既有菜单入口，其他键继续传播。图像菜单的 GtkMenu、culling/thumbtable 的鼠标、滚动、手势、绘制和 DnD
        未在本批迁移。旧事件从 227/45 降至 225/45，其他静态类别不变。已完成 GTK 3 Debug 主应用与完整构建、
        3/3 unit，并复用 Homebrew GTK 4.22.4 key-controller API `-Werror` 探针；整仓 GTK4 编译仍受
        `gui/gtk.h`、`develop/imageop.h` 的 GTK3-only 公共类型阻塞。显示器唤醒后须手工检查 culling/filmstrip/
        grid 的 Menu/Shift+F10、selected/mouse-over/空 selection、鼠标菜单、滚动、手势、DnD 和主题。
  - [x] Lib 模块通用 reset/presets 按钮已由 raw `button-press-event` 改为 all-button click controller，并收敛为
        Action 与鼠标共用的语义核心。鼠标 gesture 显式 claim，保持旧 handler 的消费语义；reset 仍清空 preset label，
        presets 仍在按下后复位按钮 active 状态。展开箭头/header、hover、DnD、GtkMenu 本体和 module action 菜单
        尚未迁移。旧事件从 225/45 降至 223/45，其他静态类别不变。已完成 GTK 3 Debug 主应用与完整构建、3/3 unit，
        并复用 Homebrew GTK 4.22.4 click/sequence-claim API `-Werror` 探针；整仓 GTK4 编译仍受 `gui/gtk.h`、
        `develop/imageop.h` 的 GTK3-only 公共类型阻塞。显示器唤醒后须手工检查所有 Lib 的 reset、presets、左/右键、
        Action/快捷键、不可展开 Lib、presets button 的 active 状态、header/DnD 和主题。
  - [x] Darkroom 的 color-assessment、raw/overexposure、second-window/profile/gamut 与 guides quickbutton
        Popover 触发已由 raw press/release event 改为每个按钮独立的 all-button click gesture。次键按下和主键长按
        仍显示同一 Popover 并 claim；普通主键保持未 claim，使按钮原有 clicked/toggle 行为继续执行。旧的全局 event
        timestamp 改为每个 gesture 的单调时间，避免跨 quickbutton 干扰；局部 gesture 不会把 cancel 合成为 release，
        保留原始取消语义。Popover 内容、每个按钮的业务 callback、
        second window 与渲染路径未在本批迁移。旧事件从 223/45 降至 222/45，其他静态类别不变。已完成 GTK 3 Debug
        主应用与完整构建、3/3 unit，并复用 Homebrew GTK 4.22.4 click/current-button/sequence-claim API `-Werror`
        探针；整仓 GTK4 编译仍受 `gui/gtk.h`、`develop/imageop.h` 的 GTK3-only 公共类型阻塞。显示器唤醒后须手工
        检查全部七个 quickbutton 的普通主键、右键、长按阈值、按下后拖出/取消、Popover anchor、toggle/业务 callback、
        second window、主题和窄窗口。
  - [x] Liquify 的 node/curve/line/point 四个工具按钮已脱离通用 raw-event
        `dt_iop_togglebutton_new()`：局部 all-button click controller 将真实 modifier 交给工具核心，Ctrl 继续进入
        continuous creation，所有鼠标输入继续消费；自定义 Action 定义使 toggle/on/off、Ctrl、right/long fallback
        直接调用同一核心，不再合成 GDK button event。工具切换、preview 中断、radio 状态、hinter、focus 与 pipe
        刷新保持；Liquify 画布、节点/曲线的鼠标/拖动/滚动以及其他菜单路径未在本批迁移。旧事件从 222/45 降至
        221/45，其他静态类别不变。已完成 GTK 3 Debug 完整构建、3/3 unit，并复用 Homebrew GTK 4.22.4
        click/current-state/sequence-claim API `-Werror` 探针；整仓 GTK4 编译仍受 `gui/gtk.h`、
        `develop/imageop.h` 的 GTK3-only 公共类型阻塞。显示器唤醒后须手工检查四种工具的主/次键、Ctrl 连续创建、
        Action/键盘、切换中的 preview 中断、node 编辑、undo/history、画布拖动/滚动、主题和窄侧栏。
  - [x] Retouch 的 editing、四个 shape、四个 algorithm、mask/suppress、copy/paste scale、display wavelet
        scale 与 auto-levels 共十五个 toolbar toggle 已脱离通用 raw-event `dt_iop_togglebutton_new()`。每个回调现在
        只接收 widget/button/state，局部工厂以 all-button click controller 调用；共享 Action 定义直接映射
        toggle/on/off、Ctrl、right/long fallback，且仅在旧核心返回未处理时回退默认 toggle。shape continuous
        creation、editing restricted mode、algorithm 的 Ctrl 当前 form/Shift 默认工具、scale copy/paste、mask/
        wavelet/auto-levels 状态、history 和 reprocess 均保持。wavelet 条画布的鼠标/拖动/滚动、Retouch canvas、
        色彩 picker 和菜单未在本批迁移。旧事件从 221/45 降至 213/45，其他静态类别不变。已完成 GTK 3 Debug
        完整构建（含 `introspection_retouch.c`）与 3/3 unit，并复用 Homebrew GTK 4.22.4
        click/current-button/current-state/sequence-claim API `-Werror` 探针；整仓 GTK4 编译仍受 `gui/gtk.h`、
        `develop/imageop.h` 的 GTK3-only 公共类型阻塞。显示器唤醒后须手工检查全部工具按钮的主/次键、Ctrl/Shift、
        Action/键盘、shape 创建与取消、algorithm 更改限制、copy/paste、mask/wavelet/auto-levels、history/undo、
        wavelet 条画布、主题和窄侧栏。
  - [x] IOP 模块标题栏的 Reset 和 Presets 按钮已由 raw `button-press-event` 改为 all-button click
        controller，并将 reset modifier 与 presets popup anchor 收敛为 Action、模块空白区右键和鼠标共用的
        语义核心。Ctrl+Reset 的 auto-preset、drawn-mask 移除、默认参数/UI/history 与 accelerator 重建保持；
        Presets 的菜单 deactivate、按钮锚点与空白区右键无锚点菜单保持。手势仅在原回调会处理时 claim；Action
        不再构造伪 `GdkEventButton`。Enable、multi-instance、header/body 的点击/展开/拖动/hover 和 presets
        滚动仍保留旧事件路径，留待对应输入与菜单阶段。重新执行的静态审计校正此前文档快照并确认旧事件从实际
        211/44 降至 208/44，其他静态类别不变。已完成 GTK 3 Debug 主应用与完整构建、3/3 unit，并以 Homebrew
        GTK 4.22.4 的 click/current-state/current-widget/sequence-claim API `-Werror` 探针编译；整仓 GTK4
        编译仍受 `gui/gtk.h`、`develop/imageop.h` 的 GTK3-only 公共类型阻塞。显示器唤醒后须手工检查每个 IOP
        的 Reset/ Ctrl+Reset、Presets、模块空白区右键、Action/快捷键、禁用模块、菜单 deactivate、header 的
        enable/multi-instance/展开/拖动/hover、presets 滚动、主题和窄侧栏。
  - [x] 同一 IOP 标题栏工厂的 Enable 与 multi-instance 按钮也已迁移为 all-button click controller，故主模块
        和复用该工厂的 modulegroups 标题栏共享同一行为。Enable 仍在每次按下通知 distortion 模块；Ctrl 只请求/
        取消 IOP focus 并 claim，其他输入不 claim 以保留 GtkToggleButton 的原默认切换。multi-instance 保持主键
        菜单、带 Action 的次键完整操作菜单、没有 Action 时的新增实例 fallback 以及中键传播；菜单锚点、deactivate
        与 active 状态复位未变。header 的 hover、body/header 展开/拖动、presets 滚动和 GtkMenu 本体仍未迁移。
        旧事件从 208/44 降至 206/44，其他静态类别不变。已完成 GTK 3 Debug 主应用与完整构建、3/3 unit，并以
        Homebrew GTK 4.22.4 的 click/current-button/current-state/current-widget/sequence-claim API `-Werror`
        探针编译；整仓 GTK4 编译仍受 `gui/gtk.h`、`develop/imageop.h` 的 GTK3-only 公共类型阻塞。显示器唤醒后
        须手工检查普通/Ctrl Enable、distortion 模块、multi-instance 的主/次/中键、Action/快捷键、所有多实例操作、
        弹出菜单 deactivate、modulegroups 标题栏、header 展开/拖动/hover、presets 滚动、主题和窄侧栏。
  - [x] IOP header/body、标题标签、标题按钮和实例重命名 Entry 的 hover 已由 raw crossing signals 改用 motion
        controllers；normal crossing 才显示/隐藏动态标题按钮，子控件的 inferior crossing、非 normal crossing 与
        `darkroom_skip_mouse_events` 仍忽略。共享 crossing 快照帮助函数现同时支持 enter/leave，标题按钮可见性 API
        也不再向公共 `imageop.h` 暴露 GTK3 `GdkEventCrossing`。header/body 的 `GDK_POINTER_MOTION_MASK` 已删除；
        Rename Entry 销毁前后、菜单 deactivate 与 darkroom 重建调用仍走同一无事件核心。header/body 的 raw click/
        release、拖放、presets 滚动、GtkMenu 和动态 width-trigger 绘制尚未迁移。旧事件从 206/44 降至 201/43，
        event mask 从 25/14 降至 23/14，其他静态类别不变。已完成 GTK 3 Debug 全量重建、3/3 unit，并以 Homebrew
        GTK 4.22.4 的 motion/current-event/enter-leave crossing API `-Werror` 探针编译；整仓 GTK4 编译仍受
        `gui/gtk.h`、`develop/imageop.h` 的 GTK3-only 公共类型阻塞。显示器唤醒后须手工检查动态标题按钮在 header/
        body/label/button/rename Entry 的进入离开、子按钮间移动、菜单 deactivate、重命名、darkroom 重建、鼠标跳过、
        拖放、展开/折叠、主题和窄侧栏。
  - [x] IOP module body 的 primary focus 与 secondary Action/presets menu 已由 raw button-press signal 改为
        all-button click controller。现有核心仍仅在主/次键时处理，controller 也只在该结果为真时 claim，因此中键和
        其他按键继续传播；右键仍优先使用完整 context menu，无法提供时才显示 presets。header release 仍须以原始
        事件确认真正目标不是嵌套标题按钮，故不在本批强行迁移；header click/release、拖放、presets 滚动、GtkMenu
        和 body 子控件传播仍留待相应阶段。旧事件从 201/43 降至 200/43，其他静态类别不变。已完成 GTK 3 Debug
        主应用与完整构建、3/3 unit，并以 Homebrew GTK 4.22.4 click/current-button/current-widget/sequence-claim
        API `-Werror` 探针编译；整仓 GTK4 编译仍受 `gui/gtk.h`、`develop/imageop.h` 的 GTK3-only 公共类型阻塞。
        显示器唤醒后须手工检查空白 module body 的主/次/中键、focus、Action/presets fallback、任意 body 子控件
        的传播、header click/release、拖放、主题和窄侧栏。
  - [x] IOP Presets 标题按钮的 raw scroll callback 与 `scroll_mask` 已删除。该回调进入时总是由
        `dt_gui_ignore_scroll()` 返回未处理，实际行为始终是把侧栏滚轮交给父级 ScrolledWindow，故直接依赖该既有
        传播路径而不建立一个会错误消费滚轮的 controller。Presets 的 click/menu、Action 与相邻 preset Action
        路径保持；header release、GtkMenu、按钮 hover 与侧栏滚动行为仍须在显示器可用后验证。旧事件从 200/43
        降至 199/43，event mask 从 23/14 降至 22/13，其他静态类别不变。已完成 GTK 3 Debug 主应用与完整构建及
        3/3 unit；整仓 GTK4 编译仍受 `gui/gtk.h`、`develop/imageop.h` 的 GTK3-only 公共类型阻塞。
  - [x] styles TreeView 的行右键 context menu 已由 raw button-press signal 改为 all-button click controller。
        仅单击次键且命中有完整名称的行时才选择该行、设 cursor、显示原菜单并 claim；空白区、非次键和双/三击继续
        传播，保持原先 `GDK_BUTTON_PRESS` 的处理范围。keyboard popup、TreeView/store/renderer、行双击 apply、
        GtkMenu 与 Action 内容尚未迁移。旧事件从 199/43 降至 198/42，其他静态类别不变。已完成 GTK 3 Debug styles
        目标与完整构建、3/3 unit，并以 Homebrew GTK 4.22.4 click/current-button/sequence-claim API `-Werror`
        探针编译；整仓 GTK4 编译仍受 `gui/gtk.h`、`develop/imageop.h` 的 GTK3-only 公共类型阻塞。显示器唤醒后须
        手工检查有/无 style 的行、空白区、次键单/双/三击、键盘 Menu/Shift+F10、行双击 apply、菜单各 Action、
        过滤、主题和窄侧栏。
  - [x] Lighttable colorlabels 的六个按钮 hover Action element 已由 raw enter-notify signal 改为 motion
        controller。进入任一按钮时仍按现有颜色/clear 映射设置 `darktable.control->element`；按下切换、次键描述编辑、
        floating Entry/Popover、focus-out 与 X11/Quartz 定位路径均未改变，留待菜单/窗口阶段。旧事件从 198/42
        降至 197/42，其他静态类别不变。已完成 GTK 3 Debug colorlabels 目标与完整构建、3/3 unit，并以 Homebrew
        GTK 4.22.4 motion/current-widget API `-Werror` 探针编译；整仓 GTK4 编译仍受 `gui/gtk.h`、
        `develop/imageop.h` 的 GTK3-only 公共类型阻塞。显示器唤醒后须手工检查六个按钮 hover 后的 Action/快捷键、
        主/次键、clear、描述编辑、floating Entry/Popover、主题和窄侧栏。
  - [x] Library module 的 header、Presets/Reset 按钮 hover 与 body 进入/离开已改用 motion controllers。header
        仍更新相同 Action element；body 仅在 normal、非-inferior crossing 时设置/清除当前 Lib，保留 popup 和子控件
        边界的过滤。原始 header release、展开箭头、拖放、GtkMenu 与 body 内部点击仍未迁移。旧事件从 197/42
        降至 195/42，其他静态类别不变。已完成 GTK 3 Debug 主应用与完整构建、3/3 unit，并以 Homebrew GTK 4.22.4
        motion/current-event/enter-leave crossing API `-Werror` 探针编译；整仓 GTK4 编译仍受 `gui/gtk.h`、
        `develop/imageop.h` 的 GTK3-only 公共类型阻塞。显示器唤醒后须手工检查各 Lib header/Presets/Reset hover、
        body 进入/离开、popup/子控件 crossing、展开箭头、拖放、Action/快捷键、主题和窄侧栏。
  - [x] Library module 展开箭头已脱离 raw button-press signal。主/次键、Shift single-module 展开、Ctrl+Shift
        拖动保护、focus 归还、context-menu/presets fallback 与不可展开模块的未处理传播均收敛为 header release 和
        controller 共用的语义核心；controller 对双/三击仍 claim，保持旧回调不展开的消费语义。header release 的
        嵌套目标识别、拖放、GtkMenu 和 body 内部输入仍未迁移。旧事件从 195/42 降至 194/42，其他静态类别不变。
        已完成 GTK 3 Debug 主应用与完整构建、3/3 unit，并以 Homebrew GTK 4.22.4 click/current-button/
        current-state/sequence-claim API `-Werror` 探针编译；整仓 GTK4 编译仍受 `gui/gtk.h`、
        `develop/imageop.h` 的 GTK3-only 公共类型阻塞。显示器唤醒后须手工检查可/不可展开 Lib 的主/次键、
        Shift、Ctrl、Ctrl+Shift、双/三击、context-menu/presets fallback、focus、header release、拖放和主题。
  - [x] Lighttable snapshots 按钮的 Ctrl 重命名焦点入口已改为 all-button click controller。其核心仍先关闭
        button focus-on-click，Ctrl 时显示对应 Entry 并只在 Entry 获得焦点时 claim；未按 Ctrl 时保持未 claim，
        所以 GtkToggleButton 默认切换和现有 raw 次键菜单定位继续生效。右键 pointer popup、keyboard popup、Entry
        activate 与 snapshot 绘制/恢复均留待菜单、列表和绘制阶段。旧事件从 194/42 降至 193/42，其他静态类别不变。
        已完成 GTK 3 Debug snapshots 目标与完整构建、3/3 unit，并以 Homebrew GTK 4.22.4 click/current-widget/
        current-state/sequence-claim API `-Werror` 探针编译；整仓 GTK4 编译仍受 `gui/gtk.h`、
        `develop/imageop.h` 的 GTK3-only 公共类型阻塞。显示器唤醒后须手工检查各 snapshot 的普通/Ctrl/次键、
        toggle、Entry 显示/激活、pointer/keyboard menu、restore、绘制与主题。
  - [x] Lighttable history 行的次键 context menu 已由 raw button-press signal 改为第二个 all-button click
        controller。仅单击次键时才在现有统一菜单封装中按当前鼠标事件弹出并 claim；主键 history 回退/Shift 聚焦
        controller 保持不消费次键，双/三击与其他按键继续传播，keyboard popup 仍按按钮锚定。菜单 Action、
        GtkMenu 本体、history 列表模型及 undo/pixelpipe 行为未改变。旧事件从 193/42 降至 190/41，`GtkMenu` 审计
        从 397/36 降至 396/36；同时按刚刚执行的审计输出校正了本节其他长期过期的静态基线。已完成 GTK 3 Debug
        history 目标与完整构建、3/3 unit，并以 Homebrew GTK 4.22.4 click/sequence-claim API `-Werror` 探针
        编译；整仓 GTK4 编译仍受 `gui/gtk.h`、`develop/imageop.h` 的 GTK3-only 公共类型阻塞。显示器唤醒后须
        手工检查每行的主/Shift/次键、双/三击、pointer/keyboard menu、两个 Action、history 回退/undo、主题和窄侧栏。
  - [x] Lighttable snapshots 按钮的次键 context menu 已同样迁移为 all-button click controller，鼠标菜单通过
        当前事件定位、keyboard popup 仍按按钮锚定；Ctrl+次键不进入菜单，保留已迁移的 Ctrl 重命名路径，普通 toggle
        与 restore 行为不变。菜单 Action、GtkMenu 本体、Entry activate、snapshot 绘制/恢复仍留待对应阶段。旧事件
        从 190/41 降至 187/40，`GtkMenu` 审计从 396/36 降至 395/36。已完成 GTK 3 Debug snapshots 目标与完整构建、
        3/3 unit；整仓 GTK4 编译仍受 `gui/gtk.h`、`develop/imageop.h` 的 GTK3-only 公共类型阻塞。显示器唤醒后须
        手工检查每项 snapshot 的普通/Ctrl/次键、Ctrl+次键、双/三击、pointer/keyboard menu、菜单 Action、Entry、
        restore、绘制和主题。
  - [x] `src/gui/gtk.h` 的 `dt_gdk_cairo_surface_create_from_pixbuf()` 已删除。全仓精确搜索确认该
        pixbuf→Cairo helper 没有消费者；其 `GdkWindow` 参数和 GTK 3 的
        `gdk_cairo_surface_create_from_pixbuf()` 因而不保留为过渡 shim。静态审计的
        `GdkWindow/widget_get_window` 从 130/17 降至 129/16。以 Homebrew GTK 4.22.4 和项目最小头搜索路径
        对 `gui/gtk.h` 执行 `clang -fsyntax-only -Werror` 后，该 helper 已不再报错；余下 11 个独立阻塞为
        `GdkEventScroll` 的公共 helper、`GtkCallback`、entry 的旧 DnD/width API、`GdkEventKey` 和 `GtkMenu`。
        已完成 GTK 3 Debug 主应用与完整构建、3/3 unit；此项没有可用的 GUI 路径，显示器唤醒后的手工产品验证不适用。
  - [x] 所有 14 处 GTK3 GtkCallback 引用已改为项目的
        dt_gui_widget_callback_t(GtkWidget *, gpointer)：Bauhaus combobox value-changed closure、
        dt_ui_container_foreach() 和其 GTK 3 container 遍历调用点均保留原有函数 ABI；只有连接到 GLib
        closure 时才以 GCallback 作边界转换。此项不迁移 container child traversal，也不改变回调触发、参数
        或所有权。Homebrew GTK 4.22.4 最小 gui/gtk.h clang -fsyntax-only -Werror 探测从 11 个公共阻塞
        降至 10 个，余下为 GdkEventScroll、entry 的旧 DnD/width API、GdkEventKey 和 GtkMenu。
        已完成 GTK 3 Debug 主应用与完整构建、3/3 unit；纯函数指针 typedef 替换没有独立手工 GUI 路径，显示器
        唤醒后的产品路径验证不适用。
  - [x] 18 处 entry width/max-width 设置已统一为项目的 GtkEditable helper；GTK 4 分支调用原生
        gtk_editable_set_width_chars()/gtk_editable_set_max_width_chars()，GTK 3 分支保留等价的 GtkEntry
        调用，因而不改变当前尺寸请求或容器分配语义。dt_ui_entry_new() 也走同一 helper；它的
        gtk_drag_dest_unset() 则保留在单独的 DnD 阶段，不能错误地用 GtkWidget:can-target 替换（后者会
        禁用整个 entry 的指针输入）。Homebrew GTK 4.22.4 gui/gtk.h 探测从 10 个公共阻塞降至 9 个，余下为
        GdkEventScroll、旧 DnD、GdkEventKey 和 GtkMenu；对其余已知阻塞作前置声明的隔离头探测已编译实际
        helper。已完成 GTK 3 Debug 主应用与完整构建、3/3 unit。显示器唤醒后仍须在 GTK 4 下手工检查 range、
        modulegroups、预设、元数据、过滤器与 IOP rename entries 的字符宽度、最大宽度、窄窗口与主题布局。
  - [x] 无消费者的 dt_gui_translated_key_state() 已从 gui/gtk.h 和 gtk.c 删除；精确全仓搜索确认它没有
        调用点，故不以新的键事件包装器保留。静态旧事件从 187/40 降至 185/40；Homebrew GTK 4.22.4
        gui/gtk.h 探测从 9 个公共阻塞降至 8 个，余下为 GdkEventScroll、旧 DnD、dt_gui_search_start() 的
        GdkEventKey 和 GtkMenu。已完成 GTK 3 Debug 主应用与完整构建、3/3 unit；删除的 helper 没有独立
        产品路径，显示器唤醒后的手工 GUI 验证不适用。
  - [x] TreeView 搜索入口已按 GTK4 模型迁移：preferences presets、shortcuts 和 actions 的 SearchEntry
        在 GTK 4 分支均以 gtk_search_entry_set_key_capture_widget() 从对应 TreeView 的 bubble phase 接收可搜索
        按键；shortcuts/actions 的 Delete controller 保持 target phase，故先消费删除而不会进入搜索。SearchEntry
        同时改按 GtkEditable 传给 TreeView、清空文本，并使用其 GTK4 专用 placeholder API。GTK3 分支仍保留原始
        key-press-event 与 dt_gui_search_start()；后者已从 GTK4 公共头排除。过期的 preferences key 声明也删除，
        静态旧事件从 185/40 降至 184/39；双版本分支使 TreeView 审计从 2331/28 至 2334/28，TreeView 本体仍留待
        列表阶段删除。Homebrew GTK 4.22.4 gui/gtk.h 探测从 8 个公共阻塞降至 7 个；SearchEntry/Editable/
        TreeView API probe 以 -Werror 编译（仅明确压制未来 TreeView 替换阶段的 deprecated 警告）。已完成 GTK 3
        Debug 主应用与完整构建、3/3 unit。显示器唤醒后仍须检查三个列表的输入即搜、Delete、箭头上下匹配、Escape、
        activate/stop-search 焦点归还、对话框取消及窄窗口主题。
  - [x] 无消费者的 dt_gui_get_scroll_delta() 已从 gui/gtk.h 和 gtk.c 删除；精确搜索确认仅有声明与定义，
        仍被使用的单位 delta 和双轴 deltas helpers 未变。静态旧事件从 184/39 降至 182/39；Homebrew GTK
        4.22.4 gui/gtk.h 探测从 7 个公共阻塞降至 6 个，余下为四个 GdkEventScroll helpers、旧 DnD 和
        GtkMenu。已完成 GTK 3 Debug 主应用与完整构建、3/3 unit；删除的 helper 没有独立产品路径，显示器
        唤醒后的手工 GUI 验证不适用。
  - [x] 无条件返回 true 的 dt_gui_ignore_scroll()、Retouch wdbar raw scroll callback 与通用 Notebook raw
        scroll callback 已一并删除：两个 callback 原先均在入口立即传播，后续参数更新/tab 切换分支不可达。
        Retouch 的 scroll signal/mask 与 Notebook 的 scroll signal/mask 同时移除；GTK3 controller proxy 直接按
        原有 side-panel 条件转发到 ScrolledWindow，保持侧栏优先滚动。静态旧事件从 182/39 降至 178/39、event
        mask 从 22/13 降至 21/13；Homebrew GTK 4.22.4 gui/gtk.h 探测从 6 个公共阻塞降至 5 个，余下为三个
        GdkEventScroll helpers、旧 DnD 和 GtkMenu。已完成 GTK 3 Debug 主应用与完整构建、3/3 unit。显示器
        唤醒后仍须检查 Retouch wdbar、IOP/Library Notebook 和左右侧栏的离散 wheel/touchpad 传播、窄侧栏和主题。
  - [x] Bauhaus 弹窗与第二预览窗口的滚轮已改接现有的 both-axes + discrete scroll controller：
        Bauhaus 继续以 x+y 的离散单位调整组合框/滑块范围，第二预览继续从 controller snapshot 取得当前位置与
        修饰键后缩放。两处的 raw scroll signal 和 scroll mask 均删除；静态旧事件从 178/39 降至 176/39，
        event mask 匹配维持 21/13（剩余复合 mask 仍被计数）。已完成 GTK 3 Debug 主应用与完整构建、3/3 unit。
        显示器唤醒后须检查 Bauhaus 下拉/滑块弹窗和第二预览窗口的鼠标滚轮、触控板、Ctrl 约束缩放及焦点/关闭路径。
  - [x] Culling 的滚轮迁移为 handled scroll controller，并新增基于 controller snapshot 的 raw/unit delta
        helper 供精确路径复用；旧 GdkEventScroll adapter 仍暂留给 Thumbtable 与全局快捷键。平滑 Ctrl 缩放、平滑
        平移、stop 事件的延迟缩放收尾及离散 Ctrl 缩放/导航均保持原先分支；缩放焦点直接采用 controller 给出的
        widget-local 坐标，删除旧 root-window 坐标换算。静态旧事件从 176/39 降至 173/39，GdkWindow/widget_get_window
        从 129/16 降至 123/16。已完成 GTK 3 Debug 主应用与完整构建、3/3 unit。显示器唤醒后须检查 Culling
        的鼠标滚轮导航、Ctrl 离散缩放、平滑触控板缩放/平移、停止后的清晰 surface 重载、焦点缩放及有/无缩放图像。
  - [x] Thumbtable 的 Filemanager 分数/单位滚动、Filmstrip 移动和 Ctrl 缩放迁移为 handled scroll controller，
        并全部复用 controller snapshot delta helper；原始 scroll signal 删除，保持其原先“消费事件、不让父
        ScrolledWindow 继续移动”的语义。静态旧事件从 173/39 降至 170/39。已完成 GTK 3 Debug 主应用与完整构建、
        3/3 unit。显示器唤醒后须检查 Filemanager 的 fractional-scrolling 开/关、触控板与鼠标滚轮、Filmstrip
        横向/Shift 移动、Ctrl 改变底部面板高度、Ctrl 改变网格缩略图大小及鼠标悬停图像更新。
  - [x] 三个公共 GdkEventScroll helper 与它们的 gui/gtk.c adapter 已删除；Culling/Thumbtable 已使用
        controller snapshot，而全局 GTK3 shortcut dispatcher 在本地将 raw event 转换为同一 snapshot，以保持
        mapping-mode 滚轮速度录制、单位 delta 累积、修饰键、事件时间和普通 scroll/pan Action 分派。
        静态旧事件从 170/39 降至 164/39。Homebrew GTK 4.22.4 的最小 gui/gtk.h clang -fsyntax-only -Werror
        探针从 5 个公共阻塞降至 2 个，余下仅为旧 DnD 的 gtk_drag_dest_unset() 和 GtkMenu。已完成 GTK 3 Debug
        主应用与完整构建、3/3 unit。显示器唤醒后须检查 mapping mode 的中键/滚轮速度录制、普通 scroll/pan
        shortcuts、Bauhaus mapping mode 与全局 Action 分派；全局 dispatcher 的 controller 化仍留在键盘/窗口阶段。
  - [x] Bauhaus widget scroll controller 不再以 gtk_get_current_event() 读取 GdkEventScroll：新增的
        shortcut dispatcher controller 入口复用同一快照状态机处理 mapping-mode 滚轮，普通 slider/combobox
        路径从 snapshot 取得修饰键；GTK3 保留原有 window 归属判断来限制 force range，GTK4 则以 controller
        目标 widget 保持同一范围。静态旧事件从 164/39 降至 163/39。已完成 GTK 3 Debug 主应用与完整构建、3/3
        unit。显示器唤醒后须检查普通 slider/combobox 滚轮、Shift/Ctrl 范围、force 操作、mapping-mode 滚轮
        速度录制、侧栏滚动传播和触控板离散累积。
  - [x] Colorequal graph 的 raw scroll signal 与 scroll mask 已替换为 handled controller：普通滚动经新的
        dt_bauhaus_widget_scroll() 保留所选节点的焦点、滑块/combobox 步进及修饰键语义，Alt 滚动经
        dt_ui_notebook_scroll() 以原 next/previous 方向切换页面，mapping mode 继续进入 shortcut dispatcher。
        不再用 gtk_widget_event() 重投事件。静态旧事件从 163/39 降至 161/39。已完成 GTK 3 Debug 主应用与完整
        构建、3/3 unit。显示器唤醒后须检查八个节点的鼠标滚轮/触控板离散累积、Alt 页面切换、Shift/Ctrl 范围、
        mapping-mode 速度录制、middle-click 显示 slider、曲线拖动、窄侧栏和主题。
  - [x] Thumbtable 的 enter/leave raw signal 已改为 target-phase motion controller；controller crossing
        snapshot 继续只在从子缩略图进入空白区域时清除 mouse-over，并在离开时继续忽略 inferior、GTK grab 和
        pointer grab。原先仅返回 true、没有状态或副作用的 navigation leave listener 同时删除。Thumbtable 的
        显式 enter/leave event mask 由 controller 在 GTK3 分支按需添加，其他 raw 输入路径不变。静态旧事件从
        161/39 降至 157/39。已完成 GTK3 Debug 主应用与完整构建、3/3 unit；显示器休眠，尚未手工验证
        Filemanager/Filmstrip/Culling 的空白区进入、缩略图间移动、grab、overlay 隐藏、mouse-over 清除、
        Ctrl 缩放、主题和窄面板。
  - [x] Retouch 的 wavelet-decomposition bar 已用 all-button click gesture 和 motion controller 取代 raw
        press/release/motion/leave signal 与显式 mask；主键点击、底部/顶部 slider cursor 拖动、当前 scale
        选择、GUI update 时的消费、focus、重绘和 history 更新沿用原有分支，其他按键也和原来一样消费。
        Cairo draw 路径暂不改动，留给 snapshot 阶段。静态旧事件从 157/39 降至 153/38，event mask 从
        21/13 降至 20/12；已完成 GTK3 Debug 主应用构建和 Homebrew GTK 4.22.4 gesture/controller -Werror
        API 探测。显示器休眠，尚未手工验证 Retouch 的左/右拖动、顶/底 slider、快速点击、离开后 cursor
        reset、history/undo、模块禁用、主题、HiDPI 和窄侧栏。
  - [x] Blend GUI 的 parametric-mask gradient sliders 已将 raw enter/leave signal 改为 motion controller；
        进入时从 controller snapshot 读取 Shift/Ctrl，保留 mask/channel preview、焦点、已排队离开恢复的取消和
        save-for-leave 状态；离开时仍以原一秒 timeout 恢复非 sticky 显示。回调原先始终传播 crossing event，
        因而不引入额外 claim。静态旧事件从 153/38 降至 151/38；已完成 GTK3 Debug 主应用构建。显示器休眠，
        尚未手工验证 Lab/RGB/RAW slider 的 Shift、Ctrl、Shift+Ctrl、快速移入移出、sticky display、焦点、
        timeout、主题与窄侧栏。
  - [x] 第二预览窗口的预览离开和顶部控制栏 enter/leave 已改用 motion controller；控制栏仍以 crossing
        snapshot 忽略进入子控件的 leave，并继续只改 opacity/overlay pass-through，保留 macOS tooltip
        tracking-area 前提。预览控件移除只为 crossing 设置的 event mask，原有按键、拖动、配置和 DnD 路径未动。
        静态旧事件从 151/38 降至 148/38，event mask 从 20/12 降至 19/12；已完成 GTK3 Debug 主应用构建。
        显示器休眠，尚未手工验证第二窗口的进入/离开、控制栏/子按钮 hover、tooltip、pin、主键拖动、中键
        1:1、双击 fit、滚轮、拖放、关闭及主题。
  - [x] 第二预览窗口的主键拖动、双击 fit、中键 1:1、释放重绘和 motion 已改用 all-button click gesture
        与同一 motion controller；主键/中键仅在原先会消费的按压时 claim，次键按压仍可继续向窗口快捷键
        dispatcher 传播。拖动仍由 current state 的 Button1 mask 与既有 logical-pixel 坐标驱动，移除了剩余
        preview event mask。静态旧事件从 148/38 降至 145/38，event mask 从 19/12 降至 18/12；已完成 GTK3
        Debug 主应用构建。显示器休眠，第二预览窗口的 pointer 输入人工验证仍保持上一项所列待办。
  - [x] Colorequal graph 的 node hit-test、hover redraw 和拖动位置更新已由 motion controller 接管，
        不再读取 GdkEventMotion；button press/release 暂保留在 Bauhaus input 阶段，因为次键仍须把原始事件
        转发给当前 slider。静态旧事件从 145/38 降至 144/38，event mask 仍为 18/12。已完成 GTK3 Debug
        主应用构建；显示器休眠，尚未手工验证八个节点的 hover、拖动、双击 reset、中键 slider 显示、次键
        Bauhaus popup、mapping、滚轮、主题与窄侧栏。
  - [x] Lighttable 缩略图背景、图像与信息栏的 hover 已改由 motion controller 驱动；主 motion 与
        DnD drag-motion 复用同一无坐标的 overlay/mouse-over 状态更新，图像/信息栏 enter/leave 继续只管理
        既有 prelight 状态。三处仅为 hover 配置的 event mask 已移除；不改 context menu、DnD 接收、绘制或
        缩略图动作。静态旧事件从 144/38 降至 141/37，event-mask 类别仍为 18/12（剩余 set-events 调用
        仍被审计）。已完成 GTK3 Debug 主应用与完整构建、3/3 unit；显示器休眠，尚未手工验证
        Filemanager/Filmstrip/Culling 的缩略图进入/离开、block/extended overlay timeout、mouse-over ID、
        图标控件、右键菜单、拖放、主题、HiDPI 与窄面板。
  - [x] Culling 的 enter/leave crossing 已改用 target-phase motion controller 和已有的 crossing snapshot；
        进入子缩略图后的空白区仍清除 mouse-over，离开到子缩略图、GTK grab 或 pointer grab 仍不算真正离开，
        隐藏 widget 时也继续只重置 mouse-inside。只为 crossing 配置的 event mask 与两个 raw signal 已移除；
        点击、拖拽、滚动、绘制和 DnD 不在本批次。静态旧事件从 141/37 降至 139/37，event-mask 类别仍为
        18/12（controller 在 GTK3 兼容分支按需添加 crossing mask）。已完成 GTK3 Debug 主应用与完整构建、
        3/3 unit；显示器休眠，尚未手工验证 Culling/Grid/Loupe 的缩略图间与空白区进入/离开、mouse-over
        清除、overlay、grab、主/中/次键、拖拽平移、缩放、滚动、右键菜单、DnD、主题、HiDPI 与窄面板。
  - [x] Darkroom navigation 缩略图的拖动位置更新已由 motion controller 接管，直接使用 controller 的
        widget-local 坐标并继续按同一 allocation 调用既有 position helper。原 button press/release（包括
        中键转发到中心画布）、滚动、pinch、绘制和尺寸逻辑均未改。静态旧事件从 139/37 降至 137/37，
        event-mask 类别仍为 18/12。已完成 GTK3 Debug 主应用与完整构建、3/3 unit；显示器休眠，尚未手工验证
        navigation 的主键点击/拖动定位、边界、缩放后拖动、中键画布转发、滚轮、pinch、视图切换、主题、HiDPI
        与窄侧栏。
  - [x] Thumbtable 的 motion 已改用 controller 的 widget-local 坐标；鼠标下缩略图查找与 Filemanager
        Ctrl 缩放焦点直接复用该缓存，不再把 root 坐标减去 widget window 原点。enter/leave controller、
        按键、选择、滚动、绘制与 DnD 保持原有路径。静态旧事件从 137/37 降至 136/37，
        GdkWindow/widget_get_window 从 123/16 降至 119/15，event-mask 类别仍为 18/12。已完成 GTK3 Debug
        主应用与完整构建、3/3 unit；显示器休眠，尚未手工验证 Filemanager/Filmstrip 的缩略图 hover、
        Ctrl 缩放焦点、缩放前后鼠标下图像保持、点击/多选、滚动、右键菜单、DnD、主题、HiDPI 与窄面板。
  - [x] dt_iop_togglebutton_new() 已改为类型化的 click-controller 工厂：回调从 controller snapshot
        接收 widget、真实 button 与 modifier，仅在原回调已处理时 claim。Blend 的 15 个工具与 Toneequal
        exposure-mask toggle 因而不再接收 GdkEventButton；blend order、mask mode、reset/invert、mask
        display 的 Shift/Ctrl 组合、suppress/polarity、shape 的 Ctrl 连续创建及 edit/restricted 分支保持原有
        处理结果。静态旧事件从 136/37 降至 120/35，event-mask 类别仍为 18/12。已完成 GTK3 Debug 主应用与
        完整构建、3/3 unit，以及 Homebrew GTK 4.22.4 click/controller -Werror API 探测；显示器休眠，
        尚未手工验证所有 Blend mask mode、primary/secondary click、Shift/Ctrl/Shift+Ctrl mask display、
        reset/invert/polarity、shape 与连续创建、edit/restricted、history/undo、Toneequal exposure mask、
        module disabled、主题、HiDPI 和窄侧栏。
  - [x] Culling 的主 pointer 状态机已由 all-button click gesture 和 motion controller 接管。按下、移动、
        释放与 refocus 缓存统一为 widget-local 坐标；主键单击/双击、次键菜单、中键缩放、Shift 仅缩放或平移
        鼠标下图像、preview hand-tool 拖动、selection 和 click/drag 判定沿用原有分支，controller 在原先
        会消费的路径 claim。旧的 root-window 换算、raw press/release/motion signal 及其 event mask 已删除；
        触控板 pinch raw event 和公共 dt_culling_zoom_add() 坐标转换不在本批次。静态旧事件从 120/35 降至
        117/35，GdkWindow/widget_get_window 从 119/15 降至 111/15，event-mask 类别仍为 18/12。已完成
        GTK 3 Debug 主应用目标与完整构建、3/3 unit；显示器休眠，尚未手工验证 Culling/Grid/Loupe 的
        主/中/次键、双击、Shift、hand-tool 拖动、click/drag 阈值、selection、context menu、refocus、
        滚轮/触控板 pinch、DnD、主题、HiDPI 与窄面板。
  - [x] Thumbtable 的按下/释放状态机已改用 all-button click gesture。次键菜单只在原 helper 已处理时
        claim；Filmstrip 的 primary press 与 Darkroom 双击激活继续不 claim，以保留拖放和延迟单选前的原始
        传播语义。Filemanager 双击预览、primary focus、空白区清选、Ctrl/Cmd toggle、Shift 范围选择、
        Filmstrip 的双击取消延迟单选和 release 时的 grid 同步均沿用原有分支；旧 raw press/release signal
        及其 event mask 已删除，motion、滚动、绘制和旧 DnD 注册暂不改。静态旧事件从 117/35 降至 115/34，
        event-mask 类别仍为 18/12。已完成 GTK 3 Debug 主应用目标与完整构建、3/3 unit；显示器休眠，
        尚未手工验证 Filemanager/Filmstrip 的 primary/secondary/middle click、双击预览/激活、拖放、
        延迟单选、Ctrl/Cmd/Shift 选择、空白区清选、键盘菜单、滚动、hover、主题、HiDPI 与窄面板。
  - [x] GtkDarktableThumbnailBtn 已删除重复的 raw enter/leave vfunc 与全事件 mask。全部九个
        thumbnail overlay 按钮构造点已连接 thumbnail.c 的 motion controller，继续负责 prelight、
        mouse-over、星级级联高亮、reject element、tooltip 和 redraw；按钮的 click/action 路径及
        Cairo draw vfunc 暂不改。静态旧事件从 115/34 降至 113/33，event-mask 从 18/12 降至 17/11。
        已完成 GTK 3 Debug 主应用目标与完整构建、3/3 unit；显示器休眠，尚未手工验证缩略图的 reject、
        星级、color label、local copy、altered、tags、group 与 audio overlay 的 hover、prelight、tooltip、
        click/action、隐藏状态、主题、HiDPI 与窄面板。
  - [x] Culling hand-tool、Color Labels 的命名 entry 与 Tagging 的浮动 tag entry 已从 raw focus-out
        event 收敛为 notify::has-focus；回调只在 has-focus 变为 false 时分别取消 hand tool 或销毁对应
        floating window/popover，因而不会在获得焦点时误清理。两条 entry 的 focus-only event mask 同时删除，
        其键盘、提交、Esc、completion、Wayland popover 与 X11/macOS 浮动窗口路径不变。静态旧事件从
        113/33 降至 111/33，event-mask 从 17/11 降至 15/9。已完成 GTK 3 Debug 主应用目标与完整构建、
        3/3 unit；显示器休眠，尚未手工验证 Culling hand-tool 的失焦取消、Color Labels 与 Tagging 在
        键盘提交/Esc、点击外部、completion、Wayland popover、X11/macOS 浮动窗口、模块销毁、主题及窄面板
        下的关闭与焦点返回。
  - [x] 快捷键设置窗口的 Actions TreeView 已由 all-button click controller 接收输入。主键单击的
        选择/折叠、双击激活、空白区清选和焦点保持，以及次键定位并展开当前 shortcut action，均沿用原有
        分支；原回调始终消费事件，因此 gesture 也无条件 claim。TreeView 模型、搜索、键盘导航、tooltip
        与 shortcut selection 联动不在本批次改动。静态旧事件从 111/33 降至 110/33。已完成 GTK 3 Debug
        主应用目标与完整构建、3/3 unit；显示器休眠，尚未手工验证 Actions TreeView 的单击、双击、右键、
        空白区、搜索、键盘导航、selection 联动、主题、HiDPI 与窄窗口。
  - [x] 左右侧栏空白区的模块显示菜单已改由 all-button click controller 接收；次键继续打开
        show/hide modules 菜单，其他按键仍像原回调一样消费。旧 raw button-press signal 与仅为该区域
        设置的 press/release mask 已删除；侧栏的 DnD、capture-phase 滚动、模块 header、边界收缩和菜单
        内容不在本批次改动。静态旧事件从 110/33 降至 109/33，event-mask 从 15/9 降至 14/9。已完成
        GTK 3 Debug 主应用目标与完整构建、3/3 unit；显示器休眠，尚未手工验证左右侧栏空白区的次键菜单、
        其他按键消费、DnD、滚动、模块增删、主题、HiDPI 与窄窗口。
  - [x] 主窗口的 urgency-hint 清除和 TreeView cell editor 的失焦提交已不再接收 GdkEvent：前者监听
        notify::is-active，后者监听 notify::has-focus 并只在焦点实际失去时 editing-done/remove-widget。
        窗口的 delete、配置、单实例路径，以及 cell renderer 编辑/取消/提交模型更新没有改动。静态旧事件从
        109/33 降至 107/33。已完成 GTK 3 Debug 主应用目标与完整构建、3/3 unit；显示器休眠，尚未手工验证
        主窗口前后台切换/urgency，以及各 TreeView cell editor 的开始编辑、失焦提交、Esc 取消、行切换、
        对话框焦点切换、主题与窄窗口。
  - [x] Import 的 Files、Places 与 Folders TreeView press handlers 已改由 all-button click controller
        接收。Files 仍只在缩略图列切换或主键双击接受导入时 claim；Places 保持命中行即消费，只有主键首击
        改变根目录；Folders 仅在原先成功选择文件夹时 claim，双击展开/收起和 expander 点击继续传播，并保留
        100ms 清除寄生选择的 idle callback。模型/排序、selection、缩略图加载、导入接受、目录扫描和对话框
        生命周期未改。静态旧事件从 107/33 降至 104/32。已完成 GTK 3 Debug 主应用目标与完整构建、3/3 unit；
        显示器休眠，尚未手工验证 Files 缩略图列/多选/双击导入，Places 根目录切换，以及 Folders 的 expander、
        空白区、双击、Shift/Ctrl、排序、异步扫描、取消、主题、HiDPI 与窄窗口。
  - [x] ashift 的 vertical/horizontal/both fit、auto/rectangle/lines structure 六个工具栏按钮已从 raw
        button-press signal 改为 all-button click controller。回调先将真实 button/modifier 提取为纯值交给
        原有布尔核心；只有核心原先会消费的路径才 claim，因此 fit/auto 的非主键继续传播，rectangle/lines
        保留原先所有按键均消费的语义。vertical/horizontal/both 的 Ctrl/Shift 拟合模式、auto 的
        Ctrl/Shift enhance、toggle 状态、异步 preview job、history、focus 和 redraw 逻辑均未改变。旧事件从
        256/50 降至 250/49，其他静态类别不变。已完成 ashift 生成式 IOP 目标的 GTK 3 Debug build/unit
        （生成的 `introspection_ashift.c` 包含该源文件）和此前 Homebrew GTK 4.22.4 click/current-button/
        modifier `-Werror` API 探针；整仓 GTK4 编译仍受 `gui/gtk.h`、`develop/imageop.h` 的 GTK3-only
        公共类型阻塞。显示器唤醒后须手工检查每个 fit 和 structure button 的主/次键、Ctrl、Shift、
        Ctrl+Shift、启用/禁用模块、preview 未就绪时的 job、history/undo、overlay、主题、HiDPI 与窄面板。
- [ ] 将 `GtkDarktablePaintCell` 与所有 cell renderer 消费者一起删除，图标绘制进入 list item
      widget factory；不为已弃用 TreeView 保留 paint-cell 兼容层。
- [ ] 迁移 `DtBauhausWidget` 的 popup、焦点、文本输入、滚轮精调和拖动。popup 使用受父控件
      管理的 popover，不依赖 keep-above、window type hint、grab 或全局窗口坐标。
- [ ] 普通 `GtkDrawingArea` 连接改成 `gtk_drawing_area_set_draw_func()`；自定义 widget 的
      `draw` vfunc 改成 `snapshot`。第一版继续通过 `gtk_snapshot_append_cairo()` 复用已验证的
      Cairo 绘制，不趁迁移重写为 GSK shader/render node。
  - [x] Lighttable 固定工作区的 `ratings` 面板已用标准 DrawingArea draw func、click 和 motion
        controller 取代 `draw`/button/motion/leave signal 与显式 event mask。星级坐标、hover 的半透明
        填充、`darktable.control->element`、tooltip、0--5/reject 快捷键和按下时对当前操作图像应用评级
        的语义不变；任意按键仍由该 rating gesture claim，与原回调始终消费 button press 一致。GTK 3
        保留显式背景绘制，GTK 4 让标准 DrawingArea CSS snapshot 先绘制背景，再直接叠加既有 Cairo
        星级，移除了只为 blit 而创建的临时 Cairo surface。旧事件从 283/56 降至 275/55，event mask 从
        34/17 降至 33/16，allocation/旧尺寸从 99/28 降至 97/27。已完成 GTK 3 Debug/unit 与 Homebrew
        GTK 4.22.4 DrawingArea draw-func/color/click/motion `-Werror` API 探针；整仓 GTK4 编译仍受
        `gui/gtk.h`、`develop/imageop.h` 的 GTK3-only 公共类型阻塞。显示器唤醒后须手工检查 Lighttable
        底部 rating 面板的 hover/leave、0--5 星和 reject 应用、多选、快捷键、主题、HiDPI、窄窗口和
        panel 重建；不得把未运行的手工路径表述为通过。
  - [x] colorharmonizer 的所有色相 swatch 由同一工厂创建，现通过已有的
        `dt_gui_drawing_area_set_draw_func()` 设置标准 `GtkDrawingArea` draw func；callback 只继续
        读取 `swatch-index`、参数和 Cairo context，不再是 boolean `draw` signal。24×24 请求尺寸、
        各规则的未使用节点灰色填充和参数变更后的 queue redraw 不变；没有输入、Action 或 DnD
        路径。静态脚本未单列 draw signal，计数不变。已完成 GTK 3 Debug/unit、Homebrew GTK 4.22.4
        原生 DrawingArea draw-func API 编译探针；显示器唤醒后须手工检查每种色彩和谐规则、custom
        hue/nodes 改动后的即时重绘，以及窄侧栏主题背景。
  - [x] exposure 的 input/target 两个纯色 `GtkDrawingArea` 也改用同一 draw-function 封装，直接
        接收 GTK 提供的 width/height，移除 `GtkAllocation` 和 `gtk_widget_get_allocation()`。两条
        ARGB 临时 surface、原有内边距/纵向 margin、源 spot 取样颜色、target LCh→Lab→XYZ→sRGB
        计算以及 lightness slider 变更时的 target redraw 均不变；无 pointer、Action 或 DnD 路径。
        allocation/旧尺寸匹配从 205/47 降至 201/46。已完成 GTK 3 Debug/unit、Homebrew GTK 4.22.4
        原生 DrawingArea draw-func API 编译探针；显示器唤醒后须手工检查 correction/measure 两种
        模式、取样 source、lightness slider 连续调节、主题背景和窄侧栏布局。
  - [x] channelmixerrgb 的 illuminant、input 与 target 三个纯色 `GtkDrawingArea` 亦接入该
        draw-function 封装，并以 callback width/height 取代各自的 `GtkAllocation` 查询。原有
        illuminant 的 RAW/色温/荧光/LED 计算、input 取样、target LCh 调整、边距、tooltip 与所有
        queue redraw 不变；没有 pointer、Action 或 DnD 路径。allocation/旧尺寸匹配从 201/46
        降至 195/45。已完成 GTK 3 Debug/unit、Homebrew GTK 4.22.4 原生 DrawingArea draw-func API
        编译探针；显示器唤醒后须手工检查 CAT 页面、各 illuminant 选项和温度控件、mapping 的
        correction/measure/use-mixing、LCh sliders 及窄侧栏布局。
  - [x] colormapping 的 source/target cluster 预览改由各自 `GtkDrawingArea` 的 draw func 绘制，
        callback 仍按 widget 身份选择完全相同的 mean/variance 数组、Cairo background、9×9 色块和
        Lab→sRGB transform，只以 callback width/height 取代 `GtkAllocation`。采集 source/target、
        参数重置与异步处理后的原有 queue redraw 不变，且没有输入、Action 或 DnD 路径。
        allocation/旧尺寸匹配从 195/45 降至 193/44。已完成 GTK 3 Debug/unit、Homebrew GTK 4.22.4
        原生 DrawingArea draw-func API 编译探针；显示器唤醒后须手工检查 source/target acquisition、
        clusters/dominance/equalization 改动、无 cluster 的背景和窄侧栏尺寸变化。
  - [x] overlay 的缩略图 `GtkDrawingArea` 绘制回调现直接接收 width/height，移除旧
        `GtkAllocation` 查询；已有 overlay 图像的居中 surface、空状态的背景/交叉线/Pango 提示、
        图像选择和参数变更后的 redraw 保持。该批刻意不迁移同一控件的旧 DnD destination、drag
        motion/leave/received 或焦点路径。allocation/旧尺寸匹配从 193/44 降至 191/43。已完成 GTK 3
        Debug/unit、Homebrew GTK 4.22.4 原生 DrawingArea draw-func API 编译探针；显示器唤醒后须
        手工检查已有/缺失 overlay、150px 预览、主题、从 filmstrip 拖入/悬停/离开/完成和布局缩放。
  - [x] blurs 的 kernel preview 已使用 draw func 和 callback width；内核缓存的失效、分配、参数
        更新及 Cairo surface 绘制仍只由 preview 宽度驱动。GTK 3 继续用原 `GtkStyleContext`/`render_background`
        绘制背景，GTK 4 交由 DrawingArea 的 CSS snapshot，避免在 GTK 4 调用该旧渲染 API。没有输入
        或 Action 路径。allocation/旧尺寸匹配从 191/43 降至 189/42。已完成 GTK 3 Debug/unit、
        Homebrew GTK 4.22.4 原生 DrawingArea draw-func API 编译探针；显示器唤醒后须手工检查三种
        blur type、全部 kernel 参数、反复侧栏缩放造成的缓存重建，以及各 bundled theme 的背景连续性。
  - [x] AGX curve graph 改为 draw func，旧 allocation/style/ink 暂存不再写入 GUI struct，均改为
        callback 局部状态；仍从完整 callback height 扣除同一 resize handle 尺寸。曲线、标签、警告区、
        Pango DPI 与 Cairo 绘制保持，GTK 3 保留 `render_background`，GTK 4 使用 DrawingArea CSS
        snapshot；不改 graph Action、焦点或曲线参数。allocation/旧尺寸匹配从 189/42 降至 187/41。
        已完成 GTK 3 Debug/unit、Homebrew GTK 4.22.4 原生 DrawingArea draw-func API 编译探针；显示器
        唤醒后须手工检查曲线 section 展开/收起、全部基础/高级参数、warning 段、图表高度拖动及主题。
  - [x] lowlight vision 的完整 transition graph 已改用 DrawingArea draw func、primary click gesture、
        target-phase motion controller 和离散 scroll controller。绘制直接接收 GTK 的宽高，并仍从完整高度
        扣除 resize handle；hover/drag 曲线、六个 x-position 指示、标签和 brush 范围保持原有 Cairo
        几何。单击继续开始主键曲线/节点拖动、双击重置默认曲线，release 或 gesture cancel 清除 dragging；
        底部 resize handle 仍优先归 wrapper。motion/leave 保留 brush/节点预览，离散 scroll adapter 继续
        处理侧栏优先级、pointer-emulated 输入过滤和触控板步进，再调整 brush radius；原 graph Action、
        参数和 history 路径未变。旧事件从 383/64 降至 378/63，allocation/旧尺寸从 187/41 降至
        181/40。已完成 GTK 3 Debug/unit、Homebrew GTK 4.22.4 DrawingArea/gesture/motion/scroll API
        编译探针；显示器唤醒后须手工检查曲线/节点拖动、双击复位、主键取消、底部高度拖动、wheel 与
        touchpad brush radius、侧栏滚动优先级、所有预设、history/undo、窄侧栏和主题。
  - [x] monochrome 的虚拟颜色滤镜网格已改用 DrawingArea draw func、primary click gesture、
        target-phase motion controller 和离散 scroll controller，并删除显式 pointer/button/leave/scroll
        event mask。8×8 Lab 网格、滤镜圆、色彩拾取器重置、单击定位后拖动、双击默认值复位、release/
        cancel 写入 history、tooltip 开关和 leave 终止拖动均保留；滚轮仍按离散步长调整滤镜 size，且经
        通用 adapter 保持侧栏优先级与 pointer-emulated 输入过滤。grid Action 和 highlights slider 未变。
        旧事件从 378/63 降至 373/62，event mask 从 43/26 降至 42/25，allocation/旧尺寸从 181/40
        降至 175/39。已完成 GTK 3 Debug/unit、Homebrew GTK 4.22.4 DrawingArea/gesture/motion/scroll
        API 编译探针；显示器唤醒后须手工检查单击/拖动与拖动后 history、双击复位、主键取消、离开网格、
        wheel/touchpad size、侧栏滚动优先级、所有 monochrome 预设、色彩拾取器、窄侧栏和主题。
  - [x] colorcorrection 的 split-toning 网格已完整迁至 DrawingArea draw func、primary click gesture、
        target-phase motion/scroll/key controller，并删除显式 event mask。8×8 Lab 格、阴影/高光端点和线、
        单击 hover/drag、端点 focus、双击按当前选中端点或全部复位、saturation slider 联动及 history
        均保持；motion 通过 controller-local modifier snapshot 识别 primary drag，箭头/KP 箭头继续按
        原 accelerator precision 调整当前端点且仅在已选中时消费，离散滚轮仍反向调整 saturation 并经
        通用 adapter 保持侧栏优先级和 pointer-emulated 输入过滤。grid Action、色彩变换和 slider 未变。
        旧事件从 373/62 降至 363/61，event mask 从 42/25 降至 41/24，allocation/旧尺寸从 175/39
        降至 171/38。已完成 GTK 3 Debug/unit、Homebrew GTK 4.22.4 DrawingArea/gesture/motion/scroll/
        key API 编译探针；显示器唤醒后须手工检查两个端点 hover/drag、主键按住、双击各端点/空白、
        wheel/touchpad saturation、箭头/KP 箭头及 modifier 加速、焦点、history/undo、侧栏滚动优先级、
        窄侧栏和主题。
  - [x] rgblevels 的 black/gray/white 曲线图已改用 DrawingArea draw func、primary click gesture、
        target-phase motion controller、leave controller 和离散 scroll controller。绘制继续从完整高度扣除
        同一 resize handle；hover 最近 handle、拖动、双击复位、gesture cancel 后停止 dragging、色彩
        picker 取消和 history 语义均保持。底部 resize handle 继续优先于曲线点击；离散滚轮仍按当前
        handle 与 accelerator modifier 调整，并经通用 adapter 保持侧栏优先级和 pointer-emulated 输入
        过滤。channel tabs、levels Action、直方图和自动 levels 未变。旧事件从 363/61 降至 358/60，
        allocation/旧尺寸从 171/38 降至 167/37。已完成 GTK 3 Debug/unit、Homebrew GTK 4.22.4
        DrawingArea/gesture/motion/scroll API 编译探针；显示器唤醒后须手工检查 R/G/B tabs、三个
        handle hover/drag、双击复位、主键取消、底部高度拖动优先级、wheel/touchpad 与 modifier、
        色彩 picker、直方图、history/undo、窄侧栏和主题。
  - [x] atrous 的 multi-scale equalizer 曲线图已改用 DrawingArea draw func、primary click gesture、
        target-phase motion/enter/leave controller 和离散 scroll controller。曲线/频率直方图、hover
        预览、上下曲线选择、拖动笔刷、x-position 移动、双击按当前曲线复位、release 或 gesture
        cancel 后恢复 mix 均保持；底部 resize handle 仍优先。离散滚轮继续调整 brush radius，Alt+
        离散滚轮以同样方向逐步切换 notebook tab，不再重投递旧 GdkEvent；侧栏优先级及
        pointer-emulated 输入过滤仍由通用 adapter 处理。graph Action、参数和 history 未变。旧事件从
        358/60 降至 352/59，allocation/旧尺寸从 167/37 降至 161/36。已完成 GTK 3 Debug/unit、
        Homebrew GTK 4.22.4 DrawingArea/gesture/motion/scroll/notebook API 编译探针；显示器唤醒后
        须手工检查所有 tab、上下曲线 hover/拖动、x-position、双击复位、主键取消、底部高度拖动、
        wheel/touchpad radius、Alt+wheel tab、history/undo、频率直方图、窄侧栏和主题。
  - [x] tonecurve 的 Lab 曲线图已迁至 DrawingArea draw func、primary click gesture、
        target-phase motion/leave/scroll/key controller，并删除模块自有 event mask。Ctrl+单击在空白
        处添加节点、双击复位、hover 选点、主键拖动或新建点、离开时清除非拖动选择、平滑 wheel
        微调和 arrows/KP arrows 的 accelerator modifier 精度、focus、history 与 L/a/b autoscale
        限制均保持。右键节点菜单仍由现有 context-menu provider 承担：该公共 provider 及其
        `GdkEventButton` payload 会在菜单阶段集中迁移，而非在本图表路径另行复制。旧事件从
        352/59 降至 343/59，event mask 从 41/24 降至 40/23，allocation/旧尺寸从 161/36 降至
        155/35。已完成 GTK 3 Debug/unit、Homebrew GTK 4.22.4 DrawingArea/gesture/motion/scroll/key
        API 编译探针；显示器唤醒后须手工检查 L/a/b tabs 与 autoscale、节点 hover/主键拖动、Ctrl+
        单击添加、双击复位、主键取消、平滑 touchpad/wheel、arrow/KP arrows 及 modifier、右键节点
        菜单、color picker、history/undo、窄侧栏和主题。
  - [x] basecurve 的 RGB tone curve 已迁至 DrawingArea draw func、primary click gesture、
        target-phase motion/leave/scroll/key controller，并删除模块自有 event mask。Ctrl+单击空白
        添加节点、双击复位、hover 选点、主键拖动或新建点、离开时清除非拖动选择、平滑 wheel 微调和
        arrows/KP arrows 的 accelerator modifier 精度、focus 与 history 均保持。右键节点菜单继续由
        现有 context-menu provider 承担，并随公共 `GdkEventButton` provider 接口在菜单阶段统一
        迁移。旧事件从 343/59 降至 338/59，event mask 从 40/23 降至 39/22，allocation/旧尺寸从
        155/35 降至 149/34。已完成 GTK 3 Debug/unit、Homebrew GTK 4.22.4
        DrawingArea/gesture/motion/scroll/key API 编译探针；显示器唤醒后须手工检查节点 hover/主键
        拖动、Ctrl+单击添加、双击复位、主键取消、平滑 touchpad/wheel、arrow/KP arrows 及 modifier、
        右键节点菜单、exposure fusion、history/undo、窄侧栏和主题。
  - [x] rgbcurve 的 RGB 曲线图已迁至 DrawingArea draw func、primary click gesture、target-phase
        motion/leave/scroll/key controller，并删除模块自有 event mask。R/G/B 通道、autoscale 限制、
        hover 选点、主键拖动或新建点、Ctrl+单击添加、双击复位、离开时清除非拖动选择、平滑 wheel
        微调、arrows/KP arrows 的 accelerator modifier 精度、focus、color-picker reset 与 history 均
        保持；`darkroom_skip_mouse_events` 下的曲线 zoom/pan 路径也改用同一 controller metadata。
        右键 Action 菜单继续通过现有 context-menu provider，在公共 `GdkEventButton` provider 接口的
        菜单阶段统一迁移。旧事件从 338/59 降至 333/59，event mask 从 39/22 降至 38/21，
        allocation/旧尺寸从 149/34 降至 139/33。已完成 GTK 3 Debug/unit、Homebrew GTK 4.22.4
        DrawingArea/gesture/motion/scroll/key API 编译探针；显示器唤醒时须手工检查 R/G/B tabs 与
        autoscale、节点 hover/主键拖动、Ctrl+单击添加、双击复位、主键取消、平滑 touchpad/wheel、
        arrow/KP arrows 及 modifier、skip-mouse zoom/pan、右键节点菜单、色彩 picker、直方图、
        history/undo、窄侧栏和主题。
  - [x] colorzones 的主曲线和底部色带均已迁至 DrawingArea draw func；主图使用 primary click
        gesture（release/cancel 后清理区域编辑 dragging）、target-phase motion/leave/离散 scroll/key
        controller，底部色带使用 primary click gesture。L/C/h tabs、普通节点 hover/拖动或新建、区域编辑
        的节点 x-position/brush、Ctrl+单击添加、双击当前曲线复位、底部色带双击 zoom reset、离开时清理
        非拖动选择、skip-mouse zoom/pan、arrows/KP arrows modifier 精度、color-picker reset、display mask
        和 history 均保持；Alt+离散滚轮以同样方向切换 notebook tab，不再重投递 `GdkEvent`。右键节点
        Action 菜单继续交由现有 context-menu provider，并在公共 `GdkEventButton` provider 接口的菜单
        阶段统一迁移。旧事件从 333/59 降至 325/59，event mask 从 38/21 降至 37/20，allocation/旧尺寸从
        139/33 降至 129/32。已完成 GTK 3 Debug/unit、Homebrew GTK 4.22.4
        DrawingArea/gesture/motion/scroll/key/notebook API 编译探针；显示器唤醒时须手工检查 L/C/h tabs、
        普通及区域编辑下的节点 hover/拖动、新建/删除/复位、brush 与 x-position、release/cancel、底部色带
        双击复位、wheel/touchpad、Alt+wheel tabs、skip-mouse zoom/pan、arrow/KP arrows 及 modifier、
        右键菜单、色彩 picker、selection mask、history/undo、窄侧栏和主题。
  - [x] colorchecker 的 24/49 色块网格已迁至 DrawingArea draw func、primary click gesture 和
        target-phase motion controller，并删除模块自有 pointer/button/leave event mask。色块绘制、hover
        Lab tooltip、单击选择、双击按色块复位、color-picking 时 Shift+单击替换或新增色块、下拉列表/slider
        同步和 history 均保持。右键色块菜单继续由现有 context-menu provider 依据 `GdkEventButton` 的
        坐标创建 patch payload，并在公共 provider 菜单阶段统一迁移。旧事件从 325/59 降至 323/59，event
        mask 从 37/20 降至 36/19，allocation/旧尺寸从 129/32 降至 121/31。已完成 GTK 3 Debug/unit、
        Homebrew GTK 4.22.4 DrawingArea/gesture/motion API 编译探针；显示器唤醒时须手工检查 24/49
        layout、hover tooltip、选择、双击复位、Shift+picker 替换/新增、右键 patch 菜单、slider/combo
        同步、history/undo、窄侧栏和主题。
  - [x] rawdenoise 的四通道 transition graph 已迁至 DrawingArea draw func、primary click gesture
        （release/cancel 清除 dragging）、target-phase motion/leave controller 和离散 scroll controller。
        曲线笔刷拖动、hover preview、双击按当前 channel 复位、离开时隐藏非拖动 preview、channel tabs、
        滚轮调整 brush radius、Alt+离散滚轮按原方向切换 tabs、history 均保持；Alt+路径不再把原始
        `GdkEvent` 重投递给 notebook。旧事件从 323/59 降至 317/58，allocation/旧尺寸从 121/31 降至
        115/30。已完成 GTK 3 Debug/unit、Homebrew GTK 4.22.4
        DrawingArea/gesture/motion/scroll/notebook API 编译探针；显示器唤醒时须手工检查 all/R/G/B
        tabs、拖动/双击复位、主键取消、hover/leave preview、wheel/touchpad brush、Alt+wheel tabs、
        history/undo、窄侧栏和主题。
  - [x] denoiseprofile 的 wavelet transition graph 已迁至 DrawingArea draw func、primary click
        gesture（release/cancel 清除 dragging）、target-phase motion/leave controller 和离散 scroll
        controller。曲线笔刷拖动、hover preview、双击按当前 channel 复位、离开时隐藏非拖动 preview、
        RGB all/R/G/B 与 Y0/U0V0 两组 tabs、滚轮调整 brush radius 和 history 均保持；Alt+离散滚轮按
        原方向切换当前颜色模式所属 notebook，不再重投递 `GdkEvent`。旧事件从 317/58 降至 311/57，
        allocation/旧尺寸从 115/30 降至 109/29。`process_variance()` 写入的 variance 数值与
        `box_variance` 上的 GTK 3 `draw` 回调属于独立的处理线程→主线程 UI 更新问题，保留给异步
        UI 阶段迁移，不能在此输入迁移中改变 pixelpipe 线程模型。已完成 GTK 3 Debug/unit、Homebrew
        GTK 4.22.4 DrawingArea/gesture/motion/scroll/notebook API 编译探针；显示器唤醒时须手工检查
        RGB/Y0U0V0 tabs、拖动/双击复位、主键取消、hover/leave preview、wheel/touchpad brush、
        Alt+wheel tabs、variance 标签刷新、history/undo、窄侧栏和主题。
  - [x] filmicrgb 的 curve graph 已迁至 DrawingArea draw func、primary click gesture 与
        target-phase motion enter/leave controller。主图的 view-mode icon 单击循环、双击重置到 look、
        axis-label icon 切换、hover 高亮/tooltip、focus 与配置持久化均保持；只有实际处理 icon 的手势
        才 claim。右键图表菜单仍由依赖 `GdkEventButton` 坐标的 context-menu provider 创建，留待菜单阶段；
        highlight reconstruction mask 已在阶段 1 通过模块局部 controller/Action 迁移。旧事件从 311/57 降至
        308/57，allocation/旧尺寸从 109/29 降至 108/29。
        已完成 GTK 3 Debug/unit、Homebrew GTK 4.22.4 DrawingArea/gesture/motion API 编译探针；
        显示器唤醒时须手工检查 graph view/labels 单击和双击、hover/tooltip、右键 view/labels 菜单、
        highlight mask toggle、颜色吸管、history/undo、窄侧栏和主题。
  - [x] toneequal 的 advanced equalizer graph 已迁至 DrawingArea draw func、all-button click
        gesture（release/cancel 提交并清理 dragging）和 target-phase motion enter/leave controller。
        单击拖动的 4 EV 曲线笔刷、双击复位、节点 hover、离开时提交并隐藏 cursor、禁用模块时的 history
        触发、graph focus 与颜色吸管解锁语义均保持；此前 graph scroll 回调始终放行侧栏，迁移后不再为
        图表安装 scroll controller，使事件自然传播。图表尺寸缓存改用 allocated width/height。notebook press
        已改为 all-button click controller：始终执行相同的模块聚焦和颜色吸管解锁，但正常 tab 点击不 claim；
        仅 GUI reset guard 的原消费路径 claim。exposure-mask 自定义 toggle 仍依赖共享旧按键接口，留待 dtgtk
        基础控件阶段。最初迁移使旧事件从 308/57 降至 303/57，event mask 从 36/19 降至 35/18，allocation/
        旧尺寸从 108/29 降至 107/29；此次 notebook 准备又使当前全仓旧事件从 248/48 降至 247/48。已完成
        toneequal 生成式 IOP 的 GTK 3 Debug build/unit、Homebrew GTK 4.22.4 click API 编译探针；整仓 GTK4
        编译仍受 `gui/gtk.h`、`develop/imageop.h` 的 GTK3-only 公共类型阻塞。显示器唤醒时须手工检查 advanced
        graph hover/节点、拖动、双击复位、主键取消与离开提交、禁用模块、graph 与 side-panel wheel、notebook
        tab 点击、mask toggle、颜色吸管、history/undo、窄侧栏和主题。
- [ ] 将 click/motion/scroll/key/focus/enter/leave/stylus 统一迁移到控制器和 gesture，明确
      capture/bubble phase、claimed sequence、modifier 与快捷键优先级。
- [ ] 逐项验证 Darkroom 画布的缩放/平移、蒙版创建编辑、吸管、crop、曲线节点、右键 Action，
      以及 Lighttable Grid/Loupe 的单击、双击、空格拖动、滚轮、选择和拖放。
- [ ] 迁移 tablet pressure、触摸板平滑滚动和 macOS modifier 映射；快捷键中的 Primary/Meta
      差异使用平台映射，不把 GTK 3 的 `<Primary>` 假设带入 GTK 4。
- [ ] 确认所有 UI Cairo surface、GdkTexture/GdkPaintable 与缩略图缓存的引用、scale factor、
      alpha 和失效范围；不把 widget 快照保存为跨帧可写像素缓存。

阶段出口：dtgtk、Bauhaus、主画布和所有保留 IOP 自绘控件在 GTK 4 下功能完整；旧事件 signal、
`draw` vfunc、EventBox、GdkWindow 和 grab 代表性搜索归零。

## 阶段 5：Actions、快捷键、菜单和异步对话框

- [ ] 保留 `dt_action_t` 的域命令、状态查询和持久化语义，将 GTK 映射统一为
      `GAction`/`GMenuModel`/`GtkShortcutController`；快捷键编辑器不能绕开现有 Action 树。
- [ ] 产品级 menubar/app menu 使用单一 menu model；主窗口、第二窗口和动态模块 action group
      的启用状态随 active view/selection 更新。
- [ ] 将 `src/gui/context_menu.c`、presets、styles、masks、tags、IOP graph 和缩略图右键菜单
      改为 `GtkPopoverMenu` 或受父 widget 管理的 `GtkPopover`。静态层级用 menu model；需要
      稳定对象 payload、slider 或复合控件的菜单使用 scoped action/普通 popover 内容。
- [ ] 延续现有“冻结 Action 上下文后再执行”的安全语义，但把 `GtkTreeRowReference` 换成稳定
      域 ID/模型对象弱引用；popover 关闭、目标删除、视图切换后 Action 必须重新验证。
- [ ] 将 alert/confirm 改为 `GtkAlertDialog` 或非阻塞自有窗口；将 file/folder 选择改为
      `GtkFileDialog` 和 `GFile`；颜色/字体选择使用所选 GTK 4 稳定版非 deprecated 的异步
      dialog API。
- [ ] 移除 `GtkDialog` 同步返回假设和所有 `gtk_dialog_run()`。批处理或启动流程用状态机继续，
      用户取消与错误路径不得递归进入 main loop。
- [ ] 更新 `src/control/conf.[ch]` 的文件选择持久化边界，使配置层接收路径/URI 或 `GFile`，
      不公开 deprecated `GtkFileChooser` 类型；覆盖 Windows 路径、macOS bookmark/权限和 Linux
      portal 返回值。
- [ ] 迁移剪贴板为 `GdkClipboard` content provider/异步读取，拖放为 `GtkDragSource`、
      `GtkDropTarget`/async drop，并保留图片 ID、文件 URI 和模块重排的数据语义。

阶段出口：旧 Menu/MenuItem/MenuShell、同步 dialog/file chooser、GtkClipboard、GtkSelectionData
和 GdkDragContext 代表性搜索归零；键盘、右键、触摸和无鼠标导航均可完成核心工作流。

## 阶段 6：列表、树、标签页和照片布局

- [ ] 为 tags、masks、styles、metadata、import、preferences、history、filters 和快捷键编辑器定义
      明确的 row GObject/域模型；使用 `GListModel`、filter/sort/selection model，避免把业务状态
      存在 view row path 中。
- [ ] 平面数据迁移到 `GtkListView` 或 `GtkColumnView`；层级数据迁移到 `GtkTreeListModel` 与
      expander。item factory 的 bind/unbind 必须成对断开 signal 和释放行引用，支持模型复用。
- [ ] 为 tag、mask 和 Action 树保留展开状态、过滤、排序、多选、内联编辑、键盘遍历、拖放和
      上下文 Action；选择变化仍通过现有数据库事务/任务队列进入业务层。
- [ ] 删除 `GtkTreeView`、`GtkTreeModel`、stores、cell renderers、row references、
      `GtkEntryCompletion` 和 `GtkComboBox` 的 deprecated 使用；搜索/补全改为 list model +
      popover/dropdown。
- [ ] 将 `GtkNotebook` 调用收敛到项目 tabs API，适配 GTK 4 page meta、事件控制器和 final
      widget 约束。保留 IOP 通道切换、鼠标滚轮、快捷键、可访问名称和当前页持久化；只有
      实测证明 Notebook 无法满足产品交互时，才另案改成 `GtkStack`/自有 tab strip。
- [ ] 为 `src/dtgtk/thumbtable.c` 与 `culling.c` 做有数据的布局 spike：比较 GTK 4
      `GtkGridView/ListView` 与保留 thumbnail 子 widget 的自定义 `GtkLayoutManager`，以十万图
      catalog 的可见项数量、滚动稳定性、内存、选择和 DnD 为门槛后选择，不直接用 `GtkFixed`
      复制旧 `GtkLayout`。
- [ ] Grid 继续按可用宽度派生每行 2–10 张、Loupe 继续保留 filmstrip 和 Fit/100% 语义；布局
      重构不能重新引入已删除的密度模式、时间线或 culling 入口。

阶段出口：所选 GTK 4 稳定版 deprecated 列表/树 API 和 GTK 3-only 标签页调用的搜索及编译
诊断归零；大 catalog 操作无明显内存、滚动或选择回退。

## 阶段 7：主题、资源、缩放和可访问性

- [ ] 将 CSS provider 从 screen 级迁移为 display 级，按 GTK 4 widget/CSS node 名重写 4 个
      主题文件；删除 EventBox、MenuItem、TreeView 等不存在节点的选择器和 GTK 3 私有属性。
- [ ] 为 dtgtk/Bauhaus 控件设置稳定 CSS name/class，集中暴露画布颜色 palette；不让业务模块
      通过脆弱的祖先层级选择器或 widget path 猜颜色。
- [ ] 使用 GTK CSS parser 和运行时诊断验证每个 bundled theme。GTK 3 用户 CSS 不作为兼容
      承诺：为用户 CSS 增加明确版本标记，旧版本拒绝加载并回退 bundled theme，不做静默
      正则转换。
- [ ] 审计图标资源路径、symbolic recolor、GResource 和 HiDPI 变体；照片与缩略图使用
      `GdkTexture`/`GtkPicture` 或受控 snapshot，不把照片误当主题图标。
- [ ] 验证 1x/2x、分数缩放、窗口跨显示器、窄侧栏、长翻译文本和系统字体变化；不使用固定
      像素修补 GTK 4 的 measure/layout 问题。
- [ ] 为自定义控件设置正确 accessible role、label、description、value 和 state；列表 factory
      重用时同步更新可访问信息。键盘 focus ring、Tab 顺序和 reduced-motion 设置必须可用。
- [ ] 更新 `DevDocs/GUI.md`、`GUI_Recipes.md`、`Notebook_UI.md`、`imageop_gui.md`、`sliders.md`
      和相关源码地图，删除 GTK 3 示例和不存在的 API。

阶段出口：所有 bundled theme 无 CSS warning，核心 UI 在缩放/主题/键盘/辅助技术下可用；文档
只描述 GTK 4 实现。

## 阶段 8：色彩管理与平台窗口集成

- [ ] 单独审计 `src/common/colorspaces.c` 的 X11 ICC property、macOS ColorSync 和 Windows 显示
      profile 获取。用 GTK 4 backend/native surface API 替换 GdkWindow/GdkScreen 接触面。
- [ ] 明确 GTK 4/GSK 合成的色彩空间与 DarkTableNext 已转换预览之间的边界，防止重复显示
      变换；记录纹理格式、alpha、transfer function 和每显示器 profile 更新时机。
- [ ] 用色块和受控照片验证主预览、第二窗口、Lighttable 缩略图、吸管和 middle-grey 背景；
      窗口跨不同 ICC/缩放显示器时必须重新绑定正确 profile，导出 CPU 金样保持不变。
- [ ] macOS 将 Quartz 条件和 native window 接口迁移到 GTK 4 macOS backend；验证菜单、Command
      快捷键、文件 dialog、双显示器、全屏、bundle 激活与 Apple Silicon/Intel。
- [ ] Windows 将 `GDK_WINDOW_HWND` 等接口迁移到 surface/native handle；验证任务栏 progress、
      文件 dialog、ICC、IME、DPI 变化、拖放和 MSVC 构建。
- [ ] Linux 分别验证 Wayland 与 X11：输入、clipboard、DnD、ICC、窗口状态、portal、缩放和
      renderer；不因 X11 可用就恢复 GTK 4 已移除的全局窗口控制。

阶段出口：三平台窗口、输入和显示色彩行为通过；UI compositor 的选择不会改变 pixelpipe
正确性，也不会使 CPU fallback 或 OpenCL 行为发生变化。

## 阶段 9：安装包、完整验证和删除 GTK 3

- [ ] 更新 macOS DMG 与 Windows MSI 的 GTK 运行时闭包，包含实际需要的 GSK/GLib schemas、
      GdkPixbuf loaders、字体/图标资源、renderer 后端和 license；依赖分析不得只验证主库。
- [ ] 在干净或隔离主机上运行已安装产物，确保不从 Homebrew、vcpkg build tree、开发机 PATH
      或系统 GTK 3 意外加载 DLL/dylib/resource。
- [ ] Linux 验证支持的系统包或自包含包格式，检查 GTK 4、portal 和 renderer 运行时版本；
      记录未打包的系统责任边界。
- [ ] 删除 `FindGTK3.cmake`、`gtk+-3.0`、GTK3 命名变量、GTK 3 条件、过渡适配、旧 CSS 和
      不再可达的资源；全仓搜索确认源码、构建、打包、文档和测试没有 GTK 3 消费者。
- [ ] 更新 `README.md`、release notes、安装说明、开发文档和第三方许可证，明确最低 GTK/
      GLib 和平台版本、GTK 3 用户 CSS 不兼容边界及移除的窗口定位行为。
- [ ] 对最终构建产物输出依赖清单和 GTK 运行时版本；保存三平台验收报告、失败恢复记录与
      已知平台限制。

阶段出口：源码和产物中不存在 GTK 3，三平台安装包从干净环境启动并完成核心照片工作流，
所有下列总验收门槛通过。

## 总验收矩阵

### 静态与构建

- `python3 configs/source_root_workflow.py --update` 能离线物化活动锁并生成预设；不产生应提交的
  本地锁、预设或依赖源码改动。
- macOS 按仓库规则交叉覆盖 Debug/Release 编译器，默认 `mac_gcc_debug` +
  `mac_clang_release`；Debug 开启测试并运行完整 `unit` 标签。
- Windows 分别完成 `win_msvc_debug`、`win_msvc_release` configure/build；Linux 在支持的
  Clang/GCC 环境完成 Debug/Release。公共头与动态模块均以 GTK 4 构建。
- `GTK_DISABLE_DEPRECATED`、`GDK_DISABLE_DEPRECATED`、编译警告和链接器 unresolved symbol 均为
  零；`git diff --check` 通过，只格式化触及代码。
- 静态残留检查覆盖 `GTK3`、`gtk+-3.0`、旧事件 signal、`GtkMenu*`、`GtkEventBox`、
  `GtkTreeView`/cell renderer、`gtk_dialog_run`、`GtkFileChooser*`、`GdkWindow`、
  `gtk_container_*`、`gtk_widget_show_all/destroy` 和旧 main-loop API。

### 自动与诊断运行

- 在 `G_DEBUG=fatal-warnings`、`G_ENABLE_DIAGNOSTIC=1` 下完成 GUI smoke；没有 GTK/GLib
  critical、CSS parser warning、对象泄漏或线程违规。
- 对可测试的 Action、selection model、异步 dialog continuation、取消和 widget dispose 增加
  unit/integration 测试；测试不能依赖真实用户数据库或修改生产配置。
- 使用 ASan/UBSan 可用配置反复打开/关闭视图、popover、偏好、第二窗口和应用，覆盖任务完成
  晚于 widget 销毁、文件 dialog 取消及导出取消。
- 默认 GSK renderer 和每个平台可用的软件 fallback 均完成启动、缩放和图像显示 smoke；记录
  renderer 与驱动，不把某个开发机 fallback 当作正式性能结论。

### 手工产品路径

- 首次/既有 workspace、实例锁冲突、数据库错误和正常退出；CLI `--version` 与无 GUI 导出。
- 导入文件/文件夹，Lighttable Grid/Loupe/filmstrip 的滚动、缩放、选择、评分、色标、筛选和
  拖放；十万图 catalog 下验证可见项复用和滚动位置。
- Darkroom 打开/切图、模块展开和重排、history、presets/styles、masks、blend、吸管、crop、
  undo/redo、第二窗口、全屏和显示器切换。
- JPEG/PNG/TIFF/原文件复制导出，覆盖进度、取消、覆盖确认、错误路径；输出与 CPU 金样一致。
- tags、metadata、styles、preferences、快捷键编辑、菜单与上下文 Action；仅键盘、鼠标、触摸板
  和可用数位笔路径分别检查。
- macOS DMG、Windows MSI 和 Linux 产物在干净环境安装、升级、启动、完成一次导入/编辑/导出
  并卸载；运行时依赖只来自声明位置。

## 工作拆分与提交纪律

- 每个提交只表达一个可回退意图：准备性 API、应用生命周期、某个控件族、某个列表模型、
  某个平台或打包闭环分别提交。
- 先迁移共享底层，再迁移消费者：application/lifecycle → dtgtk/Bauhaus → Action/dialog/model
  基础设施 → Lighttable/Darkroom → Lib/IOP 叶子 UI → theme/platform/package。
- 每批在描述中列出受影响的所有权、线程边界、实际验证和未验证平台；未运行不能写成通过。
- 删除旧 API 时同时清理头、实现、CMake、CSS、资源、配置、文档和测试，并用全仓搜索证明
  没有可达消费者。
- 未经明确要求不 commit、amend、rebase 或 push。GTK 4 迁移完成也不能自动提交用户已有改动。

## 下一次开工时的第一步

先按 Homebrew 规则确定并记录当时最新可用 `gtk4` bottle，再完成阶段 0 的正式可达 UI 清单和
三平台相容依赖证明，同时把本文件的初步 `rg` 盘点固化为脚本。不要先改
`find_package(GTK3 ...)`：在应用生命周期、同步 dialog、事件输入和 TreeView 数据所有权尚未
拆开前直接切库，只会把可管理的迁移变成无法分层验证的全仓编译失败。
