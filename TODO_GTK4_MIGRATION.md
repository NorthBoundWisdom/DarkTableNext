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

当前状态：**仅完成初步源码盘点，尚未开始 GTK 4 实现迁移**。GTK 3.24.15 仍是
`src/CMakeLists.txt` 的构建要求。

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

- 115 个源码文件直接包含 GTK/GDK 头；dtgtk 与 Bauhaus 中有 12 个自定义 GTK 子类。
- `GtkTreeView`/`GtkTreeModel`/store/path/selection 代表性匹配约 1,866 处，分布在 27 个文件；
  最大集中区为 `src/libs/tagging.c`、`src/libs/masks.c`、`src/gui/accelerators.c`、
  `src/libs/import.c`、`src/gui/preferences.c` 和样式/元数据对话框。
- 旧 `GdkEvent*` 结构或事件回调代表性匹配 423 处、72 个文件；显式事件 mask 62 处、31 个
  文件。Darkroom 画布、Lighttable 缩略图、蒙版、曲线和 Bauhaus 控件都是高风险输入路径。
- `GtkMenu`/menu shell 代表性匹配 368 处、36 个文件；`gtk_dialog_run()` 44 处、21 个文件；
  `GtkFileChooser*` 81 处、13 个文件。
- `gtk_box_pack_*` 312 处、34 个文件；通用 `gtk_container_*` 171 处、53 个文件；
  `show_all`/widget destroy 231 处、54 个文件。
- `GdkWindow`/`gtk_widget_get_window()` 代表性匹配 91 处、17 个文件；拖放和 selection data
  74 处、12 个文件；旧 main-loop API 10 处、4 个文件。
- allocation/旧尺寸接口代表性匹配 209 处、49 个文件；style context 156 处、28 个文件。
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
| TreeView/TreeModel/cell renderer | 2331 | 28 |
| 旧 GdkEvent 类型或回调 | 425 | 72 |
| 显式 widget 事件 mask | 62 | 31 |
| GtkMenu/MenuShell | 397 | 36 |
| 同步 gtk_dialog_run | 44 | 21 |
| GtkFileChooser | 90 | 13 |
| GtkContainer 通用 API | 170 | 52 |
| GtkBox pack API | 312 | 34 |
| show_all/widget destroy | 231 | 54 |
| GdkWindow/widget_get_window | 131 | 17 |
| 旧拖放/selection data | 78 | 12 |
| allocation/旧尺寸 API | 209 | 49 |
| GtkStyleContext 旧 getter | 158 | 28 |
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

- [ ] 完成核心删减计划中的 UI/IOP 保留清单；对每个仍在构建和注册表可达的 UI 模块标记
      “保留并移植 / 先删除 / 暂缓决定”，禁止移植确认将删除的模块。
- [x] 把本节静态盘点固化为 `scripts/gtk4_audit.sh` 和版本化摘要；后续每个迁移批次补齐被宏、
      函数指针、C++ 包装或运行时 signal 名称隐藏的 GTK 使用。
- [ ] 以当前 GTK 3.24 构建运行 `G_ENABLE_DIAGNOSTIC=1`，记录属性、signal、CSS 和对象生命周期
      警告；先消除基线自身的 critical/warning，避免切换后无法区分新旧问题。
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

阶段出口：范围、交互基线、数值基线和三平台依赖闭包均有可重复记录；无法得到所选 GTK 4
运行时的平台必须在继续源码切换前解决，而不是在发布阶段才发现。

## 阶段 1：在 GTK 3.24 上完成可前置准备

- [ ] 将准备环境更新到官方最新 GTK 3.24.x 补丁版，并保持 `GDK_DISABLE_DEPRECATED`、
      `GTK_DISABLE_DEPRECATED`；这只用于降低迁移噪音，不形成新的长期 GTK 3 基线。
- [ ] 将可在 GTK 3 使用的旧 widget 事件 signal 改为 `GtkEventController`/`GtkGesture`，封装
      click、motion、scroll、key、focus、stylus 和 drag 手势的项目级连接/清理模式。
- [ ] 停止直接读取或构造 `GdkEvent` 字段，统一使用访问器和局部不可变事件快照。
- [ ] 用容器专用 add/remove 和子控件 expand/align/margin 替换通用 `gtk_container_*` 与
      `GtkBox` pack child properties；记录必须在 GTK 4 切换时再改名的少量调用。
- [ ] 清理非顶层 `gtk_widget_destroy()`，把自定义 widget `destroy` vfunc 转成 `dispose`；增加
      销毁时仍有 idle、job、popover、dialog 和 DnD 回调的测试。
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
- [ ] 用 `GdkDisplay`、`GdkMonitor`、`GdkSurface` 和 widget cursor API 替换 `GdkScreen`、
      `GdkVisual`、`GdkWindow`、全局坐标、窗口移动和底层 cursor 操作。
- [ ] 重新定义窗口持久化为宽高、最大化、全屏和产品级第二窗口状态；Wayland 不支持的 x/y
      位置不再假装可恢复。同步清理失效配置键、文档和测试。

阶段出口：应用能正常启动、开关所有窗口并退出；ASan/GLib debug 下没有窗口引用环、迟到回调
或非主线程 GTK 访问；CLI 行为不变。

## 阶段 4：dtgtk、Bauhaus、自绘和输入系统

- [ ] 逐个迁移 `src/dtgtk/` 的自定义类。能组合的 EventBox/Button 子类改成组合控件；确需
      子类化的控件直接继承允许的 GTK 4 基类并实现 `measure`/`size_allocate`/`snapshot`。
- [ ] 将 `GtkDarktablePaintCell` 与所有 cell renderer 消费者一起删除，图标绘制进入 list item
      widget factory；不为已弃用 TreeView 保留 paint-cell 兼容层。
- [ ] 迁移 `DtBauhausWidget` 的 popup、焦点、文本输入、滚轮精调和拖动。popup 使用受父控件
      管理的 popover，不依赖 keep-above、window type hint、grab 或全局窗口坐标。
- [ ] 普通 `GtkDrawingArea` 连接改成 `gtk_drawing_area_set_draw_func()`；自定义 widget 的
      `draw` vfunc 改成 `snapshot`。第一版继续通过 `gtk_snapshot_append_cairo()` 复用已验证的
      Cairo 绘制，不趁迁移重写为 GSK shader/render node。
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
