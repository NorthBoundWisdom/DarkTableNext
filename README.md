# DarkTableNext

DarkTableNext 是从 darktable 图像处理核心演进而来的、面向 macOS 的照片工作流与
RAW 编辑应用。0.9 是核心收缩与维护基线：保留已经验证的 RAW 解码、非破坏编辑、色彩
管理、pixelpipe 与 GTK 前端，同时移除不属于当前产品范围的历史兼容和外围扩展。

当前版本不是面向普通用户的稳定发行版。现有行为、用户界面与启动路径均以 `src/`
中的 C/C++ 和 GTK 实现为准。

## 项目边界

| 维度 | 当前维护范围 |
| --- | --- |
| 平台 | macOS（Apple Silicon 与 Intel） |
| 图像核心 | C/C++ RAW 解码、IOP、pixelpipe、历史与色彩管理 |
| 前端 | GTK3、dtgtk 与 Bauhaus 控件 |
| GPU | OpenCL 可运行基线；CPU 路径始终保留 |
| 兼容性 | 不承诺已移除的插件、Lua、外围工作流或格式 |

核心产品范围是本地照片导入与管理、Lighttable、Darkroom、非破坏编辑、必要元数据和
本地磁盘导出。已经决定移除或继续评估的扩展、格式和遗留功能见
[核心收缩计划](TODO_CORE_REDUCTION.md)。

## 开发环境

当前构建只支持 macOS，并要求：

- CMake 3.26 或更新版本；
- Homebrew、Ninja、`ccache`；
- Clang（默认）或当前项目预设指定的 Homebrew GCC；
- 当前 CMake 配置所检查的图像、数据库与 GTK 依赖。

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

### 配置、构建与运行

```sh
cmake --preset mac_clang_debug
cmake --build --preset mac_clang_debug
./build/mac_clang_debug/bin/darktable --version
```

常用预设：

- `mac_clang_debug` / `mac_clang_release`
- `mac_gcc_debug` / `mac_gcc_release`
- `mac_xcode`

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
- [开发者源码导读](DevDocs/README.md)
- [GPU Phase 0 基线与准入规范](DevDocs/GPU_Baseline.md)
- [0.9.0 发布说明](RELEASE_NOTES.md)

## 许可证

DarkTableNext 继续采用 [GNU GPL v3](LICENSE)。
