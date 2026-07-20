# DarkTableNext Repository Instructions

## 项目定位

DarkTableNext 0.9 是 macOS-only、GPLv3 的照片工作流与 RAW 编辑维护基线。保留验证过的
图像处理核心和现有 GTK 前端，主动删除已确认不属于产品范围的历史兼容、插件和外围
工作流。

仓库根目录下的约定适用于整个仓库。`FreeCM/` 是独立子模块并由它自己的 `AGENTS.md`
管理；所有 `build/dependency_*` 内容都是父仓库工作流生成的外部源码，不得把它们当作
本仓库源码修改。

## 开始工作前

1. 运行 `git status --short --branch`，识别用户已有改动并保留它们。
2. 阅读与任务直接相关的源文件和文档，不根据上游 darktable 习惯猜测当前行为。
3. 功能删减先读 `TODO_CORE_REDUCTION.md`；GPU 任务先读 `DevDocs/GPU_Baseline.md`。
4. 跨层改动先明确所有权、生命周期、线程边界和最小验证集。

现有行为以 `src/` 为准；`DevDocs/` 是源码地图，不是兼容性承诺。产品边界以
`TODO_CORE_REDUCTION.md` 为准。

## 实施原则

- 本项目没有维护 darktable 历史插件、Lua、旧格式或旧 UI ABI 的默认义务；不要为
  已明确移除的能力增加空壳、兼容开关或迁移 shim。
- 删除功能时必须同时清理构建项、注册表、资源、配置、文档和测试；用全仓搜索确认
  没有可达消费者或残留用户入口。
- 维护现有 GTK 前端与 C/C++ 核心时，不要顺手改变 pixelpipe、IOP 算法或 CPU 图像结果。
- OpenCL 只有在 `TODO_CORE_REDUCTION.md` 的 Metal 阶段验收完成后才能删除。
- 遵循相邻 C/C++/CMake 文件的现有风格，只格式化触及的代码。不要借任务批量格式化
  遗留源码。
- 新依赖、公共 API、线程模型、数据库 schema 和产品范围变化都应在同一变更中记录
  设计理由与验证方法。
- 使用 `apply_patch` 做人工源码修改。生成器和格式化器仅用于它们明确拥有的输出。

## 现有 UI 与核心边界

- GTK 前端、dtgtk/Bauhaus 控件、Lighttable、Darkroom、导入、导出、catalog、history、
  masks、色彩空间和 pixelpipe 都保留在本仓库的既有 C/C++ 模块中。
- UI 改动必须遵循相邻 GTK 模块的生命周期、线程与信号约定；不得绕过图像缓存、数据库
  事务或任务队列直接修改共享状态。
- 不要在 UI 维护改动中引入第二套应用入口、运行时、工具包或跨仓库 UI 依赖。

## FreeCM 与依赖源码

- `source_roots.lock.jsonc.in` 是受版本控制的直接依赖基线。永久依赖变更只修改模板、
  `configs/source_roots.py` 和消费它们的 CMake 源码。
- `source_roots.lock.jsonc`、`CMakePresets.json`、`.freecm/` 与 `build/dependency_*`
  是本地状态，不提交，也不手工修补为永久方案。
- `python3 configs/source_root_workflow.py --init` 是唯一允许联网的依赖步骤；`--update`
  必须从活动锁离线物化并生成预设。
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

在 macOS 上，同一项变更需要同时验证 Debug 与 Release 时，必须交叉覆盖两套编译器：默认使用
`mac_gcc_debug` 与 `mac_clang_release`；也可根据任务使用 `mac_clang_debug` 与
`mac_gcc_release`。不要用同一编译器同时完成 Debug、Release 两项验证；单元测试运行在
所选的 Debug preset 上。此配对规则仅适用于 `mac_*` preset，不外推为其他平台的验证要求。

测试默认关闭。涉及可测试 C/C++ 行为时：

```sh
cmake --preset mac_clang_debug -DBUILD_TESTING=ON
cmake --build --preset mac_clang_debug
ctest --test-dir build/mac_clang_debug --output-on-failure -L unit
```

验证应与风险成比例：

- 纯 Markdown/代理说明：检查链接、命令、路径与 diff；无需强行完整编译。
- CMake/依赖图：至少运行 `--update` 和受影响 preset 的 configure。
- C/C++ 核心：构建受影响目标并运行相关 CTest；公共头或广泛改动运行完整 unit 标签。
- GTK/UI：构建应用并运行对应手工路径；涉及线程或状态时验证取消、错误和销毁路径。
- GPU/图像算法：除单元测试外，遵循 `DevDocs/GPU_Baseline.md` 的 CPU 金样和性能门槛。

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
