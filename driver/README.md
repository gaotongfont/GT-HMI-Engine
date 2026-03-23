# driver

平台移植层，移植到新硬件时只需修改此目录。

## 文件说明

| 文件 | 说明 |
|------|------|
| `gt_port_disp.c/h` | 显示驱动：注册帧缓冲，实现 flush 回调 |
| `gt_port_indev.c/h` | 输入设备：注册触摸/鼠标，实现 read 回调 |
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

**3. 字体配置** (`gt_font_config.c`)

配置 family ID 与字体文件的映射，`gt_btn_set_font_family(btn, 2)` 即使用 family 2 的字体。
