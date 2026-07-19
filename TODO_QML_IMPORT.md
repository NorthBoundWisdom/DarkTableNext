# QML 照片导入首版

## 已确认的产品契约

- 工作流参考 Lightroom：选择来源文件夹、审核候选照片、添加到图库、显示图库网格；
- 首版只支持 **添加到图库**，绝不复制、移动或改写源文件；
- 图库是 DarkTableNext 独立的 SQLite 数据库，不读取、写入或迁移旧 darktable 图库；
- 来源通过系统文件夹选择器选择；默认只扫描当前目录，可选择包含子文件夹；
- 同一物理文件（macOS `st_dev/st_ino`，路径回退）只能导入一次；不同目录的内容副本允许导入；
- 支持 JPEG、PNG、TIFF 与当前图像后端可识别的 RAW；其他文件明确标为不支持；
- 首版不包含复制/移动、命名模板、标签、评分、XMP、编辑历史、旧图库迁移或完整目录树。

## 所有权

| 位置 | 负责内容 |
| --- | --- |
| GeoControls | 无领域语义的 `QmlFolderDialogPage` 与其公共 QML API。 |
| DarkTableNext | SQLite catalog、格式探测、元数据/缩略图适配、扫描/取消、重复识别、候选模型、图库模型和所有照片交互。 |

## 执行清单

- [x] 确认首版范围、重复策略、递归策略与旧图库隔离策略。
- [x] 为本地联调指定独立 GeoControls checkout，而非 FreeCM 生成目录。
- [ ] 在 GeoControls 中加入并验证 `QmlFolderDialogPage`；先提交/推送后才能更新宿主固定版本。
- [ ] 创建独立版本化 SQLite catalog，路径位于 `QStandardPaths::AppDataLocation`。
- [ ] 建立无 GTK、无旧图库依赖的图片探测与缩略图边界。
- [ ] 实现 worker-thread 扫描、取消、递归、候选选择和事务导入。
- [ ] 实现 QML 空图库、导入审核页、缩略图候选网格和导入后图库网格。
- [ ] 添加 catalog、扫描、去重、取消与重启持久化测试；运行 QML lint、宿主构建和手工验收。
- [ ] GeoControls 已推送后，更新 `source_roots.lock.jsonc.in` 的固定 commit 并回到 pinned 验证。

## 验收

1. 全新启动时不创建或访问旧 darktable 图库，空图库提供“导入照片”入口。
2. 选择文件夹后，界面在扫描期间保持响应；取消不会留下半完成候选结果。
3. 支持照片可预览或显示明确占位状态；损坏、不支持、无权限和重复文件都具有可见原因。
4. 点击“添加”后只写入新 catalog；提交时再次去重，源文件变化或消失不会使整个批次崩溃。
5. 成功后显示最近导入资产；重启应用仍能从新 catalog 读取它们。
