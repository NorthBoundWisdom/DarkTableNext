# Ravo Migration Policy

## 目标

Ravo 最终整体取代冻结的 `src/`，而不是永远与它并存。迁移期间新所有权只进入 `Ravo/`，不为分步
迁移修改旧应用；达到发行切换门槛后，才在阶段 7 删除 `src/`、旧构建和资源。单纯复制代码而没有
新契约、验证与最终退役计划，不算迁移。

## 单向边界

允许：

- Ravo 测试读取 `darktable-tests/` 中冻结的原图、XMP 和金样；
- Ravo 通过 FreeCM 直接消费已固定的第三方依赖。

禁止：

- Ravo 生产 target 包含 `src/` 私有头或链接 `libdarktable`；
- Ravo 加载旧动态 IOP、读取全局 `darktable`，或让旧 GTK/配置类型进入公开 API；
- 配置、编译或运行旧 CLI、旧 CTest、`darktable-tests/run` 或旧打包 target；
- 让冻结的 `src` 调用 Ravo facade，或为迁移在旧应用中增加 adapter、入口和构建依赖；
- 为了让测试先绿而由 Ravo CLI 暗中转发到旧 CLI；
- 把旧文件复制进 Ravo 后同时保留两份可达实现；
- 以永久 shim 掩盖尚未决定的数据或产品兼容性。

生产依赖始终保持完全独立；不得形成 `src` → Ravo 或 Ravo → `src`。

## 一个迁移单元的步骤

每个 capability、共享算法或 operation 按以下顺序迁移：

1. **盘点**：列出源码 owner、CMake 注册、调用者、数据格式、线程、缓存、GPU、配置、资源和测试。
2. **冻结行为**：读取已提交的真实 RAW、XMP/参数、像素/元数据输出和容差；不重编或运行旧 CPU 路径。
3. **定义新契约**：写 canonical schema、输入输出、所有权、取消/失败语义和不兼容项。
4. **实现 Ravo**：只迁入所需数学与行为；去除 GUI、旧生命周期、全局状态、动态 ABI 和后端特有类型。
5. **分层验证**：新单元/合成测试、旧 XMP 映射、真实 RAW 差分、错误/取消、资源和可行平台构建。
6. **Ravo 验收**：让新 CLI/服务成为 Ravo 的正式消费者，记录该 capability 已由新产品支持；冻结的
   旧应用仍保持原样。
7. **产品切换后退役**：只有整个 Ravo 达到发行切换门槛后，才清理 `src`、CMake、注册表、资源、
   配置、文档和重复测试，并全仓搜索剩余消费者。
8. **记录结果**：更新本文 ledger、根级 TODO、兼容性结论、实际验证与未覆盖风险。

一次变更不必同时完成全部步骤，但不得把“已复制”或“新版本可编译”标为迁移完成。

## “已被 Ravo 吃掉”的定义

“Ravo 已验收”与“旧实现已删除”是两个状态。某项能力只有同时满足以下条件才算最终被 Ravo 吃掉：

- Ravo 是受支持实现，并拥有 schema、CPU 行为、错误、取消和资源契约；
- 所有承诺 fixture 和新分层测试达到阈值；
- 发行切换已完成，生产构建中没有第二份可达旧实现；
- `src` 对应源码、构建项、动态注册、配置、资源和用户入口已删除；
- 旧历史/sidecar 的迁移或显式拒绝策略已记录并测试；
- 文档不再把旧路径描述为当前 owner；
- `rg` 与链接图检查没有意外消费者或反向依赖。

## 迁移顺序原则

- 先做可无头验收的基础契约、recipe、codec、CPU pixelpipe 和 CLI。
- 再迁共享设施：色彩、ROI/分块、mask/blend、插值、缓存与内存预算。
- operation 按产品保留清单和管线依赖顺序迁移，不按文件数量排序。
- catalog、history、styles 和 UI 只在无头阶段 3 出口后开始。
- GPU 只在 CPU 参考路径完成后进入 Ravo；旧 OpenCL 不在 0.9 内迁移，阶段 7 随旧应用整体退役。
- 最后切换发行物并删除旧应用入口，而不是在早期让两个应用共享可写 catalog。

## 初始迁移 ledger

| Capability | 旧 owner | Ravo owner | 状态 | Ravo 验收证据 |
| --- | --- | --- | --- | --- |
| 基础错误/任务/取消 | `src/common`, `src/control` | `foundation` | 实现中 | [取消与 deadline 契约测试](tests/foundation_test.cpp)；无旧类型泄漏 |
| Recipe 与 operation schema | IOP params/introspection/XMP | `recipe` | 实现中 | [版本化格式与验证测试](tests/recipe_test.cpp)；legacy adapter 仅接受已证明的映射 |
| 输入检查与 RAW 解码 | common/imageio/LibRaw 路径 | `engine` + codec adapter | 实现中 | [真实 `mire1.cr2` inspect 与 render 测试](tests/engine_test.cpp)；当前仅覆盖 LibRaw 16-bit Bayer RAW |
| CPU pixelpipe/ROI/分块 | `src/develop` | `engine` | 实现中 | [有界 PNG、内存预算、取消与 exposure 亮度测试](tests/engine_test.cpp)；完整 ROI/分块仍未完成 |
| 色彩管理 | common/develop/iop | `engine` | 实现中 | 首个 RAW 切片应用 camera WB、LibRaw camera-to-sRGB 矩阵与 sRGB 编码；冻结金样、ICC/metadata 契约仍未验收 |
| Mask/blend | `src/develop` | `recipe` + `engine` | 未开始 | schema、重放和像素差分 |
| 保留 operation | `src/iop` | `engine` | 未开始 | 每项 operation 验收清单 |
| 本地输出 | `src/imageio` | engine + codec adapter | 实现中 | [PNG 原子写入与冲突测试](tests/engine_test.cpp)；JPEG/TIFF/原文件复制及完整 metadata 尚未实现 |
| CLI | `src/cli` | `cli` | 实现中 | [版本化 JSON、XMP、真实 RAW inspect/render 与错误契约测试](tests/cli_test.cpp)；批处理和完整导出仍在 Phase 2/3 |
| Catalog/history/styles | common/libs | `domain` + `services` | 延后 | 阶段 3 出口与数据兼容决定 |
| 桌面 UI | gui/dtgtk/bauhaus/views/libs | `desktop` | 延后 | 阶段 4 框架 ADR 与服务契约 |

状态只使用“未开始 / 基线冻结 / 实现中 / Ravo 已验收 / 旧实现已删除 / 延后 / 不支持”。每次改变状态
必须链接到证据或在同一变更说明中记录证据。

表内证据只决定“Ravo 已验收”；所有旧 owner 的物理删除统一等待阶段 7 的完整产品切换。
