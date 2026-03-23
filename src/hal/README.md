# hal

硬件抽象层，隔离平台差异。上层代码通过 HAL 接口访问硬件，移植时只需实现 `driver/` 下对应的 port 文件。

## 文件说明

| 文件 | 说明 |
|------|------|
| `gt_hal_disp.c/h` | 显示 HAL：帧缓冲注册、脏区管理 |
| `gt_hal_indev.c/h` | 输入设备 HAL：触摸/鼠标/键盘事件读取 |
| `gt_hal_fs.c/h` | 文件系统 HAL：文件读写抽象 |
| `gt_hal_src.c/h` | 资源 HAL：图片/字体资源加载抽象 |
| `gt_hal_tick.c/h` | Tick HAL：系统时间基准，`gt_tick_get()` |
| `gt_hal_vf.c/h` | 虚拟文件系统 HAL |
| `gt_hal_file_header.c/h` | 资源文件头解析 |

## HAL 与 driver 对应关系

| HAL | driver port |
|-----|-------------|
| `gt_hal_disp` | `driver/gt_port_disp.c` |
| `gt_hal_indev` | `driver/gt_port_indev.c` |
| `gt_hal_src` | `driver/gt_port_src.c` |
| `gt_hal_vf` | `driver/gt_port_vf.c` |
