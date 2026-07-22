# Ravo Repository Instructions

本文件适用于 `Ravo/` 整个子树，并补充根目录 `AGENTS.md`。两者冲突时，以更严格且更具体的约束为准。

## 开工前

1. 运行 `git status --short --branch`，保留用户已有改动。
2. 阅读 `Ravo/README.md`、`ARCHITECTURE.md`、`MIGRATION.md`、`TESTING.md` 和相关 ADR。
3. 涉及旧行为或算法时，阅读对应 `src/` 实现和 fixture；不得根据上游 darktable 习惯猜测。
4. 确认本次工作属于当前阶段。阶段 3 出口前不得创建 desktop target；`Qt6::Core` 是允许的新工程
   基础依赖，Qt GUI、QML、Widgets 类型或 target 仍被禁止。
5. 写明所有权、生命周期、线程边界、错误/取消路径和最小验证集后再跨层修改。

## 当前技术边界

- 第一方新代码统一使用 C++20、CMake 与 FreeCM；不加入 Rust/Cargo 构建图。
- CPU 是正确性参考和可靠回退。GPU 只能在 CPU 金样与无头内核验收完成后作为 adapter 引入。
- `ravo` CLI 是正式客户端；算法必须在 engine 中，CLI 只负责输入输出、参数、进度和错误呈现。
- operation 首版为内建注册，不恢复旧动态 IOP ABI、GTK ABI 或插件兼容层。
- recipe、operation ID、参数 schema 和机器 JSON 必须版本化；不得序列化对象内存布局或 UI 状态。

## 依赖规则

- `foundation` 不依赖 recipe、engine、CLI、catalog、UI 或平台实现。
- `recipe` 只依赖 foundation；它不知道像素执行器、数据库或 UI。
- `engine` 依赖 foundation/recipe 及自己声明的端口；可以直接使用 QtCore，其他第三方裸类型优先留在
  私有 adapter。
- `cli` 依赖 engine facade 和 adapter composition；不得包含算法源码或访问 engine 私有状态。
- 后续 services 依赖 engine/domain；desktop 只依赖 services 和只读预览资源契约。
- Ravo 生产代码不得包含 `src/` 头、链接旧库、`dlopen` 旧模块或读取旧全局状态。测试只能读取已冻结
  fixture 与源码；不得配置、编译或运行旧 CLI、旧 CTest 或 `darktable-tests/run`。
- 冻结的旧应用不复用 Ravo，也不增加 adapter；生产依赖必须保持完全独立，直到 Ravo 达到发行切换
  门槛后整体退役旧应用。

## C++ 实施规则

- 使用值语义、不可变快照、RAII 和明确 owner；拥有资源的裸指针不得跨公开边界。
- view/span/string_view 必须有可证明且写入接口文档的生命周期。
- 异步工作使用受 owner 管理的执行器、任务句柄与取消令牌；禁止 detached thread。
- 错误使用可检查结果和结构化错误；异常不得穿过 target ABI、C 回调、任务或未来 FFI 边界。
- 不引入等价于全局 `darktable` 的可写服务集合，也不以 singleton 绕过依赖注入。
- 只格式化触及的代码；公共 API、依赖、线程或数据格式变化必须同时更新 ADR/架构和验证说明。

## 算法迁移

- 迁移单位是可由 CLI 观察的 capability/operation 批次，不是目录或行数。
- 直接阅读冻结 C 源码并重写 C++20 行为；用聚焦 Ravo UT 覆盖旧实现的关键分支，但不得编译或运行旧 UT。
- 可以移植可理解的数学思想或纯算法，但不得连带复制 GUI、旧 module lifecycle、配置 shim、
  动态注册、OpenCL 类型或无消费者代码。
- Ravo 通过完整产品验收并完成发行切换后，才在阶段 7 删除 `src` 所有权。删除必须覆盖 CMake、
  资源、配置、文档与测试，并以全仓搜索证明没有可达消费者；迁移期间不逐项修改冻结的旧实现。
- 旧 OpenCL 保持冻结并随 0.9 整体退役；Ravo GPU 只能在自身 CPU 路径验收后以独立 adapter 实现。

## 验证与交付

- Windows 上的新工程辅助工具只使用 Python 或 PowerShell；不得引入旧应用 runner。CMake/MSVC 只配置
  和编译 `Ravo/` target。

- 纯文档：检查真实路径、相对链接、命令、术语、diff 和 `git diff --check`。
- 新 C++ 单元：只链接 Ravo target，并运行相关 unit/contract 标签和 sanitizer 可行集。
- operation：运行参数/schema、合成边界、旧 XMP 映射，并把 Ravo 输出与已提交 RAW/PNG/metadata
  fixture 比较；不启动旧进程生成即时差分。
- 公共头或调度广泛改动：运行完整 Ravo unit/contract、保留 fixture 和跨平台可行构建。
- 不把未运行测试写成通过，不把 macOS 构建写成全平台通过。
- 未经明确要求，不提交、amend、rebase 或 push。

Ravo 的 Windows/MSVC 构建命令和 FreeCM 项目命令分别记录在 `README.md` 与
`../configs/freecm.commands.jsonc`；这些入口只能配置、构建、运行和测试 Ravo。
