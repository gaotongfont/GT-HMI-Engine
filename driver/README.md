# driver

平台移植层，移植到新硬件时只需修改此目录。

## 文件说明

| 文件 | 说明 |
|------|------|
| `gt_port_disp.c/h` | 显示驱动：注册帧缓冲，实现 flush 回调 |
| `gt_port_indev.c/h` | 输入设备：注册触摸/键盘/按键，实现 read 回调 |
| `gt_port_src.c/h` | 资源驱动：实现图片/字体文件读取（RAM 数组模式） |
| `gt_port_vf.c/h` | 虚拟文件系统：实现外接 Flash 资源读取 |
| `gt_font_config.c/h` | 字体配置：family 表、字体文件路径 |
| `gt_gui_driver.c/h` | 驱动初始化入口，统一调用各 port 注册函数 |

## 移植步骤

**1. 显示驱动** (`gt_port_disp.c`)

```c
static void _flush_cb(gt_disp_drv_st * drv, gt_area_st * area, gt_color_t * color) {
    lcd_draw_bitmap(area->x, area->y, area->w, area->h, (uint16_t *)color);
    gt_disp_flush_ready(drv);
}
```

**2. 输入驱动** (`gt_port_indev.c`)

```c
static void _read_cb(gt_indev_drv_st * drv, gt_indev_data_st * data) {
    data->point.x = touch_get_x();
    data->point.y = touch_get_y();
    data->state   = touch_is_pressed() ? GT_INDEV_STATE_PRESSED : GT_INDEV_STATE_RELEASED;
}
```

**3. 资源索引** (`gt_port_src.c` / `gt_port_vf.c`)

`gt_src_st` / `gt_vfs_st` 的 `color_format` 字段必须与实际数据格式一致（`@ref gt_img_color_format_et`）：

| 数据类型 | color_format 值 |
|---------|----------------|
| 原始 RGB565 数组 | `GT_IMG_CF_RGB565`（2） |
| 原始 RGB888 数组 | `GT_IMG_CF_RGB888`（3） |
| 原始 ARGB8888 数组 | `GT_IMG_CF_ARGB8888`（4） |
| PNG / SVG / JPEG 文件格式 | `GT_IMG_CF_UNKNOWN`（0，解码器自行设置） |

`gt_port_src.c` 需显式 `#include "../src/core/gt_img_decoder.h"` 才能使用枚举（该文件不包含 `gt.h`）。

**4. 字体配置** (`gt_font_config.c`)

配置 family ID 与字体文件的映射，`gt_btn_set_font_family(btn, 2)` 即使用 family 2 的字体。

---

## 输入设备类型

### GT_INDEV_TYPE_POINTER（触摸/鼠标）

适用：电容屏、电阻屏、鼠标。

```c
data->point.x = touch_get_x();
data->point.y = touch_get_y();
data->state   = touch_is_pressed() ? GT_INDEV_STATE_PRESSED : GT_INDEV_STATE_RELEASED;
// 无数据时返回 GT_INDEV_STATE_INVALID
```

- `RELEASED` 只上报一次，之后返回 `INVALID`
- `PRESSED` 持续上报，引擎靠它处理长按和滚动

### GT_INDEV_TYPE_KEYPAD（键盘）

适用：矩阵键盘、编码器、PC 键盘。

```c
data->key   = get_current_key();   // GT_KEY_UP/DOWN/LEFT/RIGHT/ENTER/ESC/...
data->state = key_is_pressed() ? GT_INDEV_STATE_PRESSED : GT_INDEV_STATE_RELEASED;
// 无按键时返回 INVALID
```

| 按键值 | 功能 |
|--------|------|
| `GT_KEY_UP/DOWN/LEFT/RIGHT` | 焦点移动（需 `GT_USE_UD_LR_TO_CONTROL_FOCUS_EN=1`） |
| `GT_KEY_NEXT/PREV` | 焦点切换（Tab/Shift+Tab 风格） |
| `GT_KEY_ENTER` | 确认/激活焦点控件，触发 press color 效果 |
| `GT_KEY_ESC` | 取消/退出锁定状态 |
| `GT_KEY_MENU/BACK` | 全局事件（F1/F12） |

- `RELEASED` 上报一次后必须重置为 `INVALID`，否则引擎每帧重复触发释放
- `PRESSED` 持续上报同一 key，引擎靠 `last_key` 去重

### GT_INDEV_TYPE_BUTTON（物理按键）

适用：独立按键、侧键，通过 `button_id` 映射到 `gt_btn` 控件。

```c
data->btn_id = get_pressed_button_id();  // 0~254，0xFF 表示无效
data->state  = button_is_pressed() ? GT_INDEV_STATE_PRESSED : GT_INDEV_STATE_RELEASED;
// 无事件时返回 INVALID
```

控件侧绑定：
```c
gt_btn_set_button_id(btn, 0);  // 物理按键 id=0 → 此 btn 控件
```

引擎查找顺序：layer_top → 当前屏幕 → fallback 焦点控件。

### 完整注册示例

```c
void gt_port_indev_init(void) {
    static gt_indev_drv_st drv_touch;
    gt_indev_drv_init(&drv_touch);
    drv_touch.type    = GT_INDEV_TYPE_POINTER;
    drv_touch.read_cb = _touch_read_cb;
    gt_indev_drv_register(&drv_touch);

    static gt_indev_drv_st drv_keypad;
    gt_indev_drv_init(&drv_keypad);
    drv_keypad.type    = GT_INDEV_TYPE_KEYPAD;
    drv_keypad.read_cb = _keypad_read_cb;
    gt_indev_drv_register(&drv_keypad);

    static gt_indev_drv_st drv_btn;
    gt_indev_drv_init(&drv_btn);
    drv_btn.type    = GT_INDEV_TYPE_BUTTON;
    drv_btn.read_cb = _button_read_cb;
    gt_indev_drv_register(&drv_btn);
}
```

时间参数（可选）：
```c
drv.limit_time_long_pressed = 500;   // 长按触发 ms（默认 500）
drv.limit_time_pressing     = 300;   // 长按后重复间隔 ms（默认 300）
```
