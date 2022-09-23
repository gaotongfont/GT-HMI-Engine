# GT-GUI 快速上手移植手册

Copyright (c) 2014-2022, Company Genitop. Co., Ltd.

Genitop GUI 快速上手移植，运行框架的指南手册。

## 1 最低运行环境

+ 16、32或64位微控制器（处理器）
+ 时钟频率：建议使用大于或等于72MHz
+ 编译器：建议C99及以上
+ ROM：建议大于或等于64KB（拓展使用扩展功能组件，建议大于或等于128KB）
+ RAM：
  + 堆（含动态数据）：建议大于或等于8KB（使用多个对象，建议使用大于或等于24KB），可通过gt_conf.h中配置堆内存大小
  + 显示缓存：根据显示方式，分为三种刷新方式；占用空间等于或大于 [显示行数(推荐10行) x 显示屏水平分辨率 x 单像素的色深] (Byte)
  + ---
  + 静态RAM：取决于用户使用功能和对象类型
  + MCU或外部显示控制器中的帧缓冲区

## 2 目录结构

GUI顶层目录结构:

>
> |-driver/     -> 移植驱动底层
>
> |-src/        -> gui 代码
>
> |-examples/   -> gui的使用示例
>

### 2.1 driver说明

实现包含GUI应用层可能需要用到的硬件资源接口，提供应用层显示、输入、定时器等硬件资源。

> driver/ 目录下的文件:
>
> gt_port_disp.c/h    显示屏的硬件刷屏接口
>
> gt_port_indev.c/h   输入设备的硬件通讯接口
>
> gt_port_src.c/h     素材资源管理(本地RAM数组模式)
>
> gt_port_vf.c/h      素材资源管理(外接Flash模式)

#### gt_port_disp.h 文件

三种缓冲模式:

```c

/* GT_REFRESH_STYLE type don't change */
#define GT_REFRESH_STYLE_0      0   /* no cache buf, It is suitable for MCU with low refresh requirements and low performance */
#define GT_REFRESH_STYLE_1      1   /* cache 10 lines buf, It is suitable for MCU with low refresh requirements and low performance */
#define GT_REFRESH_STYLE_2      2   /* cache 10+10 lines buf, It is suitable for MCU with high refresh requirements and high performance */
#define GT_REFRESH_STYLE_3      3   /* cache all screen buf, It is suitable for MCU with high refresh requirements and high performance */


/* 显示屏的刷新缓冲区模式 */
#if(GT_REFRESH_STYLE == GT_REFRESH_STYLE_0)

#elif(GT_REFRESH_STYLE == GT_REFRESH_STYLE_1)
    static gt_color_t buf1[GT_SCREEN_WIDTH * GT_REFRESH_FLUSH_LINE_PRE_TIME];
#elif(GT_REFRESH_STYLE == GT_REFRESH_STYLE_2)
    static gt_color_t buf1[GT_SCREEN_WIDTH * GT_REFRESH_FLUSH_LINE_PRE_TIME];
    static gt_color_t buf2[GT_SCREEN_WIDTH * GT_REFRESH_FLUSH_LINE_PRE_TIME];
#elif(GT_REFRESH_STYLE == GT_REFRESH_STYLE_3)
    static gt_color_t buf_all[GT_SCREEN_WIDTH * GT_SCREEN_HEIGHT];
#endif

```

1. 每次LCD更新10行的显示刷新模式，单缓冲区工作方式；（行数可修改，下同）
2. 每次LCD更新10行的显示刷新模式，双缓冲区工作方式；
3. LCD全屏的显示刷新模式，单缓冲区工作模式。

```c
    void (* flush_cb)(struct _gt_disp_drv_s * drv, gt_area_st * area, gt_color_t * color);
```


### 2.2 src说明
