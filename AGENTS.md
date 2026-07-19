# DarkTableNext Repository Instructions

## 项目定位

DarkTableNext 0.9 是 macOS-only、GPLv3 的照片工作流与 RAW 编辑重构基线。目标是
保留验证过的图像处理核心，主动删除历史产品兼容、插件和外围工作流，并以 Qt 6/QML
重写 UI。当前可运行前端仍是 GTK3；目标架构不能被描述成已经完成。

仓库根目录下的约定适用于整个仓库。`FreeCM/` 是独立子模块并由它自己的
`AGENTS.md` 管理；所有 `build/dependency_*` 内容都是父仓库工作流生成的外部源码，
不得把它们当作本仓库源码修改。

## 开始工作前

1. 运行 `git status --short --branch`，识别用户已有改动并保留它们。
2. 阅读与任务直接相关的源文件和文档，不根据旧 darktable 习惯猜测当前行为。
3. UI/GeoControls 任务先读 `GEOCONTROLS_INTEGRATION.md`；功能删减先读
   `TODO_CORE_REDUCTION.md`；GPU 任务先读 `DevDocs/GPU_Baseline.md`。
4. 跨层或跨仓改动先明确所有权、生命周期、线程边界和最小验证集。

现有行为以 `src/` 为准；`DevDocs/` 是源码地图，不是兼容性承诺。目标产品边界以
`TODO_CORE_REDUCTION.md` 和 `GEOCONTROLS_INTEGRATION.md` 为准。

## 实施原则

- 本项目没有维护 darktable 历史插件、Lua、旧格式或旧 UI ABI 的默认义务；不要为
  已明确移除的能力增加空壳、兼容开关或迁移 shim。
- 删除功能时必须同时清理构建项、注册表、资源、配置、文档和测试；用全仓搜索确认
  没有可达消费者或残留用户入口。
- 不要把大规模 UI 重写与 pixelpipe/IOP 算法重写放在同一个变更中。先建立稳定
  adapter 和垂直切片，再替换产品界面。
- 保留 CPU 正确路径。OpenCL 只有在 `TODO_CORE_REDUCTION.md` 的 Metal 阶段验收完成
  后才能删除；UI 工作不能顺手改变 GPU 路由或浮点结果。
- 遵循相邻 C/C++/CMake 文件的现有风格，只格式化触及的代码。不要借任务批量格式化
  遗留源码。
- 新依赖、公共 API、线程模型、数据库 schema 和产品范围变化都应在同一变更中记录
  设计理由与验证方法。
- 使用 `apply_patch` 做人工源码修改。生成器和格式化器仅用于它们明确拥有的输出。

## UI 与 GeoControls 边界

目标依赖方向只有：

```text
GeoControls <- DarkTableNext.Controls <- DarkTableNext.App
```

- GeoControls 只拥有主题、基础控件、通用数值/列表/弹窗机制和无照片语义的 Qt Quick
  绘制原语。
- DarkTableNext 拥有 IOP、参数描述、历史/撤销、pixelpipe、catalog、mask、色彩空间、
  histogram 和 Lighttable/Darkroom 语义。
- QML 只能通过有明确生命周期的 `QObject`、`QAbstractItemModel` 或值快照访问核心；
  不暴露裸 C 指针，不在 QML 主线程直接访问数据库或执行图像处理。
- 通用控件可以提供 preview/commit 信号，但一次交互如何合并成历史项由
  DarkTableNext 决定。
- 产品 `ModulePanel`、ImageCanvas、filmstrip 和图像 overlay 留在本仓库；可以组合
  GeoControls 原语，但不能把领域对象下沉到通用库。
- 首批 QML 工作使用真实垂直切片验证参数、历史、预览和销毁路径，不机械翻译 GTK
  widget tree。

完整矩阵与双仓顺序见 `GEOCONTROLS_INTEGRATION.md`。

## FreeCM 与依赖源码

- `source_roots.lock.jsonc.in` 是受版本控制的直接依赖基线。永久依赖变更只修改模板、
  `configs/source_roots.py` 和消费它们的 CMake 源码。
- `source_roots.lock.jsonc`、`CMakePresets.json`、`.freecm/` 与
  `build/dependency_*` 是本地状态，不提交，也不手工修补为永久方案。
- `python3 configs/source_root_workflow.py --init` 是唯一允许联网的依赖步骤；
  `--update` 必须从活动锁离线物化并生成预设。
- 正常开发使用 `depsMode: "pinned"`。联调依赖时可在活动锁使用 `manual`，但
  `depsManualPath` 必须指向真实、独立的 checkout，不能指向 seed/source root 或
  install 目录。
- 不在 `build/dependency_seed_repos` 或 `build/dependency_source_roots` 中运行依赖
  bootstrap/update；这些目录由宿主仓库管理，随时可能被重新物化。
- 跨仓改动先在依赖仓库提交并推送，再让本仓库锁模板固定该 commit，最后提交宿主
  适配与 UI 改动。不得固定未推送 commit 或 dirty worktree。
- 不使用 `FetchContent`、CMake 网络下载、submodule 或源码复制绕过已有工作流。

## 构建与验证

首次准备工作区：

```sh
git submodule update --init FreeCM
python3 configs/source_root_workflow.py --init
python3 configs/source_root_workflow.py --update
```

默认开发构建：

```sh
cmake --preset mac_clang_debug
cmake --build --preset mac_clang_debug
```

测试默认关闭。涉及可测试 C/C++ 行为时：

```sh
cmake --preset mac_clang_debug -DBUILD_TESTING=ON
cmake --build --preset mac_clang_debug
ctest --test-dir build/mac_clang_debug --output-on-failure -L unit
```

验证应与风险成比例：

- 纯 Markdown/代理说明：检查链接、命令、路径、frontmatter 和 diff；无需强行完整编译。
- CMake/依赖图：至少运行 `--update`（若活动锁可用）和受影响 preset 的 configure。
- C/C++ 核心：构建受影响目标并运行相关 CTest；公共头或广泛改动运行完整 unit 标签。
- QML/GeoControls：两个仓库各自通过配置、构建和 lint/测试，再运行宿主垂直切片。
- GPU/图像算法：除单元测试外，遵循 `DevDocs/GPU_Baseline.md` 的 CPU 金样和性能门槛。
- 性能结论使用 Release 构建、固定输入和多次测量，不从 Debug 单次结果推断。

如果环境缺依赖或测试数据，应先做所有仍可执行的静态/局部验证，并准确报告未运行项
及原因；不要把“未运行”写成“通过”。

## 文档与生成文件

- 架构、构建命令、依赖约定或产品范围变化时，同步更新 `README.md`、相关根级文档和
  `DevDocs/` 索引。
- 不编辑生成的 `CMakePresets.json` 作为最终修复；修改锁模板/生成逻辑后运行
  `--update`。
- 不提交构建树、临时报告、本地活动锁、IDE 文件或依赖 checkout。
- 链接必须指向仓库中真实存在的文件；删除/重命名文档时用全仓搜索修复引用。

## Git 与交付

- 未经明确要求不要提交、amend、rebase 或 push。
- 用户要求“提交全部 diff”时，先审查 staged、unstaged 和 untracked 的完整内容，
  区分本次变更与用户已有变更，并执行与风险相称的验证。
- 提交前检查 `git diff --check`、完整 diff 和 `git diff --cached`；不要意外提交本地
  锁、预设、构建产物、密钥或绝对个人路径。
- 一个提交应表达一个可回退意图。跨 GeoControls/DarkTableNext 的工作保持两个仓库
  各自提交，并遵守“依赖先推送、宿主后固定”的顺序。
- 交付说明列出结果、主要文件、实际运行的验证和仍存在的风险，不把计划中的功能说成
  已实现。

## 仓库级 Codex 工作流

`.codex/skills/` 提供四个可复用工作流：

- `$build-repo`：依赖物化、配置、构建、测试与构建诊断；
- `$qml-ui-integration`：GeoControls 边界和 Qt/QML 垂直切片；
- `$review-and-commit`：完整 diff 审查、比例验证和显式提交；
- `$context-handoff`：为长周期重构记录可继续执行的状态。

调用这些工作流时仍以本文件和更深层 `AGENTS.md` 为最高仓库约束。
