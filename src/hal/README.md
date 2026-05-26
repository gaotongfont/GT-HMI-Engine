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

## gt_src_st / gt_vfs_st color_format 字段

`color_format` 枚举（`@ref gt_img_color_format_et`），**不是位深**：

| 枚举值 | 数值 | bpp | 含义 |
|--------|------|-----|------|
| `GT_IMG_CF_UNKNOWN` | 0 | 0（自动） | 回退 RGB565，用于 PNG/SVG/JPEG |
| `GT_IMG_CF_RGB565` | 2 | 2 | 原始 RGB565 数组 |
| `GT_IMG_CF_RGB888` | 3 | 3 | 原始 RGB888 数组（BGR 字节序） |
| `GT_IMG_CF_BGRA8888` | 4 | 4 | 原始 BGRA8888 数组 |
| `GT_IMG_CF_RGBA8888` | 5 | **4** | 原始 RGBA8888 数组（注意枚举≠bpp） |
| `GT_IMG_CF_RGB565_SWAP` | 6 | **2** | RGB565 字节互换（SPI LCD），枚举≠bpp |

> **注意**：`RGBA8888` 和 `RGB565_SWAP` 的枚举值 ≠ bpp。代码中通过 `gt_img_cf_bpp()`（`gt_img_decoder.h`）做映射，**不要**直接用枚举值当 bytes/pixel 计算内存偏移。

该字段经 `gt_hal_src.c` → `gt_img_decoder.c` 传递，引擎通过 `gt_img_cf_bpp()` 获取实际步长（stride）。填错会导致颜色错乱。

#### 各格式字节布局参考

**RGB565 (value=2, bpp=2)** — 小端 uint16：

```
 Byte[0]: [R4 R3 R2 R1 R0][G5 G4 G3]  =  R[4:0] << 3 | G[5:3]
 Byte[1]:           [G2 G1 G0][B4 B3 B2 B1 B0]  =  G[2:0] << 5 | B[4:0]
 LE uint16: 0b_RRRRR_GGGGGG_BBBBB
```

**RGB565_SWAP (value=6, bpp=2)** — SPI LCD 字节互换：

```
 Byte[0]: [G2 G1 G0][B4 B3 B2 B1 B0]  → 与 RGB565 Byte[1] 相同
 Byte[1]: [R4 R3 R2 R1 R0][G5 G4 G3]  → 与 RGB565 Byte[0] 相同
```

**RGB888 (value=3, bpp=3)**：

```
 Byte[0]=Blue, Byte[1]=Green, Byte[2]=Red  （无 Alpha）
```

**BGRA8888 (value=4, bpp=4)**：

```
 Byte[0]=Blue, Byte[1]=Green, Byte[2]=Red, Byte[3]=Alpha
 LE uint32: 0xAARRGGBB
```

**RGBA8888 (value=5, bpp=4)**：

```
 Byte[0]=Red, Byte[1]=Green, Byte[2]=Blue, Byte[3]=Alpha
 LE uint32: 0xAABBGGRR  → 引擎自动 R↔B 交换
```

**is_alpha=1 分离 alpha 布局**：

```
|<──  color_data = w×h×bpp  ──>|<──  alpha_data = w×h  ──>|
```

alpha 偏移 = `w × h × gt_img_cf_bpp(cf)`，alpha 部分每像素 1 字节（8-bit 灰度）。

> 详细 bit 级布局图见 `gt_img_decoder.h` 枚举 Doxygen 注释和 `src/core/README.md`。
