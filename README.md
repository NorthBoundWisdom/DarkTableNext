# DarkTableNext

DarkTableNext 是从 darktable 图像处理核心演进而来的、面向 macOS 的照片工作流与
RAW 编辑应用。0.9 是核心收缩与维护基线：保留已经验证的 RAW 解码、非破坏编辑、色彩
管理、pixelpipe 与 GTK 前端，同时移除不属于当前产品范围的历史兼容和外围扩展。

当前版本不是面向普通用户的稳定发行版。现有行为、用户界面与启动路径均以 `src/`
中的 C/C++ 和 GTK 实现为准。

## 项目边界

| 维度 | 当前维护范围 |
| --- | --- |
| 平台 | macOS（Apple Silicon 与 Intel）；Windows MSVC/vcpkg bootstrap |
| 图像核心 | C/C++ RAW 解码、IOP、pixelpipe、历史与色彩管理 |
| 前端 | GTK3、dtgtk 与 Bauhaus 控件 |
| GPU | OpenCL 可运行基线；CPU 路径始终保留 |
| 兼容性 | 不承诺已移除的插件、Lua、外围工作流或格式 |

核心产品范围是本地照片导入与管理、Lighttable、Darkroom、非破坏编辑、必要元数据和
本地磁盘导出。已经决定移除或继续评估的扩展、格式和遗留功能见
[核心收缩计划](TODO_CORE_REDUCTION.md)。

## 开发环境

macOS 构建要求：

- CMake 3.26 或更新版本；
- Homebrew、Ninja、`ccache`；
- Clang（默认）或当前项目预设指定的 Homebrew GCC；
- 当前 CMake 配置所检查的图像、数据库与 GTK 依赖。

Windows bootstrap 使用 Visual Studio 的 x64 开发者环境和
`C:\OpenSource\vcpkg` 的 `x64-windows` triplet。FreeCM 生成的
`win_msvc_debug` 与 `win_msvc_release` 预设会接入该工具链。除已安装的 GTK3
及图像库外，构建工具需要：

```powershell
C:\OpenSource\vcpkg\vcpkg.exe install "libxml2[tools]:x64-windows" "libxslt[tools]:x64-windows" gtk3:x64-windows pthreads:x64-windows
```

Microsoft C does not implement C99 complex arithmetic. The MSVC bootstrap therefore omits the
optional Liquify IOP; choosing FDC for X-Trans input uses the existing Markesteijn fallback.

外部源码不使用临时 CMake 下载或复制进仓库的 vendored 副本，而由 FreeCM 和
`source_roots.lock.jsonc.in` 统一管理。

### 首次初始化

```sh
git submodule update --init FreeCM
python3 configs/source_root_workflow.py --init
python3 configs/source_root_workflow.py --update
```

`--init` 会准备依赖 seed repository，是唯一允许联网的依赖工作流；`--update`
根据活动锁离线物化源码根并重新生成 CMake 预设。

### 运行时持久化模式

`source_roots.lock.jsonc.in` 固定 `AppConfigs.DevMode: false`，因此 release 构建使用
标准用户配置、缓存和数据库根，并在启动时使用一个由内核自动释放的应用实例锁；同一用户
不能同时运行两个生产进程，即使它们指定了不同的 `--library` 或 `--configdir`。

开发 checkout 可在被忽略的 `source_roots.lock.jsonc` 中设置
`"AppConfigs": { "DevMode": true }`，然后重新执行 `--update` 和 CMake configure。开发
构建会根据解析后的 checkout 路径生成稳定哈希，并将默认持久化根隔离在
`darktable-dev/<checkout-hash>` 下；该根包含 `library.db`、`data.db`、配置和缓存，同时 D-Bus
服务名也按 checkout 隔离。不同 checkout 可以并存；同一 checkout 仍由数据库锁保护。
显式传入 `--configdir` 或 `--cachedir` 仍会覆盖该默认根，应只用于明确需要共享状态的场景。

### 配置、构建与运行

```sh
cmake --preset mac_clang_debug
cmake --build --preset mac_clang_debug
./build/mac_clang_debug/bin/darktable --version
```

`bin/darktable` 是开发构建的稳定启动入口。它会启动 CMake 原生生成的同目录
`darktable.app`，因此直接从终端运行时也保有标准的 macOS bundle 身份、图标、菜单栏和
Accessibility 命令发现；也可用 `open build/mac_clang_debug/bin/darktable.app` 按 LaunchServices
路径启动。

要生成可验收的 macOS DMG，先配置 release preset，再调用 FreeCM 接线的打包目标：

```sh
cmake --preset mac_clang_release
cmake --build --preset mac_clang_release --target package-macos
open build/mac_clang_release/darktable-0.9.0-macos-arm64.dmg
```

该目标先经正常 CMake install graph 将 DarkTableNext 的核心动态库、模块和数据复制进
`darktable.app/Contents/Resources`，并携带可重定位的 GTK 图像加载器；再由 FreeCM 的 `native`
macOS packager 收集允许的 Homebrew Mach-O 依赖、重写 rpath、签名并制作带 `/Applications` 链接的
DMG。默认是 ad-hoc 签名，适合本机验收；正式对外发布时，在配置阶段传入
`-DDT_MACOS_CODESIGN_IDENTITY="Developer ID Application: …"`，并在发布环境完成公证。

Windows 配置与构建：

```powershell
cmake --preset win_msvc_debug
cmake --build --preset win_msvc_debug
```

### 开发构建的模块重链

应用会在启动时动态加载 IOP、Lighttable、view 与 imageio 模块。正常构建只会在模块自身源码或
其公共 ABI 头变更时重链这些模块；核心库的实现性改动不会重写全部模块，从而保留 macOS 的文件
页缓存并缩短编译后的首次启动。

对于确实被重写而失去文件页缓存的模块，启动过程会在数据库、collection 与 GTK 初始化期间用
独立只读线程预热模块文件，并在首次动态加载前汇合；该线程不访问 GTK、数据库或模块全局状态。
启动画面的模块进度更新按 50ms 节流并使用非阻塞 display flush，避免为每个模块执行一次 Quartz
同步往返。这两项优化不改变模块 ABI、加载顺序或缓存失效规则。

`DT_MODULE_VERSION` 是明确的模块 ABI 失效边界。凡是修改了 loadable module 可见的
`lib_darktable` 函数、类型、结构布局或调用约定，必须在
[`src/common/darktable.h`](src/common/darktable.h) 提升该值，再完整构建模块。运行时加载器会拒绝
ABI epoch 不匹配的模块。

需要排查 ABI 问题时，可恢复每次核心库变动都重链模块的保守模式：

```sh
cmake --preset mac_clang_debug -DDT_MODULE_RELINK_ON_CORE_CHANGE=ON
cmake --build --preset mac_clang_debug
```

该选项只影响本地生成的构建图，不影响数据库、缩略图缓存或 OpenCL 内核缓存的失效规则。

常用预设：

- `mac_clang_debug` / `mac_clang_release`
- `mac_gcc_debug` / `mac_gcc_release`
- `mac_xcode`
- `win_msvc_debug` / `win_msvc_release`

### 测试

测试默认关闭，开发时需要显式启用：

```sh
cmake --preset mac_clang_debug -DBUILD_TESTING=ON
cmake --build --preset mac_clang_debug
ctest --test-dir build/mac_clang_debug --output-on-failure -L unit
```

GPU 基线工具及其隔离安装、浮点像素比较方法见
[benchmarks/README.md](benchmarks/README.md)。

## 依赖

`source_roots.lock.jsonc.in` 是受版本控制的依赖基线。以下内容是本地生成状态，
已被 Git 忽略，不应提交：

- `source_roots.lock.jsonc`
- `CMakePresets.json`
- `build/dependency_seed_repos/`
- `build/dependency_source_roots/`

正常开发使用 `depsMode: "pinned"`。需要同时修改外部依赖时，可在活动锁中切换为
`manual`，并让对应 `depsManualPath` 指向真实、独立的 Git checkout；不要指向
FreeCM 管理的 `build/dependency_*` 目录。

## 仓库地图

| 路径 | 用途 |
| --- | --- |
| `src/` | 应用、图像核心、IOP、GTK 前端与测试 |
| `cmake/` | CMake 模块与构建策略 |
| `configs/` | FreeCM 源码根和工作流绑定 |
| `data/` | 运行时数据、配置和资源 |
| `benchmarks/` | 不进入产品包的可复现性能工具 |
| `DevDocs/` | 核心源码导读与基线资料 |
| `.codex/skills/` | 构建、审查提交与上下文交接工作流 |

开始修改前请阅读 [AGENTS.md](AGENTS.md)。

## 路线图与资料

- [核心收缩与重写计划](TODO_CORE_REDUCTION.md)
- [动态上下文菜单计划与覆盖状态](TODO_CONTEXT_MENUS.md)
- [macOS 系统命令与快捷键重构记录](TODO_MACOS_COMMANDS.md)
- [开发者源码导读](DevDocs/README.md)
- [GPU Phase 0 基线与准入规范](DevDocs/GPU_Baseline.md)
- [0.9.0 发布说明](RELEASE_NOTES.md)

## 许可证

DarkTableNext 继续采用 [GNU GPL v3](LICENSE)。
