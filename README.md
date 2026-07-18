# DarkTableNext

DarkTableNext 是从 darktable 演进而来的照片工作流与 RAW 处理应用。0.9 是重写前的清理基线：保留可运行的图像处理核心，同时移除上游的打包、发布和兼容包袱。

## 当前支持范围

- 仅保证 macOS（Apple Silicon 与 Intel）构建。
- 构建入口只有 CMake；依赖源码由 FreeCM 和 `source_roots.lock.jsonc.in` 管理。
- 支持 Homebrew 的最新 Clang，以及当前 Homebrew GCC；所有 macOS 预设都使用 ccache。
- 用户界面、发布与安装流程会在后续重做。现有行为以 `src/` 为准。

## 初始化

```sh
git submodule update --init FreeCM
python3 configs/source_root_workflow.py --init
python3 configs/source_root_workflow.py --update
```

`source_roots.lock.jsonc.in` 是受版本控制的依赖基线。`source_roots.lock.jsonc`、依赖工作树和 `CMakePresets.json` 均在本地生成，不应提交。

## 构建

```sh
cmake --preset mac_clang_debug
cmake --build --preset mac_clang_debug
./build/mac_clang_debug/bin/darktable --version
```

可用的 macOS 预设：

- `mac_clang_debug` / `mac_clang_release`
- `mac_gcc_debug` / `mac_gcc_release`
- `mac_xcode`

构建环境应提供 CMake、Ninja、Homebrew 依赖和 `ccache`。若缺少运行库，先安装对应 Homebrew 包，再重新配置预设。

## 测试与基准

启用 `-DBUILD_TESTING=ON` 后，单元测试由 CTest 管理。基准测试不再默认下载图像资产；如需上游标准图像，显式初始化其独立源根：

```sh
python3 configs/benchmark_source_roots.py init-seeds
python3 configs/benchmark_source_roots.py materialize
src/tests/benchmark/darktable-bench
```

也可以向 `darktable-bench` 传递自己的 RAW 图像：`--image /path/to/image.raw`。

## 开发资料

保留的源码导读在 [DevDocs](DevDocs/README.md)。完整的清理与重写路线图在 [TODO.md](TODO.md)。

## 版本

当前版本：0.9.0。详见 [RELEASE_NOTES.md](RELEASE_NOTES.md)。
