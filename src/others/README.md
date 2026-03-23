# others

基础数据结构与工具，被所有其他模块依赖。

## 文件说明

| 文件 | 说明 |
|------|------|
| `gt_types.h` | 全局类型定义：`gt_area_st`、`gt_point_st`、布局枚举、`GT_DPX/GT_DPY` 宏 |
| `gt_color.c/h` | 颜色类型与转换：`gt_color_hex()`、`gt_color_black()` 等 |
| `gt_area.c/h` | 区域运算：交集、并集、包含判断 |
| `gt_anim.c/h` | 动画引擎：`gt_anim_init/start`，支持 exec_cb / ready_cb |
| `gt_log.h` | 日志宏：`GT_LOGD/GT_LOGI/GT_LOGW/GT_LOGE` |
| `gt_math.c/h` | 数学工具：三角函数、插值等 |
| `gt_txt.c/h` | 文本工具：UTF-8/GBK 编解码、字符串操作 |
| `gt_ll.c/h` | 双向链表 |
| `gt_tlsf.c/h` | TLSF 内存分配器实现 |
| `gt_assert.h` | 断言宏 |
| `gt_defs.h` | 全局常量与宏定义 |
| `gt_crc.c/h` | CRC 校验 |
| `gt_gc.c/h` | 垃圾回收辅助 |

---

## 弹性适配 — 编译时分辨率缩放

### 原理

`gt_types.h` 中定义四个编译时缩放宏，将设计稿坐标按比例映射到实际屏幕坐标。
编译器对常量表达式做折叠，**MCU 运行时零额外开销**。

```c
#define GT_DPX(x)   ((x) * GT_SCREEN_WIDTH  / GT_DESIGN_WIDTH)
#define GT_DPY(y)   ((y) * GT_SCREEN_HEIGHT / GT_DESIGN_HEIGHT)
#define GT_DPW(w)   GT_DPX(w)
#define GT_DPH(h)   GT_DPY(h)
```

### 配置（gt_conf.h）

```c
// 目标硬件实际分辨率
#define GT_SCREEN_WIDTH   480
#define GT_SCREEN_HEIGHT  272

// UI 设计稿基准分辨率（原始坐标按此编写）
// 默认等于 GT_SCREEN_WIDTH/HEIGHT，即不缩放
#define GT_DESIGN_WIDTH   800
#define GT_DESIGN_HEIGHT  480
```

### 使用

```c
// UI 代码用宏写坐标，换分辨率只改 gt_conf.h，不改 UI 代码
gt_obj_set_pos(btn, GT_DPX(100), GT_DPY(50));
gt_obj_set_size(btn, GT_DPW(80), GT_DPH(40));

// 编译时展开（800→480）：
// GT_DPX(100) = 100 * 480 / 800 = 60
// GT_DPY(50)  = 50  * 272 / 480 = 28
```

### 注意事项

- 字体大小不参与自动缩放，需手动指定
- `GT_DESIGN_WIDTH/HEIGHT` 默认等于屏幕分辨率，现有项目无需修改
- 整数除法可能有 1px 误差，对 UI 布局影响可忽略
