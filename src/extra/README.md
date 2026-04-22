# extra

扩展功能，包含第三方库集成、混合渲染和串口协议层。

## 目录结构

```
extra/
├── draw/       # 像素混合渲染（按颜色格式分发）
├── gif/        # GIF 解码库
├── jpeg/       # SJPEG 解码库
├── png/        # PNG 解码库
├── qrcodegen/  # QR 码生成库
├── md4c/       # Markdown 解析库
├── libtexprintf/ # 文本格式化库
└── serial/     # 串口协议层（HMI 串口指令集）
```

## draw — 混合渲染

像素混合核心，按颜色格式分发：

| 文件 | 说明 |
|------|------|
| `gt_draw_blend.c/h` | 混合调度入口 |
| `gt_draw_blend_with_rgb565.c/h` | RGB565 格式 |
| `gt_draw_blend_with_rgb888.c/h` | RGB888 格式 |
| `gt_draw_blend_with_argb888.c/h` | ARGB8888 格式 |
| `gt_img_transform.c/h` | 图片旋转/缩放（需 `GT_USE_IMG_TRANSFORM`） |

### gt_img_transform — 图片旋转/缩放

**算法**：逆映射 + Q8 定点双线性插值，无 FPU 要求，无变换矩阵 malloc。

**角度单位**：0.1°（0~3599），例如 `900 = 90°`
**缩放单位**：256 = 1.0x，`128 = 0.5x`，`512 = 2.0x`

#### 两种使用模式

**模式 A：直接绘制（每帧解码）**

```c
// 适合单次绘制，内部自动解码 + 变换 + 混合
gt_img_transform_draw(draw_ctx, blend_dsc, dsc_img,
                      angle,    // 0.1° 单位
                      zoom,     // 256 = 1.0x
                      pivot_x, pivot_y);  // 相对图片左上角的旋转中心
```

**模式 B：预解码缓存（动画推荐）**

```c
// 第一步：解码到缓存（调用一次）
uint16_t w, h;
bool is_rom;
gt_opa_t * alpha = NULL;
gt_color_t * cache = gt_img_decode_to_cache(dsc, &w, &h, &is_rom, &alpha);

// 第二步：每帧用缓存绘制（无解码开销）
gt_img_transform_draw_cached(draw_ctx, coords, opa,
                             cache, alpha, w, h,
                             angle, zoom, pivot_x, pivot_y,
                             use_nn,    // true=最近邻(快), false=双线性(平滑)
                             masking);  // 可选圆角/边缘模糊遮罩

// 第三步：动画结束后释放（is_rom=true 时不需要 free）
if (!is_rom) gt_mem_free(cache);
if (alpha)   gt_mem_free(alpha);
```

#### 双线性插值原理

```
c00(x0,y0) --- c10(x1,y0)
     |               |
     +-----(fx)------+   top = mix(c10, c00, fx)
     |               |
c01(x0,y1) --- c11(x1,y1)
     |               |
     +-----(fx)------+   bot = mix(c11, c01, fx)
              |
           (fy) mix(bot, top, fy)  → 结果
```

`fx/fy` 为 Q8 小数部分（0=完全左/上，255=完全右/下）。
内部分 fast path（边界内，无 clamp）和 safe path（边界像素，有 clamp）。

#### 逆映射公式（Q8 定点）

```c
// 每行起始源坐标（pivot 为旋转中心）
int32_t sx_q8 = sin_z * ry + (pivot_x << 8) - cos_z * pivot_x;
int32_t sy_q8 = cos_z * ry + (pivot_y << 8) + sin_z * pivot_x;
// 每列步进
int32_t sx_step = cos_z >> 7;
int32_t sy_step = -(sin_z >> 7);
// cos_z/sin_z = cos/sin * 256 / zoom（已含缩放）
```

#### 格式转换（_decode_full_image）

`color_format`（bytes/pixel）决定解码路径：

| src_cf | depth=16 | depth=32 |
|--------|---------|---------|
| `GT_IMG_CF_RGB565`（2） | 直接 memcpy | 展开为 ARGB8888 |
| 其他 | 逐行 read_line | 逐行 read_line |

`need_conv = (src_cf != sizeof(gt_color_t))`，单位是 bytes/pixel，不是位深。

#### gt_color_masking_st（可选遮罩）

```c
typedef struct {
    gt_size_t radius;  // 圆角半径（0=不裁剪）
    gt_size_t gap;     // 边缘模糊宽度（0=不模糊）
} gt_color_masking_st;
```

传 `NULL` 表示无遮罩。内部通过 `gt_mask_radius` + `gt_mask_edge_blur` 实现，每行调用 `gt_mask_get` 修改 opa 缓冲。

#### 静态行缓冲

```c
static gt_color_t _s_out_line[GT_SCREEN_WIDTH];
static gt_opa_t   _s_out_opa[GT_SCREEN_WIDTH];
```

避免每帧 malloc/free，单线程 MCU 安全。大小由 `GT_SCREEN_WIDTH` 决定。

## svg — SVG 解码

基于 nanosvg + nanosvgrast（SVG Tiny 1.2）。

| 文件 | 说明 |
|------|------|
| `gt_svg.c/h` | SVG 解码器，注册到图片解码器框架 |
| `nanosvg.h` | SVG 解析（单头文件库） |
| `nanosvgrast.h` | SVG 光栅化（单头文件库） |

**颜色格式**：nanosvg 输出 RGBA8888，`_rgba_to_gt_color()` 原地转换：
- depth=32：RGBA→BGRA（R↔B 交换）
- depth=16：RGBA→RGB565（压缩为 2B/pixel）

`color_format` 字段必须与实际数据格式一致（depth=16 时为 `GT_IMG_CF_RGB565`），否则渲染引擎计算行步长错误。

## serial — HMI 串口协议

实现 GT-HMI 串口指令集，支持上位机通过串口控制 HMI 界面控件。

| 文件 | 说明 |
|------|------|
| `gt_serial_cfg.c/h` | 协议配置 |
| `gt_serial_command.c/h` | 指令解析与分发 |
| `gt_serial_event.c/h` | 串口事件处理 |
| `gt_serial_show.c/h` | 显示类指令 |
| `gt_serial_system.c/h` | 系统类指令 |
| `gt_serial_resource.c/h` | 资源类指令 |
| `gt_serial_value_passing.c/h` | 数值传递 |
| `gt_serial_var_*.c/h` | 各类变量控件指令（文本/数字/图标/进度条/滑块等） |
