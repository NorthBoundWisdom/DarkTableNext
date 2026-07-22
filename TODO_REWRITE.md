# Ravo：C++20 无头内核、CLI 与后续 UI 重写计划

> 状态（截至 2026-07-22）：Phase 0 静态 fixture 盘点与 Phase 1 C++20/CLI 骨架已建立，Phase 2 首个
> RAW/PNG 纵切片已可运行。当前生产解码仍由 Ravo 直接构建固定 LibRaw 源码；自有 rawspeed fork 已清理
> 嵌入式 CMake、发布并固定到依赖锁，但尚未接入 `ravo_engine`，不能把依赖准备写成解码迁移完成。
> 当前主线不是继续扩充 harness，而是尽快阅读冻结的 C 源码、在 Ravo 中重写完整 CPU 行为并交付可用 CLI。QtCore 是允许的新工程
> 基础依赖；桌面 UI 延后，且只能复用同一内核/API。它不是 GTK 3 → GTK 4 的迁移。
> 0.9 从现在起冻结，不再配置、编译、修改或运行；它只作为静态源码、算法说明与 fixture 基线。
> 原计划在 0.9 上完成的剩余工作全部转入 Ravo，最终由 Ravo 验收后整体退役 `src/`。

## 决策与范围

现有应用把 GTK、dtgtk/Bauhaus、动态 IOP 模块、像素管线、数据库、任务队列和 OpenCL 类型交织在
同一编译图中。例如 `develop/imageop.h` 同时包含 GTK、dtgtk、OpenCL 与像素管线头文件，
`src/CMakeLists.txt` 又把 GUI、控制层、处理层和动态模块一起构建。这使更换 UI 框架不只是控件替换，
而是一次跨越生命周期、线程和数据边界的重写。

因此，本计划采用**并行、headless-first 的完整 C++20 重写**：直接阅读相对清晰的旧 C 实现，以可运行
CLI 垂直切片带动 RAW 解码、pixelpipe、operation、色彩、metadata 和导出迁移，再逐步覆盖冻结 0.9
仍保留的全部产品能力。架构与测试只保护新旧隔离、可编译性和已迁行为，不得为了先造通用 harness、
全量分类或治理材料而阻塞功能代码。catalog、应用服务和桌面 UI 在 CLI/engine 基本可用后开始；不得
把旧 GTK 代码或旧 IOP ABI 包在兼容层后继续当作新实现。

0.9 与 Ravo 共同遵守的最低产品范围是：本地目录/导入、图库、暗房、必要的非破坏性开发管线、
蒙版、历史、色彩管理，以及 JPEG、PNG、TIFF 和原文件复制到本地磁盘。0.9 输入保留 RAW、JPEG、
PNG、TIFF、RGBE/HDR、QOI 与原文件复制；GPS 只保留已有坐标的读取、保存和展示，不保留地图、
位置搜索、GPX 或主动地理标记。本计划不重新引入已删除的 Lua、脚本、历史插件/UI ABI、打印、
幻灯片、联机拍摄、远程发布或 AI/ONNX 工作流。

第一阶段 CLI 不需要提前实现图库或交互 UI，但必须完整表达后续桌面产品会消费的解码、recipe、
操作图、蒙版/混合、色彩管理、预览/全尺寸渲染和本地导出能力。下文的“0.9 冻结基线与 Ravo
承接项”记录旧计划如何转入新架构；它不授权修改 0.9，也不授权 Ravo 依赖旧 OpenCL、动态 IOP ABI
或 `src`。

## 已确认的技术决策

- [x] 采用并行、clean-slate 的完整重写；旧 `src/` 只作为行为参考、算法来源和差分测试对象。
- [x] 下一代项目与软件统一命名为 **Ravo**：命令行程序为 `ravo`，内核称 Ravo Engine，未来桌面
      产品暂称 Ravo Studio；源码与文档所有权进入仓库根目录 `Ravo/`。
- [x] 整个新实现统一使用 **C++20**。无头阶段只创建 foundation、recipe、engine、adapters、CLI；
      后续 domain、services 与 desktop 也不引入 Rust/Cargo 构建图，避免同时承担 FFI、双工具链和
      两套所有权模型。
- [x] **先交付无头内核与 CLI，暂不写前端。** CLI 是新 engine 的第一个正式客户端、自动化接口和
      日常手工/自动验证入口，不是日后删除的测试脚本；桌面 UI 在 CLI/engine 基本覆盖冻结工作流后立项。
- [x] 继续使用 CMake 与 FreeCM 管理固定依赖和跨平台预设；新旧实现拥有独立 target、构建目录、
      安装规则、运行时目录和测试标签。
- [x] FreeCM 项目命令只配置、编译、运行、测试和安装 Ravo；不得再暴露旧应用 target、旧 UT 或旧包。
- [x] **允许 Ravo 依赖 QtCore。** QtCore 可用于路径、文件、JSON、Unicode、时间和通用运行时设施，
      不必人为限制在单个 filesystem adapter；但 Qt GUI/QML/Widgets 仍不进入无头阶段，公开持久化 schema
      也不得序列化 Qt 对象内存布局。
- [x] CPU 是参考实现和可靠回退。GPU 后端在 CPU 垂直切片通过金样后再设计，GPU 类型不得进入
      recipe、领域模型、UI 或公开操作契约。
- [x] 不迁移旧 GTK、dtgtk/Bauhaus、动态 IOP ABI、全局 `darktable` 状态或 OpenCL API；需要保留的
      数值算法只能在去除 UI、全局状态和旧生命周期后进入新 engine。
- [x] UI 框架暂不选择，但桌面实现同样以 C++20 为语言基线。阶段 4 必须用原型验证框架后才能写
      正式 UI；不得因为旧代码使用 GTK 就默认继续使用 GTK。

以下产品与兼容性决定并行维护，不再作为开始重写 CPU/CLI 的前置条件：

- [ ] 指定第一版产品 owner 和 Ravo code review owner；记录 0.9 冻结点，后续不再向旧实现合入变更。
      决策登记见 [`Ravo/docs/phase0/product-decision-register.md`](Ravo/docs/phase0/product-decision-register.md)。
      源码/fixture 冻结点已识别为 `320970bf7c9cbbc6611cfc3eb60f8f2b0424b782`；旧工程及其 CLI、CTest、
      图像 runner 均禁止配置、编译或执行，`darktable-tests` 只作为已提交静态 fixture。owner 记录仍是
      产品治理项，因此本项保持未完成。
      每次 Ravo 开工前可用 `python Ravo/tools/check_freeze_reference.py` 验证未触碰冻结 source/fixture、
      root CMake graph、`cmake/`、`data/` 与 `packaging/`。
- [x] 第一版默认重写冻结 0.9 当前仍保留的完整产品能力；已明确删除的 Lua、远程发布、打印、地图等
      不恢复。具体 operation 遇到明确产品删减时再记录例外，不先等待 76 项逐项签字。
- [ ] 补充正式离线/隐私要求和发布门槛。Ravo 开发目标同时覆盖 Windows、macOS 与 Linux；当前设备只
      验证 Windows/MSVC，不能把本机验证范围误写成产品平台限制。
- [ ] 确定新应用与旧 catalog、XMP、styles、presets 和编辑历史的兼容策略；不承诺的项目必须
      写成显式不兼容项。待决项和完成证据要求见
      [`Ravo/docs/phase0/product-decision-register.md`](Ravo/docs/phase0/product-decision-register.md)。
- [ ] 为并行工程分配 CI 目标、测试语料、签名/包标识和用户数据目录，避免新旧应用相互污染。
      本轮不搭建 CI/CD；当前只维护本机编译与本地测试命令，CI 配置留待单独工作项。

旧 GTK 4 迁移计划与范围清单已经退役，当前 `src/` 只作为旧应用源码地图和静态算法参考；不再启动
0.9 修复、删减或 GTK 迁移。新 UI 的工作集由冻结后的产品用例和应用服务契约重新产生。

## 目标架构

### 分层与依赖方向

```text
第一交付阶段

脚本/用户 ──▶ ravo CLI ──▶ engine facade ──▶ recipe + CPU 图像引擎
                                           │
                                           ▼
                              QtCore + codec/filesystem adapters

无头内核验收后

桌面 UI ──▶ application services ──▶ 同一个 engine facade
                │
                └──▶ catalog/history 仓库端口及其 adapters
```

`ravo` 只做参数解析、输入输出和进度/错误呈现；算法状态与执行全部属于 engine。后续 UI 只能向内
依赖 services/engine，不能启动 CLI 子进程、解析终端文本或复制算法。具体 adapter 反向依赖由
services/engine 拥有的抽象端口，并只在 composition root 中装配。domain 和 engine 不包含 SQLite、
平台或 UI 头。特别是：

| 所有者 | 可以知道 | 禁止知道 |
| --- | --- | --- |
| 图像引擎 | recipe、操作描述、像素/色彩/ROI 契约、任务取消令牌 | widget、窗口、事件循环、GTK/任何 UI 框架类型、catalog SQL 细节 |
| CLI | engine facade、命令参数、文件/流、结构化输出和退出状态 | 算法内部、可变模块状态、UI、catalog SQL、旧 IOP ABI |
| 领域与应用服务 | 稳定的引擎接口和持久化仓库接口 | 像素缓冲内部布局、UI 控件对象、全局 UI 状态 |
| UI | 用例命令、查询快照、预览资源句柄和进度事件 | 数据库连接、像素管线内部、算法模块的可变参数内存 |
| 平台适配 | 所实现的抽象契约 | 调用方的业务规则或 UI 生命周期 |

新核心的头文件和链接依赖不得传递任何 UI 框架。检查项包括：核心目录中没有 UI include，核心目标
不链接 UI 运行时，且无模块可以在初始化、处理或销毁阶段创建控件。

### 稳定契约

- **编辑 recipe**：每张编辑版本由操作图、操作稳定 ID、参数 schema 版本、启用状态、顺序、蒙版
  引用和输入资产标识组成。不得把 C/C++ struct 的内存布局、指针或 UI 控件状态直接序列化。
- **操作描述**：每个保留的处理操作声明参数 schema、输入/输出色彩与像素格式、ROI/分块能力、
  mask/blend 能力、CPU 参考实现和版本迁移函数。第一版只包含产品范围冻结后需要的内建操作；不恢复
  旧动态插件 ABI。
- **CLI**：命令、退出码、stdout/stderr 分工、JSON schema、确定性执行选项和版本信息形成受测试的
  自动化契约。人类日志进入 stderr；机器输出使用版本化 JSON；失败或取消不得留下看似成功的输出。
- **应用服务**：导入、查询/选择、创建编辑版本、变更 recipe、生成预览、提交导出、取消任务和
  撤销/重做均以命令、查询及结果对象表达。UI 不能直接修改共享 catalog、history 或引擎状态。
- **预览资源**：UI 只持有带尺寸、色彩空间、有效期和只读语义的资源句柄；引擎可替换 CPU/GPU
  存储而不改变 UI 或领域模型。
- **持久化**：catalog schema、XMP/sidecar 映射和导出任务记录有独立版本与迁移测试。数据库只是
  仓库实现，不得成为 UI 或算法 API。

### CLI 第一版能力

命令名在 ADR 中最终冻结，至少提供以下等价能力：

```text
ravo inspect <input> --json
ravo operations --json
ravo recipe import-xmp <legacy.xmp> --asset-id <id> --input <input-uri> --output <recipe>
ravo recipe validate <recipe>
ravo render <input> --recipe <recipe> --output <image> --backend cpu
ravo --version --json
```

- `inspect` 报告输入格式、尺寸、RAW 元数据、色彩信息和可解码性，不创建 catalog。
- `operations` 输出稳定 operation ID、参数 schema 版本、能力和限制，供测试与未来 UI 生成参数模型。
- `recipe import-xmp` 是显式 legacy adapter；它只负责已批准的旧数据读取，不让 XMP struct 或旧模块
  版本进入 engine 内部。
- `render` 支持固定线程数、内存预算、输出尺寸、插值器和 CPU-only 确定性模式；同一环境重复运行必须
  产生符合既定容差的结果。
- CLI 支持文件输入输出，并为未来 stdin/stdout 流式处理保留契约空间；首版不得为模仿 FFmpeg 而
  提前实现没有真实消费者的复杂 filter 语法。
- `SIGINT`/平台等价取消、损坏输入、无效 recipe、未知 operation、内存不足和磁盘写入失败都有稳定
  非零退出码、结构化错误和原子输出语义。
- CLI 持续作为受支持的批处理工具发布。未来 GUI 直接调用 engine/services API，而不是 shell out 到
  CLI；两者用契约测试保证同一 recipe 产生相同结果。

### 所有权、生命周期与线程

- 服务容器在应用启动时创建，在所有后台任务停止、预览资源释放和持久化刷写后销毁；没有等价于
  现有全局 `darktable` 可写状态的逃生通道。
- 所有领域快照、recipe 和操作描述在跨线程传递时不可变。可变缓存属于单个任务或明确所有者，不能
  由 UI 线程和 worker 线程共同写入。
- UI 框架线程只处理输入、渲染和快照更新。导入、解码、评估、写数据库、缩略图和导出在任务执行器
  中完成；完成事件必须携带任务/版本 ID，过期结果一律丢弃。
- 取消、错误、窗口销毁和应用退出是每个异步用例的必测路径。GPU 失败或资源耗尽必须丢弃不可信
  输出，并从有效 CPU 输入重新执行或报告可恢复错误。

### C++20 工程约束

- 公开接口优先使用值语义、不可变快照、`std::span`、`std::string_view`、`std::unique_ptr` 和明确的
  资源句柄；拥有资源的裸指针、隐式共享可变对象和未注明生命周期的 view 不得跨模块边界。
- RAII 管理文件、数据库事务、像素缓冲、线程、任务、GPU 资源和第三方库句柄。析构顺序必须由
  owner 关系决定，不能依赖进程退出或 UI teardown 顺便释放。
- 错误以可检查的结果类型和结构化错误传播；异常不得穿过模块 ABI、任务边界、第三方 C 回调或未来
  的 FFI 边界。失败路径与成功路径具有相同的资源释放和测试要求。
- 并发使用显式执行器、任务句柄和取消令牌；禁止 detached thread、跨线程 UI 对象、无 owner 的
  callback 以及靠全局布尔量控制退出。
- 新 target 默认启用严格警告。Debug/CI 分层运行 AddressSanitizer、UndefinedBehaviorSanitizer、
  ThreadSanitizer（平台可用时）和静态分析；警告或 sanitizer 排除项必须带理由和最小作用域。
- QtCore 是新工程允许的基础依赖，可在有直接收益的 Ravo target 中使用；其他第三方 C/C++ 库和平台
  专用 Objective-C++/系统 API 仍优先封装在 adapters 或 desktop 私有实现，其裸句柄、宏和异常不得
  泄漏到稳定公开 API。
- 旧 C 代码可以直接作为算法与控制流参考。先做清晰、等价、可编译的 C++20 重写，再在真实重复出现
  时抽象；不要求为每段移植先设计通用 port、mock 或 harness。历史 shim、动态插件 ABI、GTK 状态和
  已删除功能不迁移。

## 代码与构建隔离

开工后在已经建立的 `Ravo/` 文档边界内增加 C++20 源码，而不是在 `src/` 内穿插新旧实现：

```text
Ravo/
  foundation/       平台无关的基础类型、错误、任务和测试工具
  recipe/           operation schema、recipe、mask/blend 与版本迁移
  engine/           无 UI 的 RAW/像素/色彩/导出核心
  adapters/         文件系统、编解码器和以后加入的 GPU/平台实现
  cli/              正式的 inspect、recipe、render、诊断与批处理入口
  tests/            单元、契约、集成和金样测试

  # 无头阶段验收后才创建
  domain/           catalog、照片版本、history、styles 与仓库端口
  services/         面向桌面 UI 的应用用例
  desktop/          选定 UI 框架的窗口、视图、输入和可访问性
```

- [x] 新工程使用独立 CMake target、install 规则和 app/CLI 名称；默认构建不混入旧 `libdarktable`
      或其动态模块，也不调用 Cargo/Rust 构建步骤。
- [ ] 旧代码只作为静态行为与算法参考。按处理链直接阅读对应 C 实现并重写为 C++20；为迁入行为补
      最小聚焦 UT，但不编译、链接或运行旧 UT，也不得让新核心包含 `src/` 内部头文件。
- [x] 每个新目录有明确 owner、公开头文件边界和禁止依赖清单；跨层 include 在 code review 中失败。
- [x] 新旧依赖都继续遵守 FreeCM 源根工作流，不使用网络下载、FetchContent 或未固定的源码复制。
      永久基线只改 `source_roots.lock.jsonc.in`、`configs/source_roots.py` 和实际消费者；本地联调只改被忽略的
      活动锁并运行离线 `--update`。完整规则见
      [`DevDocs/Dependency_Workflow.md`](DevDocs/Dependency_Workflow.md)。
- [x] 自有 rawspeed fork 的嵌入式构建已清理并发布为
      `dfbd5090cd28d5d201249cb493c0d26932275107`：作为 subproject 时默认只构建库，关闭测试、工具、benchmark、
      fuzzer、文档、安装和 developer policy，同时保留 XML/JPEG/ZLIB 解码能力；Windows/MSVC 已分别验证
      standalone Release 与 subproject Debug。父仓库模板和当前活动锁均以 pinned 模式解析到该提交，
      `python configs/source_roots.py verify` 通过。该项只表示依赖可干净嵌入；当前 `Ravo/CMakeLists.txt` 仍
      只消费 `LIBRAW_SOURCE_ROOT`，rawspeed 尚不是 Ravo 生产 RAW decoder。
- [x] 阶段 3 出口前，构建图中不存在 `desktop` target、Qt GUI/QML/Widgets 或其他 UI toolkit 依赖；
      `Qt6::Core` 可由 Ravo 新代码直接使用。CLI target 自身不包含算法源文件，只链接 engine facade
      和必要的新工程 target。

## 旧测试的静态基线与复用方式

截至 2026-07-21，仓库内测试不能简单概括为“已有 UT 足以兜底”：

| 测试资产 | 当前数量与内容 | 重写中的用途 |
| --- | --- | --- |
| CTest `unit` target | 历史清单有 3 个：变量展开、数据库 schema，以及 1 个不执行断言的空 sample | 只作为待重新表达的语义清单；旧 target 不再配置、编译或执行 |
| `darktable-tests` | 158 组已提交 XMP + `expected.png` 静态图像 fixture，覆盖 68 个 operation 名；当前冻结 CMake 清单有 76 个非注释 IOP 注册项，部分受历史构建条件控制 | 作为 Ravo CPU 输出的只读参考输入，并补齐 Ravo 自有参数边界、失败路径、浮点和 metadata 证据 |
| UI 自动测试 | 当前保留测试明确排除了 UI 测试 | 依据新应用服务和 UI 交互重新建立，不能从“图像输出一致”推导 UI 正确 |
| GPU 基线工具 | CPU/OpenCL 像素差、连续 GPU 段和性能采样 | 新 engine 的 CPU 正确性完成后再适配；不作为首个垂直切片前提 |

当前快照由 Python 清单与一致性检查保护；Windows 上只使用 Python/PowerShell 辅助工具：

```powershell
python Ravo/tools/freeze_legacy_manifest.py --check
python Ravo/tools/check_capability_inventory.py
python Ravo/tools/check_freeze_reference.py
```

复用规则：

- [x] 以 `legacy_manifest.json` 固定全部 158 组 fixture、5 个原图及其哈希；每次 Ravo 开工只运行
      Python 静态检查，不配置、编译或执行旧 CTest、旧 CLI 或 `darktable-tests/run`。
- [x] 保存已提交原图、XMP 和 `expected.png` 作为冻结参考输入；新实现开发期间不得修改这些资产，
      也不得以 Ravo 输出覆盖旧金样。
- [x] 为新 CLI 提供仅用于迁移读取的 legacy adapter：读取已批准的旧 XMP 并映射为版本化 recipe。
      旧 CLI 参数、配置键和模块 ABI 不因此成为新产品兼容承诺。
- [ ] 每个保留 fixture 只运行 Ravo CPU，并与已提交 PNG、Ravo 自有 32-bit float/metadata 证据比较
      像素、NaN/Inf、尺寸/ROI、alpha 和色彩契约；不要求或允许即时复现旧 CPU 运行。
- [ ] 单测参考冻结 C 源码的分支、参数和边界直接在 Ravo 中重建；不得运行或移植旧 UT harness。
      每个迁移批次至少覆盖核心数学、解析失败和一条真实 RAW/fixture 路径，更多合成边界在实现稳定后
      按风险补充，不让测试脚手架先于功能无限扩张。
- [ ] 变量展开和数据库 schema 测试只迁移仍属于新产品的语义。空 sample 删除，不为保持测试数量
      而复制。旧测试编译通过但仍调用旧库不算重写进度。
- [ ] UI 以服务契约测试为第一层，再增加键盘、指针、焦点、取消、快速切图、窗口销毁和可访问性测试；
      不要求新 UI 复制旧 widget 树或像素布局。

测试数量会随产品删减和新增覆盖变化。每次更新此快照都必须同时记录统计日期、统计命令和 operation
兼容性结论，不能只追求“158 个全绿”而保留已明确删除的功能。

## 分阶段待办与出口

### 阶段 0：静态冻结与最小开工基线

- [x] 先冻结无头用例：inspect、列出 operation/schema、导入旧 XMP、验证 recipe、CPU 预览/全尺寸
      render、本地导出、批处理、取消、错误恢复和离线运行；每个命令给出机器可验证的完成条件。
- [x] 单独记录后续桌面用例：catalog、Grid/Loupe、单图编辑、history、masks、styles 和批量导出；
      它们用于校验 engine 契约是否足够，但不进入前三个阶段的实现范围。
- [ ] 对现有 IOP、蒙版、metadata、styles、presets 和导出能力做“保留 / 延后 / 不支持”盘点，
      并为每一项旧数据写迁移、只读或显式拒绝策略。
      `Ravo/docs/phase0/capability-inventory.md` 已逐项列出 76 个冻结 registry IOP；
      `python Ravo/tools/check_capability_inventory.py` 约束其 fixture 标记与当前 manifest 一致。
      未决定项默认进入“按冻结实现迁移”队列；owner 可以并行决定删减或延后，但不阻塞已理解模块的
      C++ 重写。
- [x] 静态盘点现有 3 个历史 CTest target 与 158 组 `darktable-tests`，冻结现有 RAW、XMP、PNG 和
      阈值资产；旧 target、旧 CLI 与图像 runner 不再配置、编译或执行。
      [`fixture classification ledger`](Ravo/tests/fixtures/fixture_classification_ledger.json) 已以 manifest
      的 158 个 fixture 建立完整条目；全部资产均存在且受哈希保护。条目保持 `unclassified` 只表示
      产品 disposition 与 Ravo CPU 验收尚未完成，不表示缺少 fixture 或需要外部输入。
- [ ] 记录当前 CLI 解码、首张预览、全尺寸导出、峰值内存和失败恢复的基线；性能目标以后续
      端到端测量为准，不以单个 kernel 或控件替换计数为准。
- [x] 形成架构决策记录（ADR）：记录已选 C++20 和不采用首版 Rust 的原因，并继续决定公开 API/ABI、
      CLI/JSON/recipe 版本策略、线程模型、色彩管理、GPU 与许可/打包策略。UI 框架只记录评价标准，
      不在本阶段选型。
      证据：[`ADR-0001`](Ravo/docs/adr/0001-cpp20-headless-first.md)、
      [`ADR-0003`](Ravo/docs/adr/0003-versioned-machine-contract.md)、
      [`ADR-0004`](Ravo/docs/adr/0004-freeze-09-ravo-only-growth.md)、
      [`ADR-0005`](Ravo/docs/adr/0005-qtcore-filesystem-adapter.md)、
      [`ADR-0006`](Ravo/docs/adr/0006-explicit-colour-contract.md) 与 `Ravo/ARCHITECTURE.md` 的所有权/线程边界。

出口：冻结边界、完整 fixture 清单、首个垂直切片输入和关键 ADR 可读；旧工程永不运行。未完成的
产品治理、性能记录与全量分类并行推进，不阻塞 Ravo CPU/CLI 实现。

### 阶段 1：无头工程骨架与 CLI 契约

- [x] 创建 C++20 `foundation`、`recipe`、`engine`、必要 `adapters`、`cli` 和 `tests` target；构建图
      不得出现 desktop、GTK、旧 `libdarktable`、Rust runtime 或 catalog 数据库。
- [x] 实现版本化 `AssetDescriptor`、`Recipe`、`OperationDescriptor`、`ParameterValue`、`Mask`、
      `RenderRequest`、`RenderResult` 和 `TaskError`；为序列化、升级和损坏输入写新 C++ 单元测试。
- [x] 定义 engine facade：inspect、operation registry、recipe validate/upgrade、render、取消和进度；
      CLI 与未来 services 必须调用该 facade，不能各自拼装 pixelpipe。
- [x] 建立 CLI 参数、退出码、JSON envelope、日志、`--version` 和确定性执行契约；加入 snapshot/
      contract test，保证 stdout 不被人类日志污染。
- [x] 实现 legacy XMP adapter 的解析和诊断骨架，只产出 canonical recipe 或结构化不兼容错误；adapter
      不加载旧动态模块，也不链接旧核心。
- [x] 规定 facade、任务、buffer、codec 与取消令牌的所有权和关闭顺序，并测试无效输入、重复取消、
      超时和输出路径冲突。

出口：`ravo --version --json`、`operations --json`、`recipe validate` 和错误路径可重复运行；新测试
只链接新 target，依赖图没有 UI、catalog、旧核心或 Rust。

验证记录（2026-07-22，Windows x64，MSVC 19.51.36248.0，CMake 4.4.0）：通过
`Ravo/tools/freecm_project.ps1` 实际完成 Ravo Debug Configure、Build、Run 与 Test；新 CLI 输出有效
`ravo-cli/v1` 版本 JSON，39/39 新工程测试通过（`ravo-unit` 9、`ravo-contract` 30）。Ravo Release
Configure、Build 与 staged Install 同样通过，安装内容为 Ravo CLI、库、公开头、LibRaw 静态 archive
以及 Qt6Core、libpng、zlib runtime；从 staging 目录直接执行 `ravo --version --json` 成功。整个过程
没有配置、编译或执行旧 target、旧 UT、旧 CLI、旧打包图或图像 runner，也没有搭建 CI/CD。

### 阶段 2：尽快跑通 CPU/CLI 垂直切片

- [x] 沿 `mire1.cr2` 的真实调用链阅读冻结 C 源码，直接重写最小 RAW 解码、基础色彩链、pixel buffer、
      operation 求值与 PNG 输出；先得到 CLI 可观察结果，再提炼重复接口。首版通过 FreeCM 固定的
      LibRaw 解出 16-bit Bayer CFA，执行裁剪、black/white 归一化、camera white balance、LibRaw
      camera-to-sRGB 矩阵、3×3 Bayer 插值、exposure 与 sRGB 编码，并由 libpng/QSaveFile 原子写 PNG；它是可工作的起点，不代表已达到
      冻结 `expected.png` 的完整 pixelpipe/色彩一致性。
- [ ] 实现输入资产、预览/全尺寸输出、ROI/分块和安全内存预算的必要契约；只创建当前垂直切片马上
      使用的抽象，不为尚未迁移的模块预建通用 harness。
      `RenderRequest` 已有输出宽高、内存预算、worker 数、确定性、CPU backend、取消 token 和关联 ID；
      当前实现能做有界全图缩放并在估算工作集超预算时拒绝。预览/全尺寸语义、真实 ROI/tiling、worker
      调度、缓存以及资源/性能验收仍未完成，因此本项保持未勾选。
- [x] 先实现 nop、基础 RAW/色彩链和 exposure 所需内建操作，以及必要参数验证和 CPU 执行器。冻结
      `nop.xmp` 的基础 rawprepare/temperature/highlights/demosaic/colorin/colorout/gamma/flip 只有在版本、
      参数和默认 blend 完全匹配时才由内建 RAW 管线吸收；manual exposure v5 映射为版本化 operation，
      CPU 按 `exp2(exposure_ev)` 求值，其他历史继续结构化拒绝。
- [ ] 为首批需要的蒙版与混合实现 canonical 表示和 CPU 行为；不得把旧 GUI 数据或 module struct
      当作 recipe。
- [ ] 实现可取消的预览渲染和 JPEG/PNG/TIFF/原文件复制导出；每种输出都有色彩、元数据和原子写入
      的验收测试。
      当前同步 CPU render 会在开始和逐输出行检查取消 token，PNG 使用 `QSaveFile` 原子提交并拒绝覆盖
      已有输出；CLI 的 SIGINT/平台等价中断、JPEG/TIFF/原文件复制、完整 metadata/ICC、磁盘满和部分写入
      故障测试尚未完成。
- [ ] 通过 CLI 完成首条真实闭环：inspect RAW → import legacy XMP → validate recipe → CPU render →
      PNG/浮点输出，并对照阶段 0 金样；至少选择 nop、基础 RAW/色彩链和一个可见参数 operation。
      其中 nop 子路径已在 Windows/MSVC 实测：`mire1.cr2` inspect 为 Canon EOS 40D 3908×2602，
      `nop.xmp` 导入 canonical recipe 后输出 64×48 PNG；剩余门槛是 exposure fixture 的完整历史映射、
      `expected.png`/浮点金样比较和色彩容差，而不是再次搭建运行框架。
- [x] 用普通 GoogleTest/CTest 和少量 Python/PowerShell 检查运行首批 Ravo fixture；不另造旧 runner
      兼容层，不修改 `expected.png`，也不调用任何旧可执行文件。

出口：真实 RAW 闭环无需桌面 UI、catalog 或旧核心即可重复运行；CPU 结果、recipe 重放、取消、原子
输出和错误恢复均通过自动测试。

### 阶段 3：批量重写无头内核并完成 CLI 首版

- [ ] 按冻结 pixelpipe 顺序和共享设施分批迁入首版 operation。以旧 C 实现为主要阅读材料，每批先交付
      可编译 CPU 行为、legacy 参数映射和聚焦 UT，再补真实 fixture 与数值容差；未支持能力由 CLI
      明确拒绝。
- [ ] 完成共享的蒙版、混合、色彩转换、ROI/分块、插值、缓存和内存预算设施，避免每个 operation
      私自实现生命周期或调度。
- [ ] 完成 RAW/JPEG/PNG/TIFF 输入范围和 JPEG/PNG/TIFF/原文件复制输出；验证 ICC、EXIF/metadata、
      alpha、方向、尺寸、原子替换和磁盘满路径。
- [ ] 随 operation 批次逐步点亮 158 个冻结 fixture；分类账只记录进度与明确不兼容，不作为开始迁移
      下一模块的前置审批。首版承诺保留的 fixture 最终必须在 Ravo CPU engine 通过，不能静默 skip。
- [ ] 完成 CLI 用户与机器文档、shell completion（若采用）、版本化 JSON schema、稳定退出码、批处理、
      中断取消、资源限制、确定性模式和故障诊断。
- [ ] 在 Windows/MSVC、macOS/Clang 与 Linux/Clang 开发机上持续 configure/build/test CLI；每次只声明
      实际运行的平台。记录全尺寸 render 延迟、峰值内存、吞吐和长批处理稳定性。
- [ ] 发布一个仅含无头 engine/CLI 的内部候选版本，供脚本、回归和真实照片批处理试用；CLI 不是桌面
      发布的附属品，后续继续独立测试和发布。

出口：冻结的无头范围、保留 fixture、正确性、错误恢复、资源门槛和可用平台全部达到验收；CLI 可在
没有旧应用和 UI runtime 的安装环境中独立使用。**该出口完成前不得创建 desktop target。**

### 阶段 4：应用服务、catalog 与桌面壳

- [ ] 在已验收 engine facade 之上实现 catalog、照片版本、history、styles、导入/选择/导出任务和
      持久化仓库；SQLite 等实现只存在于 adapter。
- [ ] 用同一个 facade/services 制作候选 UI 框架的最小原型，至少验证 macOS、Windows、Linux 的
      构建、窗口生命周期、键盘/指针/手势、HiDPI、色彩显示、可访问性和打包可行性。
- [ ] 按已记录标准选择一个 UI 框架：跨平台成熟度、图像呈现能力、输入与可访问性、许可、原生
      打包、长期维护成本，以及不污染核心的能力。选择结果写入 ADR。
- [ ] 实现应用壳、路由、命令分发、通知/错误呈现、设置存储和任务进度订阅；UI 状态可随时由服务
      快照重建。
- [ ] 实现预览资源生命周期：快速切图、任务取消、窗口销毁时不显示过期帧、不泄漏资源、不卡住 UI
      线程；同一 recipe 必须与 CLI render 结果一致。

出口：三平台至少完成 configure/build；代表平台完成可访问窗口、目录选择、照片列表和异步缩略图
浏览手工验收。UI 不含算法或 SQL，engine/CLI 测试无需 UI 仍可独立运行。

### 阶段 5：完整桌面产品工作流

- [ ] 完成 Lighttable：导入、collection/filter、选择、Grid/Loupe、评分/色标、metadata、标签、
      styles、批量任务和恢复错误。
- [ ] 完成 Darkroom：操作面板、参数编辑、历史、撤销/重做、蒙版、取样/辅助叠加、预览质量与
      导出前检查；界面只映射 schema 和服务状态。
- [ ] 完成导出队列、预设、目标冲突策略、失败重试、应用退出确认和可观测日志。
- [ ] 针对第一版承诺的旧 catalog、XMP、styles、presets 和 history 实现导入器，并提供干运行、
      迁移报告、备份提示和不可支持项的可读错误。
- [ ] 覆盖空目录、损坏 RAW、磁盘满、取消、重复导入、长时间切图、连续导出和关闭窗口等场景。

出口：冻结产品清单中的端到端工作流在新桌面应用中可用，且 CLI 与 UI 通过同一 engine facade
消费一致的 recipe 并产生一致导出结果。

### 阶段 6：性能、GPU 与跨平台发布

- [ ] 先满足 CPU 正确性、内存和交互门槛；GPU 不是新应用正确性的前提，也不是阶段 5 的阻塞项。
- [ ] 若测量证明有端到端收益，再在 engine adapter 内实现后端中立 GPU 接口和连续操作链；UI、
      recipe、领域模型和公开操作 API 不得泄漏 Metal、OpenCL 或其他后端类型。
- [ ] 为 CPU/GPU 结果、资源状态、设备丢失和 CPU 回退建立自动验收。GPU 失败时从可信输入重跑
      CPU，绝不保留部分写入的预览或导出。
- [ ] 在 macOS、Windows、Linux 的可用工具链上分别 configure/build；每个平台验证安装包、首次
      启动、导入、编辑、导出和卸载后的用户数据保留策略。
- [ ] 达到性能、稳定性、可访问性、迁移成功率和崩溃/错误恢复门槛后，才创建候选发布版本。

出口：新应用在承诺平台上交付可安装包，CPU 路径完整可靠，GPU 仅作为受测的可选加速器。

### 阶段 7：切换与旧实现退役

- [ ] 先进行并行试用和可逆迁移；保留旧应用、用户 catalog/sidecar 备份和明确的回滚说明，直到
      新版本满足发布门槛。
- [ ] 确认新应用覆盖冻结范围、迁移报告可读、支持流程可处理失败后，再将默认发行物切换到新应用。
- [ ] 旧 `src/`、GTK 资料、冻结的 OpenCL 实现和旧打包图只能在切换决定完成后分批删除；每一批
      都要同步清理 CMake、资源、文档、测试、配置和消费者，并保留可回退的提交边界。
- [ ] 退役完成后更新本计划、README、架构索引和产品边界，明确哪些旧数据/功能不再受支持。

出口：发行物、源码、构建图和文档只有一个受支持的实现；没有运行时可达的旧 UI ABI、旧动态
IOP ABI 或旧 GPU 后端残留。

## 验收与防回归

- [x] 新 engine 的 UT 只链接新 C++ target，参考冻结 C 源码重新搭建，不编译或运行旧 UT。测试优先覆盖
      当前迁移批次的核心数学、解析和错误分支，不要求先完成跨全工程的 mock/harness。
      本地 `Ravo/tools/check_ravo_dependency_boundary.py` 拒绝生产源包含冻结 `src` 私有头、链接旧核心、
      GTK/catalog API、动态 legacy module 或 desktop Qt；QtCore 是允许的新工程依赖。
- [ ] `ravo` 的 inspect、operations、recipe import/validate、render、JSON、退出码、取消和原子输出
      具有独立 contract test；CLI 与未来 UI 对同一 engine request 产生一致结果。
      当前 contract 已覆盖版本 JSON、operation 列表、真实 RAW inspect、recipe validate、空/nop/manual
      exposure XMP、结构化拒绝、bounded PNG render、输出冲突和 JSON 失败；engine 层另覆盖取消、进度和
      `+1 EV` 亮度变化，foundation 层覆盖 deadline。CLI 中断取消、批处理、完整 I/O 失败矩阵以及未来 UI
      一致性仍缺，因此本项保持未勾选。
- [ ] 全部保留的旧图像 fixture 必须通过 legacy adapter 在新 CPU 引擎运行；不支持的 fixture 必须
      对应已批准的产品删减或显式不兼容决定，不能静默跳过。
- [ ] 随实现为 recipe、operation、history、mask/blend、色彩、ROI/分块和导出补聚焦 UT；旧源码中的
      分支与边界是测试设计参考。每个 bug 修复添加最小可重现样本，但测试脚手架不得阻塞下一段可工作
      C++ 行为落地。
- [ ] 对首批完整工作流维护像素金样、感知图像比对与元数据比对。允许的浮点误差必须逐项记录，
      不接受未解释的视觉或语义变化。
- [ ] 无头阶段运行 RAW inspect、XMP→recipe、CPU render、批量导出、取消和进程重启恢复；阶段 4
      再增加新/旧 catalog、history 和桌面服务端到端测试。
      当前 inspect、XMP→recipe、单文件 CPU→PNG 和 engine 取消路径已进入 Ravo 自有测试；批量导出、
      CLI 中断与进程重启恢复尚未实现。
- [ ] 对桌面 UI 自动或手工验证键盘操作、焦点、缩放/拖动、HiDPI、读屏/可访问性、错误对话框和
      窗口销毁；不得只验证“能显示第一张图片”。
- [ ] 在 CI 中检查依赖方向、公开头文件、许可证、schema 迁移和跨平台 build。核心不得因链接 UI
      库、包含 UI 头或依赖 `src/` 私有头而通过。
- [ ] 每个阶段完成时记录实际平台、编译器、测试语料、性能数据和未验证项；不将单一 macOS 结果
      表述为全平台通过。

## 与现有计划的关系

- 本文已合并原“核心收缩与 GPU 后端改造计划”。其中未完成的产品收缩、IOP 决策、语料补齐、GPU
  抽象与 Metal 工作全部由 Ravo 承接；0.9 保持冻结，不再形成第二条实施路线。
- 已删除的 `TODO_GTK4_MIGRATION.md` 和 `DevDocs/GTK4_Migration_Scope.md` 不再是实施路线，也不因
  开始新 UI 而恢复；新 UI 的选择由阶段 4 原型决定。
- 旧 `src` 的 OpenCL 保持原样直到整个旧应用退役，不在 0.9 内替换为 Metal。Ravo 首先交付 CPU
  参考路径；任何 GPU 投入都必须遵循阶段 6 的后端隔离和基准验收。
- 本文不能单独改变历史编辑、styles、presets 或 XMP 的兼容性承诺。每项具体决定要同时更新
  产品边界、迁移说明和相应测试。

## 0.9 冻结基线与 Ravo 承接项

本节合并原 0.9 核心收缩与 GPU 后端计划，但不再保留任何 0.9 实施待办。`src/`、GTK 前端、动态
IOP、pixelpipe 和 OpenCL 以当前状态冻结，只能作为静态源码与 fixture 参考；旧工程的配置、编译、
执行、CTest、打包和 CLI 均被禁止。所有未完成项改由 Ravo 在自己的 target、契约和 adapter 中实现，
最后通过阶段 7 整体删除冻结的旧应用。

### 已完成的收缩基线

- [x] 删除 Lua 运行时、绑定、脚本、构建接线、测试、`USE_LUA` 和所有 Lua 兼容路径；完成无 Lua
      构建图、动态链接和版本输出验证。
- [x] 删除幻灯片、打印、地图、联机拍摄、相机/实时取景、打印/地图设置、位置查询、GPX 和主动
      地理标记；删除 MIDI、游戏手柄、邮件和 Piwigo，仅保留本地磁盘导出。
- [x] 输入/输出收缩至 RAW、JPEG、PNG、TIFF、RGBE/HDR、QOI 和原文件复制；对外导出范围为 JPEG、
      PNG、TIFF 和原文件复制。
- [x] 删除 GraphicsMagick、ImageMagick、G'MIC 压缩 LUT、Colord、非产品可执行文件、旧通用
      profiling、AI/ONNX、神经修复、对象蒙版及其模型、文档、偏好项和测试。
- [x] Lighttable 收缩：删除顶部快捷筛选/排序栏、规则置顶状态、筛选/排序快照历史和时间线；左侧
      筛选器为唯一规则编辑入口，底部保留固定评分/色标/布局/缩放工具栏与共享 collection/selection
      的横向照片浏览条。Grid 按可用宽度派生每行 2–10 张，网格按钮只调整目标缩略图尺寸。
- [x] Lighttable Grid/Loupe 成为显式状态：`G` 返回 Grid，`E` 或底栏方框进入保留 filmstrip 和面板的
      Loupe；单击切换 Fit/100%，放大后空格加左键拖动平移，双击在两者之间切换。
- [x] 删除全局顶部工具箱、分组、缩略图叠加层、上下文帮助、快捷键映射、全局偏好入口、底栏显示
      ICC/焦点峰值/日志历史按钮、首次启动欢迎向导和空集合帮助覆盖层；焦点峰值、日志历史等保留的
      能力只从相应快捷键或 `View` 菜单进入。
- [x] 删除 duplicate manager、全局 color picker/live samples 旧面板，以及 selection 中重复的
      metadata 页签；保留多版本数据、Lighttable duplicate 操作、模块吸管和 metadata editor 的
      复制/粘贴、合并、EXIF 刷新及单色标记能力。
- [x] 开发构建使用内嵌并签名 app bundle 加载项目模块；XML 验证、ISOBMFF/CR3、LibRaw 与 ICU
      是固定构建行为。

已删除的构建开关：`USE_LUA`、`USE_DARKTABLE_PROFILING`、`USE_XMLLINT`、`USE_ISOBMFF`、
`USE_LIBRAW`、`USE_AI`、`USE_ICU`、`FORCE_COLORED_OUTPUT`、`USE_MAP`、`BUILD_PRINT`、
`USE_CAMERA_SUPPORT`、`USE_PORTMIDI`、`USE_SDL2`、`USE_OPENJPEG`、`USE_JXL`、`USE_WEBP`、
`USE_AVIF`、`USE_HEIF`、`USE_XCF`、`USE_OPENEXR`、`USE_GRAPHICSMAGICK`、`USE_IMAGEMAGICK`、
`USE_GMIC`、`USE_COLORD` 与 `BUILD_CMSTEST`。

### Ravo 承接的数据与 IOP 边界

Ravo 决定是否支持某个 IOP 前，必须先决定旧历史记录、styles 和 presets 的迁移、只读或显式拒绝
策略；不得因 0.9 中存在 operation 名就自动承诺兼容。创意或专用候选包括 bloom、soften、overlay、
velvia、vignette、split-toning、grain、borders、liquify、retouch、watermark、censorize、negadoctor 与
agx。产品范围排除的能力由 Ravo legacy adapter 返回结构化不兼容错误；旧 0.9 模块本身不再删除或
修改。最终保留清单同时决定 Ravo CPU operation、fixture 分类和未来 GPU 覆盖范围。

### 原 0.9 待办的 Ravo 归属

| 原计划工作 | 新归属 | 0.9 处理方式 |
| --- | --- | --- |
| 第二轮外围/过时代码清理 | 阶段 0 决定新产品不支持项；阶段 7 随旧 owner 整体删除 | 不再清理或重构 |
| 最终保留 IOP 与历史兼容清单 | 阶段 0 inventory、阶段 1 schema/legacy adapter、阶段 3 operation 验收 | 不删除旧 IOP |
| RAW/XMP、浮点金样、metadata 与性能语料 | 阶段 0–2 的只读冻结 fixture 与 Ravo 自有证据 | 不构建或运行旧实现生成基线 |
| 后端中立 GPU API、资源状态与错误回退 | 阶段 6 的 Ravo engine 私有 port/adapter | 不改 `pixelpipe_hb.c` 或 OpenCL API |
| Metal runtime、shader、连续热点链与性能验收 | 阶段 6 的 Ravo Metal adapter | 不在 0.9 增加 Metal |
| 删除 OpenCL、动态模块和旧配置 | 阶段 7 在 Ravo 完成替代后删除整个对应 `src` 所有权 | OpenCL 随冻结的 0.9 保留到退役 |

因此当前顺序只有一条：先完成 Ravo 阶段 0–3，使 headless engine/CLI 在真实 CPU 工作流上基本稳定；
再完成 Ravo 产品与桌面工作流；最后在阶段 6 为 Ravo 增加受测 GPU adapter，并在阶段 7 整体退役
0.9。不存在“回到 0.9 做第二轮清理或 Metal 改造”的中间阶段。

### Ravo 承接的 GPU 准入与 Metal 路线

旧代码面（63 个 `process_cl()` IOP、71 个直接接触 OpenCL 的 IOP、41 个约 1.8 万行 `.cl` 内核、
约 5,300 行 `common/opencl*`/`common/dlopencl*`，以及职责过载的 `pixelpipe_hb.c`）只作为反面边界和
算法研究资料，不是移植目标。Ravo 不建立 OpenCL adapter，也不机械翻译 `.cl`；只在 CPU engine 与
CLI 已验收后，从已迁入且通过金样的 operation 中按端到端收益选择 GPU 链。

Ravo GPU API 使用自有不透明 port/资源类型，不能向 recipe、operation schema、UI 或公共 facade 泄漏
Metal/OpenCL 对象、错误码或 shader 标识。每个 CPU/GPU 边界显式表达资源驻留、唯一写 owner、同步、
ROI/tiling 与缓存状态；统一内存不等于无同步或无格式代价。GPU 失败必须丢弃部分结果并从可信 CPU
输入安全重跑。

详细工作负载、报告 schema 与比较方法继续使用 [`DevDocs/GPU_Baseline.md`](DevDocs/GPU_Baseline.md)，
但 runner 要改为比较 Ravo CPU 与 Ravo GPU，而不是推动旧 pixelpipe 演进。正式准入包含四类版本化
RAW/XMP（细节、噪声、几何、蒙版）和快速预览、100% 暗房、全尺寸 32-bit float TIFF 三条路径；记录
输入、recipe/XMP、二进制和色彩配置哈希，并测量首帧、交互、内存、能耗、吞吐及连续 GPU 段。

初始正确性门槛继续沿用：普通浮点算子 `RMSE <= 2e-6`、`max_abs <= 2e-5`；迭代、归约、去噪或
去马赛克为 `RMSE <= 2e-5`、`max_abs <= 2e-4`；显示参考图 CIEDE2000 P99 `<= 0.25`、最大值
`<= 1.0`。所有输出必须有限，尺寸、通道、ROI、alpha、蒙版、几何和离散标签语义必须一致，且人工
检查边缘、暗部、高光与蒙版边界。

阶段 6 内部按以下顺序执行：

1. **基线与准入**：在稳定 Ravo CPU 工作流上完成语料、金样、性能/能耗和连续热点链选择；若边界
   成本无法被端到端收益抵消，则暂停 GPU 工作。
2. **后端中立边界**：在 engine 私有 port 中定义设备、队列、命令、资源状态、同步、错误和计时；
   CPU 测试与公开 API 不依赖 GPU runtime。
3. **Metal 最小闭环**：macOS 私有 Objective-C++ adapter，构建期预编译并打包 shader；先完成 CPU
   输入—连续 Metal 操作—可信结果/CPU 回退闭环，再优化资源池和 in-flight 策略。
4. **按收益迁移**：先实现色彩/格式、混合/蒙版、缩放/插值及可复用算法设施，再迁测得的热点
   operation；未迁移项保持 Ravo CPU，不降低精度、不跳过蒙版。
5. **验收**：分层运行合成图与真实 RAW 的 CPU/Metal 测试，覆盖预览、导出、ROI/tiling、色彩、
   取消、设备失败、长批处理、内存压力和资源销毁，并设置端到端性能、内存和能耗门槛。
6. **旧后端退役**：Ravo 自身不含 OpenCL；只有 Ravo 全产品达到阶段 7 切换门槛时，才随整个冻结的
   0.9 源码、构建、配置与包删除旧 OpenCL。

## 下一次开工的最小任务

1. [x] 冻结旧工程及全部 158 组 fixture；后续只读源码和资产，不配置、编译、修改或运行旧代码与旧 UT。
2. [x] 建立独立 C++20 foundation/recipe/engine/CLI 骨架，并把 FreeCM Config/Build/Run/Test/Package 项目
       命令在 Windows、macOS、Linux 上全部切到 Ravo；QtCore 作为允许的新工程依赖。当前只实测
       Windows/MSVC，macOS/Linux 由对应开发机验证。依赖联调规则已固化，rawspeed fork 也已发布并 pin
       为可嵌入基线，但生产 RAW 路径仍是 LibRaw，后续切换必须作为独立 codec 迁移验收。
3. [ ] 从 `mire1.cr2`、nop 和 exposure 出发，沿旧 C pixelpipe 调用链实现 RAW inspect → XMP import →
       CPU render → PNG 的第一条真实 CLI 闭环。nop 子路径已可运行；下一步直接补 exposure fixture 的
       其余必要历史映射并开始与冻结金样比较，不再新增通用 harness。
4. [x] 为已落地的 nop/RAW/exposure 最小切片补 GoogleTest：真实 RAW inspect、nop XMP 严格吸收、
       bounded PNG 尺寸/非空像素、输出冲突、取消和 +1 EV 亮度变化均由 Ravo target 覆盖；绝不运行旧
       CTest 或旧图像 runner。完整 exposure fixture 与像素容差测试随第 3 项继续增加。
5. [ ] 第一条闭环后按共享基础设施与 pixelpipe 顺序批量迁移 operation，每个批次保持可编译、可由 CLI
       观察并有聚焦 UT；全量分类账、产品 owner 和细粒度兼容决定并行维护，不阻塞下一批 C++ 重写。

当前唯一代码优先级仍是第 3 项的剩余部分：完成 exposure fixture 所需历史、对照冻结金样并继续扩展
CPU pixelpipe。除冻结边界、Ravo 编译失败或已迁行为回归外，文档、分类、通用测试框架和 CI/CD 都不能
成为继续实现 CPU/CLI 的前置门槛。
