# Ravo Architecture Decision Records

ADR 记录已接受且会约束后续实现的重要决定。新 ADR 使用递增四位编号，包含状态、日期、背景、决定、
后果和被否决方案。已接受 ADR 不静默改写；方向变化时新增 ADR 并标记被替代关系。

| ADR | 状态 | 决定 |
| --- | --- | --- |
| [0001](0001-cpp20-headless-first.md) | Accepted | C++20、无头 engine/CLI 优先、首版不使用 Rust |
| [0002](0002-ravo-consumes-src.md) | Superseded by 0004 | Ravo 单向替代并最终删除旧 `src` 所有权 |
| [0003](0003-versioned-machine-contract.md) | Accepted | CLI JSON、recipe 和 operation schema 的版本化机器契约 |
| [0004](0004-freeze-09-ravo-only-growth.md) | Accepted | 冻结 0.9；Ravo 是唯一增长路径，发行切换后整体退役旧应用 |
| [0005](0005-qtcore-filesystem-adapter.md) | Accepted | Qt6::Core 可由无头 Ravo target 按需直接使用 |
| [0006](0006-explicit-colour-contract.md) | Accepted | 颜色状态在 engine 边界显式、版本化；第三方色彩类型保持私有 |
