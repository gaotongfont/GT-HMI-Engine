# core

GUI 内核，负责绘制、渲染、事件、布局、对象系统等核心逻辑。

## 文件说明

| 文件 | 说明 |
|------|------|
| `gt_draw.c/h` | 绘制引擎：文本、图形、图片绘制 |
| `gt_disp.c/h` | 显示管理：帧缓冲、脏区提交、suspend/resume |
| `gt_refr.c/h` | 帧刷新调度：定时器驱动，调用 `gt_layout_flush_dirty` |
| `gt_event.c/h` | 事件系统：事件分发、回调注册 |
| `gt_style.c/h` | 样式/坐标：`gt_obj_set_pos/size`，触发布局更新 |
| `gt_layout.c/h` | Flex 布局：`gt_layout_init`、`gt_layout_set_type`、`gt_layout_flush_dirty` |
| `gt_mem.c/h` | 内存管理：TLSF 堆分配封装 |
| `gt_img_decoder.c/h` | 图片解码：PNG/JPEG/BMP/C 数组 |
| `gt_fs.c/h` | 文件系统抽象 |
| `gt_indev.c/h` | 输入设备管理 |
| `gt_indev_effects.c/h` | 输入特效（涟漪等） |
| `gt_timer.c/h` | 软件定时器 |
| `gt_handler.c/h` | 消息处理器 |
| `gt_mask.c/h` | 裁剪遮罩 |
| `gt_obj_pos.c/h` | 对象位置计算辅助 |
| `gt_obj_scroll.c/h` | 滚动逻辑 |
| `gt_scroll_tip.c/h` | 滚动提示条 |
| `gt_scr_stack.c/h` | 屏幕栈管理（页面切换/动画） |
| `gt_graph_base.c/h` | 图形基础绘制 |

---

## 资源占用

### 内存池

使用 TLSF 分配器，O(1) 分配/释放，无堆碎片风险。

```c
#define GT_MEM_CUSTOM   1           // 1=TLSF 内存池，0=系统 malloc
#define GT_MEM_SIZE     (24*1024U)  // 默认 24 KB
```

### 帧缓冲区

大小公式：`W × H × sizeof(gt_color_t)`

| 模式 | 缓冲行数 | 680×480 RGB565 | 适用场景 |
|------|---------|----------------|---------|
| STYLE_0 | 无缓冲 | 0 | 极低资源 MCU |
| STYLE_1 | 10 行 | **13.3 KB** | Cortex-M0/M3 |
| STYLE_2 | 10+10 行双缓冲 | **26.6 KB** | 中等性能 MCU |
| STYLE_3 | 全屏 | **637.5 KB** | 高性能 / 外部 SRAM |

**RAM 总览（680×480 RGB565，内存池 24 KB）**

| 配置 | 合计 |
|------|------|
| 最小（STYLE_1，无截图缓存） | ≈ **37 KB** |
| 默认（STYLE_3，无截图缓存） | ≈ **662 KB** |
| 典型（STYLE_3 + 截图缓存） | ≈ **1.3 MB** |

### 可选 RAM 开销

| 功能 | 配置项 | 默认 | 额外 RAM |
|------|--------|------|---------|
| 顶层截图缓存 | `GT_LAYER_TOP_SCREENSHOT_CACHE` | 关 | 启用时 W×H×bpp（Dialog/Toast 依赖） |
| 软件旋转缓冲 | `GT_DISP_SW_ROTATE_SUPPORT` | 关 | W×H×bpp（静态分配） |
| 字形 LRU 缓存 | `GT_USE_FONT_GLYPH_CACHE` | 关 | 按字号×槽数 |
| 图片解码缓存 | `GT_USE_IMG_CACHE` | 关 | 用户提供缓冲区 |

### 单个控件对象

```
gt_obj_st 基础字段：~80 B
布局字段（GT_USE_WIDGET_LAYOUT）：+8 B
事件回调：每个 +24 B
子对象指针：每个 +8 B
```

典型单屏 20 个控件 ≈ **2.4 KB**（不含控件私有数据）

---

## 弹性适配 — Flex 布局

### 快速上手

```c
// 创建容器，添加子控件后启用 Flex
gt_obj_container_st ct = {
    .layout_type     = GT_LAYOUT_TYPE_FLEX,
    .flex_direction  = GT_LAYOUT_FLEX_DIR_ROW,
    .justify_content = GT_LAYOUT_JUSTIFY_CONTENT_SPACE_BETWEEN,
    .align_items     = GT_LAYOUT_ALIGN_ITEMS_CENTER,
};
gt_layout_init(bar, &ct);
```

### 运行时切换 FIXED ↔ FLEX

```c
// 切换到 FLEX：自动保存子元素相对坐标，重新排列
gt_layout_set_type(container, GT_LAYOUT_TYPE_FLEX);

// 切回 FIXED：精确恢复子元素原始相对坐标，无残影
gt_layout_set_type(container, GT_LAYOUT_TYPE_FIXED);
```

### 布局参数

| 参数 | 可选值 | 说明 |
|------|--------|------|
| `flex_direction` | `ROW` / `COLUMN` / `ROW_REVERSE` / `COLUMN_REVERSE` | 主轴方向 |
| `justify_content` | `START` / `END` / `CENTER` / `SPACE_BETWEEN` / `SPACE_AROUND` / `SPACE_EVENLY` | 主轴对齐 |
| `align_items` | `START` / `END` / `CENTER` | 副轴对齐 |
| `shrink` | `DISABLE` / `ENABLE` | 子元素是否收缩到父容器范围内 |
| `gap` | `uint8_t` | 子元素间距（像素） |

### 内部实现流程

```
gt_obj_set_pos/size
  └─ gt_layout_mark_dirty(parent)     // 标记脏，不立即计算

帧刷新前（gt_refr_timer）：
  └─ gt_layout_flush_dirty()          // 统一处理所有 dirty 容器
       └─ _adjust_flex_content()
            ├─ _gt_disp_refr_suspend()            // 阻断中间状态脏区
            ├─ _childs_sort_by_flex_direction()   // 按主轴排列
            ├─ _childs_adjust_justify_content()   // 主轴对齐调整
            └─ _gt_disp_refr_resume()             // 统一提交脏区
```

### API 汇总

```c
void gt_layout_init(gt_obj_st * obj, gt_obj_container_st const * const container);
void gt_layout_set_type(gt_obj_st * obj, enum gt_layout_type_e type);
void gt_layout_mark_dirty(gt_obj_st * obj);
void gt_layout_flush_dirty(void);
```
