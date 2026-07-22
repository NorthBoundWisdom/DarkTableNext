# DarkTableNext Repository Instructions

## 项目定位

DarkTableNext 0.9 是跨 macOS、Windows 与 Linux、GPLv3 的冻结照片工作流与 RAW 编辑基线。
它保留当前图像处理核心、GTK 前端、OpenCL 实现和 fixture，只允许静态读取源码与已提交测试资产；
不再配置、编译、运行或修改旧工程。新增产品实现、功能收缩和架构演进统一进入 Ravo。

仓库根目录下的约定适用于整个仓库。`Ravo/` 是下一代 C++20 无头内核与 CLI 的独立所有权边界，
并由它自己的 `AGENTS.md` 增加约束；`FreeCM/` 是独立子模块并由它自己的 `AGENTS.md` 管理。所有
`build/dependency_*` 内容都是父仓库工作流生成的外部源码，不得把它们当作本仓库源码修改。

## 开始工作前

1. 运行 `git status --short --branch`，识别用户已有改动并保留它们。
2. 阅读与任务直接相关的源文件和文档，不根据上游 darktable 习惯猜测当前行为。
3. 产品删减先读 `TODO_REWRITE.md` 的“0.9 冻结基线与 Ravo 承接项”；GPU 任务先读
   `DevDocs/GPU_Baseline.md`。
4. 跨层改动先明确所有权、生命周期、线程边界和最小验证集。

现有行为以冻结的 `src/` 和 fixture 为准；`DevDocs/` 是源码地图，不是兼容性承诺。新产品边界以
`TODO_REWRITE.md` 为准。除非用户明确重新开放 0.9，任务不得修改 `src/`、旧 UI、旧构建图或旧资源。

## Ravo 下一代边界

- `Ravo/` 先交付 C++20 Ravo Engine 与正式 `ravo` CLI；无头阶段验收前不创建 desktop target、
  UI toolkit 依赖或 catalog 数据库。
- Ravo 生产代码不得依赖 `src/` 私有头、旧库、动态 IOP、GTK 类型或全局 `darktable` 状态。验证只能
  静态读取冻结源码与 fixture；不得配置、编译或执行旧 CLI/旧测试工程。
- 迁移期间 Ravo 与冻结的 `src` 独立并行；不创建 `src` → Ravo 或 Ravo → `src` 的生产依赖。只有
  Ravo 全产品达到切换门槛后，才在退役阶段删除旧实现、构建项、资源、配置和重复测试。
- 在 `Ravo/` 工作前必须阅读 `Ravo/AGENTS.md`、`Ravo/ARCHITECTURE.md`、`Ravo/MIGRATION.md` 和
  `Ravo/TESTING.md`。

## 实施原则

- 本项目没有维护 darktable 历史插件、Lua、旧格式或旧 UI ABI 的默认义务；不要为
  已明确移除的能力增加空壳、兼容开关或迁移 shim。
- Ravo 决定不支持某项旧功能时，先记录显式拒绝或迁移策略，不在冻结的 0.9 中提前删除。阶段 7
  退役旧 owner 时才同步清理构建项、注册表、资源、配置、文档和测试，并用全仓搜索确认无消费者。
- 不维护或修改现有 GTK 前端与旧 C/C++ 核心；发现差异或缺陷时记录为 oracle 限制，不用修补 `src`
  推进 Ravo。
- 0.9 OpenCL 保持冻结，不在旧应用内替换为 Metal；它只在阶段 7 随整个旧实现退役。Ravo GPU 按
  `TODO_REWRITE.md` 阶段 6 独立实现，不复用 OpenCL API。
- 遵循相邻 C/C++/CMake 文件的现有风格，只格式化触及的代码。不要借任务批量格式化
  遗留源码。
- 新依赖、公共 API、线程模型、数据库 schema 和产品范围变化都应在同一变更中记录
  设计理由与验证方法。
- 使用 `apply_patch` 做人工源码修改。生成器和格式化器仅用于它们明确拥有的输出。

## 现有 UI 与核心边界

- GTK 前端、dtgtk/Bauhaus 控件、Lighttable、Darkroom、导入、导出、catalog、history、masks、
  色彩空间和 pixelpipe 保留为只读旧实现；新 UI/服务工作只进入 Ravo 的后续阶段。
- 不在旧 UI 中增加入口、运行时、工具包、adapter 或 Ravo 调用。需要的行为只通过只读源码研究和
  已提交 fixture 取证，不运行独立旧进程。

## FreeCM 与依赖源码

- 当前 checkout 的 source-root 真相源首先是被忽略的活动锁 `source_roots.lock.jsonc`；开始依赖排查前
  先运行 `python configs/source_roots.py show --format json`、`resolve --format json` 和 `verify`，
  不得只根据 committed template 猜测当前实际路径或 mode。
- `source_roots.lock.jsonc.in` 是受版本控制的直接依赖 pinned 基线。永久依赖、公共 CMake 默认值或
  source-root API 变化只修改模板、`configs/source_roots.py` 和消费它们的 CMake 源码；根锁只声明
  direct dependencies，transitive closure 从本地 seed 中的依赖模板递归解析。
- `pinned` 使用精确 commit；`latest` 只解析 seed 中本地可见的最新提交；`manual` 对非空的
  `depsManualPath.<dependency>` 使用真实 checkout，空项回退到受管解析。`manual`/`latest` 与机器路径
  只属于活动锁，不得进入发布模板。
- 联调依赖时不得修改 `build/dependency_source_roots/*`。先把活动锁切到 `depsMode=manual`，将对应
  `depsManualPath` 指向开发者提供的真实 checkout（或经明确确认、干净且被当作真实仓库使用的 seed），
  再运行 `python configs/source_root_workflow.py --update` 并用 `show`/`resolve`/`verify` 确认接线。
- `python configs/source_root_workflow.py --init` 是唯一允许联网、克隆 seed 或准备远端资产的依赖步骤；
  `--update` 必须纯离线，只读取活动锁和现有 seed、解析/物化 closure、生成根 `CMakePresets.json`，
  不 fetch、不编译依赖，也不隐式配置或测试 Ravo。
- `source_roots.lock.jsonc`、`CMakePresets.json`、`.freecm/` 与 `build/dependency_*` 是本地状态，不提交，
  不手工修补为永久方案。活动锁或 manual path 变化后必须先 `--update`，再配置/编译 Ravo。
- 依赖仓提交必须先按拓扑顺序 push，并用 `git ls-remote <remote> <published-ref>` 确认已发布的
  branch/tag 返回目标 SHA，之后才能更新父仓库模板或 gitlink；禁止提交依赖尚未发布的本机 SHA。
- 不使用 `FetchContent`、CMake 网络下载、替代 submodule、源码复制或生成目录补丁绕过工作流。完整
  联调步骤见 `DevDocs/Dependency_Workflow.md`。

## 构建与验证

旧 0.9 构建、CTest、图像 runner 和打包 target 全部冻结，不再运行。所有 configure/build/run/test/
install 动作只允许进入 `Ravo/`；Windows 辅助脚本只使用 Python 或 PowerShell。

首次准备工作区时运行：

```text
git submodule update --init FreeCM
python configs/source_root_workflow.py --init
python configs/source_root_workflow.py --update
```

依赖已准备后，Windows 使用 Ravo PowerShell 入口：

```powershell
& .\Ravo\tools\freecm_project.ps1 -Action Configure -Configuration Debug
& .\Ravo\tools\freecm_project.ps1 -Action Build -Configuration Debug
```

macOS/Linux 使用同一 Ravo 项目的 Python 入口：

```sh
python3 Ravo/tools/freecm_project.py --action Configure --configuration Debug
python3 Ravo/tools/freecm_project.py --action Build --configuration Debug
```

涉及跨平台构建、公共头或平台分支的改动，应在可用的 Windows、macOS 和 Linux 工具链上分别完成
Ravo configure/build；当前环境缺少相应工具链时，执行可行的静态检查并明确报告未验证的平台，不能
把单一平台结果表述为全平台通过。测试只运行 Ravo 自有 unit/contract 入口；不得运行旧 CTest、旧 CLI
或 `darktable-tests/run`。

验证应与风险成比例：

- 纯 Markdown/代理说明：检查链接、命令、路径与 diff；无需强行完整编译。
- CMake/依赖图：至少运行离线 `--update`、Ravo configure 和受影响 target 的 build。
- C/C++ 核心：构建受影响 Ravo target；任务要求行为验证时运行相关 Ravo unit/contract，公共头或广泛
  改动运行完整 Ravo 测试集。
- CLI/服务：验证结构化错误、取消、输出冲突和资源销毁路径，不启动旧进程作即时 oracle。
- GPU/图像算法：除 Ravo 单元/fixture 验证外，遵循 `DevDocs/GPU_Baseline.md` 的 CPU 金样和性能门槛。

如果环境缺依赖或测试数据，应先做所有仍可执行的静态/局部验证，并准确报告未运行项
及原因；不要把“未运行”写成“通过”。

## 文档与生成文件

- 架构、构建命令、依赖约定或产品范围变化时，同步更新 `README.md`、相关根级文档和
  `DevDocs/` 索引。
- 不编辑生成的 `CMakePresets.json` 作为最终修复；修改锁模板/生成逻辑后运行 `--update`。
- 不提交构建树、临时报告、本地活动锁、IDE 文件或依赖 checkout。
- 链接必须指向仓库中真实存在的文件；删除/重命名文档时用全仓搜索修复引用。

## Git 与交付

- 未经明确要求不要提交、amend、rebase 或 push。
- 用户要求“提交全部 diff”时，先审查 staged、unstaged 和 untracked 的完整内容，区分
  本次变更与用户已有变更，并执行与风险相称的验证。
- 提交前检查 `git diff --check`、完整 diff 和 `git diff --cached`；不要意外提交本地锁、
  预设、构建产物、密钥或绝对个人路径。
- 一个提交应表达一个可回退意图。
- 交付说明列出结果、主要文件、实际运行的验证和仍存在的风险，不把计划中的功能说成
  已实现。

## 仓库级 Codex 工作流

`.codex/skills/` 提供三个可复用工作流：

- `$build-repo`：依赖物化、配置、构建、测试与构建诊断；
- `$review-and-commit`：完整 diff 审查、比例验证和显式提交；
- `$context-handoff`：为长周期维护记录可继续执行的状态。

调用这些工作流时仍以本文件和更深层 `AGENTS.md` 为最高仓库约束。
