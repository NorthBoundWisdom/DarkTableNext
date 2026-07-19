# DarkTableNext

DarkTableNext 是从 darktable 图像处理核心演进而来的、面向 macOS 的照片工作流与
RAW 编辑应用。本项目保留已经验证的 RAW 解码、非破坏编辑、色彩与 pixelpipe
能力，同时主动放弃旧版产品兼容、外围扩展和历史 UI 包袱。

当前版本是 **0.9.0 重构基线**，不是面向普通用户的稳定发行版。现有 GTK3 前端
仍用于维持可运行基线和核对行为；目标前端是 Qt 6/QML，并计划通过 GeoControls
复用通用控件。锁模板与 FreeCM 源码根已经登记 GeoControls 的候选版本，但主 CMake
尚未消费它，Qt/QML 应用目标也不存在；相关内容目前仍属于目标架构。

## 项目边界

| 维度 | 当前基线 | 目标 |
| --- | --- | --- |
| 平台 | macOS（Apple Silicon 与 Intel） | 继续优先保证 macOS |
| 图像核心 | C/C++ RAW 解码、IOP、pixelpipe、历史与色彩管理 | 保留并逐步建立稳定的 UI 适配层 |
| 前端 | GTK3 | Qt 6.8+ / QML |
| 通用控件 | 旧 GTK/dtgtk/Bauhaus 控件 | 独立的 GeoControls |
| 产品 UI | 直接耦合旧模块与控件 | `DarkTableNext.Controls` + `DarkTableNext.App` |
| GPU | OpenCL 可运行基线 | 验收后迁移 Metal，CPU 始终可回退 |
| 兼容性 | 不承诺 darktable 历史插件、脚本或外围工作流兼容 | 以新的产品边界为准 |

核心产品范围是照片导入与管理、Lighttable、Darkroom、非破坏编辑、必要的元数据、
本地磁盘导出，以及可验证的 CPU/GPU 图像处理。已经决定移除或继续评估的扩展、
格式和遗留功能见 [核心收缩计划](TODO_CORE_REDUCTION.md)。

## 架构方向

目标 UI 分为三个边界清楚的层：

- **GeoControls**：主题、基础控件、弹窗外壳、通用滑块/曲线/列表等可跨产品复用的
  QML 与 Qt Quick 原语。
- **DarkTableNext.Controls**：照片编辑领域控件，以及连接现有 C/C++ 核心的
  `QObject`、`QAbstractItemModel` 和参数/历史事务适配器。
- **DarkTableNext.App**：Lighttable、Darkroom、导航、布局和完整产品工作流。

GeoControls 与本仓库的详细职责、未来接入步骤和双仓联调规则见
[GeoControls 联调与代码边界](GEOCONTROLS_INTEGRATION.md)。在迁移过程中，GTK
代码只作为现有行为与语义参考；不要把 GTK 控件逐文件机械翻译成 QML。

## 开发环境

当前构建只支持 macOS，并要求：

- CMake 3.26 或更新版本；
- Homebrew、Ninja、`ccache`；
- Clang（默认）或当前项目预设指定的 Homebrew GCC；
- 当前 CMake 配置所检查的 Homebrew 图像、数据库和 UI 依赖。

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

## 依赖与联调

`source_roots.lock.jsonc.in` 是受版本控制的依赖基线。以下内容是本地生成状态，
已被 Git 忽略，不应提交：

- `source_roots.lock.jsonc`
- `CMakePresets.json`
- `build/dependency_seed_repos/`
- `build/dependency_source_roots/`

正常开发使用 `depsMode: "pinned"`。需要同时修改外部依赖时，可在活动锁中切换为
`manual`，并让对应 `depsManualPath` 指向一个真实、独立的 Git checkout；不要指向
FreeCM 管理的 `build/dependency_*` 目录，也不要在这些目录中运行依赖自己的初始化
或更新脚本。依赖改动应先在依赖仓库提交并推送，再更新本仓库的固定 commit。

## 仓库地图

| 路径 | 用途 |
| --- | --- |
| `src/` | 当前应用、图像核心、IOP 与 GTK 基线 |
| `cmake/` | CMake 模块与构建策略 |
| `configs/` | FreeCM 源码根和工作流绑定 |
| `data/` | 运行时数据、配置和资源 |
| `tests/`、`src/tests/` | 测试支持与单元测试 |
| `benchmarks/` | 不进入产品包的可复现性能工具 |
| `DevDocs/` | 现有核心与迁移所需的源码导读 |
| `.codex/skills/` | 本仓库的构建、QML 联调、审查提交与交接工作流 |

开始修改前请阅读 [AGENTS.md](AGENTS.md)。它记录了仓库级约束、验证要求和
GeoControls 双仓提交顺序。

## 路线图与资料

- [核心收缩与重写计划](TODO_CORE_REDUCTION.md)
- [GeoControls 联调与代码边界](GEOCONTROLS_INTEGRATION.md)
- [开发者源码导读](DevDocs/README.md)
- [GPU Phase 0 基线与准入规范](DevDocs/GPU_Baseline.md)
- [0.9.0 发布说明](RELEASE_NOTES.md)

## 许可证

DarkTableNext 继续采用 [GNU GPL v3](LICENSE)。引入或扩展 GeoControls 之前，必须
确认其仓库具备明确、兼容的开源许可证，并保留第三方代码、字体和图标的来源信息。
