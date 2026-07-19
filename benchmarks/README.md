# Performance benchmarks

这里存放不参与应用打包的、可复现的性能测量工具。构建生成输入仍只放在
`tools/`；基线报告与导出图片必须写到构建树或临时目录，不提交到仓库。

## CPU / OpenCL 基线

`gpu_baseline.py` 用隔离的配置、缓存和内存数据库，分别执行 CPU 与 OpenCL
TIFF 导出。它保留逐次日志和输出，使用 libtiff 直接比较浮点像素，并生成结构化
的 `report.json` 与便于阅读的 `summary.md`。

先将当前构建安装到一个空目录，避免增量构建目录中已经删除的插件被动态加载：

```sh
cmake --install build/mac_clang_release --prefix /tmp/dtn-gpu-stage
python3 benchmarks/gpu_baseline.py \
  --input /path/to/input.raw \
  --xmp /path/to/input.raw.xmp \
  --output-dir /tmp/dtn-gpu-report \
  --cli /tmp/dtn-gpu-stage/bin/darktable-cli \
  --data-dir /tmp/dtn-gpu-stage/share/darktable \
  --module-dir /tmp/dtn-gpu-stage/lib/darktable \
  --require-opencl
```

默认每种后端预热一次、测量三次，像素门槛为 `RMSE <= 2e-5` 且
`max_abs <= 2e-4`。对正式对比应保持机器、电源模式、输入、XMP、构建类型和尺寸
参数一致。输出 SHA-256 只标识具体文件；TIFF 中的时间与路径元数据会变化，画质
判断以浮点像素指标为准。
