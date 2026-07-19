# DarkTableNext

DarkTableNext 是从 darktable 演进而来的照片工作流与 RAW 处理应用。0.9 是重写前的清理基线：保留可运行的图像处理核心，同时移除上游的打包、发布和兼容包袱。

## 当前支持范围

- 仅保证 macOS（Apple Silicon 与 Intel）构建。
- 构建入口只有 CMake；依赖源码由 FreeCM 和 `source_roots.lock.jsonc.in` 管理。
- 支持 Homebrew 的最新 Clang，以及当前 Homebrew GCC；所有 macOS 预设都使用 ccache。
- 0.9 仅提供英语界面和英语手册链接；不再构建、安装或加载 gettext 翻译目录。
- 用户界面、发布与安装流程会在后续重做。现有行为以 `src/` 为准。

## 初始化

```sh
git submodule update --init FreeCM
python3 configs/source_root_workflow.py --init
python3 configs/source_root_workflow.py --update
```

`source_roots.lock.jsonc.in` 是受版本控制的依赖基线。`source_roots.lock.jsonc`、依赖工作树和 `CMakePresets.json` 均在本地生成，不应提交。

### FreeCM 依赖契约

锁文件中的每项依赖都必须有明确角色；不要以子模块、复制源码或 CMake 下载替代它。当前角色如下：

| 依赖 | 角色 | 应用包处理 |
| --- | --- | --- |
| RawSpeed | 编译期库与相机数据 | 链接库并部署其运行时数据 |
| OpenCL、libxcf、whereami | 编译期源根（头文件或静态链接） | 不单独部署 |
| LibRaw | 编译期源根与 RAW 解码库 | 随应用链接 |
| Imath、OpenEXR、Exiv2、inih | 图像与元数据的编译/运行时库 | 使用活跃 macOS 工具链的库；GCC 为保持 libstdc++ ABI 而从固定源根构建 |

ONNX Runtime 不属于源根依赖：AI 构建只接受本地包管理器安装的运行时，绝不在配置阶段下载 SDK。基准图像资产亦不属于默认依赖，见下文的按需初始化命令。

## 构建

```sh
cmake --preset mac_clang_debug
cmake --build --preset mac_clang_debug
./build/mac_clang_debug/bin/darktable --version
```

需要单元测试时，显式启用并运行 CTest：

```sh
cmake --preset mac_clang_debug -DBUILD_TESTING=ON
cmake --build --preset mac_clang_debug
ctest --test-dir build/mac_clang_debug --output-on-failure -L unit
```

可用的 macOS 预设：

- `mac_clang_debug` / `mac_clang_release`
- `mac_gcc_debug` / `mac_gcc_release`
- `mac_xcode`

构建环境应提供 CMake、Ninja、Homebrew 依赖和 `ccache`。若缺少运行库，先安装对应 Homebrew 包，再重新配置预设。

## 测试

启用 `-DBUILD_TESTING=ON` 后，当前受支持的测试全部是无需外部资产的 CTest `unit` 测试。旧版基准脚本及其 3.x/4.x sidecar 已随 0.9 数据兼容边界删除；新的集成与基准方案将在新架构确定后重新引入。

## 开发资料

保留的源码导读在 [DevDocs](DevDocs/README.md)。完整的清理与重写路线图在 [TODO.md](TODO.md)。

## 版本

当前版本：0.9.0。详见 [RELEASE_NOTES.md](RELEASE_NOTES.md)。
