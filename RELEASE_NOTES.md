# DarkTableNext 0.9.0

DarkTableNext 从 `0.9.0` 开始独立演进。

## 当前基线

- 仅支持 macOS（Apple Silicon 与 Intel）。
- 构建入口为 CMake；不再提供项目专用构建脚本。
- 依赖源码由 FreeCM 和 `source_roots.lock.jsonc.in` 管理。
- 旧有跨平台打包、CI 与兼容实现已移除。

## 后续方向

- 重做用户界面。
- 建立新的 macOS 发布与打包流程。
- 逐步替换遗留的上游功能与文档。
