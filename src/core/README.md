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
| `gt_img_decoder.c/h` | 图片解码框架：插件式解码器注册，支持 PNG/JPEG/SVG/RAM 数组 |
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
| `gt_gradient.c/h` | 渐变色描述符 + 3D 阴影描述符（`GT_USE_GRADIENT` / `GT_USE_SHADOW`） |

---

## 渐变色（GT_USE_GRADIENT）

### 功能开关

```c
// gt_conf.h 或编译宏
#define GT_USE_GRADIENT  1   // 0 = 禁用，节省代码体积
```

### 渐变方向

| 枚举 | 效果 |
|------|------|
| `GT_GRAD_DIR_HOR` | 左 → 右 |
| `GT_GRAD_DIR_VER` | 上 → 下 |
| `GT_GRAD_DIR_DIAG_TL` | 左上 → 右下 |
| `GT_GRAD_DIR_DIAG_TR` | 右上 → 左下 |
| `GT_GRAD_DIR_SYM_VER` | 中心 → 上下对称 |
| `GT_GRAD_DIR_SYM_HOR` | 中心 → 左右对称 |

### 快速上手

```c
// 双色水平渐变
gt_gradient_dsc_st * g = gt_gradient_dsc_create(GT_GRAD_DIR_HOR);
gt_gradient_dsc_add_stop(g, gt_color_hex(0x0000FF), gt_color_hex(0xFF0000), 60);
gt_rect_set_gradient(rect, g);

// 三色垂直渐变
gt_gradient_dsc_st * g = gt_gradient_dsc_create(GT_GRAD_DIR_VER);
gt_gradient_dsc_add_stop(g, gt_color_hex(0xFF0000), gt_color_hex(0xFFFF00), 30);
gt_gradient_dsc_add_stop(g, gt_color_hex(0xFFFF00), gt_color_hex(0x00FF00), 30);
gt_rect_set_gradient(rect, g);

// 彩虹色（预设 6 段 HSV 循环）
gt_rect_set_gradient(rect, gt_gradient_dsc_rainbow(GT_GRAD_DIR_HOR, 30));
```

### 抗色带（Floyd-Steinberg 抖动）

RGB565 显示器 R/B 仅 5bit（32 级），大面积渐变会出现明显阶梯色带。
`draw_bg_fill` 内部已对所有渐变方向启用 Floyd-Steinberg 误差扩散抖动：
- 理想颜色通过 `gt_gradient_dsc_get_color24_pos` 在 8bit 空间插值（避免 RGB565 预量化误差）
- 量化误差按 7/16、3/16、5/16、1/16 扩散到相邻像素
- 无需用户干预，自动生效

### API

```c
gt_gradient_dsc_st * gt_gradient_dsc_create(gt_grad_dir_et dir);
void gt_gradient_dsc_destroy(gt_gradient_dsc_st * dsc);
void gt_gradient_dsc_add_stop(gt_gradient_dsc_st * dsc, gt_color_t start, gt_color_t end, uint16_t steps);
void gt_gradient_dsc_set_loop(gt_gradient_dsc_st * dsc, uint8_t loop);
gt_gradient_dsc_st * gt_gradient_dsc_rainbow(gt_grad_dir_et dir, uint16_t steps_per_seg);
```

---

## 3D 阴影（GT_USE_SHADOW）

### 功能开关

```c
#define GT_USE_SHADOW  1   // 0 = 禁用
```

### 原理

阴影 = 偏移实心矩形，绘制在控件下方。无模糊，适合资源受限 MCU。

```
控件区域:                    阴影区域 (offset_x=4, offset_y=4, spread=2):
┌──────────┐                  ┌────────────┐
│  Widget  │──┐               │   Shadow   │
└──────────┘  │ ← 右侧可见    └────────────┘
     └────────┘ ← 下方可见

sh_area.x = widget.x + offset_x - spread
sh_area.y = widget.y + offset_y - spread
sh_area.w = widget.w + spread * 2
sh_area.h = widget.h + spread * 2
```

### 预设样式

```c
gt_shadow_dsc_st * sh = gt_shadow_dsc_create(GT_SHADOW_STYLE_BOTTOM_RIGHT);
gt_rect_set_shadow(rect, sh);
```

| 枚举 | offset | spread | opa | 适用场景 |
|------|--------|--------|-----|---------|
| `GT_SHADOW_STYLE_BOTTOM_RIGHT` | (4,4) | 0 | 50% | 经典 3D 按钮 |
| `GT_SHADOW_STYLE_BOTTOM` | (0,6) | 2 | 50% | 卡片底部阴影 |
| `GT_SHADOW_STYLE_AROUND` | (0,0) | 6 | 40% | 四周发光 / 浮层 |
| `GT_SHADOW_STYLE_DEEP` | (6,8) | 4 | 70% | 深度感强 |
| `GT_SHADOW_STYLE_SUBTLE` | (2,2) | 4 | 30% | 微弱扁平阴影 |
| `GT_SHADOW_STYLE_CUSTOM` | (4,4) | 0 | 50% | 自行修改字段 |

### 自定义

```c
gt_shadow_dsc_st * sh = gt_shadow_dsc_create(GT_SHADOW_STYLE_CUSTOM);
sh->offset_x = 8;
sh->offset_y = 8;
sh->spread   = 4;
sh->color    = gt_color_hex(0xFF6600);  // 彩色阴影
sh->opa      = GT_OPA_40;
gt_rect_set_shadow(rect, sh);
```

### 注意事项

- 阴影描述符由控件 `deinit_cb` 自动释放，无需手动 `gt_shadow_dsc_destroy`
- `draw_bg_fill` 内部需 `sh_dsc.reg.is_fill = 1`，已在 `draw_bg` 中正确设置
- 阴影颜色默认深灰 `0x404040`，可通过 `sh->color` 修改

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

---

## 输入设备（gt_indev）

引擎支持三种输入设备类型，通过 `gt_indev_drv_st.type` 区分，统一由 `gt_indev_handler()` 每帧调度。

### 三种类型对比

| 类型 | 枚举 | 典型硬件 | 核心数据 |
|------|------|---------|---------|
| 触摸/鼠标 | `GT_INDEV_TYPE_POINTER` | 电容屏、电阻屏、鼠标 | `data.point.x/y` + `state` |
| 键盘 | `GT_INDEV_TYPE_KEYPAD` | 矩阵键盘、编码器、PC 键盘 | `data.key` + `state` |
| 物理按键 | `GT_INDEV_TYPE_BUTTON` | 独立按键、侧键 | `data.btn_id` + `state` |

---

### GT_INDEV_TYPE_POINTER（触摸/鼠标）

**用途**：坐标输入，点击、滑动、抛掷滚动、手势识别。

**驱动 read_cb 规范**：

```c
static void _touch_read_cb(gt_indev_drv_st * drv, gt_indev_data_st * data) {
    data->point.x = touch_get_x();
    data->point.y = touch_get_y();
    data->state = touch_is_pressed() ? GT_INDEV_STATE_PRESSED : GT_INDEV_STATE_RELEASED;
    // 驱动无数据时返回 INVALID，引擎跳过
}
```

**注意事项**：
- `RELEASED` 只需上报一次，之后返回 `INVALID`
- 坐标在 `RELEASED` 时可保持最后一次有效值
- 支持软件旋转：`gt_disp_drv_st.rotated` 设置后引擎自动转换坐标

**事件序列**：
```
PRESSED → INPUT_PRESSED
PRESSED（持续 500ms）→ INPUT_LONG_PRESSED
PRESSED（持续，每 300ms）→ INPUT_PRESSING
RELEASED → INPUT_RELEASED 或 INPUT_PRESS_LOST（手指移出控件时）
```

---

### GT_INDEV_TYPE_KEYPAD（键盘）

**用途**：焦点导航（上下左右切换焦点）、Enter 确认、ESC 取消、文本输入。

**驱动 read_cb 规范**：

```c
static void _keypad_read_cb(gt_indev_drv_st * drv, gt_indev_data_st * data) {
    data->key   = get_current_key();   // GT_KEY_UP/DOWN/LEFT/RIGHT/ENTER/ESC/...
    data->state = key_is_pressed() ? GT_INDEV_STATE_PRESSED : GT_INDEV_STATE_RELEASED;
    // 无按键时返回 INVALID
}
```

**按键映射（`gt_key_et`）**：

| 按键值 | 功能 |
|--------|------|
| `GT_KEY_UP/DOWN/LEFT/RIGHT` | 焦点移动（需 `GT_USE_UD_LR_TO_CONTROL_FOCUS_EN=1`） |
| `GT_KEY_NEXT/PREV` | 焦点切换（Tab/Shift+Tab 风格） |
| `GT_KEY_ENTER` | 确认/激活焦点控件，触发 `INPUT_PRESSED` + `INPUT_RELEASED` |
| `GT_KEY_ESC` | 取消/退出锁定状态 |
| `GT_KEY_MENU/BACK` | 全局事件（F1/F12），由顶层捕获 |
| 其他字符键 | 发送 `INPUT_KEY` 事件，由控件自行处理（如 input 框） |

**焦点锁定机制（`GT_USE_UD_LR_TO_CONTROL_FOCUS_EN=1`）**：
- 未锁定：方向键移动焦点，Enter 锁定当前控件（`lock=1`）
- 锁定后：方向键事件传给控件内部（如 slider 调值），ESC 解锁
- 释放时：`lock` 自动清除，焦点恢复导航模式

**注意事项**：
- `RELEASED` 只上报一次后必须重置为 `INVALID`，否则引擎每帧都触发释放逻辑
- `PRESSED` 持续上报是必须的，引擎依赖它处理长按和 pressing 重复事件
- 同一时刻只能有一个 key 处于 PRESSED 状态（单键模型）

**事件序列（Enter 键）**：
```
PRESSED → INPUT_FOCUSED + INPUT_KEY + INPUT_PRESSED（lock=1）
PRESSED（持续 500ms）→ INPUT_LONG_PRESSED
PRESSED（持续，每 300ms）→ INPUT_PRESSING
RELEASED → INPUT_KEY_RELEASED + INPUT_RELEASED（lock 清除）
```

---

### GT_INDEV_TYPE_BUTTON（物理按键）

**用途**：独立物理按键，通过 `button_id` 映射到屏幕上的 `gt_btn` 控件，触发点击/长按效果。

**驱动 read_cb 规范**：

```c
static void _button_read_cb(gt_indev_drv_st * drv, gt_indev_data_st * data) {
    data->btn_id = get_pressed_button_id();  // 0~254，0xFF(GT_BTN_ID_INVALID) 表示无效
    data->state  = button_is_pressed() ? GT_INDEV_STATE_PRESSED : GT_INDEV_STATE_RELEASED;
    // 无事件时返回 INVALID
}
```

**button_id 映射**：

```c
// 给 btn 控件绑定物理按键 id
gt_btn_set_button_id(btn, 0);   // 物理按键 id=0 → 此控件

// 引擎查找优先级：
// 1. layer_top（Dialog/Toast 浮层）中按 button_id 查找
// 2. 当前屏幕中按 button_id 查找
// 3. 均无匹配 → fallback 到焦点控件
```

**注意事项**：
- `button_id` 范围 0~254，`0xFF` 为 `GT_BTN_ID_INVALID`（不参与 id 匹配）
- 驱动每次只上报一个按键（单键模型）
- 与 KEYPAD 不同：BUTTON 不涉及焦点导航，只做点击触发

**事件序列**：
```
PRESSED → INPUT_PRESSED
PRESSED（持续 500ms）→ INPUT_LONG_PRESSED
PRESSED（持续，每 300ms）→ INPUT_PRESSING
RELEASED → INPUT_RELEASED
```

---

### 驱动注册（gt_port_indev.c）

```c
void gt_port_indev_init(void) {
    static gt_indev_drv_st drv_touch;
    gt_indev_drv_init(&drv_touch);
    drv_touch.type    = GT_INDEV_TYPE_POINTER;
    drv_touch.read_cb = _touch_read_cb;
    gt_indev_drv_register(&drv_touch);

    // 键盘（可选）
    static gt_indev_drv_st drv_keypad;
    gt_indev_drv_init(&drv_keypad);
    drv_keypad.type    = GT_INDEV_TYPE_KEYPAD;
    drv_keypad.read_cb = _keypad_read_cb;
    gt_indev_drv_register(&drv_keypad);

    // 物理按键（可选）
    static gt_indev_drv_st drv_btn;
    gt_indev_drv_init(&drv_btn);
    drv_btn.type    = GT_INDEV_TYPE_BUTTON;
    drv_btn.read_cb = _button_read_cb;
    gt_indev_drv_register(&drv_btn);
}
```

**时间参数调整**：

```c
drv.limit_time_long_pressed = 500;   // 长按触发时间 ms（默认 500）
drv.limit_time_pressing     = 300;   // 长按后重复间隔 ms（默认 300）
```

---

## 刷新区域像素对齐（GT_REFR_AREA_ALIGN）

部分 LCD 控制器要求刷新区域的起始坐标和尺寸必须是特定像素值的整数倍（如 4 像素对齐）。

### 配置（gt_refr.h）

```c
#define GT_REFR_AREA_ALIGN_HOR          0   // 水平对齐开关
#define GT_REFR_AREA_ALIGN_HOR_PIXEL    4   // 水平对齐像素值
#define GT_REFR_AREA_ALIGN_VER          0   // 垂直对齐开关
#define GT_REFR_AREA_ALIGN_VER_PIXEL    4   // 垂直对齐像素值
```

### 公共函数

```c
// gt_refr.h — static inline，两个宏均为 0 时编译器优化掉
void gt_refr_area_align(gt_area_st * area);
```

两步法（HOR/VER 对称）：
1. 将起点向低地址对齐，同步补偿尺寸
2. 无条件将尺寸向上对齐到整数倍

### 调用位置

| 位置 | 说明 |
|------|------|
| `gt_refr_timer()` | 每帧取出脏区后对齐，再驱动绘制 |
| `gt_refr_force_flush()` | 强制刷新时对齐 |
| `gt_disp.c: _update_area_flush_by_calc_partly_redraw_area()` | Y 轴坐标映射后对齐（VER） |

---

## 图片解码框架（gt_img_decoder）

### 架构

插件式链表，每种格式注册一个 `_gt_img_decoder_st`，`gt_img_decoder_open` 遍历链表直到某个解码器成功。

```
gt_img_decoder_open(path)
  └─ 遍历 _gt_img_decoder_ll
       ├─ info_cb(path, &header)   // 识别格式，填 w/h/color_format
       └─ open_cb(dsc)             // 打开文件，填 dsc->fp / dsc->img
```

内置解码器（`_gt_img_decoder_built_in_*`）处理 RAM 数组和虚拟文件系统资源。
PNG/JPEG/SVG 各自在 `src/extra/` 中注册额外解码器。

### 关键数据结构

```c
/* 图片头信息 */
typedef struct _gt_img_info_s {
    uint32_t color_format : 5;  // @ref gt_img_color_format_et（bytes/pixel）
    uint32_t w : 13;            // 宽度（0~8191）
    uint32_t h : 13;            // 高度（0~8191）
    uint8_t  type;              // @ref gt_img_decoder_type_em
} _gt_img_info_st;

/* 图片描述符（open 后有效） */
typedef struct _gt_img_dsc_s {
    _gt_img_decoder_st * decoder;
    gt_fs_fp_st * fp;       // 文件对象
    uint8_t * img;          // RAM 图片数据指针（RAM 类型时有效）
    gt_opa_t * alpha;       // 独立 alpha 通道（可选）
    _gt_img_info_st header; // 宽/高/格式/类型
    /* ... */
} _gt_img_dsc_st;
```

### color_format 枚举（gt_img_color_format_et）

| 枚举值 | 数值 | bpp | 含义 |
|--------|------|-----|------|
| `GT_IMG_CF_UNKNOWN` | 0 | 0（自动） | 回退 RGB565，用于 PNG/SVG/JPEG |
| `GT_IMG_CF_RGB565` | 2 | 2 | 2 B/pixel |
| `GT_IMG_CF_RGB888` | 3 | 3 | 3 B/pixel |
| `GT_IMG_CF_BGRA8888` | 4 | 4 | 4 B/pixel |
| `GT_IMG_CF_RGBA8888` | 5 | **4** | 4 B/pixel（枚举≠bpp） |
| `GT_IMG_CF_RGB565_SWAP` | 6 | **2** | 2 B/pixel（枚举≠bpp） |

> **注意**：`RGBA8888` 和 `RGB565_SWAP` 的枚举值 ≠ bpp。必须通过 `gt_img_cf_bpp()`（`gt_img_decoder.h`）获取实际 bytes/pixel，**禁止**直接用枚举值做 stride 计算。

`_gt_img_decoder_built_in_read_line` 通过 `gt_img_cf_bpp()` 获取 stride：
```c
uint8_t bpp = gt_img_cf_bpp(dsc->header.color_format);
if (bpp == 0) bpp = GT_IMG_CF_RGB565;   // 默认 RGB565
uint32_t offset = (w * y + x) * bpp;
```

#### RGB565 / RGB565_SWAP — per-byte bit layout

```
 RGB565 (LE, GT_COLOR_16_SWAP=0):
  Byte[0]              Byte[1]
 ┌──────┬─────┐       ┌─────┬──────┐
 │ R4   │ G5  │       │ G2  │ B4   │
 │ R3   │ G4  │       │ G1  │ B3   │
 │ R2   │ G3  │       │ G0  │ B2   │
 │ R1   │──G5─│       │──G3─│ B1   │
 │ R0   │ G4  │       │ G2  │ B0   │
 └──────┴─────┘       └─────┴──────┘
 LE uint16: 0b_RRRRR_GGGGGG_BBBBB

 RGB565_SWAP (SPI LCD):
  Byte[0]              Byte[1]
 ┌─────┬──────┐       ┌──────┬─────┐
 │ G2  │ B4   │       │ R4   │ G5  │
 │ G1  │ B3   │       │ R3   │ G4  │
 │ G0  │ B2   │       │ R2   │ G3  │
 │──G3─│ B1   │       │ R1   │──G5─│
 │ G2  │ B0   │       │ R0   │ G4  │
 └─────┴──────┘       └──────┴─────┘
```

**SWAP 与正常 RGB565 不是简单的 `htons()` 关系。** SWAP 是将每个 uint16 拆成两个字节后交换位置，适用于 SPI LCD 控制器的字节序要求。

#### RGB888 — 3 B/px, BGR order

```
  Byte[0]   Byte[1]   Byte[2]
 ┌───────┐ ┌───────┐ ┌───────┐
 │ Blue  │ │ Green │ │ Red   │
 │ 8-bit │ │ 8-bit │ │ 8-bit │
 └───────┘ └───────┘ └───────┘
```

BGR 字节序匹配 `gt_color32_t` 的 `{blue, green, red}` 通道顺序，depth=32 时可直接 memcpy 到 `gt_color32_t`（alpha 补 0xFF）。

#### BGRA8888 / RGBA8888 — 4 B/px

```
 BGRA8888 (matches gt_color32_t):
  Byte[0]   Byte[1]   Byte[2]   Byte[3]
 ┌───────┐ ┌───────┐ ┌───────┐ ┌───────┐
 │ Blue  │ │ Green │ │ Red   │ │ Alpha │
 │ 8-bit │ │ 8-bit │ │ 8-bit │ │ 8-bit │
 └───────┘ └───────┘ └───────┘ └───────┘
 LE uint32: 0xAARRGGBB

 RGBA8888 (PNG/OpenGL native):
  Byte[0]   Byte[1]   Byte[2]   Byte[3]
 ┌───────┐ ┌───────┐ ┌───────┐ ┌───────┐
 │ Red   │ │ Green │ │ Blue  │ │ Alpha │
 │ 8-bit │ │ 8-bit │ │ 8-bit │ │ 8-bit │
 └───────┘ └───────┘ └───────┘ └───────┘
 LE uint32: 0xAABBGGRR  → 需 R↔B swap 转为 BGRA
```

RGBA8888 是 PNG 解码器和 OpenGL 的原生输出格式。depth=32 时引擎在 `gt_draw.c` 中通过 `_cf_rgba_to_32()` 做逐像素 R↔B 交换。

#### is_alpha=1 分离 alpha 布局

```
 |<── color_data = w × h × bpp ──>|<── alpha_data = w × h ──>|
 |  BGR888 pixel[0..N-1]          |  alpha[0..N-1]            |
```

- `color_offset` 从 0 开始，`color_stride = w × bpp`
- `alpha_offset = w × h × gt_img_cf_bpp(cf)`，`alpha_stride = 1 B/px`
- 仅 color_data 部分按 color_format 解析，alpha 部分始终 8-bit 灰度

### 解码器类型枚举（gt_img_decoder_type_em）

| 枚举 | 说明 |
|------|------|
| `GT_IMG_DECODER_TYPE_NONE` | 未识别 |
| `GT_IMG_DECODER_TYPE_PNG` | PNG 格式 |
| `GT_IMG_DECODER_TYPE_SJPG` | JPEG/SJPG 格式 |
| `GT_IMG_DECODER_TYPE_RAM` | RAM/ROM 数组 |
| `GT_IMG_DECODER_TYPE_SVG` | SVG 格式 |

### 主要 API

```c
/* 查询图片信息（不打开文件） */
gt_res_t gt_img_decoder_get_info(const char * name, _gt_img_info_st * header);
gt_res_t gt_img_decoder_get_info_raw(const char * raw_data, uint32_t raw_len, _gt_img_info_st * header);

/* 打开图片（路径 / RAM 数组 / 文件头 / 直接地址） */
gt_res_t gt_img_decoder_open(_gt_img_dsc_st * dsc, const char * name);
gt_res_t gt_img_decoder_open_raw(_gt_img_dsc_st * dsc, const char * raw_data, uint32_t data_len);

/* 逐行读取像素数据 */
gt_res_t gt_img_decoder_read_line(_gt_img_dsc_st * dsc, gt_size_t x, gt_size_t y, gt_size_t len, uint8_t * buf);

/* 关闭 */
gt_res_t gt_img_decoder_close(_gt_img_dsc_st * dsc);

/* 注册自定义解码器 */
_gt_img_decoder_st * gt_img_decoder_create(void);
void gt_img_decoder_register(_gt_img_decoder_st * decoder);
void gt_img_decoder_set_info_cb(_gt_img_decoder_st *, gt_img_decoder_get_info_t);
void gt_img_decoder_set_open_cb(_gt_img_decoder_st *, gt_img_decoder_open_t);
void gt_img_decoder_set_read_line_cb(_gt_img_decoder_st *, gt_img_decoder_read_line_t);
void gt_img_decoder_set_close_cb(_gt_img_decoder_st *, gt_img_decoder_close_t);
```

### 扩展访问模式（条件编译）

| 宏 | 访问方式 | 对应 API |
|----|---------|---------|
| `GT_USE_FILE_HEADER` | 文件头模式（资源包偏移） | `gt_img_decoder_fh_open` |
| `GT_USE_DIRECT_ADDR` | 直接地址模式（Flash 绝对地址） | `gt_img_decoder_direct_addr_open` |
| `GT_USE_DIRECT_ADDR_CUSTOM_SIZE` | 自定义尺寸直接地址 | `gt_img_decoder_custom_size_addr_open` |

### 注册自定义解码器示例

```c
static gt_res_t _my_info_cb(struct _gt_img_decoder_s * dec, const void * src, _gt_img_info_st * hdr) {
    if (!strstr((const char *)src, ".myimg")) return GT_RES_INV;
    hdr->w = MY_IMG_W; hdr->h = MY_IMG_H;
    hdr->color_format = GT_IMG_CF_RGB565;
    hdr->type = GT_IMG_DECODER_TYPE_RAM;
    return GT_RES_OK;
}

void my_decoder_register(void) {
    _gt_img_decoder_st * dec = gt_img_decoder_create();
    gt_img_decoder_set_info_cb(dec, _my_info_cb);
    gt_img_decoder_set_open_cb(dec, _my_open_cb);
    gt_img_decoder_set_read_line_cb(dec, _my_read_line_cb);
    gt_img_decoder_set_close_cb(dec, _my_close_cb);
    gt_img_decoder_register(dec);
}
```

---

## gt_draw_arc() 结构

`gt_draw_arc()` 拆分为三个函数，主函数约 55 行：

| 函数 | 说明 |
|------|------|
| `_arc_geo_st` | 几何上下文结构体（角度、半径、mask 等） |
| `_arc_draw_caps_grad()` | 渐变路径端点圆绘制，含短弧颜色插值 |
| `_arc_draw_solid()` | 实色/图片路径绘制 |
| `gt_draw_arc()` | 主函数：初始化 geo，分发到对应路径 |

渐变弧端点圆颜色取自 `gt_gradient_dsc_get_color_pos()`（起点 pos=0，终点 pos=65535）。
短弧（两端点距离 < `2×width`）时，终点圆颜色向起点颜色插值，消除硬边界。
