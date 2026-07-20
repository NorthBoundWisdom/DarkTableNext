# macOS 系统命令与快捷键重构计划

## Goal

在保留 DarkTableNext 现有 GTK3 前端和 `dt_action` 动作体系的前提下，为 macOS 建立规范的
应用命令出口：适合全局调用的动作出现在系统主菜单中，简单键盘快捷键成为菜单项的原生
key equivalent，并可被 Paletro 等通过 macOS Accessibility 读取菜单的工具发现。

这不是重新实现一套快捷键系统。`dt_action`、用户快捷键配置和既有执行/状态接口仍是唯一
事实来源；新增层只把其中可稳定表达的部分投影为系统命令。鼠标、滚轮、长按、多击、移动、
数位板、悬停对象和短生命周期 payload 等高级输入继续由现有调度器处理。

完成后的核心验收标准：

- macOS 菜单栏能看到按产品语义组织的命令，而不是内部 Action 树的无差别转储；
- Paletro 能读取这些菜单命令及其当前键盘快捷键；
- 菜单点击和键盘触发最终都只调用一次 `dt_action_invoke()`；
- 启用、禁用、勾选和单选状态与 `dt_action_get_status()` 一致；
- 用户修改快捷键后，原生菜单无需重启即可同步；
- 现有复杂输入、Lighttable/Darkroom 上下文、多实例 IOP 和图像处理结果不发生回归。

## Product/architecture boundary

本重构属于 macOS GTK 前端、应用生命周期和动作调度边界，不改变 pixelpipe、IOP 算法、
CPU/OpenCL 结果、数据库 schema 或文件格式。产品范围继续以
[`TODO_CORE_REDUCTION.md`](TODO_CORE_REDUCTION.md) 为准；右键菜单已有基础和对象上下文边界见
[`TODO_CONTEXT_MENUS.md`](TODO_CONTEXT_MENUS.md)。

架构约束如下：

```text
用户快捷键配置 / dt_action 树（唯一事实来源）
                    │
                    ├── 现有 dispatcher
                    │     └── 鼠标、滚轮、长按、多击、移动、数位板、上下文输入
                    │
                    └── system command projection
                          ├── GAction：稳定命令 ID、启用/状态、激活入口
                          ├── GMenuModel：macOS 主菜单的产品结构
                          └── GtkApplication accelerators：简单键盘组合
                                      │
                                      └── macOS 菜单 / Accessibility / Paletro
```

- 继续使用 GTK3、dtgtk 和 Bauhaus，不引入 QML、Qt、第二套应用入口或跨仓库 UI 依赖。
- 首选 GTK3 标准的 `GtkApplication`、`GAction`、`GMenuModel` 和
  `gtk_application_set_accels_for_action()`；先用当前 Homebrew GTK 3.24 Quartz 运行时验证
  Accessibility 可见性，再决定是否进入完整迁移。
- 若标准 GTK3 路径不能产生可访问的原生菜单，只允许在相同投影接口后增加最小 AppKit
  bridge；它不得拥有业务动作、快捷键配置或第二套菜单模型。
- 不默认恢复已经退出构建的 `gtk-mac-integration`。若原型证明必须依赖它，应先单独评估
  依赖来源、维护状态、生命周期和打包成本。
- 系统菜单只暴露“可稳定解析的应用命令”。依赖命中图元、树行、Widget 或临时 payload
  的 provider-only 动作继续留在右键菜单中。

## 已实施的设计

### 生命周期与所有权

- 现有 `dt_init()` → `dt_gui_gtk_init()` → `gtk_main()` → `dt_cleanup()` 顺序保持不变；不把
  `gtk_main()` 机械替换为 `g_application_run()`，以免改变数据库锁、无 GUI、启动失败和嵌套对话框
  的既有生命周期。
- GUI Action 树和用户快捷键加载完成后，`src/common/darktable.c` 注册一个
  `G_APPLICATION_NON_UNIQUE` 的 `GtkApplication`，并附着已存在的主窗口；退出时在 GUI/Action
  所有者仍有效时先清理该投影。
- 这不是第二应用入口。`GtkApplication` 只拥有系统菜单、`GAction` 和 accelerator 投影；业务
  状态仍由既有 GTK/control 生命周期拥有。

### 单一事实来源与命令资格

- 新增 `src/gui/system_commands.[ch]`。它从 `dt_action` 树收集可稳定解析的
  `DT_ACTION_TYPE_COMMAND`，并显式接入 Preferences、Shortcuts、focus peaking 与导入/导出入口。
- 每个 `GSimpleAction` 的名称由完整 Action ID、element、effect、instance 的 SHA-256 派生；激活时
  重新按 ID 解析 Action，查询 `dt_action_get_status()` 后才调用一次 `dt_action_invoke()`。
  不保存 Widget、模块或 Action 裸指针。
- provider-only、鼠标/滚轮/长按/多击/move、命中对象、临时 payload 和不具备唯一实例策略的动作
  不会成为系统命令，仍由既有 dispatcher 或右键 provider 处理。

### 菜单、状态与快捷键

- 菜单按产品语义投影为 App、File、Edit、Image、Selection、View、Lighttable、Darkroom、Actions
  和 Help；不直接转储内部 Action 树。
- `dt_action_get_status()` 映射到 `GAction` enabled 与 boolean state，并在视图、选择、活动图像、
  历史和 style 变化时刷新。style 变化会重建可动态注册的命令，再安全同步菜单。
- `accelerators.[ch]` 提供只读的可表示键盘 chord 查询与共享的
  `dt_shortcut_normalize_modifiers()`。macOS 以 `<Primary>` 导出 Command key equivalent；投影层和
  既有 dispatcher 使用同一修饰键归一化，避免 Quartz 的 Meta 标记造成失配。
- 旧 dispatcher 仍先让焦点控件处理编辑输入；只有系统投影拥有的简单按键才经同一 `GAction`
  激活入口执行，因此菜单点击和按键不会双重调用。`dt_shortcuts_load()` / `dt_shortcuts_save()`
  会在主循环空闲时同步原生 accelerators。
- 已删除不可达的 `MAC_INTEGRATION` 菜单和进度条残留；当前只有此一菜单实现。
- 开发构建的 `bin/darktable` 保持为稳定命令行入口，但会 `exec` 同目录
  `darktable.app/Contents/MacOS/darktable`。标准 `Info.plist` 声明
  `org.darktable.darktable`，并把 build tree 的 `lib`、`share` 和核心 dylib 以 bundle 内链接提供。
  这避免裸 Mach-O 没有 bundle identifier，致使 Paletro、KeyClu 一类按应用身份筛选的
  Accessibility 客户端完全忽略菜单的问题。

## 已完成

- [x] 用纯 GTK3 `GtkApplication`、`GAction` 与 `GMenuModel` 建立 macOS 系统命令投影；不引入
      AppKit、Objective-C++、QML 或第二个 UI 运行时。
- [x] 从稳定 Action 身份重新解析并执行命令，状态刷新不依赖副作用探测。
- [x] 实现 Action → 原生菜单、原生 accelerator 与既有按键 dispatcher 的单一执行路径。
- [x] 实现快捷键加载/保存后的异步同步，并过滤复杂/上下文依赖输入。
- [x] 清理死 `MAC_INTEGRATION` 路径，更新项目和开发者文档。
- [x] 保持 `bin/darktable` 调用方式的同时生成可识别的 `darktable.app`，并按实际可执行文件路径
      解析 bundle 资源，避免 CoreFoundation 在终端启动 GTK 进程时把工作目录误判为资源根。

## 验证记录

在 macOS Quartz 的独立 Debug 实例（临时配置目录、内存图库）中已验证：

- 系统 Accessibility 菜单栏实际列出 `Apple, darktable, File, Edit, Image, Selection, View,
  Lighttable, Darkroom, Actions, Help`；说明 GTK3 `GMenuModel` 的原生菜单路径可用。
- AX 属性显示 Quit 为 `Q, 0`（Command-Q），focus peaking 为 `F, 1`（Command-Shift-F）；不是
  之前错误的裸字母 key equivalent。
- 通过 `bin/darktable` 启动的 Debug 实例由 `NSRunningApplication` 识别为
  `org.darktable.darktable`，bundle URL 为 `build/mac_clang_debug/bin/darktable.app`；其 AX 菜单仍列出
  `File`、`Edit`、`Image`、`Selection`、`View`、`Lighttable`、`Darkroom`、`Actions` 和 `Help`。
  此前直接运行裸可执行文件时 bundle identifier 为 `NULL`，正是 KeyClu / Paletro 未响应的根因。
- 菜单点击与 `⌘⇧F` 均能切换 focus peaking 的 AX 勾选；连续两次按键得到“未勾选 → 勾选 → 未勾选”，
  验证状态回写与单次执行路径。
- `cmake --build --preset mac_clang_debug --target darktable` 和
  `cmake --build --preset mac_clang_release --target darktable` 均已通过；启用测试的 Debug preset 下
  `ctest --test-dir build/mac_clang_debug --output-on-failure -L unit` 为 3/3 通过。

调试构建直接打开 Preferences 时因本地构建树未安装 GSettings schema 而退出；这是已存在的运行时
打包前置条件，不是菜单 Action 的 GLib/GTK 警告或应用逻辑崩溃。系统菜单和状态动作的独立验证
不依赖该 schema。

## 收尾项

- [x] 构建 macOS Clang Debug/Release，并运行当前 unit CTest（3/3）。
- [ ] 以实际物理全局快捷键在 KeyClu、Paletro 各做一次最终人工发现验证；它们的热键不会响应
      AppleScript 合成按键，故不自动化该 UI。当前 build 入口已具备这两者需要的 bundle identity 与
      AX 菜单属性。
- [ ] 在真实 Preferences > Shortcuts 中修改、清除和恢复一个简单 chord，确认菜单无需重启即更新；
      实现已在保存/加载钩子同步，仍需产品界面验收。
- [ ] 进行完整的产品回归：文本输入、对话框、Dock/file-open、数据库锁、关闭窗口，以及复杂输入和
      provider-only 右键动作。本变更不改变这些路径的业务实现。

## Do not do

- 不用 QML/Qt 重写 UI，不引入第二套应用入口或第二套 Action/快捷键数据库。
- 不为让 Paletro“看到更多”而把所有内部动作无差别塞进主菜单。
- 不把鼠标、长按、多击、move、数位板或对象 payload 强行编码成系统 accelerator。
- 不用翻译文本、`GtkWidget *`、`dt_action_t *` 或模块地址作为持久命令 ID。
- 不在菜单回调中直接修改 IOP 参数、SQLite、选择集或共享状态；统一调用现有 Action/业务入口。
- 不同时让原生 accelerator 和现有 dispatcher 执行同一键盘动作。
- 不在 GTK3 原型未经当前 macOS 运行时验收前大规模改写启动代码。
- 不直接复活 `gtk-mac-integration`、新增 AppKit 依赖或 Objective-C++ 文件而不记录设计与验证。
- 不顺手改变 pixelpipe、IOP 算法、CPU/OpenCL 输出、数据库 schema 或产品范围。
- 不把 `build/dependency_*`、活动锁、生成的 `CMakePresets.json` 或本地探针产物提交到仓库。
