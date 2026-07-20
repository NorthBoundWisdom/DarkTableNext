# C11 基线与条件编译收敛计划

## 目标与边界

将 DarkTableNext 自有 C 源码统一以 ISO C11 编译、C++ 源码统一以 ISO C++20 编译，并在
macOS、Windows 与 Linux 上持续保持可配置、可编译；逐步消除为历史方言或编译器拼写保留的
分支。本计划不改变 pixelpipe、IOP 数值算法、RAW 解码结果、数据库 schema 或动态模块 ABI。

不以“没有预处理器”为目标。以下分支是产品或平台能力，而非历史方言兼容，必须保留并
在每次触及时以能力说明替代笼统的编译器判断：

- macOS 与 Windows 的 SDK、文件系统、线程、动态加载和 ABI 差异；
- CPU 架构、OpenMP/SIMD、OpenCL/Metal 以及性能路径；
- C99 complex 在 MSVC 中仍不可用的模块选择；
- 已检查的第三方库 API 或工具链能力。

## 执行批次

- [x] 将 ISO C11 和 ISO C++20 设为顶层 CMake 基线，使本仓库自有目标都显式继承
      `C_STANDARD 11`、`CXX_STANDARD 20`、对应的 `*_STANDARD_REQUIRED` 与禁用 GNU 扩展的
      设置；FreeCM 物化的外部项目保留其自有语言要求，MSVC 的 C 原子支持仍保留为明确的
      能力开关。
- [x] 移除顶层仅允许 macOS/Windows 的 CMake 入口限制；由各平台的依赖与能力检查决定配置
      是否可继续，Linux 不再被产品策略提前拒绝。
- [x] 删除 Windows `near` 宏的局部 `#ifdef _MSC_VER`/`#undef` 补丁：重命名所有受影响
      的内部字段、形参和局部变量，不依赖宏污染的反向撤销。
- [ ] 建立全仓清单，将剩余 `__GNUC__`、`__clang__`、`_MSC_VER`、GNU attributes 和 builtin
      使用按“标准方言兼容 / 编译器能力 / 平台 SDK / 性能专用”分类。
- [ ] 将可由 C11 表达的原子、对齐、静态断言和类型能力统一到标准头或项目单一封装；对
      不可标准化的 attribute 与 builtin 提供特性检测，而不扩散编译器名称判断。
- [ ] 单独审计每个 GCC 专属优化 pragma；只有具备 CPU 基线、性能测量和 Clang/MSVC 等价
      策略的路径才可改写，避免把性能条件编译误当作方言兼容删除。
- [ ] 最后收敛 Windows 兼容头：保留必要的 POSIX/SDK 映射，删除仅为旧方言模拟且已有
      C11 替代的宏；不得用空定义掩盖缺失的语义。
- [ ] 在 Windows 与 Linux 工具链上分别完成 C11/C++20 configure、完整构建和 unit 测试；把
      平台特有失败归入 SDK、依赖或能力缺口，而不是重新引入旧方言开关。

## 每批验收

- CMake configure 与 `mac_gcc_debug`、`mac_clang_release` 完整构建；
- 触及公共头或核心代码时，在 Debug preset 运行完整 `unit` 标签；
- `git diff --check` 与触及 C/C++ 文件的 `git clang-format --diff`；
- 若触及图像或 SIMD 路径，另行遵循 `DevDocs/GPU_Baseline.md` 的 CPU 金样要求。

## 当前批次（C11 基线与 `near`）

本批只变更构建语言约束和内部标识符；不修改条件编译所保护的运行时行为。

- [x] C++20 对 lens 参数枚举与独立修改位枚举之间的隐式位运算提出诊断；在不改变参数
      布局的前提下，经单一显式整数转换辅助函数完成位测试。
- [x] 盘点 `src/` 中剩余 37 处编译器条件：GCC 专属项均为有限数值语义或局部性能 pragma；
      MSVC 专属项均为 Windows SDK/POSIX 差异或预处理器限制；Clang 专属项为静态分析或 SIMD
      pragma。它们不是历史 C 方言分支，保留到各自专项审计。
- [x] 将 4 个 C 局部或静态数组对象的 GNU `aligned` 属性替换为 C11 `_Alignas`；不修改
      `packed`、typedef 对齐或结构体 ABI 对齐。
- [x] 将线程安全静态分析注解从 `__clang__` 名称判断改为 `__has_attribute(capability)`
      特性检测；不支持时保持无语义的空注解。
- [x] 让公共 `darktable.h` 自行保护 `target_clones` 特性查询；MSVC 或其他未定义
      `__has_attribute` 的编译器不再依赖强制包含的兼容头才能解析该宏。
