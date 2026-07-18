# DarkTableNext 清理与重写 TODO

> 审计日期：2026-07-18。本文是清理清单，不代表其中项目已经执行。
>
> 当前基线是 0.9.0、仅保证 macOS 构建、以 CMake + FreeCM 为唯一构建和依赖入口。未来仍计划支持 Linux 和 Windows，因此不能把可移植抽象与“当前未编译到的分支”混为一谈。

## 审计结论

- 根目录的 `README.md` 仍是上游 darktable 的完整说明，包含已删除的 `build.sh`、`packaging/`、Linux/Windows 发布渠道和 5.6.0 身份；应尽快改为 DarkTableNext 0.9 的项目入口。
- `UserDocs/` 是完整的上游 Hugo 用户手册镜像，约 80 MiB（713 个受跟踪文件）。项目已决定以源码行为为准时，它不应继续随源码仓库维护。
- `source_roots.lock.jsonc.in` 中的 `darktable-tests` 会被默认物化；当前 `src/tests/CMakeLists.txt` 只构建变量测试和少量 CMocka 单测，并未注册或使用该集成测试源码。该依赖应改为按需测试配置，而不是默认开发依赖。
- 当前 macOS 构建仍产生 Linux 桌面入口与 AppStream 元数据，并因此强制要求 `intltool-merge`。这是最明确的残留打包链路之一。
- 历史数据库、XMP/模块参数和 IOP 顺序兼容占据了很大的核心代码面：90 个文件、182 个 `legacy_params` 调用点；`database.c`、`iop_order.c`、`blend.c`、`imageop.c`、`masks.c`、`lightroom.c` 合计约 19,000 行。移除它们必须以 0.9 的数据兼容边界为前提。

## P0 — 可先做、收益明确

- [ ] 删除 `UserDocs/`。
  - 它是上游 `dtdocs` 的完整 Hugo 网站、PDF/EPUB 构建脚本、主题和翻译，而不是当前应用的构建输入。
  - 删除前仅需确认不再发布或本地生成用户手册；同时删除其 README 中的上游部署说明。

- [ ] 将根目录 `README.md` 重写为 DarkTableNext 的最小入口。
  - 保留：项目定位、0.9 支持范围、FreeCM 初始化/更新、CMake 预设、Clang/GCC + ccache、开发文档链接。
  - 删除：上游徽章和站点、5.6.0 下载链接、Linux/Windows 安装说明、已删除 `build.sh`/`packaging/` 引用、旧社区和贡献入口。

- [ ] 把 `darktable-tests` 从默认 FreeCM 物化集合改为显式测试配置。
  - 涉及：`source_roots.lock.jsonc.in`、`configs/source_roots.py`、`src/tests/benchmark/`。
  - 目标：普通 `--init`/`--update` 和应用构建不下载测试资产；需要集成/基准测试时使用单独 profile 或明确命令。
  - 前置验证：为基准和集成测试补一个可重复的 CTest/脚本入口，避免把测试资产变成无人使用的锁定依赖。

- [ ] 删除 Linux 桌面集成与 AppStream 产物。
  - 涉及：`data/org.darktable.darktable.desktop.in`、`data/org.darktable.darktable.appdata.xml.in`、`data/CMakeLists.txt` 的 desktop/appdata 分支，以及根 `CMakeLists.txt` 中的 `intltool-merge`、`desktop-file-validate`、`appstream-util` 探测。
  - 这些文件仍记录上游 5.6.0 发布历史、上游 URL 和 Linux MIME 关联；macOS 当前不消费它们。
  - 删除后复核：macOS `cmake --preset mac_clang_debug` 不再要求 `intltool-merge`。

- [ ] 删除旧源码包/发布归档链。
  - 涉及：`.gitattributes` 的 `export-subst`、根 `CMakeLists.txt` 的 `SOURCE_PACKAGE`/`git archive`/tarball 版本分支、`tools/release/`、`tools/get_git_version_string.sh` 中的 `release-*` 约定。
  - 目标：版本只来自 `PROJECT_VERSION`（当前由 FreeCM 预设提供）或一个明确的 0.9 版本文件；后续 macOS 发布流程另行设计。

- [ ] 删除与 macOS 无关的安装资源：`data/darktable.bash`、`data/gdb_commands` 及其 `data/CMakeLists.txt` 安装规则。
  - `gdb_commands` 仅被崩溃处理路径引用，而当前开发基线使用 Xcode/lldb；若仍需崩溃诊断，应先确定新的 macOS 调试方案。
  - Bash completion 只有在新的 CLI 分发方案中明确支持时才保留。

- [ ] 清理当前已失效的忽略规则。
  - `doc/usermanual/`、`doc/api/`、`tools/iop_deps.pdf`、KDevelop/Eclipse/旧 IDE 规则与已删除目录有关。
  - 先保留构建产物、FreeCM、本地锁文件和 macOS 常用编辑器规则；其余按团队实际工具链缩减。

## P1 — 构建与依赖收敛

- [ ] 收敛 `DefineOptions.cmake`，移除已经没有意义的“系统库/内置副本”回退开关。
  - `DONT_USE_INTERNAL_LUA`、`DONT_USE_INTERNAL_LIBRAW` 继承自上游的分发包策略；当前 Lua 与 LibRaw 的来源应由 FreeCM 锁和明确的构建策略决定。
  - `BINARY_PACKAGE_BUILD`、`CUSTOM_CFLAGS`、`VALIDATE_APPDATA_FILE` 属于旧打包/兼容场景，应删除或替换为明确的开发预设。
  - 对仍是产品功能的选项（OpenCL、Lua、AI、打印、地图、格式编解码器）先做产品取舍，不要仅因它们是选项而删除。

- [ ] 将 macOS 前提显式化并删除无效的旧系统版本兼容分支。
  - `src/common/cups_print.c` 仍包含 macOS 10.8/10.9 以前的条件代码，与当前 macOS 14+ 基线不符。
  - 根 `CMakeLists.txt` 的 MacPorts 回退、`-L/usr/lib` 注入和旧 CMake policy 应在确定 Homebrew/Xcode 支持矩阵后收敛。
  - CMake 最低版本应提升到实际预设所要求的版本，再删除为旧 CMake 保留的政策和写法。

- [ ] 将第三方构建策略写成单一的 FreeCM 契约。
  - `src/external/CMakeLists.txt` 同时含有源根解析、RawSpeed 全局变量保存/还原、Lua 脚本复制，以及为 Homebrew GCC/libstdc++ 追加 Imath/OpenEXR/Exiv2/inih 源码构建的特殊链。
  - 保留当前可用的 GCC 修复，但把它从“编译器特例”演进为目标级依赖和 ABI 策略；避免继续扩展全局 `CMAKE_*` 变量改写。
  - 每个锁定依赖应标明：运行时必需、仅编译必需、仅测试必需，以及是否应安装到应用包。

- [ ] 审查并缩减自定义 `Find*.cmake` 模块。
  - `cmake/modules/` 中保留大量上游包发现逻辑（Colord、Gphoto2、IsoCodes、OSMGpsMap、PortMidi、Saxon 等）。
  - 先从 macOS 默认功能集中移除不支持的功能和对应 `find_package()`，再删除其模块；不要仅根据文件名删除仍由核心功能使用的 finder。

- [ ] 删除/隔离跨平台 ONNX Runtime 下载与安装链。
  - 涉及：`cmake/modules/FindONNXRuntime.cmake`、`data/ort_gpu.json`、`tools/ai/install-ort-*`、`tools/ai/README.md`、`DevDocs/AI.md`。
  - 当前 macOS 可先定义唯一的 AI 运行时策略；CUDA、ROCm、DirectML、OpenVINO 的下载器和 Windows PowerShell 脚本应放入未来平台实现，而非主构建路径。

- [ ] 固化 CMake 预设的职责。
  - `CMakePresets.json` 是生成且忽略的文件；其模板和版本只能由 `source_roots.lock.jsonc.in`/FreeCM 生成。
  - 保留当前 Clang、GCC、Xcode 和 ccache 配置；补充一个最小“配置、构建、运行、测试”验证命令，避免 README 与实际预设再次偏离。

## P1 — 文档、脚本和上游身份

- [ ] 删除或重写所有仍指向 `darktable-org/darktable`、`darktable.org` 和上游 5.x 的材料。
  - 重点：`README.md`、`data/darktableconfig.xml.in`、`data/org.darktable.darktable.appdata.xml.in`、`data/watermarks/promo.svg`、`tools/generate_darktablerc_doc.xsl`、AI 安装脚本和 Lua 文档。
  - `DevDocs/` 是已挑选回收的源码导读，应保留，但需逐篇标记“仍准确 / 待重写 / 已过时”。

- [ ] 删除 `tools/lua_doc/old_api/` 的历史 Lua API 快照，并决定是否保留整个 `tools/lua_doc/` 文档生成器。
  - `old_api` 仅服务旧版本 API 文档，不参与应用构建。
  - 若项目不再发布 Lua API 文档，连同生成器一并移除；若保留，只维护当前 API 一份。

- [ ] 删除内嵌 Lua 的非构建文档。
  - `src/external/lua/doc/` 不参与 CMake 编译；`src/external/lua/README` 与 `Makefile` 也应检查是否仍需要。
  - `src/external/LuaAutoC/README.md` 是第三方说明，可在确认许可证/使用方式后移至外部链接或删除。

- [ ] 对 `tools/` 做白名单化。
  - 明确保留：版本生成、配置/XSLT 生成、introspection、当前测试和数据生成所需工具。
  - 候选删除或移出主仓库：`tools/release/`、过时的发布统计、上游相机/RawSpeed/白平衡同步脚本、旧 Lua 文档工具、一次性迁移脚本。
  - 每个保留脚本应在 README 或开发文档中有调用者、输入、输出和维护责任；没有调用者的脚本不要随代码库继续演进。

## P2 — 数据、功能和 UI 重写前的取舍

- [ ] 设定 0.9 数据兼容边界，然后分阶段删除数据库、XMP 和模块参数的历史迁移。
  - 核心位置：`src/common/database.c`、`src/common/iop_order.c`、`src/develop/imageop.c`、`src/develop/blend.c`、`src/develop/masks/masks.c`、`src/develop/lightroom.c`。
  - 需要先决定：是否支持导入旧 darktable 数据库/XMP、支持的最低版本、是否提供一次性外部迁移工具。
  - 建议：保留一个独立导入/迁移工具，而不是让主应用长期携带每一代 schema 和参数转换代码。

- [ ] 处理 17 个已标记 `IOP_FLAGS_DEPRECATED` 的模块。
  - 当前模块：`basicadj`、`channelmixer`、`clahe`、`clipping`、`colisa`、`colortransfer`、`defringe`、`equalizer`、`filmic`、`globaltonemap`、`invert`、`levels`、`relight`、`spots`、`tonemap`、`vibrance`、`zonesystem`。
  - 对每个模块选择“删除”“保留为兼容导入”“迁移到替代模块”之一；删除必须与旧 XMP/历史堆栈兼容策略一起完成。

- [ ] 以新 UI 的功能清单驱动数据资产清理，而不是盲删运行时资源。
  - 可候选移除：旧主题、图标、样式、快捷键、预置水印、图库导出 (`data/pswp/` + `imageio/storage/gallery.c`)、LaTeX 图书导出 (`data/latex/` + `imageio/storage/latex.c`)。
  - 当前仍被运行时代码加载、暂不应直接删除：`data/wb_presets.json`（约 4.3 MiB）、`data/noiseprofiles.json`（约 1.7 MiB）、watermarks、shortcuts、OpenCL kernels、RawSpeed 相机数据。

- [ ] 确定国际化范围。
  - `po/` 含 23 个应用翻译、约 22 MiB，并参与当前 `.desktop`/AppStream 生成和应用消息编译。
  - 选择“英文优先、暂时只保留 en”“维持现有翻译”“迁移到外部翻译仓库”之一；若缩减为英文，先移除对应 Gettext、`intltool` 和安装规则。

- [ ] 清点传统 GTK UI 与新的 UI 边界。
  - `src/gui/`、`src/libs/`、`src/dtgtk/`、`src/views/` 和各 IOP 的 GUI 部分仍是上游 GTK 结构。
  - 先定义可复用的图像处理核心、数据模型、命令/动作与渲染接口；随后删除旧 UI，而不是在现有 UI 文件中累积更多过渡层。

## P2 — 平台代码的正确处理方式

- [ ] 将平台差异收敛到少量明确的接口，再清除死分支。
  - 当前 macOS 路径集中在 `src/osx/`、Keychain、文件定位、启动/菜单、打印和 AI；同时核心中仍散布 Windows、Linux、BSD、旧 macOS 条件编译。
  - 立即可删：已确定低于 macOS 14 支持范围的 macOS 版本检查。
  - 不应立即删：POSIX/Windows 抽象、动态加载、路径、线程、信号、密码存储等未来 Linux/Windows 仍会需要的边界。应先建立新的平台层和测试，再删除旧实现。

- [ ] 使 `USE_MAC_INTEGRATION` 与 macOS-only 策略一致。
  - 若 MacIntegration 是 0.9 的必需体验，则改为必需依赖并删除开关；若不是，则移除该功能和 finder，而不是保留默认开启的可选遗留路径。

## P3 — 测试与工程卫生

- [ ] 让测试依赖和测试目标对应起来。
  - 当前 `src/tests/` 只直接注册变量测试和 CMocka 单测；基准、集成资产和 FreeCM `darktable-tests` 的关系不透明。
  - 建立 `ctest` 标签（unit / integration / benchmark / requires-assets），并让每个标签按需物化相应源根。

- [ ] 清理历史格式与编辑器遗留的后续残项。
  - modeline 已移除；继续审查 `.clang-format` 中 Windows 头文件排序例外、过时 IDE 配置和未使用的分析工具配置。
  - 格式化、静态分析与构建验证应由 CMake 预设或新的开发脚本承载，而不是散落的工具说明。

- [ ] 每次删除后执行最小回归集合。
  - `python3 configs/source_root_workflow.py --update`
  - `cmake --preset mac_clang_debug`
  - `cmake --build --preset mac_clang_debug`
  - 对受影响功能补充 CTest 或可执行冒烟测试；对数据兼容破坏增加明确的迁移/拒绝行为测试。

## 现阶段应保留

- `FreeCM/`、`source_roots.lock.jsonc.in`、`configs/source_roots.py` 与运行时必需的锁定依赖；它们是目前唯一受管的依赖入口。
- `DevDocs/` 中已回收的源码导读，直到新架构文档取代它们。
- `LICENSE` 及所有保留第三方代码所需许可证和版权声明。
- 当前构建所需的 LibRaw、RawSpeed、OpenCL、Lua、白平衡/降噪数据、内省与配置生成工具；删除前必须先移除其构建或运行时调用者。
