# ADR-0002: Ravo consumes and retires legacy `src`

- Status: Superseded by ADR-0004
- Date: 2026-07-21

ADR-0004 keeps the one-way replacement goal but supersedes this ADR's
incremental old-side adapters and capability-by-capability deletion strategy.

## Context

并行重写容易形成两套永久实现：Ravo 持续增加代码，而 `src` 因兼容理由永远不删。这样会扩大维护面，
也会诱使新代码通过旧头文件、旧库或 shim 取得短期进度。

期望状态是 Ravo 覆盖能力逐渐增加，`src` 的可达能力逐渐减少，最终 Ravo 取代整个旧应用。

## Decision

- 迁移以 capability/operation 为单位，并为每项建立旧 CPU 基线和新契约。
- Ravo 生产代码永不依赖 `src`；差分只通过 fixture 或独立旧进程完成。
- 必要过渡依赖只能是 `src` → Ravo 稳定 facade，而且必须带来可衡量的旧代码删除。
- 一项能力只有在 Ravo 验收、消费者切换、旧源码/构建/资源/配置/文档删除后才算迁移完成。
- 最终发行切换后删除旧应用入口和剩余 `src`，Ravo 成为唯一受支持实现。

## Consequences

- 迁移期间可能存在并行可执行文件，但不存在 Ravo 对旧核心的编译或运行时依赖。
- 短期实现速度可能慢于直接复制；长期不会维护两份可达算法或永久兼容层。
- 删除是每个迁移单元的一部分，需要风险相称的回归和兼容性决定。
- 源码行数可用于观察趋势，但完成度以可达消费者、测试和所有权为准。

## Rejected alternatives

- **Ravo 永久链接 `libdarktable`**：耦合方向错误，无法独立发布或删除旧核心。
- **先复制全部源码再整理**：形成第二份遗留系统，无法证明每项行为或所有权边界。
- **只在最终一次性删除 `src`**：长期累积重复实现，切换风险不可控。
