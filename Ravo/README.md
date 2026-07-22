# Ravo

Ravo 是 DarkTableNext 仓库中的下一代照片处理内核。它将先作为无头 C++20 图像引擎和正式 CLI
交付，后续 catalog、应用服务与桌面 UI 只能建立在同一套 engine facade 之上。

当前状态：**Phase 0 文档、决策记录和 fixture 清单已冻结；Phase 1 已建立独立的 C++20
foundation、recipe、engine、adapter、CLI 和 GoogleTest target。** `ravo --version --json`、
`ravo operations --json` 与 `ravo recipe validate <recipe> --json` 已实现并受测试保护。RAW inspect、
CPU render 仍返回结构化 `unsupported`，不得当作已实现的像素工作流。legacy XMP import 目前只接受空
history；任何旧 operation 都返回 `unsupported_legacy_operation`，直到其 canonical 参数映射经过验证。

## 构建与测试

先运行仓库根目录的 `python3 configs/source_root_workflow.py --update`，它会从活动锁生成根
`CMakePresets.json`，其中包含 Windows Qt SDK 的 `CMAKE_PREFIX_PATH`。随后从已加载 MSVC 环境的
终端运行：

```powershell
Set-Location Ravo
cmake --preset ravo_win_msvc_debug -DBUILD_TESTING=ON
cmake --build --preset ravo_win_msvc_debug --parallel
ctest --preset ravo_win_msvc_debug
```

`Ravo/CMakePresets.json` 只继承根目录生成的 Windows preset，不复制 Qt SDK 路径。`Qt6::Core` 只在
`ravo_adapters` 私有实现中处理 UTF-8 本地路径、文件 I/O 与未来原子输出；它不把 Qt 类型暴露给
engine/recipe/CLI 头文件，也不引入 Qt GUI、QML、Widgets 或 desktop target。Qt 的版本、许可证和
运行时部署决定见 [ADR-0005](docs/adr/0005-qtcore-filesystem-adapter.md)。

当前的 XMP 导入命令要求调用者显式提供资产身份，不从 sidecar 路径猜测：

```text
ravo recipe import-xmp <legacy.xmp> --asset-id <id> --input <input-uri> --output <recipe> --json
```

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
Ravo 覆盖并验收的能力增加；0.9 保持冻结
                    ↓
Ravo 全产品达到发行切换门槛
                    ↓
旧应用的 src、构建项、资源和测试整体退役
```

Ravo 不得包含 `src/` 私有头、链接 `libdarktable`、加载旧 IOP，或把旧全局状态包成新 API。0.9 在
迁移期间保持冻结，也不得增加调用 Ravo 的 adapter；两套生产构建完全独立，直到 Ravo 达到发行切换
门槛后整体删除旧应用。完整规则见 [MIGRATION.md](MIGRATION.md)。

## 计划目录

已建立的无头目录及其所有权如下；后续目录仍不能在无头阶段验收前创建：

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
- [MIGRATION.md](MIGRATION.md)：Ravo 独立实现、验收并在最终切换后整体退役 `src` 的规则；
- [TESTING.md](TESTING.md)：旧回归复用、新测试层次与验收门槛；
- [ADR 索引](docs/adr/README.md)：已确认且需要长期保留理由的决策；
- [根级重写计划](../TODO_REWRITE.md)：阶段、出口与完整产品路线。

仓库整体继续采用 GPLv3，详见根目录 [LICENSE](../LICENSE)。
