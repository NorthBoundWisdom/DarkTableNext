# Ravo

Ravo 是 DarkTableNext 仓库中的下一代照片处理内核。它将先作为无头 C++20 图像引擎和正式 CLI
交付，后续 catalog、应用服务与桌面 UI 只能建立在同一套 engine facade 之上。

当前状态：**只有设计文档，尚未创建源码、CMake target 或可执行文件。** 不要把文档中的示例命令
描述为已经可用。

## 名称与产物

| 名称 | 用途 |
| --- | --- |
| Ravo | 项目与下一代软件总称 |
| `ravo` | 首个正式 CLI |
| Ravo Engine | 无 UI 的解码、recipe、像素处理、色彩与导出内核 |
| Ravo Studio | 后续桌面应用的暂定产品名；无头阶段不创建 |

## 第一交付目标

第一版只建立可独立安装和测试的无头能力：

- 检查 RAW/JPEG/PNG/TIFF 输入与元数据；
- 枚举稳定 operation ID、参数 schema、输入输出约束和能力；
- 将已批准的旧 XMP 显式转换为 canonical recipe；
- 验证、升级并重复执行版本化 recipe；
- 通过 CPU 参考引擎生成预览或全尺寸输出；
- 写入 JPEG、PNG、TIFF 或复制原文件；
- 以稳定退出码和版本化 JSON 支持脚本、回归测试和批处理。

在无头验收完成前，不创建 UI toolkit 依赖、desktop target、catalog 数据库或第二套业务入口。

## 与 `src/` 的关系

`src/` 是 0.9 旧应用当前行为的事实来源；Ravo 是下一代实现的唯一增长方向。理想趋势是：

```text
Ravo 覆盖并验收的能力增加
              ↓
旧消费者被切换或由新产品替代
              ↓
src 中对应实现、构建项、资源和测试被删除
```

Ravo 不得包含 `src/` 私有头、链接 `libdarktable`、加载旧 IOP，或把旧全局状态包成新 API。迁移期间
允许旧侧通过窄 adapter 调用已稳定的 Ravo facade，但依赖方向只能是 `src` → Ravo；该 adapter 必须
实际减少旧代码，不能成为永久兼容层。完整规则见 [MIGRATION.md](MIGRATION.md)。

## 计划目录

这些目录只表示后续所有权；当前不应为了占位而创建空源码：

```text
Ravo/
  foundation/       值类型、错误、任务、取消和资源契约
  recipe/           operation schema、recipe、mask/blend 与版本迁移
  engine/           RAW、像素管线、色彩、CPU 参考处理与导出
  adapters/         文件系统、编解码器及以后加入的平台/GPU 实现
  cli/              `ravo` 命令行前端
  tests/            单元、契约、差分、金样和集成测试

  # 无头阶段验收后才创建
  domain/           catalog、照片版本、history、styles 与仓库端口
  services/         桌面应用用例
  desktop/          Ravo Studio
```

## 文档入口

- [AGENTS.md](AGENTS.md)：Ravo 子树内的实施约束；
- [ARCHITECTURE.md](ARCHITECTURE.md)：目标边界、契约、所有权与线程模型；
- [MIGRATION.md](MIGRATION.md)：让 Ravo 逐步替代并删除 `src/` 的规则；
- [TESTING.md](TESTING.md)：旧回归复用、新测试层次与验收门槛；
- [ADR 索引](docs/adr/README.md)：已确认且需要长期保留理由的决策；
- [根级重写计划](../TODO_REWRITE.md)：阶段、出口与完整产品路线。

仓库整体继续采用 GPLv3，详见根目录 [LICENSE](../LICENSE)。
