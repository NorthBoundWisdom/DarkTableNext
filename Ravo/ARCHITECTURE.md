# Ravo Architecture

## 核心结论

Ravo 的第一产品是一个可嵌入的 C++20 图像引擎和一个可独立使用的 `ravo` CLI。CLI 是 engine facade
的首个客户端；后续 Ravo Studio 也是该 facade 的客户端。两者不能拥有两套 recipe、调度或算法实现。

```text
旧回归 wrapper ─┐
                 ├──▶ ravo CLI ─────────────┐
脚本/用户 ───────┘                          │
                                            ▼
                                    Ravo Engine Facade
                                    │ inspect
                                    │ operation registry
                                    │ recipe validate/upgrade
                                    │ render / cancel / progress
                     ┌──────────────┴──────────────┐
                     ▼                             ▼
             Recipe / Operation Model         CPU Image Engine
                     │                             │
                     └──────────────┬──────────────┘
                                    ▼
                        Foundation + Abstract Ports
                                    ▲
                                    │ implements
                        Codec / Filesystem Adapters

无头阶段验收后：

Ravo Studio ──▶ Application Services ──▶ 同一个 Ravo Engine Facade
                       │
                       └──▶ Catalog / History Repository Ports
```

## 计划 target

| Target | 所有权 | 允许依赖 | 禁止依赖 |
| --- | --- | --- | --- |
| `ravo_foundation` | errors、IDs、任务、取消、基础资源契约 | 标准库 | recipe、engine、CLI、数据库、UI |
| `ravo_recipe` | recipe、operation schema、mask/blend、版本升级 | foundation | codec、像素执行、数据库、UI |
| `ravo_engine` | inspect、registry、render facade、CPU pixelpipe | foundation、recipe、抽象端口 | CLI、catalog、UI、旧 `src` |
| `ravo_adapters` | codec、文件系统、平台实现 | 对应端口、固定第三方依赖；`Qt6::Core` 仅限私有文件系统实现 | 业务/UI 状态、Qt GUI/QML/Widgets 类型 |
| `ravo_cli` | 参数、流、日志、JSON、退出码、composition root | engine facade、adapters | 算法源码、engine 私有头、旧核心 |
| `ravo_domain`（后续） | catalog、版本、history、styles | foundation、recipe、仓库端口 | UI、数据库实现 |
| `ravo_services`（后续） | 桌面用例与任务编排 | domain、engine | widget、SQL、算法内部 |
| `ravo_desktop`（后续） | 窗口、输入、可访问性和呈现 | services、预览资源 API | SQL、算法、engine 私有头 |

第三方依赖不是架构层。LibRaw、lcms、Exiv2、SQLite、`Qt6::Core` 或平台 API 必须由私有 adapter
包装，裸句柄、宏、异常和头文件不能泄漏到公开契约。

## 核心数据契约

### Recipe

canonical recipe 至少包含：

- recipe schema 版本；
- 输入资产身份与适用约束；
- 稳定 operation ID 和每项参数 schema 版本；
- operation 启用状态、顺序和实例身份；
- mask/blend 图及稳定引用；
- 输出相关但不属于 UI 的处理意图。

不得包含 C++ 对象布局、指针、GTK/Bauhaus 状态、旧 module struct、GPU 句柄、数据库行地址或缓存键。

### Operation descriptor

每个 operation 声明：

- 稳定 ID、显示元数据和参数 schema；
- 输入/输出像素格式和色彩空间；
- ROI、几何、分块、mask/blend 能力；
- CPU 参考执行器及其资源预算；
- 参数/recipe 版本升级；
- 不支持输入的结构化失败；
- 允许的数值容差和必需 fixture。

首版 operation 内建注册。除非未来有明确消费者、威胁模型和版本策略，不设计公共插件 ABI。

### Render request/result

`RenderRequest` 显式携带输入、recipe、输出尺寸/质量、内存预算、线程预算、确定性选项、backend 偏好、
取消令牌和关联 ID。`RenderResult` 返回可信输出或结构化失败，不能返回“部分成功”的未验证 buffer。

## 所有权与线程

- composition root 创建 engine、adapter 和执行器，并在任务全部终止后按反向顺序销毁。
- recipe、descriptor 与跨线程领域快照不可变；修改产生新版本，而不是共享写入。
- pixel buffer 有唯一写 owner；只读共享必须绑定明确有效期。
- codec、operation、cache 和任务不能保存 CLI 或未来 widget 指针。
- 完成事件携带 request/version ID；取消或更新后的过期结果直接丢弃。
- 资源不足、codec/operation 错误或未来 GPU 失败时，只能从可信输入安全重试或失败，不能继续使用
  部分写入输出。

## CLI 边界

CLI 负责：

- 参数与文件/流输入输出；
- composition root；
- 人类日志、版本化 JSON 与稳定退出码；
- 进度、取消和原子输出协调。

CLI 不负责：

- 解析或执行算法内部结构；
- 私自修改 recipe；
- 维护 catalog 或 UI 会话；
- 暴露旧配置键、旧 IOP ABI 或 engine 私有对象。

未来 UI 直接调用 facade/services，不通过 shell 执行 `ravo`。CLI 仍是独立受支持的批处理工具。

## 阶段性非目标

- 阶段 0–3 不实现桌面 UI、widget、catalog 或旧界面复刻。`Qt6::Core` 仅作为 adapter 私有的路径与
  文件 I/O 实现；Qt GUI、QML、Widgets 和任何 desktop target 仍被禁止。
- CPU 正确性完成前不实现 GPU backend。
- 不追求逐行翻译 0.9，也不以源码行数作为迁移完成度。
- 不恢复已从产品范围删除的插件、格式、在线服务或历史 ABI。
- 不为尚无消费者的网络服务、复杂 filter 语言或公共 SDK 提前冻结 ABI。
