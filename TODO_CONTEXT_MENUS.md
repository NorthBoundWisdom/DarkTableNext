# 动态上下文菜单实施计划

## 状态与结论

DarkTableNext 0.9 继续维护现有 GTK3、dtgtk 和 Bauhaus 前端。本计划不引入 QML、Qt、
第二套应用入口或新的数据库，而是在现有 Action/快捷键系统之上实现动态右键菜单。

目标不是为若干界面手写固定菜单，而是让菜单、快捷键和其他输入方式共享同一份动作
定义、可用状态与执行入口。右键命中不同对象时，菜单应展示该对象当前适用的全部操作，
并正确处理多选、模块实例、撤销/历史、后台任务和既有右键行为。

本计划属于 UI 与动作调度改造，不改变 pixelpipe、IOP 图像算法、CPU/OpenCL 结果、数据库
schema 或文件格式支持。产品范围继续以 [`TODO_CORE_REDUCTION.md`](TODO_CORE_REDUCTION.md)
为准；当前 GTK 架构地图见 [`DevDocs/GUI.md`](DevDocs/GUI.md)。

## 当前实现进度

已落地的基础闭环：

- [x] `src/gui/accelerators.[ch]` 已提供 Action 树遍历、标签/快捷键查询、实例解析、无副作用
      状态查询和按指针执行入口。
- [x] `src/gui/context_menu.[ch]` 将离散 Action 投影为 GTK 菜单；普通按钮、开关、下拉框、
      slider、entry、Lib 和 IOP 模块头与快捷键共用执行路径。
- [x] Action 具备 provider-only 元数据：依赖树行、图元或其他 provider payload 的命令仍走统一
      调用入口，但不会被所属模块的通用菜单错误投影为无上下文的可点击项。
- [x] Mask 树、四个曲线/Color Zones 图表、Color Checker patch 与 Filmic RGB 图表的对象级
      Action 已标为 provider-only：模块标题的通用菜单不会生成缺少行/节点/按钮 payload 的空操作，
      原位专用 provider 仍暴露它们的完整离散操作。
- [x] IOP 与 Lib 模块标题、主体的空白事件区域已绑定到所属模块 Action；辅助点击显示完整的
      显示、启用、复位、预设及合法多实例操作，不再仅弹出预设。预设按钮自身仍保留
      “在新实例应用”等对象专属手势。
- [x] IOP 多实例按钮的辅助点击已改为 Action 菜单，显式列出新建、复制、移动、删除与重命名，
      并沿用原有可移动/可删除条件禁用对应项；左键的紧凑菜单保持不变。
- [x] 菜单激活前会重新校验目标；`act_on` 操作在激活期使用菜单弹出时复制的图片列表和主图片
      ID，后台 job 因而只接收稳定的领域 ID，不持有 GtkWidget。
- [x] Lighttable 缩略图、Darkroom filmstrip、thumbnail overlays、Culling 和 Preview 已接入右键；
      后两者共用 `dt_culling_t` 事件入口，辅助点击不会再落入平移路径，并支持菜单键/Shift+F10。
- [x] 缩略图 overlay 通过 image provider 优先走完整图片菜单，避免 rating/color-label 控件的
      单一 Action 抢先消费右键事件。
- [x] 选择集图片操作复用既有 history、复制、评分、色标和 control job API。
- [x] 图片历史 Action 显式覆盖复制、选择性复制、粘贴、选择性粘贴、压缩、丢弃和写入
      sidecar；其状态通过 Action 自身的只读 callback 根据冻结选择集给出“无选择、单图要求、
      无复制历史”等原因。
- [x] 缩略图 command 在 Lighttable 与 Darkroom 均可见；除既有评分、色标、历史、复制/移动/
      删除外，也显式覆盖旋转、本地副本、EXIF 刷新和单色/彩色操作。
- [x] Collection 的 folder/filmroll 行以及 Tagging 的 attached/dictionary 行已迁移为
      provider-owned Action 项；每项保存 `GtkTreeRowReference` 和弱 `GtkTreeView` 引用，
      触发前重新定位同一行，不再依赖菜单打开后的当前选择。
- [x] Styles、History 和 Snapshots 行已接入 provider；Style 使用行引用，History/Snapshot
      使用命中行 Widget 的弱引用，在列表重建或快照重排后安全拒绝过期菜单项。
- [x] Metadata 文本行保留 GTK 原生剪贴板菜单，并将“选择已有值”的动态条目包装为带弱文本框
      引用的 Action 项，避免通用 Entry 菜单遮住既有编辑能力。
- [x] Mask 管理器树保留它的层级菜单与复选状态，但每项都绑定到同一 Tree Action；菜单打开时
      冻结 `GtkTreeRowReference`，激活时先恢复并校验原选择，再复用既有 shape/group/history
      回调，避免操作漂移到菜单打开后的选择。
- [x] Timeline 的辅助点击已由直接删除最后一条时间规则改为“移除时间筛选”Action 菜单；当末条
      collection 规则不是时间规则时，菜单项保留但会给出禁用原因。
- [x] modulegroups 的“active modules”辅助菜单已将“show all history modules”绑定为命令
      Action；它携带目标布尔值并继续使用原有 preset 保存与 IOP 可见性更新。
- [x] modulegroups 的分组与 quick-access 辅助菜单已将添加/移除操作绑定为命令 Action；菜单
      payload 冻结分组索引、分组名称和模块 Action 路径/operation，而不是保留 GtkWidget 或
      Action 裸指针，激活时会重新解析并校验当前布局与 IOP 所属关系。
- [x] RGB Curve、Tone Curve、Base Curve 与 Color Zones 的节点右键已迁移为专用 provider：节点 payload 只
      保存通道（如适用）、索引和坐标快照，激活时重新校验曲线，复用原有参数、history 与
      颜色拾取重置路径。
- [x] Crop 画布的单一辅助点击语义已迁移为“复位裁剪”Action 菜单；菜单不保留模块裸指针，
      Action 仍经由既有参数与 history 路径完成复位。
- [x] 通用单元素 Action 表、Darkroom/Lighttable 视图表、modulegroups 循环表和 tabs 表均补齐
      `effects == NULL` 终止项；菜单投影按 effect 枚举时不再越界读取。该修复对应
      2026-07-20 缩略图右键崩溃报告中的 `dt_action_get_effect_label()` / `g_dpgettext()` 栈。
- [x] HDR 合并菜单项除了最少两张图片外，还会基于冻结选择集检查 RAW 标志；包含非 RAW
      图片时保留菜单项并显示禁用原因，而不是等后台 job 启动后才失败。
- [x] 图片菜单还会拒绝阻塞任务期间的操作，以及源目录不可写时的删除、移动和 sidecar 写入；
      这些检查复用 control 层和文件操作的既有前提，不对尚未选择的目标目录作猜测。
- [x] Color Checker patch 右键已由专用 provider 接管，显示“复位 patch / 移除 patch”；菜单
      payload 保存索引与 Lab 原始/目标值，激活时若 patch 已变更则安全拒绝。
- [x] Filmic RGB 图表的离散右键语义已迁移为 provider：图表类型显示全部四种视图并标示当前项，
      坐标标签显示开关；左键循环和双击复位仍保留为快捷手势。
- [x] Color Picker 的 live sample 色块右键已改为 provider：载入到当前拾色器、锁定/解锁和移除
      全部通过同一 payload-validated Action；样本销毁后菜单项安全拒绝，左键锁定仍保留。
- [x] RGB Levels 的 auto-region 画布辅助点击已改为 Action 菜单：激活时显示“取消自动区域选择”，
      复用既有 `auto region` toggle 的显式 off effect；双击仍保留为快速取消手势。

仍需继续完成的范围：仍由绘制手势承载语义的 Darkroom 图元，以及最终运行时覆盖审计。
它们保持为下列阶段的未完成项；复杂画布的现有辅助点击在有专用 provider 前不被通用菜单
拦截。`history stack` 与
`actions on selection` 的整模块投影暂不启用：2026-07-20 的崩溃根因已修复，但这些模块的
对象命中、确认流程与专用状态仍需 provider，不能仅因 effect 枚举已安全就重新把整棵 Lib
Action 树加入缩略图菜单。

## 辅助点击覆盖审计（2026-07-20）

本次以 `GDK_BUTTON_SECONDARY`、GTK popup 和 `popup-menu` 为入口做了全仓静态盘点。下表的
“保留”不是遗漏：这些代码把右键定义为画布编辑手势，通用菜单不得抢占它。后续新增 provider
必须从这张表中选择一项，并补充命中测试、拖拽判定和 Action 元数据后才能改变状态。

| 对象类别 | 源码位置 | 现状 | 原因 / 后续边界 |
| --- | --- | --- | --- |
| 普通 Action 控件、Bauhaus、IOP/Lib 模块标题和空白主体 | `gui/accelerators.c`、`bauhaus/`、`develop/imageop.c`、`libs/lib.c` | 已覆盖 | `dt_action_define*()` 自动附加通用菜单；模块事件区域投影所属模块的完整离散 Action，按钮保留其对象专属手势。 |
| 图片缩略图、filmstrip、Culling、Preview | `dtgtk/thumbnail.c`、`dtgtk/thumbtable.c`、`dtgtk/culling.c` | 已覆盖 | Culling/Preview 复用同一事件实现；弹出时冻结图片 ID/选择集，并通过既有 job/history API 执行。 |
| Collection folder/filmroll | `libs/collect.c` | 已覆盖 | 行引用在激活前恢复正确行；保留原确认和数据库流程。 |
| Tagging attached/dictionary | `libs/tagging.c` | 已覆盖 | 行引用 payload 驱动 attach/detach/编辑/删除等既有业务回调。 |
| Styles | `libs/styles.c` | 已覆盖 | 仅样式叶子提供 apply/edit/export/remove；分类节点保留展开语义。 |
| History 条目 | `libs/history.c` | 已覆盖 | 辅助点击只打开 activate/focus provider，不再先执行历史跳转。 |
| Snapshot 条目 | `libs/snapshots.c` | 已覆盖 | 仅覆盖显示、恢复、重命名；画布分割线/旋转手势仍不拦截。 |
| Metadata 文本行 | `libs/metadata.c` | 已覆盖 | 保留 GTK 原生编辑项；动态历史值经 provider Action 写回。 |
| Mask 管理器树 | `libs/masks.c` | 已覆盖 | 保留形状、分组和布尔操作的专用层级菜单；所有菜单项经 RowReference-backed Tree Action 调用既有 history 路径。 |
| Timeline | `libs/tools/timeline.c` | 已覆盖 | 辅助点击展示“移除时间筛选”Action；时间轴主键拖拽仍是原有选择手势。 |
| Modulegroups 分组、active 与 quick-access tabs | `libs/modulegroups.c` | 已覆盖 | active 状态、分组模块和 quick-access widget 的添加/移除均经 Action；直接布局菜单保存可校验的领域快照，编辑预设对话框仍是其自身的显式编辑界面。 |
| 左右侧栏的模块可见性管理 | `gui/gtk.c` | 保留 | 侧栏空白处仍打开完整的显示/隐藏与“恢复默认”管理菜单；它管理当前视图的面板布局，不命中单一领域对象，后续若注册为 Action 才并入通用投影。 |
| RGB Curve、Tone Curve、Base Curve、Color Zones 节点 | `iop/{rgbcurve,tonecurve,basecurve,colorzones}.c` | 已覆盖 | 右键菜单提供节点删除/端点或节点重置和当前曲线复位；payload 为可校验的标量快照。 |
| Crop 画布 | `iop/crop.c` | 已覆盖 | 原有单一“复位裁剪”辅助点击改为同名 Action 菜单，不抢占拖拽。 |
| RGB Levels 自动区域选择 | `iop/rgblevels.c` | 已覆盖 | 激活区域选择后，画布辅助点击显示“取消自动区域选择”；菜单调用既有 toggle 的 off effect，不直接修改 picker 状态。 |
| Color Checker patch | `iop/colorchecker.c` | 已覆盖 | 命中 patch 后显示复位/移除菜单；参数快照校验避免在菜单存续期间误操作重排后的 patch。 |
| Filmic RGB 图表按钮 | `iop/filmicrgb.c` | 已覆盖 | 图表模式与坐标标签是离散 UI 配置，右键显示显式选择菜单；保留左键/双击快捷手势。 |
| 其他 Darkroom 画布、曲线和几何 | `views/darkroom.c`；`iop/{ashift,graduatednd,liquify}.c` | 保留 | 右键承担节点选择、取消、旋转或编辑；待专用图元 provider。 |
| 蒙版创建和路径编辑 | `develop/masks/{brush,circle,ellipse,gradient,path}.c` | 保留 | 右键是创建/关闭/删除点的手势，不能改为无命中上下文菜单。 |
| Color Picker live sample 色块 | `libs/colorpicker.c` | 已覆盖 | 右键展示载入、锁定/解锁与移除；payload 持有色块弱引用并在激活时重新确认样本仍在 live-sample 列表。 |
| 范围、渐变、颜色拾取画布 | `dtgtk/{range,gradientslider}.c`、`gui/color_picker_proxy.c` | 保留 | 需先区分数值编辑、颜色拾取和拖拽语义。 |
| Presets 子菜单 | `gui/presets.c` | 部分保留 | 模块标题/主体的完整菜单已提供预设入口；预设条目上的辅助点击仍是“应用到新 IOP 实例”，含多实例与长按判定，因此保留为对象专属手势。 |
| Filtering filename/misc | `libs/filters/{filename,misc}.c` | 保留 | 辅助点击打开带多选树、当前输入和可见性过滤的编辑 popover，不是固定命令菜单。 |
| Color-label 描述编辑 | `libs/tools/colorlabels.c` | 保留 | 辅助点击打开当前色标的文本编辑浮层；普通点击仍是批量标记 Action。 |
| Log history | `libs/tools/log_history.c` | 排除 | 打开应用日志面板，不是当前命中对象的离散业务操作。已移除的 Global toolbox 不再是上下文菜单候选。 |
| GTK 内部菜单、shortcut fallback | `gui/gtk.c`、`gui/accelerators.c` | 排除 | 不是产品对象操作，不能作为上下文菜单条目。 |

## 产品契约

### “全部可用操作”的定义

- **适用操作**：与右键命中的对象、所属模块、当前视图或当前图片选择有关的动作。
- **可执行操作**：适用且当前状态允许立即执行的动作。
- **暂不可执行操作**：仍与对象有关，但因没有选择、没有可粘贴内容、任务运行中或状态
  限制而不能执行。此类动作保留在菜单中并禁用，必要时提供简短原因，避免功能忽隐忽现。
- **无关操作**：与当前对象和其上下文无关的动作，不进入第一层菜单；当前视图的其他动作
  可进入“更多操作”子菜单。

菜单展示的是动作，而不是仅展示已经绑定快捷键的动作。已有快捷键显示在对应菜单项右侧，
未绑定快捷键的动作同样可见。

### 选择语义

图片类对象统一采用以下规则：

1. 右键已选中的图片时保留当前多选，图片操作作用于完整选择集。
2. 右键未选中的图片时先将其设为唯一选择，再创建菜单上下文。
3. 在 Darkroom filmstrip 中不得意外移除当前正在编辑的图片所需选择状态。
4. 菜单弹出时冻结目标图片列表和主图片 ID；菜单打开后鼠标移动、视图重绘或选择信号不得
   将操作转移到其他图片。
5. 空白区域只提供集合级操作，例如全选、取消选择和反选，不伪造图片目标。

### 菜单层级

菜单按相关性组织，禁止把当前视图的全部动作平铺成一个超长列表：

```text
对象直接操作
对象所属模块或容器操作
当前选择集操作
──────────────
更多操作 >
  当前视图操作
  全局操作（仅保留适合菜单调用的动作）
```

破坏性操作放在菜单底部并与普通操作分隔。删除、移出图库、覆盖和清除历史继续使用现有
确认对话框、撤销机制、数据库事务和任务队列，不在菜单层重新实现业务逻辑。

### 输入方式

- 支持鼠标右键和 macOS 触控板辅助点按。
- 支持 macOS Control+主键单击，前提是不与目标对象已有组合键语义冲突。
- 支持键盘菜单键或 Shift+F10；键盘调用时使用当前焦点对象和当前选择。
- 不改变已有快捷键的绑定和执行结果。

## 当前代码基础与缺口

现有 `dt_action_t` 已记录动作类型、ID、标签、目标、所有者和层级；
`src/gui/accelerators.c` 已具备动作树构建、视图过滤、IOP 多实例定位和统一执行逻辑。
Bauhaus、IOP、Lib、Lighttable 图片操作也已经注册了大量 element/effect。

需要补齐的能力：

- 面向调用方的只读 Action 枚举接口；
- 独立于“尝试执行”的适用性、可用性和当前状态查询；
- 菜单分组、排序、危险级别和是否允许出现在上下文菜单中的元数据；
- 从命中 Widget 向父级、模块、视图和具体对象解析上下文的统一入口；
- 对缩略图、树行、曲线节点、蒙版节点等非普通 GtkWidget 对象的 provider；
- 直接按 `dt_action_t`、实例、element 和 effect 执行的公共入口；
- 当前快捷键的人类可读描述查询；
- 菜单打开期间的目标快照、弱引用检查和重入保护。

不能简单遍历整个 Action 树后逐项调用现有 `process(..., DT_READ_ACTION_ONLY)` 判断可用性：
部分 command/button 动作不提供可靠只读结果，查询不得触发 GTK 信号、修改参数或创建历史。

## 设计边界

### Action 上下文

新增的上下文模型至少表达：

- 当前 view；
- 命中的 GtkWidget 及其最近 Action；
- Lib/IOP 所有者和准确的 IOP 实例；
- 对象类别，例如图片、模块、参数控件、树行、曲线节点或蒙版对象；
- 图片目标快照、主图片 ID 和选择数量；
- 特殊对象 provider 拥有的短生命周期 payload；
- 菜单调用来源，例如鼠标、触控板或键盘。

通用动作描述不应依赖图片、蒙版或数据库的具体结构。特殊 payload 的解释和释放均由注册
它的 provider 负责。

### Action 状态

状态查询至少返回：

- 是否适用于当前上下文；
- 是否启用；
- 普通、选中、未选中或混合状态；
- 可选禁用原因；
- 菜单分组、排序和危险级别。
- 是否只允许由携带对象 payload 的 provider 投影；这类命令不进入拥有者的通用菜单。

查询必须无副作用并在 GTK 主线程执行。菜单项激活前再次进行轻量有效性检查，避免菜单
打开期间视图切换、模块销毁或 Widget 消失后使用悬空目标。

### 执行边界

- 菜单只调用统一 Action 执行入口，不直接写 `self->params`、SQLite 表或共享选择状态。
- 领域操作继续通过已有 history、undo、cache、signal、job 和 transaction API 执行。
- 所有 GTK 菜单创建、状态更新和销毁都在主线程完成。
- 后台任务只接收菜单弹出时冻结的领域数据，不持有 GtkWidget 或菜单对象。
- Action 目标使用现有弱引用规则；目标失效时安全拒绝执行并关闭菜单。

### 右键事件策略

禁止在应用顶层无条件吞掉所有辅助点击。接入采用显式 provider，并遵守以下规则：

1. 普通 Widget、Bauhaus、模块标题和缩略图可以优先接入通用菜单。
2. 已有右键直接行为必须先登记为 Action，然后才能由菜单替代。
3. 菜单触发模拟辅助点击的旧 Action 时必须设置重入保护，不能再次打开菜单。
4. Darkroom 画布、蒙版、裁剪和曲线等交互区在专用 provider 完成前保持原行为。
5. 按下、移动、释放构成的右键拖拽不能被误判为菜单点击。

## 代码所有权建议

| 位置 | 责任 |
| --- | --- |
| `src/common/action.h` | 保持轻量 Action 基础类型；不引入 GTK 菜单对象或领域对象所有权。 |
| `src/gui/accelerators.[ch]` | Action 枚举、状态查询、快捷键描述、实例解析和统一执行入口。 |
| `src/gui/context_menu.[ch]` | 上下文生命周期、通用 GtkMenu 投影、分组、状态和重入保护。 |
| `src/bauhaus/`、`src/dtgtk/` | 控件与缩略图 provider，不复制领域操作。 |
| `src/libs/`、`src/views/` | 树行、模块和视图专用 provider。 |
| `src/develop/`、`src/iop/` | 最后接入 Darkroom 画布和特殊图形对象。 |

若实施中发现 `src/gui/context_menu.[ch]` 会反向拥有领域逻辑，应停止扩张通用层，把命中和
状态查询留在对应 provider 中。

## 分阶段实施

### Phase 0：动作与右键行为盘点

- [x] 增加只读审计输出，列出运行时 Action 树、类型、所有者、element/effect、目标状态和
      当前快捷键（重新初始化输入设备时写入用户配置目录的 `all_actions`）。
- [x] 盘点所有 `GDK_BUTTON_SECONDARY`、`button == 3`、GTK popup 和长按处理。
- [x] 将对象分为普通 Widget、Bauhaus、模块、图片、树行和 Darkroom 图形对象。
- [x] 为每个既有右键行为记录“保留直接行为 / 改为菜单项 / 暂不接入”。
- [x] 建立覆盖清单；无法自动暴露的动作必须记录原因，不能静默遗漏。

阶段出口：可以回答每个已注册 Action 属于哪个上下文、是否可执行，以及每个既有右键行为
由谁接管。

### Phase 1：Action 只读接口与状态模型

- [x] 将现有私有的定义查找、视图推导、完整标签/ID、element/effect 枚举和实例定位整理为
      有边界的内部公共 API。
- [x] 增加无副作用的 applicability/enabled/state 查询。
- [x] 增加按 Action 指针执行的入口，避免菜单重新拼接字符串路径。
- [x] 增加快捷键描述查询，并处理用户自定义绑定、无绑定和多个绑定。
- [x] 为 command、button、toggle、entry、value、preset、Lib、IOP 和多实例动作定义统一行为。
- [ ] 为状态过滤、排序、混合状态和无效目标添加不依赖真实窗口的单元测试。

当前仓库的 unit 集仅保留核心算法与数据库测试；右键菜单没有另行恢复 UI 测试目标。其状态和
生命周期通过 Action 审计、受影响 C/C++ 构建与手工交互矩阵验证。

阶段出口：不创建菜单也不改变任何输入行为，但测试能够枚举和调用当前 view 中指定对象的
动作。

### Phase 2：通用 GTK 菜单投影

- [x] 实现 `src/gui/context_menu.[ch]` 的菜单构造与生命周期。
- [x] Command 显示为普通项，Toggle 显示为勾选项，Combo 显示为单选子菜单。
- [x] Slider 提供输入值、重置、最小和最大；增减类连续动作不逐项制造噪声。
- [x] Entry 提供聚焦、清除等有意义的离散操作。
- [x] Lib/IOP 提供显示、启用、重置、预设和合法的多实例操作。
- [x] 显示快捷键、禁用状态、混合状态、分隔和危险级别。
- [x] 支持键盘调用和焦点恢复。

阶段出口：普通 GTK/Bauhaus 控件和模块标题可动态生成菜单，菜单执行结果与原快捷键一致。

### Phase 3：图片对象

- [x] 接入 Lighttable filemanager thumbtable。
- [x] 接入 Darkroom filmstrip，保留当前编辑图片和延迟单选语义。
- [x] 接入 Culling 和 Preview 的独立选择模型，并阻止辅助点击误启动平移（两种模式由
      `dt_culling_new()` 的同一事件处理器覆盖）。
- [x] 提供打开 Darkroom、评分、颜色标签、复制/粘贴/清除历史、创建副本、移动、复制、
      从图库移除和移到废纸篓等现有动作。
- [x] 对剪贴板为空、无有效图片、只读文件、任务冲突和不可操作格式正确禁用菜单项。
- [ ] 验证单选、多选、分组、折叠组、悬停模式和选择模式。

阶段出口：所有保留的图片级 Action 均可从对应图片上下文访问，目标在菜单打开期间保持
稳定。

### Phase 4：树、列表与元数据对象

- [x] 接入 Collection/Filtering 条目。
- [x] 接入 History、Styles 和 Snapshots 条目。
- [x] 接入 Tagging 和 Metadata 行。
- [ ] 将已有手写 popup 迁移为共享 Action 菜单或 provider；删除重复构造逻辑前确认没有
      丢失专用状态与确认流程。
- [ ] 对重命名、查找、复制、附加、分离和删除等对象级动作补充状态查询。

已完成的子项：

- [x] Collection 的 folder/filmroll provider。
- [x] Tagging 的 attached/dictionary provider。
- [x] Styles 的 style-leaf provider。
- [x] Darkroom History 和 Snapshots 的行 provider。
- [x] Metadata 文本行的原生编辑菜单与动态值 Action provider。
- [x] Mask 管理器树的 provider-owned Action 菜单；鼠标与菜单键共用同一菜单构造和 RowReference
      选择快照。

阶段出口：树形对象不再各自维护无法审计的孤立菜单，现有功能和快捷键保持一致。

### Phase 5：Darkroom 图形对象

- [ ] 逐个接入曲线节点、色阶节点、裁剪边框、渐变、蒙版形状和路径节点。
- [x] RGB Curve、Tone Curve、Base Curve 与 Color Zones 节点已先注册为 Action，再由专用
      provider 显示“删除节点/重置端点或节点/重置曲线”。
- [x] 四个 Curve/Color Zones provider 使用通道（如适用）、节点序号和坐标的短生命周期标量
      快照；不保存裸 GUI 指针，并在激活时复核曲线未变。
- [x] Crop 的非拖拽辅助点击通过“复位裁剪”Action 弹出单项菜单；它不携带画布坐标或 Crop
      模块指针，仍由 Action 的 GTK 弱目标解析模块实例。
- [x] Color Checker 命中 patch 已通过 provider 显示复位/移除操作；其已有双击复位和
      Shift+拾色替换手势仍保留。
- [x] Filmic RGB 图表按钮已通过 provider 显式列出图表视图和标签开关；它不拦截曲线绘制区或
      其他 Darkroom 手势。
- [ ] 区分点击、拖拽、创建中、编辑中和取消状态。
- [ ] 保证颜色拾取、旋转、蒙版创建/删除及 IOP 自定义画布事件不被通用菜单抢占。

阶段出口：Darkroom 已纳入覆盖清单的图形对象都能显示完整菜单；所有未接入对象都有明确
原因并保持原右键行为。

### Phase 6：覆盖审计与收尾

- [ ] 对比运行时 Action 审计与 provider 覆盖表，确认没有遗漏适用动作。
- [x] 全仓搜索并评估剩余手写 GtkMenu 和辅助点击回调；其保留/排除原因记录在上方覆盖表。
- [ ] 删除已经被共享菜单完全替代的重复代码，但不删除仍有专用语义的实现。
- [x] 更新 `README.md`、`DevDocs/GUI.md` 和 Action/IOP 开发说明。
- [x] 记录有意不进入菜单的动作及理由，例如连续拖拽内部步骤或仅供诊断的命令。

阶段出口：新增动作只需注册一次并提供状态元数据，即可同时被快捷键系统和正确的上下文
菜单发现；不再要求每个界面手写菜单。

## 首个可交付切片

第一轮开工只完成以下闭环，不碰 Darkroom 主画布：

1. Action 枚举、状态和执行 API；
2. 通用 GtkMenu 投影器；
3. Bauhaus slider/combobox、IOP 模块标题；
4. Lighttable 缩略图和 Darkroom filmstrip；
5. 评分、颜色标签、历史、创建副本、移出图库和删除操作；
6. 快捷键显示、键盘菜单调用和 macOS 辅助点按；
7. 单元测试、Debug/Release 构建和手工交互矩阵。

这个切片验收后，再决定先扩展树形对象还是 Culling/Preview。不得为了追求覆盖数字提前
拦截 Darkroom 主画布的辅助点击。

## 验证矩阵

### 自动验证

- Action 上下文过滤、排序和继承；
- view/Lib/IOP 实例解析；
- command、toggle、combo、slider 和 preset 菜单投影；
- enabled、checked、mixed、destructive 和无效目标状态；
- 目标快照不随全局 hover/selection 缓存变化；
- 菜单关闭、目标销毁和 view 切换后的生命周期；
- Action 审计中“适用但无 provider”的差异报告。

### 手工验证

- Lighttable filemanager、culling、preview；
- Darkroom filmstrip、模块标题和普通参数控件；
- 鼠标右键、触控板辅助点按、Control+单击和 Shift+F10；
- 无选择、单选、多选、分组、悬停和跨视图切换；
- 菜单打开时删除目标、切换图片、折叠模块和退出应用；
- 删除/移出/清除历史的确认、取消、错误和后台任务路径；
- 快捷键与菜单分别执行同一动作后，参数、history、undo 和最终 UI 状态一致；
- 无 GLib/GObject/Gtk critical、悬空回调、重复菜单或输入卡死。

每个涉及 C/C++ 的阶段至少运行：

```sh
cmake --preset mac_clang_debug -DBUILD_TESTING=ON
cmake --build --preset mac_clang_debug
ctest --test-dir build/mac_clang_debug --output-on-failure -L unit

cmake --build --preset mac_clang_release --target darktable
```

## 主要风险与控制

| 风险 | 控制措施 |
| --- | --- |
| 菜单过长 | 对象优先、分层显示，“更多操作”承载 view/global 动作。 |
| 右键旧行为被破坏 | 先盘点、后迁移；复杂画布使用显式 provider。 |
| 菜单递归打开 | 区分真实输入与 Action 合成事件，并设置执行期重入保护。 |
| 操作目标漂移 | 弹出时冻结选择和对象句柄，不依赖后续 hover 状态。 |
| Widget/模块销毁 | 使用弱引用和激活前有效性检查。 |
| 菜单状态不准确 | 独立、无副作用的状态查询；禁止靠试执行判断。 |
| 多选状态不一致 | 支持 mixed 状态，Action 负责定义批量操作语义。 |
| 领域逻辑进入菜单层 | 菜单只投影和分发，业务继续走现有公共 API。 |
| Action 注册不完整 | 运行时审计和 provider 覆盖差异作为阶段出口。 |
| UI 卡顿 | 静态描述可缓存，动态状态按需查询；禁止菜单构造执行重型数据库扫描。 |

## 明确不做

- 不恢复或继续构建现有 `TODO_QML_IMPORT.md` 中的 QML 应用外壳。
- 不引入 Qt、GeoControls、GTK4 或第二套 GUI 运行时。
- 不借此任务调整 IOP 算法、pixelpipe、OpenCL/Metal 或图像结果。
- 不新增数据库 schema，也不绕过现有 selection、history、undo 和 job API。
- 不为了“全部”而展示连续鼠标移动的每个内部步骤、调试命令或与对象无关的全局动作。
- 不一次性全局拦截 Darkroom 主画布右键。

## 完成标准

- 用户可以通过右键发现当前对象全部适用的离散操作，并看到对应快捷键与当前状态。
- 菜单、快捷键和现有按钮调用同一 Action 执行路径，不存在三份业务实现。
- 新增一个已注册且允许上下文展示的动作后，无需在多个菜单中手工复制。
- 图片和图形对象操作不会因菜单打开后的 hover、选择或生命周期变化作用到错误目标。
- 现有快捷键、历史、撤销、后台任务和 Darkroom 交互没有回归。
- 运行时覆盖审计无未解释缺口，所有有意排除项均在文档中列明原因。
