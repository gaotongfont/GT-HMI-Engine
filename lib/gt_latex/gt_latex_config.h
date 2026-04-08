/**
 * @file gt_latex_config.h
 * @brief GT-Latex 编译期配置 / Compile-time configuration
 */

#ifndef _GT_LATEX_CONFIG_H_
#define _GT_LATEX_CONFIG_H_

/* ----------------------------------------------------------------
 * 顶层总开关 / Top-level enable switch
 * 设为 0 可关闭整个 GT-Latex 库 / Set to 0 to disable the whole library
 * ---------------------------------------------------------------- */
#ifndef GT_LATEX_ENABLE
    #define GT_LATEX_ENABLE     1
#endif

#if GT_LATEX_ENABLE

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

/* ----------------------------------------------------------------
 * 3级模式默认字号（像素）/ Default font sizes for 3-level mode (pixels)
 * ---------------------------------------------------------------- */
#ifndef GT_LATEX_FONT3_SIZE_L0
    #define GT_LATEX_FONT3_SIZE_L0  24  /* 基础字体 / Base font */
#endif

#ifndef GT_LATEX_FONT3_SIZE_L1
    #define GT_LATEX_FONT3_SIZE_L1  16  /* 一级脚本 / Script level 1 */
#endif

#ifndef GT_LATEX_FONT3_SIZE_L2
    #define GT_LATEX_FONT3_SIZE_L2  12  /* 二级脚本 / Script level 2 */
#endif

/* ----------------------------------------------------------------
 * 5级模式默认字号（像素）/ Default font sizes for 5-level mode (pixels)
 * ---------------------------------------------------------------- */
#ifndef GT_LATEX_FONT5_SIZE_L0
    #define GT_LATEX_FONT5_SIZE_L0  32  /* 大型运算符 / Large operator */
#endif

#ifndef GT_LATEX_FONT5_SIZE_L1
    #define GT_LATEX_FONT5_SIZE_L1  24  /* 基础字体 / Base font */
#endif

#ifndef GT_LATEX_FONT5_SIZE_L2
    #define GT_LATEX_FONT5_SIZE_L2  16  /* 一级脚本 / Script level 1 */
#endif

#ifndef GT_LATEX_FONT5_SIZE_L3
    #define GT_LATEX_FONT5_SIZE_L3  12  /* 二级脚本 / Script level 2 */
#endif

#ifndef GT_LATEX_FONT5_SIZE_L4
    #define GT_LATEX_FONT5_SIZE_L4  10  /* 小标注 / Small annotation */
#endif

/* ----------------------------------------------------------------
 * 动态内存分配 / Dynamic memory allocation
 * 默认使用标准库，可通过 gt_latex_mem_register() 注册自定义分配器
 * Default: standard library. Use gt_latex_mem_register() to override.
 * ---------------------------------------------------------------- */
#include <stdlib.h>

/* ----------------------------------------------------------------
 * 功能裁剪开关 / Feature enable switches
 * ---------------------------------------------------------------- */
#ifndef GT_LATEX_CFG_ENABLE_FRAC
    #define GT_LATEX_CFG_ENABLE_FRAC        1   /* 分数 \frac / Fraction */
#endif

#ifndef GT_LATEX_CFG_ENABLE_SQRT
    #define GT_LATEX_CFG_ENABLE_SQRT        1   /* 根号 \sqrt / Square root */
#endif

#ifndef GT_LATEX_CFG_ENABLE_LARGE_OP
    #define GT_LATEX_CFG_ENABLE_LARGE_OP    1   /* 大型运算符 \sum \int \prod / Large operators */
#endif

#ifndef GT_LATEX_CFG_ENABLE_ARROW
    #define GT_LATEX_CFG_ENABLE_ARROW       1   /* 箭头符号 / Arrow symbols */
#endif

#ifndef GT_LATEX_CFG_ENABLE_ACCENT
    #define GT_LATEX_CFG_ENABLE_ACCENT      1   /* 上下划线 \overline \underline / Overline and underline */
#endif

#ifndef GT_LATEX_CFG_ENABLE_DELIM
    #define GT_LATEX_CFG_ENABLE_DELIM       1   /* 自适应括号 \left \right / Adaptive delimiters */
#endif

#ifndef GT_LATEX_CFG_ENABLE_MATHFN
    #define GT_LATEX_CFG_ENABLE_MATHFN      1   /* 数学函数名 \sin \cos \tan 等 / Math function names */
#endif

#ifndef GT_LATEX_CFG_ENABLE_LIMOP
    #define GT_LATEX_CFG_ENABLE_LIMOP       1   /* 极限运算符 \lim \limsup \liminf / Limit operators */
#endif

#ifndef GT_LATEX_CFG_ENABLE_MATRIX
    #define GT_LATEX_CFG_ENABLE_MATRIX      1   /* 矩阵 \begin{matrix}...\end{matrix} / Matrix */
#endif

#ifndef GT_LATEX_CFG_ENABLE_CASES
    #define GT_LATEX_CFG_ENABLE_CASES       1   /* 分段函数 \begin{cases}...\end{cases} / Cases */
#endif

#ifndef GT_LATEX_CFG_ENABLE_ALIGN
    #define GT_LATEX_CFG_ENABLE_ALIGN       1   /* 多行对齐 \begin{align*}...\end{align*} / Align */
#endif

/* ----------------------------------------------------------------
 * 调试与断言 / Debug and assert
 * ---------------------------------------------------------------- */
#ifndef GT_LATEX_CFG_ASSERT_ENABLE
    #define GT_LATEX_CFG_ASSERT_ENABLE  0   /* 断言开关，裸机建议关闭 / Assert, disable on bare-metal */
#endif

#ifndef GT_LATEX_CFG_DEBUG_ENABLE
    #define GT_LATEX_CFG_DEBUG_ENABLE   0   /* 调试输出开关 / Debug output */
#endif

#endif /* GT_LATEX_ENABLE */

#endif /* _GT_LATEX_CONFIG_H_ */
