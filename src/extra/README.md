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
