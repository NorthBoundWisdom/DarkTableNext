# Ravo Testing Strategy

## 当前静态基线

截至 2026-07-21：

- 冻结的 `src/tests` 清单注册 3 个历史 CTest target；其中一个 sample 不执行断言；
- `darktable-tests/` 有 158 组 XMP + `expected.png` 静态图像 fixture；
- fixture 覆盖 68 个 operation 名；当前 IOP CMake 清单有 76 个非注释注册项，部分受条件控制；
- 保留的图像回归明确排除了 UI 测试。

以下 Python 检查验证清单、哈希与冻结边界，不配置或执行旧 target：

```powershell
python Ravo/tools/freeze_legacy_manifest.py --check
python Ravo/tools/check_capability_inventory.py
python Ravo/tools/check_freeze_reference.py
```

这些数字是静态资产盘点，不是覆盖率。旧工程、旧 CLI、旧 CTest 和 `darktable-tests/run` 全部冻结且
禁止运行；已提交 fixture 只能作为 Ravo 端到端输出的只读参考，不能替代新模型、参数、边界、所有权
和错误路径测试。

## Test framework boundary

All new Ravo C++ unit, contract, and integration tests use GoogleTest;
GoogleMock is permitted when a port interaction itself is the contract under
test. CMocka is a legacy `src/tests` dependency only and must not be linked by
a Ravo target. The Phase 1 CMake graph finds the already installed
`gtest:x64-windows` 1.17.0 package through the existing vcpkg toolchain; it
does not use `FetchContent`, a CMake network download, or a legacy library.

The Windows test graph also requires Qt 6.11.1 `Core`, discovered from the
FreeCM-generated root CMake preset. QtCore is an allowed Ravo dependency and
its runtime is copied beside `ravo` and contract tests on Windows so test
discovery does not depend on a global `PATH`. The actual
commands are in [Ravo/README.md](README.md#构建与测试). Current labels are
`ravo-unit` and `ravo-contract`; regression, sanitizer and
performance labels remain future work.

Current contract coverage includes versioned JSON/exit semantics, Unicode
local paths through the Qt adapter, atomic recipe output, empty-history legacy
XMP import, the narrowly proven manual exposure-v5 mapping, and the explicit
rejection of unmapped legacy operations, blend/mask state, and multi-entry
history. It also covers real `mire1.cr2` inspection, strict absorption of the
frozen `nop.xmp` baseline, bounded non-empty PNG output, output conflict,
memory/cancellation paths, command-line input binding, and the visible
brightness increase from `+1 EV`.

## 测试层次

| 层次 | 目标 | 典型内容 |
| --- | --- | --- |
| Unit | 一个纯类型或算法 | schema、参数范围、矩阵/曲线、ROI、升级、数值边界 |
| Contract | target 与公开契约 | facade、CLI JSON/退出码、codec/FS 端口、错误、取消 |
| Synthetic image | 可定位像素行为 | 小图、边缘、alpha、NaN/Inf、mask、几何、分块 |
| Legacy mapping | 旧数据读取 | XMP operation/version/params → canonical recipe 或显式拒绝 |
| Golden/reference | Ravo CPU 对冻结 fixture | 真实 RAW、已提交 PNG、Ravo 32-bit float、metadata、容差 |
| Integration | 完整无头工作流 | inspect → import XMP → validate → render → atomic output |
| Performance/resource | 可交付性 | 延迟、吞吐、峰值内存、长批处理、取消、磁盘满 |
| UI（后续） | 桌面行为 | service contract、焦点、输入、快速切图、销毁、可访问性 |

## 冻结 fixture 复用

- 开工前运行 Python 静态检查，确认 158 组 fixture、5 个原图、哈希与冻结树一致。
- 不配置、编译或执行旧工程、旧 CLI、旧 CTest、旧打包 target 或 `darktable-tests/run`；Windows 辅助
  脚本只使用 Python 或 PowerShell。
- `tests/fixtures/fixture_classification_ledger.json` 必须与 `legacy_manifest.json` 的 fixture ID 集合完全一致；
  初始 `unclassified` 不是缺少输入或 skip。后续分类只能为 `frozen_fixture_reference`、`missing_asset`、
  `product_approved_incompatibility`、`deferred_ravo_operation` 或 `unclassified`，且非 `unclassified` 项必须
  链接可读静态、产品决定或 Ravo 验收证据。
- 已提交原图、XMP 和 `expected.png` 是权威冻结参考输入；Ravo 自有 32-bit float 输出、元数据摘要、
  配置和容差作为新证据另存，不覆盖冻结资产。
- 每个保留 fixture 只运行 Ravo CPU，比较像素、NaN/Inf、尺寸/ROI、alpha、色彩、metadata 和错误状态。
- 已从 Ravo 产品范围删除的 fixture 只有在兼容性决定记录后才能排除，并必须测试可读的拒绝错误。

## 每个 operation 的最低验收

- operation descriptor 与参数 schema 单元测试；
- 默认值、上下限、非法值、版本升级和序列化 round-trip；
- 至少一个小型合成图，用于定位通道、边缘、ROI 或 mask 行为；
- 至少一个真实 RAW + canonical recipe 金样；
- 若旧 XMP 支持，至少一个 legacy 映射与差分 fixture；
- NaN/Inf、空/异常 ROI、内存不足、取消和不支持输入行为；
- 记录 CPU 数值容差、性能/内存预算和实际验证平台。

一张 8-bit 最终 PNG 不能单独满足上述门槛。

## 确定性模式

`ravo render` 的测试模式必须能固定：

- CPU backend；
- worker 数和调度策略；
- 内存预算与 tiling 决策输入；
- 输出尺寸、插值器、ICC intent 和 metadata 策略；
- 随机种子（若算法需要）；
- engine、recipe schema、operation 和第三方依赖版本。

确定性不要求所有浮点位跨架构完全相同，但每项允许差异必须有可解释容差。NaN/Inf、几何、操作顺序、
mask 语义和色彩空间不允许以浮点差异为理由变化。

## 本地测试标签

首个构建骨架落地时，应建立独立 Ravo 标签，至少分为：

- `ravo-unit`：快速、纯 C++、无旧库和 UI；
- `ravo-contract`：facade、adapter 和 CLI；
- `ravo-regression`：选定 RAW/金样；
- `ravo-sanitizer`：ASan/UBSan，平台可用时增加 TSan；
- `ravo-performance`：报告型或带经批准门槛的代表性工作负载。

当前已建立本地 `ravo-unit` 和 `ravo-contract` target/命令；后者还运行只读的 manifest、vertical-slice
candidate、capability inventory、fixture classification ledger、freeze-reference 与 production
dependency-boundary Python contract check。本轮只维护本机 Windows/MSVC 配置与编译，不新增 CI/CD；
其余标签在相应本地测试实际存在前仍不得描述为已通过。
