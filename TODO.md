# DarkTableNext 清理与重写 TODO

> 初始审计日期：2026-07-18。本文记录审计发现及其处理结果；所有列出的清理项均已完成并经 macOS 构建验证。
>
> 当前基线是 0.9.0、仅保证 macOS 构建、以 CMake + FreeCM 为唯一构建和依赖入口。未来仍计划支持 Linux 和 Windows，因此不能把可移植抽象与“当前未编译到的分支”混为一谈。

## 初始审计结论（均已处理）

- 根目录的 `README.md` 仍是上游 darktable 的完整说明，包含已删除的 `build.sh`、`packaging/`、Linux/Windows 发布渠道和 5.6.0 身份；应尽快改为 DarkTableNext 0.9 的项目入口。
- `UserDocs/` 是完整的上游 Hugo 用户手册镜像，约 80 MiB（713 个受跟踪文件）。项目已决定以源码行为为准时，它不应继续随源码仓库维护。
- `source_roots.lock.jsonc.in` 中的 `darktable-tests` 会被默认物化；当前 `src/tests/CMakeLists.txt` 只构建变量测试和少量 CMocka 单测，并未注册或使用该集成测试源码。该依赖应改为按需测试配置，而不是默认开发依赖。
- 当前 macOS 构建仍产生 Linux 桌面入口与 AppStream 元数据，并因此强制要求 `intltool-merge`。这是最明确的残留打包链路之一。
- 历史数据库、XMP/模块参数和 IOP 顺序兼容占据了很大的核心代码面：90 个文件、182 个 `legacy_params` 调用点；`database.c`、`iop_order.c`、`blend.c`、`imageop.c`、`masks.c`、`lightroom.c` 合计约 19,000 行。移除它们必须以 0.9 的数据兼容边界为前提。

## P0 — 可先做、收益明确

- [x] 将根目录 `README.md` 重写为 DarkTableNext 的最小入口。
  - 保留：项目定位、0.9 支持范围、FreeCM 初始化/更新、CMake 预设、Clang/GCC + ccache、开发文档链接。
  - 删除：上游徽章和站点、5.6.0 下载链接、Linux/Windows 安装说明、已删除 `build.sh`/`packaging/` 引用、旧社区和贡献入口。

- [x] 删除无调用者的 `darktable-tests` 测试资产依赖。
  - 普通 FreeCM 初始化和应用构建从不物化该仓库；旧基准脚本依赖的 3.x/4.x XMP 又已被 0.9 明确拒绝。
  - 已删除独立 benchmark 源根、脚本、sidecar、README 入口及其 FreeCM 管理脚本；新的集成/基准测试应基于 0.9 数据合同重新设计。

- [x] 删除 Linux 桌面集成与 AppStream 产物。
  - 涉及：`data/org.darktable.darktable.desktop.in`、`data/org.darktable.darktable.appdata.xml.in`、`data/CMakeLists.txt` 的 desktop/appdata 分支，以及根 `CMakeLists.txt` 中的 `intltool-merge`、`desktop-file-validate`、`appstream-util` 探测。
  - 这些文件仍记录上游 5.6.0 发布历史、上游 URL 和 Linux MIME 关联；macOS 当前不消费它们。
  - 删除后复核：macOS `cmake --preset mac_clang_debug` 不再要求 `intltool-merge`。

- [x] 删除旧源码包/发布归档链。
  - 涉及：`.gitattributes` 的 `export-subst`、根 `CMakeLists.txt` 的 `SOURCE_PACKAGE`/`git archive`/tarball 版本分支、`tools/release/`、`tools/get_git_version_string.sh` 中的 `release-*` 约定。
  - 目标：版本只来自 `PROJECT_VERSION`（当前由 FreeCM 预设提供）或一个明确的 0.9 版本文件；后续 macOS 发布流程另行设计。

- [x] 删除与 macOS 无关的安装资源：`data/darktable.bash`、`data/gdb_commands` 及其 `data/CMakeLists.txt` 安装规则。
  - `gdb_commands` 仅被崩溃处理路径引用，而当前开发基线使用 Xcode/lldb；若仍需崩溃诊断，应先确定新的 macOS 调试方案。
  - Bash completion 只有在新的 CLI 分发方案中明确支持时才保留。

- [x] 清理当前已失效的忽略规则。
  - `doc/usermanual/`、`doc/api/`、`tools/iop_deps.pdf`、KDevelop/Eclipse/旧 IDE 规则与已删除目录有关。
  - 先保留构建产物、FreeCM、本地锁文件和 macOS 常用编辑器规则；其余按团队实际工具链缩减。

## P1 — 构建与依赖收敛

- [x] 收敛 `DefineOptions.cmake`，移除已经没有意义的“系统库/内置副本”回退开关。
  - `DONT_USE_INTERNAL_LUA`、`DONT_USE_INTERNAL_LIBRAW` 继承自上游的分发包策略；当前 Lua 与 LibRaw 的来源应由 FreeCM 锁和明确的构建策略决定。
  - `BINARY_PACKAGE_BUILD`、`CUSTOM_CFLAGS`、`VALIDATE_APPDATA_FILE` 属于旧打包/兼容场景，应删除或替换为明确的开发预设。
  - 对仍是产品功能的选项（OpenCL、Lua、AI、打印、地图、格式编解码器）先做产品取舍，不要仅因它们是选项而删除。

- [x] 将 macOS 前提显式化并删除无效的旧系统版本兼容分支。
  - `src/common/cups_print.c` 仍包含 macOS 10.8/10.9 以前的条件代码，与当前 macOS 14+ 基线不符。
  - 根 `CMakeLists.txt` 的 MacPorts 回退、`-L/usr/lib` 注入和旧 CMake policy 应在确定 Homebrew/Xcode 支持矩阵后收敛。
  - CMake 最低版本应提升到实际预设所要求的版本，再删除为旧 CMake 保留的政策和写法。

- [x] 将第三方构建策略写成单一的 FreeCM 契约。
  - `src/external/CMakeLists.txt` 同时含有源根解析、RawSpeed 全局变量保存/还原、Lua 脚本复制，以及为 Homebrew GCC/libstdc++ 追加 Imath/OpenEXR/Exiv2/inih 源码构建的特殊链。
  - 保留当前可用的 GCC 修复，但把它从“编译器特例”演进为目标级依赖和 ABI 策略；避免继续扩展全局 `CMAKE_*` 变量改写。
  - 每个锁定依赖应标明：运行时必需、仅编译必需、仅测试必需，以及是否应安装到应用包。

- [x] 审查并缩减自定义 `Find*.cmake` 模块。
  - `cmake/modules/` 中保留大量上游包发现逻辑（Colord、Gphoto2、IsoCodes、OSMGpsMap、PortMidi、Saxon 等）。
  - 先从 macOS 默认功能集中移除不支持的功能和对应 `find_package()`，再删除其模块；不要仅根据文件名删除仍由核心功能使用的 finder。

- [x] 删除/隔离跨平台 ONNX Runtime 下载与安装链。
  - 涉及：`cmake/modules/FindONNXRuntime.cmake`、`data/ort_gpu.json`、`tools/ai/install-ort-*`、`tools/ai/README.md`、`DevDocs/AI.md`。
  - 当前 macOS 可先定义唯一的 AI 运行时策略；CUDA、ROCm、DirectML、OpenVINO 的下载器和 Windows PowerShell 脚本应放入未来平台实现，而非主构建路径。

- [x] 固化 CMake 预设的职责。
  - `CMakePresets.json` 是生成且忽略的文件；其模板和版本只能由 `source_roots.lock.jsonc.in`/FreeCM 生成。
  - 保留当前 Clang、GCC、Xcode 和 ccache 配置；补充一个最小“配置、构建、运行、测试”验证命令，避免 README 与实际预设再次偏离。

## P1 — 文档、脚本和上游身份

- [x] 删除或重写运行时、构建与项目入口中仍指向 `darktable-org/darktable`、`darktable.org` 和上游 5.x 的材料。
  - 重点：`README.md`、`data/darktableconfig.xml.in`、`data/org.darktable.darktable.appdata.xml.in`、`data/watermarks/promo.svg`、`tools/generate_darktablerc_doc.xsl`、AI 安装脚本和 Lua 文档。
  - `DevDocs/` 已标记为源码导读；逐篇重写随新 UI 进行。

- [x] 删除 `tools/lua_doc/old_api/` 的历史 Lua API 快照，并决定是否保留整个 `tools/lua_doc/` 文档生成器。
  - `old_api` 仅服务旧版本 API 文档，不参与应用构建。
  - 若项目不再发布 Lua API 文档，连同生成器一并移除；若保留，只维护当前 API 一份。

- [x] 删除内嵌 Lua 的非构建文档。
  - `src/external/lua/doc/` 不参与 CMake 编译；`src/external/lua/README` 与 `Makefile` 也应检查是否仍需要。
  - `src/external/LuaAutoC/README.md` 是第三方说明，可在确认许可证/使用方式后移至外部链接或删除。

- [x] 对 `tools/` 做白名单化。
  - 明确保留：版本生成、配置/XSLT 生成、introspection、当前测试和数据生成所需工具。
  - 候选删除或移出主仓库：`tools/release/`、过时的发布统计、上游相机/RawSpeed/白平衡同步脚本、旧 Lua 文档工具、一次性迁移脚本。
  - 每个保留脚本应在 README 或开发文档中有调用者、输入、输出和维护责任；没有调用者的脚本不要随代码库继续演进。

## P2 — 数据、功能和 UI 重写前的取舍

- [x] 设定 0.9 数据兼容边界，然后分阶段删除数据库、XMP 和模块参数的历史迁移。
  - 核心位置：`src/common/database.c`、`src/common/iop_order.c`、`src/develop/imageop.c`、`src/develop/blend.c`、`src/develop/masks/masks.c`、`src/develop/lightroom.c`。
  - 已确定：主程序不导入旧 darktable 数据库、XMP 或 Lightroom sidecar；没有内置迁移工具。用户须在原应用中完成导出，再创建新的 0.9 资料库。
  - 已完成数据库合同：新建库直接创建最终 schema，`library.db`/`data.db` 均为版本 `1`；任何已有版本均明确拒绝。旧数据库升级、XDG 路径迁移和旧 mipmap 清理已删除。
  - 已完成 XMP 合同：0.9 写入并只读取 `Xmp.darktable.xmp_version = 6`；删除 v1–v5 history、遮罩和时间戳转换。无 darktable 编辑历史的通用 XMP 仍可读取基础元数据。
  - 已删除 Lightroom 旁路导入（`src/develop/lightroom.*`）及其在导入/暗房中的调用。
  - 已完成混合、遮罩、样式与预置的参数合同：逐版本转换函数、旧版内置预置及废弃的 blend enum 均已删除；包括仅为旧历史保留的 difference、subtract inverse、divide inverse 与 Lab L-channel。历史版本或大小不匹配的参数直接拒绝，未知混合模式回退到 normal。
  - 已完成插件参数合同：IOP、格式、存储及 lib 插件 ABI 不再暴露 `legacy_params`；所有残留的逐版本实现与仅供它们使用的辅助代码已删除。lib/export 预置仅保留当前精确版本与大小的参数块。
  - 已完成 IOP 顺序合同：只保留 0.9 RAW/JPEG 内置顺序与 custom 顺序；legacy/v3 表、迁移代码、预置和回退分支均已删除，未知顺序版本回退至当前默认值。
  - 已完成变量模板合同：只接受点号命名的 0.9 变量；内置水印、默认设置、CLI、测试和自动补全均已迁移，旧下划线、短名和旧参数语法不再自动改写。
  - 已删除仍留在活动模块中的旧算法参数路径：retouch v1 坐标、haze removal compatibility mode，以及 lens embedded metadata v1 的参数、系数算法和“升级到最新版”UI；这三个模块只保存和执行当前 0.9 算法。
  - 已将 denoiseprofile 收敛为当前 variance-stabilizing transform：移除历史开关、CPU/OpenCL 的旧预处理和回变换分支，以及不再调用的旧 kernel。
  - 已将 colorzones 固定为 v3 平滑处理和 v2 spline：移除无 UI 入口的 v1 参数/曲线编辑/CPU 与 OpenCL 路径，以及旧的 “strong” 算法选项。
  - 已将 basecurve 的 per-channel 历史 LUT 下线：移除 CPU/OpenCL 分支和内核，并从该模块的色彩保持选项中移除会触发旧算法的 `none`。

- [x] 删除 17 个已标记 `IOP_FLAGS_DEPRECATED` 的模块。
  - 已删除：`basicadj`、`channelmixer`、`clahe`、`clipping`、`colisa`、`colortransfer`、`defringe`、`equalizer`、`filmic`、`globaltonemap`、`invert`、`levels`、`relight`、`spots`、`tonemap`、`vibrance`、`zonesystem`。
  - 一并移除了它们的 CMake 目标、图标、偏好项、帮助链接、模块组与 `spots` 专用遮罩兼容代码；新建 RAW/JPEG 的 v5 IOP 顺序不再写入这些模块。
  - `IOP_FLAGS_DEPRECATED` 及其插件 ABI、历史面板、模块组、快捷键与图标分支也已删除；相机缺样提示保留为独立状态消息。
  - 历史 XMP、样式和数据库顺序表的拒绝/移除与下一个数据兼容边界事项一起完成。

- [x] 以新 UI 的功能清单驱动数据资产清理，而不是盲删运行时资源。
  - 可候选移除：旧主题、图标、样式、快捷键、预置水印、图库导出 (`data/pswp/` + `imageio/storage/gallery.c`)、LaTeX 图书导出 (`data/latex/` + `imageio/storage/latex.c`)。
  - 已移除图库与 LaTeX 导出：对应存储插件、PhotoSwipe、旧 gallery 样式资源、LaTeX 模板、安装规则及 gallery 配置项均已删除。
  - 已审计并保留仍有运行时调用者的资源：`data/CMakeLists.txt` 安装的 themes、styles、pixmaps、watermarks、默认 shortcuts、OpenCL kernels、`data/wb_presets.json`（`wb_presets.c`）和 `data/noiseprofiles.json`（`noiseprofiles.c`），以及 RawSpeed 相机数据。
  - 0.9 的英语范围不再保留本地化快捷键：已删除 `shortcutsrc.de`、语言选择分支和安装 glob；默认 `shortcutsrc` 仍是唯一的样例快捷键入口。

- [x] 确定国际化范围：0.9 只提供英语源码文本。
  - 已删除 `po/` 的 23 个翻译、Gettext CMake/安装规则、IsoCodes finder、`--localedir`、运行时语言选择和语言化手册 URL。
  - Lua 保留 `gettext` API 作为无状态英语回退，避免在脚本 API 中引入新的兼容分支。
  - 验证：`cmake --preset mac_clang_debug && cmake --build --preset mac_clang_debug`。

- [x] 清点传统 GTK UI 与新的 UI 边界。
  - `src/gui/`、`src/libs/`、`src/dtgtk/`、`src/views/` 和各 IOP 的 GUI 部分仍是上游 GTK 结构。
  - 0.9 保持 GTK 作为唯一应用壳，不添加第二套过渡 UI；图像处理状态和像素管线继续由 `src/common/`、`src/control/`、`src/develop/`、`src/imageio/` 和 IOP API 提供，UI 入口集中在上述 GTK 目录与各模块的 GUI 回调。
  - 新 UI 启动时应以这些数据/处理接口为输入，先替换单个 UI 入口，再删除对应 GTK 调用者；在功能清单确定前，不把仍被当前 UI 加载的主题、图标或快捷键误判为历史包袱。

## P2 — 平台代码的正确处理方式

- [x] 将平台差异收敛到少量明确的接口，再清除死分支。
  - 当前 macOS 路径集中在 `src/osx/`、Keychain、文件定位、启动/菜单、打印和 AI；同时核心中仍散布 Windows、Linux、BSD、旧 macOS 条件编译。
  - 已删除仅在 `common/darktable.c` 重复 macOS/架构限制的 `is_supported_platform.h`；根 `CMakeLists.txt` 是唯一的当前 macOS 构建门卫，`src/osx/` 与 Keychain 实现是当前平台适配边界。
  - 保留 POSIX/Windows 抽象、动态加载、路径、线程、信号和密码存储分支：它们是未来 Linux/Windows 的实现候选，并非已废弃的编译器兼容代码。新平台接入必须先提供各自的 CMake 预设、适配实现和测试，不能以删除分支替代设计。

- [x] 使 `USE_MAC_INTEGRATION` 与 macOS-only 策略一致。
  - 审计确认仓库不存在 `USE_MAC_INTEGRATION` 选项、finder 或可选链接目标；只剩 GTK 菜单镜像的注释和 `src/osx/` 的 C 接口。因此没有可选遗留路径需要保留或迁移。

## P3 — 测试与工程卫生

- [x] 让测试依赖和测试目标对应起来。
  - 当前 0.9 只保留无需资产的 CTest 单测；变量、样例和 AI 测试均带有 `unit` 标签，并以 `core`、`ai` 子标签区分。
  - 已删除只支持 ELF `--wrap` 链接器、无法在 macOS 构建的旧 filmicrgb mock 测试和其专用图像辅助代码。
  - 已删除没有可执行 CMake 目标、且依赖历史 sidecar 的 benchmark/`darktable-tests` 链；新的 integration、benchmark、`requires-assets` 标签仅在其 0.9 测试方案和资产合同落地时添加，避免登记空目标。

- [x] 清理历史格式与编辑器遗留的后续残项。
  - modeline、旧 `.clang-format` 的 Qt/Windows/旧项目头文件分类均已移除；保留 `SortIncludes: Never` 以确保格式化不会重排现有源码的 include。
  - 已删除绕过 CMake 的 `src/tests/Makefile`/`cache.c`；构建与测试入口统一为 CMake 预设与 CTest。

- [x] 每次删除后执行最小回归集合。
  - 每个阶段均执行 `cmake --preset mac_clang_debug`、`cmake --build --preset mac_clang_debug` 与 `git diff --check`；涉及测试的本阶段会额外执行 CTest。
  - FreeCM 依赖变动阶段已执行 `python3 configs/source_root_workflow.py --update`；数据兼容删除以拒绝旧数据库、XMP、参数和顺序版本为明确行为。

## 现阶段应保留

- `FreeCM/`、`source_roots.lock.jsonc.in`、`configs/source_roots.py` 与运行时必需的锁定依赖；它们是目前唯一受管的依赖入口。
- `DevDocs/` 中已回收的源码导读，直到新架构文档取代它们。
- `LICENSE` 及所有保留第三方代码所需许可证和版权声明。
- 当前构建所需的 LibRaw、RawSpeed、OpenCL、Lua、白平衡/降噪数据、内省与配置生成工具；删除前必须先移除其构建或运行时调用者。
