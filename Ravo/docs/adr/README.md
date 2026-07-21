# Ravo Architecture Decision Records

ADR 记录已接受且会约束后续实现的重要决定。新 ADR 使用递增四位编号，包含状态、日期、背景、决定、
后果和被否决方案。已接受 ADR 不静默改写；方向变化时新增 ADR 并标记被替代关系。

| ADR | 状态 | 决定 |
| --- | --- | --- |
| [0001](0001-cpp20-headless-first.md) | Accepted | C++20、无头 engine/CLI 优先、首版不使用 Rust |
| [0002](0002-ravo-consumes-src.md) | Accepted | Ravo 单向替代并最终删除旧 `src` 所有权 |
