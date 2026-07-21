# Ravo：C++20 无头内核、CLI 与后续 UI 重写计划

> 状态：名称与路线已确认，尚未开工（2026-07-21）。Ravo 的第一交付物是 C++20 无头图像引擎库和
> 长期受支持的 CLI；桌面 UI 明确延后，且只能复用同一内核/API。它不是 GTK 3 → GTK 4 的迁移。
> 本文不授权立即删除或替换 `src/`。在新内核通过验收前，0.9 现有实现仍是行为、CPU 结果和兼容性
> 判断的唯一基线。

## 决策与范围

现有应用把 GTK、dtgtk/Bauhaus、动态 IOP 模块、像素管线、数据库、任务队列和 OpenCL 类型交织在
同一编译图中。例如 `develop/imageop.h` 同时包含 GTK、dtgtk、OpenCL 与像素管线头文件，
`src/CMakeLists.txt` 又把 GUI、控制层、处理层和动态模块一起构建。这使更换 UI 框架不只是控件替换，
而是一次跨越生命周期、线程和数据边界的重写。

因此，本计划采用**并行、由底向上、headless-first 的完整重写**：先建立独立 recipe 模型、无 UI
图像引擎和 CLI，用它运行旧图像回归、批处理真实照片并冻结公开契约；catalog、应用服务和桌面 UI
在无头内核达到出口后再开始。不得把旧 GTK 代码或旧 IOP ABI 包在“兼容层”后继续当作新架构内部
实现；这只会把当前耦合移动到新的目录中。

新产品的最低工作流仍以 [`TODO_CORE_REDUCTION.md`](TODO_CORE_REDUCTION.md) 为准：本地导入与
目录、图库、暗房、非破坏性编辑、蒙版、历史、色彩管理，以及 JPEG、PNG、TIFF 和原文件复制到
本地磁盘。该文件的功能删减和 IOP 兼容性决定仍然有效；本计划不重新引入已删除的工作流。
第一阶段 CLI 不需要提前实现图库或交互 UI，但必须完整表达后续桌面产品会消费的解码、recipe、
操作图、蒙版/混合、色彩管理、预览/全尺寸渲染和本地导出能力。

## 已确认的技术决策

- [x] 采用并行、clean-slate 的完整重写；旧 `src/` 只作为行为参考、算法来源和差分测试对象。
- [x] 下一代项目与软件统一命名为 **Ravo**：命令行程序为 `ravo`，内核称 Ravo Engine，未来桌面
      产品暂称 Ravo Studio；源码与文档所有权进入仓库根目录 `Ravo/`。
- [x] 整个新实现统一使用 **C++20**。无头阶段只创建 foundation、recipe、engine、adapters、CLI；
      后续 domain、services 与 desktop 也不引入 Rust/Cargo 构建图，避免同时承担 FFI、双工具链和
      两套所有权模型。
- [x] **先交付无头内核与 CLI，暂不写前端。** CLI 是新 engine 的第一个正式客户端、自动化接口和
      旧回归测试入口，不是日后删除的测试脚本；桌面 UI 必须等无头验收门槛完成后再立项。
- [x] 继续使用 CMake 与 FreeCM 管理固定依赖和跨平台预设；新旧实现拥有独立 target、构建目录、
      安装规则、运行时目录和测试标签。
- [x] CPU 是参考实现和可靠回退。GPU 后端在 CPU 垂直切片通过金样后再设计，GPU 类型不得进入
      recipe、领域模型、UI 或公开操作契约。
- [x] 不迁移旧 GTK、dtgtk/Bauhaus、动态 IOP ABI、全局 `darktable` 状态或 OpenCL API；需要保留的
      数值算法只能在去除 UI、全局状态和旧生命周期后进入新 engine。
- [x] UI 框架暂不选择，但桌面实现同样以 C++20 为语言基线。阶段 4 必须用原型验证框架后才能写
      正式 UI；不得因为旧代码使用 GTK 就默认继续使用 GTK。

以下产品与兼容性决定仍需在写代码前冻结：

- [ ] 指定第一版产品 owner，明确 0.9 并行维护窗口和只允许进入旧实现的修复类型。
- [ ] 冻结第一版重写产品范围、支持平台、离线/隐私要求和发布门槛。
- [ ] 确定新应用与旧 catalog、XMP、styles、presets 和编辑历史的兼容策略；不承诺的项目必须
      写成显式不兼容项。
- [ ] 为并行工程分配 CI 目标、测试语料、签名/包标识和用户数据目录，避免新旧应用相互污染。

旧 GTK 4 迁移计划与范围清单已经退役，当前 `src/` 才是旧应用源码地图。除 0.9 阻塞修复外，不再
启动大规模 GTK 4 迁移；新 UI 的工作集由冻结后的产品用例和应用服务契约重新产生。

## 目标架构

### 分层与依赖方向

```text
第一交付阶段

旧回归 wrapper ─┐
                 ├──▶ ravo CLI ──▶ engine facade ──▶ recipe + CPU 图像引擎
脚本/用户 ───────┘                         │
                                           ▼
                         foundation + codec/filesystem adapters

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
ravo recipe import-xmp <legacy.xmp> --output <recipe>
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
- 第三方 C/C++ 库和平台专用 Objective-C++/系统 API 只允许出现在 adapters 或 desktop 的私有实现；
  其裸句柄、宏、异常和头文件不得泄漏到 domain、services 或 engine 的公开 API。
- 重写不是把 C 文件机械改为 C++。每次迁入只保留已冻结的产品行为和经测试的算法，不复制历史 shim、
  未使用配置、动态插件兼容或旧 UI 状态机。

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

- [ ] 新工程使用独立 CMake target、install 规则和 app/CLI 名称；默认构建不混入旧 `libdarktable`
      或其动态模块，也不调用 Cargo/Rust 构建步骤。
- [ ] 旧代码只能作为行为研究和金样生成来源。要复用经验证的算法时，先抽出无 UI/无全局状态的最小
      单元，写特征测试和所有权说明，再移植或以独立库引入；不得让新核心反向包含 `src/` 内部头文件。
- [ ] 每个新目录有明确 owner、公开头文件边界和禁止依赖清单；跨层 include 在 code review 中失败。
- [ ] 新旧依赖都继续遵守 FreeCM 源根工作流，不使用网络下载、FetchContent 或未固定的源码复制。
- [ ] 阶段 3 出口前，构建图中不存在 `desktop` target 或 UI toolkit 依赖；CLI target 自身不包含算法
      源文件，只链接 engine facade 和必要 adapters。

## 旧测试的真实基线与复用方式

截至 2026-07-21，仓库内测试不能简单概括为“已有 UT 足以兜底”：

| 测试资产 | 当前数量与内容 | 重写中的用途 |
| --- | --- | --- |
| CTest `unit` target | 3 个：变量展开、数据库 schema，以及 1 个不执行断言的空 sample | 按行为重写为新 C++ 单元测试；不能链接旧 `libdarktable` 后冒充新核心测试 |
| `darktable-tests` | 158 组 XMP + `expected.png` CLI 图像回归，覆盖 68 个 operation 名；当前 CMake 清单有 76 个非注释 IOP 注册项，部分受构建条件控制 | 作为最重要的新旧 CPU 差分 oracle，并补齐未覆盖 IOP、参数边界和失败路径 |
| UI 自动测试 | 当前保留测试明确排除了 UI 测试 | 依据新应用服务和 UI 交互重新建立，不能从“图像输出一致”推导 UI 正确 |
| GPU 基线工具 | CPU/OpenCL 像素差、连续 GPU 段和性能采样 | 新 engine 的 CPU 正确性完成后再适配；不作为首个垂直切片前提 |

当前快照使用以下只读统计命令；更新数字时必须重跑并检查差异原因：

```sh
rg -n 'add_test\(|add_cmocka_test\(' src/tests
find darktable-tests -mindepth 1 -maxdepth 1 -type d -name '[0-9][0-9][0-9][0-9]-*' | wc -l
find darktable-tests -mindepth 2 -maxdepth 2 -type f -name '*.xmp' | wc -l
find darktable-tests -mindepth 2 -maxdepth 2 -type f -name 'expected.png' | wc -l
rg -o 'operation="[^"]+"' darktable-tests -g '*.xmp' | sed 's/.*operation="//;s/"$//' | sort -u | wc -l
rg -n '^[[:space:]]*add_iop\(' src/iop/CMakeLists.txt | wc -l
```

复用规则：

- [ ] 开工前在固定工具链与配置下运行全部 158 组旧 CPU 图像测试，记录通过、失败、缺失原图和平台
      波动项；只有实际通过的输出才能成为冻结基线。
- [ ] 保存原图、XMP、旧 CPU `expected.png`、必要的 32-bit float TIFF、元数据摘要、命令配置和容差。
      基线一旦冻结，新实现开发期间不得用新输出执行 `--update-expected`。
- [ ] 为新 CLI 提供仅用于测试/迁移的 legacy adapter：读取旧 XMP 并映射为版本化 recipe。现有 runner
      可以通过 wrapper 调用新引擎，但旧 CLI 参数、配置键和模块 ABI 不因此成为新产品兼容承诺。
- [ ] 同一 fixture 依次运行旧 CPU 与新 CPU，分别比较像素、NaN/Inf、尺寸/ROI、alpha、色彩空间和
      元数据。8-bit `expected.png` 继续做端到端回归，浮点中间结果用于定位精度和顺序差异。
- [ ] 每迁入一个 operation，增加新 C++ 参数/schema 单元测试、合成图边界测试、旧 XMP 映射测试和
      至少一条真实 RAW 管线测试；一张最终 PNG 不能代替这些测试。
- [ ] 变量展开和数据库 schema 测试只迁移仍属于新产品的语义。空 sample 删除，不为保持测试数量
      而复制。旧测试编译通过但仍调用旧库不算重写进度。
- [ ] UI 以服务契约测试为第一层，再增加键盘、指针、焦点、取消、快速切图、窗口销毁和可访问性测试；
      不要求新 UI 复制旧 widget 树或像素布局。

测试数量会随产品删减和新增覆盖变化。每次更新此快照都必须同时记录统计日期、统计命令和 operation
兼容性结论，不能只追求“158 个全绿”而保留已明确删除的功能。

## 分阶段待办与出口

### 阶段 0：立项、产品冻结与可测基线

- [ ] 先冻结无头用例：inspect、列出 operation/schema、导入旧 XMP、验证 recipe、CPU 预览/全尺寸
      render、本地导出、批处理、取消、错误恢复和离线运行；每个命令给出机器可验证的完成条件。
- [ ] 单独记录后续桌面用例：catalog、Grid/Loupe、单图编辑、history、masks、styles 和批量导出；
      它们用于校验 engine 契约是否足够，但不进入前三个阶段的实现范围。
- [ ] 对现有 IOP、蒙版、metadata、styles、presets 和导出能力做“保留 / 延后 / 不支持”盘点，
      并为每一项旧数据写迁移、只读或显式拒绝策略。
- [ ] 盘点并实跑现有 3 个 CTest target 与 158 组 `darktable-tests`；冻结有效的 RAW/JPEG、XMP、
      PNG/浮点输出、元数据摘要和导出样本，为每个保留工作流记录视觉说明与允许的数值误差。
- [ ] 记录当前 CLI 解码、首张预览、全尺寸导出、峰值内存和失败恢复的基线；性能目标以后续
      端到端测量为准，不以单个 kernel 或控件替换计数为准。
- [ ] 形成架构决策记录（ADR）：记录已选 C++20 和不采用首版 Rust 的原因，并继续决定公开 API/ABI、
      CLI/JSON/recipe 版本策略、线程模型、色彩管理、GPU 与许可/打包策略。UI 框架只记录评价标准，
      不在本阶段选型。

出口：无头用例、operation 兼容性清单、CLI/recipe 契约草案、语料、验收阈值和 ADR 都进入版本控制；
团队可在不阅读旧 GTK 源码的情况下描述第一条端到端 render 工作流。

### 阶段 1：无头工程骨架与 CLI 契约

- [ ] 创建 C++20 `foundation`、`recipe`、`engine`、必要 `adapters`、`cli` 和 `tests` target；构建图
      不得出现 desktop、GTK、旧 `libdarktable`、Rust runtime 或 catalog 数据库。
- [ ] 实现版本化 `AssetDescriptor`、`Recipe`、`OperationDescriptor`、`ParameterValue`、`Mask`、
      `RenderRequest`、`RenderResult` 和 `TaskError`；为序列化、升级和损坏输入写新 C++ 单元测试。
- [ ] 定义 engine facade：inspect、operation registry、recipe validate/upgrade、render、取消和进度；
      CLI 与未来 services 必须调用该 facade，不能各自拼装 pixelpipe。
- [ ] 建立 CLI 参数、退出码、JSON envelope、日志、`--version` 和确定性执行契约；加入 snapshot/
      contract test，保证 stdout 不被人类日志污染。
- [ ] 实现 legacy XMP adapter 的解析和诊断骨架，只产出 canonical recipe 或结构化不兼容错误；adapter
      不加载旧动态模块，也不链接旧核心。
- [ ] 规定 facade、任务、buffer、codec 与取消令牌的所有权和关闭顺序，并测试无效输入、重复取消、
      超时和输出路径冲突。

出口：`ravo --version --json`、`operations --json`、`recipe validate` 和错误路径可重复运行；新测试
只链接新 target，依赖图没有 UI、catalog、旧核心或 Rust。

### 阶段 2：CPU 参考图像引擎与最小垂直切片

- [ ] 实现输入资产、RAW 解码、色彩管理、预览/全尺寸输出、操作图求值、ROI/分块和安全内存预算的
      明确契约。
- [ ] 先按冻结清单实现最小的内建操作集，以及每项操作的参数验证、版本迁移和 CPU 参考执行器。
- [ ] 为首批需要的蒙版与混合实现 canonical 表示和 CPU 行为；不得把旧 GUI 数据或 module struct
      当作 recipe。
- [ ] 实现可取消的预览渲染和 JPEG/PNG/TIFF/原文件复制导出；每种输出都有色彩、元数据和原子写入
      的验收测试。
- [ ] 通过 CLI 完成首条真实闭环：inspect RAW → import legacy XMP → validate recipe → CPU render →
      PNG/浮点输出，并对照阶段 0 金样；至少选择 nop、基础 RAW/色彩链和一个可见参数 operation。
- [ ] 让现有 runner 能经测试 wrapper 调用新 CLI 跑首批 fixture；wrapper 只转换调用方式，不修改
      `expected.png`、隐藏参数或放宽容差。

出口：真实 RAW 闭环无需桌面 UI、catalog 或旧核心即可重复运行；CPU 结果、recipe 重放、取消、原子
输出和错误恢复均通过自动测试。

### 阶段 3：完成无头内核与 CLI 首版

- [ ] 按冻结清单逐项迁入所有首版保留 operation；每项具有 schema、CPU 实现、legacy 映射、合成
      单测、真实 RAW fixture、数值容差和失败行为，未支持能力由 CLI 明确拒绝。
- [ ] 完成共享的蒙版、混合、色彩转换、ROI/分块、插值、缓存和内存预算设施，避免每个 operation
      私自实现生命周期或调度。
- [ ] 完成 RAW/JPEG/PNG/TIFF 输入范围和 JPEG/PNG/TIFF/原文件复制输出；验证 ICC、EXIF/metadata、
      alpha、方向、尺寸、原子替换和磁盘满路径。
- [ ] 对全部 158 个旧 fixture 做分类：保留并通过、等待后续 operation、已批准不兼容、基线自身失败。
      首版承诺保留的 fixture 必须经 legacy adapter 在新 CPU engine 通过，不能静默 skip。
- [ ] 完成 CLI 用户与机器文档、shell completion（若采用）、版本化 JSON schema、稳定退出码、批处理、
      中断取消、资源限制、确定性模式和故障诊断。
- [ ] 在 macOS、Windows、Linux 可用工具链上分别 configure/build/test CLI；记录全尺寸 render 延迟、
      峰值内存、吞吐和长批处理稳定性，并与旧 CPU 基线比较。
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
- [ ] 旧 `src/`、GTK 迁移资料、OpenCL 迁移实现和旧打包图只能在切换决定完成后分批删除；每一批
      都要同步清理 CMake、资源、文档、测试、配置和消费者，并保留可回退的提交边界。
- [ ] 退役完成后更新本计划、README、架构索引和产品边界，明确哪些旧数据/功能不再受支持。

出口：发行物、源码、构建图和文档只有一个受支持的实现；没有运行时可达的旧 UI ABI、旧动态
IOP ABI 或旧 GPU 后端残留。

## 验收与防回归

- [ ] 新 engine 的 unit/contract test 只能链接新 C++ target；CI 检查其链接图，禁止把旧库调用包装成
      “新实现通过原 UT”。
- [ ] `ravo` 的 inspect、operations、recipe import/validate、render、JSON、退出码、取消和原子输出
      具有独立 contract test；CLI 与未来 UI 对同一 engine request 产生一致结果。
- [ ] 全部保留的旧图像 fixture 必须通过 legacy adapter 在新 CPU 引擎运行；不支持的 fixture 必须
      对应已批准的产品删减或显式不兼容决定，不能静默跳过。
- [ ] 为 recipe 解析、参数范围、操作版本迁移、history 重放、mask/blend、色彩转换、ROI/分块和
      导出写单元与契约测试；每个 bug 修复优先添加最小可重现样本。
- [ ] 对首批完整工作流维护像素金样、感知图像比对与元数据比对。允许的浮点误差必须逐项记录，
      不接受未解释的视觉或语义变化。
- [ ] 无头阶段运行 RAW inspect、XMP→recipe、CPU render、批量导出、取消和进程重启恢复；阶段 4
      再增加新/旧 catalog、history 和桌面服务端到端测试。
- [ ] 对桌面 UI 自动或手工验证键盘操作、焦点、缩放/拖动、HiDPI、读屏/可访问性、错误对话框和
      窗口销毁；不得只验证“能显示第一张图片”。
- [ ] 在 CI 中检查依赖方向、公开头文件、许可证、schema 迁移和跨平台 build。核心不得因链接 UI
      库、包含 UI 头或依赖 `src/` 私有头而通过。
- [ ] 每个阶段完成时记录实际平台、编译器、测试语料、性能数据和未验证项；不将单一 macOS 结果
      表述为全平台通过。

## 与现有计划的关系

- `TODO_CORE_REDUCTION.md` 继续定义 0.9 的产品边界和当前代码的删减/GPU 约束。重写期间旧应用只做
  已批准的维护与生成基线，不再承担下一代架构演进。
- 已删除的 `TODO_GTK4_MIGRATION.md` 和 `DevDocs/GTK4_Migration_Scope.md` 不再是实施路线，也不因
  开始新 UI 而恢复；新 UI 的选择由阶段 4 原型决定。
- 现有 Metal/OpenCL 工作不应在新架构定型前扩张。新引擎首先交付 CPU 参考路径；任何 GPU 投入都
  必须遵循阶段 6 的后端隔离和基准验收。
- 本文不能单独改变历史编辑、styles、presets 或 XMP 的兼容性承诺。每项具体决定要同时更新
  产品边界、迁移说明和相应测试。

## 下一次开工的最小任务

1. [ ] 指定第一版产品 owner、0.9 维护窗口以及 Ravo 代码 review owner。
2. [x] 以 [`Ravo/docs/adr/0001-cpp20-headless-first.md`](Ravo/docs/adr/0001-cpp20-headless-first.md)
       固化 C++20、CMake/FreeCM、CPU-first、无旧库链接和无首版 Rust 的决定；以
       [`ADR-0002`](Ravo/docs/adr/0002-ravo-consumes-src.md) 固化 Ravo 单向取代 `src` 的迁移方向。
3. [ ] 建立产品/IOP/数据兼容性盘点表，完成“保留 / 延后 / 不支持 / 只迁移读取”结论。
4. [ ] 在未修改基线的前提下实跑并归档 3 个 CTest target 与 158 组旧 CPU 图像测试，列出测试缺口。
5. [ ] 固定首个无头垂直切片的 RAW、XMP、canonical recipe、PNG/浮点金样、元数据和性能基线。
6. [ ] 在 `Ravo/` 中创建隔离的 C++20 构建骨架，先让无 UI、无 catalog、无旧库依赖的
       foundation/recipe/engine/CLI 测试通过。

在这六项完成前，不开始改写控件、不把旧模块套入新 UI、也不删除旧实现。等待开工期间只维护本文
与基线资产，不提前创建会与最终 ADR 冲突的代码骨架。
