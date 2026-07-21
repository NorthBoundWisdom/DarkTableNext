# Ravo Testing Strategy

## 当前旧基线

截至 2026-07-21：

- `src/tests` 注册 3 个 CTest target；其中一个 sample 不执行断言；
- `darktable-tests/` 有 158 组 XMP + `expected.png` 图像回归；
- fixture 覆盖 68 个 operation 名；当前 IOP CMake 清单有 76 个非注释注册项，部分受条件控制；
- 保留的图像回归明确排除了 UI 测试。

重跑统计：

```sh
rg -n 'add_test\(|add_cmocka_test\(' src/tests
find darktable-tests -mindepth 1 -maxdepth 1 -type d -name '[0-9][0-9][0-9][0-9]-*' | wc -l
find darktable-tests -mindepth 2 -maxdepth 2 -type f -name '*.xmp' | wc -l
find darktable-tests -mindepth 2 -maxdepth 2 -type f -name 'expected.png' | wc -l
rg -o 'operation="[^"]+"' darktable-tests -g '*.xmp' | sed 's/.*operation="//;s/"$//' | sort -u | wc -l
rg -n '^[[:space:]]*add_iop\(' src/iop/CMakeLists.txt | wc -l
```

这些数字是测试资产盘点，不是覆盖率。旧图像测试适合作为端到端 oracle，但不能替代新模型、参数、
边界、所有权和错误路径测试。

## Test framework boundary

All new Ravo C++ unit, contract, and integration tests use GoogleTest;
GoogleMock is permitted when a port interaction itself is the contract under
test. CMocka is a legacy `src/tests` dependency only and must not be linked by
a Ravo target. GoogleTest 1.17.0 is installed for this Windows workspace, but
there is not yet a Ravo target or a command to run it. The first CMake change
must document its fixed dependency source and the actual test invocation
without adding `FetchContent` or a network download.

## 测试层次

| 层次 | 目标 | 典型内容 |
| --- | --- | --- |
| Unit | 一个纯类型或算法 | schema、参数范围、矩阵/曲线、ROI、升级、数值边界 |
| Contract | target 与公开契约 | facade、CLI JSON/退出码、codec/FS 端口、错误、取消 |
| Synthetic image | 可定位像素行为 | 小图、边缘、alpha、NaN/Inf、mask、几何、分块 |
| Legacy mapping | 旧数据读取 | XMP operation/version/params → canonical recipe 或显式拒绝 |
| Differential/golden | 新旧 CPU 一致性 | 真实 RAW、最终 PNG/TIFF、32-bit float、metadata、容差 |
| Integration | 完整无头工作流 | inspect → import XMP → validate → render → atomic output |
| Performance/resource | 可交付性 | 延迟、吞吐、峰值内存、长批处理、取消、磁盘满 |
| UI（后续） | 桌面行为 | service contract、焦点、输入、快速切图、销毁、可访问性 |

## 旧回归复用

- 开工前用固定旧构建、依赖、线程、内存和 CPU backend 实跑全部 fixture。
- 只冻结实际通过且原图齐全的基线；基线失败或平台波动必须分类，不能默认标绿。
- 保存原图、XMP、`expected.png`、必要的 32-bit float 输出、元数据摘要、命令配置和容差。
- 新实现期间禁止用 Ravo 输出执行 `--update-expected`。
- legacy wrapper 可以转换命令行调用，但不能修改 XMP、暗加参数、放宽容差或转发到旧 CLI。
- 同一 fixture 独立运行旧 CPU 和 Ravo CPU；比较像素、NaN/Inf、尺寸/ROI、alpha、色彩、metadata
  和错误状态。
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

## CI 门槛

首个构建骨架落地时，应建立独立 Ravo 标签，至少分为：

- `ravo-unit`：快速、纯 C++、无旧库和 UI；
- `ravo-contract`：facade、adapter 和 CLI；
- `ravo-regression`：选定 RAW/金样；
- `ravo-legacy-diff`：需要旧 CLI/fixture 的隔离任务；
- `ravo-sanitizer`：ASan/UBSan，平台可用时增加 TSan；
- `ravo-performance`：报告型或带经批准门槛的代表性工作负载。

当前没有这些 target 或命令。创建时必须同步更新本文件，不得把计划中的 CI 写成已经通过。
