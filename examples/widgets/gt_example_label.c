/**
 * @file gt_example_label.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2022-10-10 11:35:27
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_label.h"
#include <stdio.h>


/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES


/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_examples_label(void)
{
    gt_obj_st * screen;
    screen = gt_obj_create(NULL);

    uint16_t y_pos = 0;  /* 统一 Y 坐标累加器 */

    /* 1. 普通多行文本 */
    gt_obj_st * label = gt_label_create(screen);
    gt_obj_set_pos(label, 0, y_pos);
    gt_obj_set_size(label, 780, 28);
    gt_label_set_text(label, "This is label test.");
    y_pos += 38;

    /* 2. 单行显示（超出宽度直接截断，不换行，不省略） */
    gt_obj_st * label_single = gt_label_create(screen);
    gt_label_set_text(label_single, "Single line: long text will be clipped without ellipsis.");
    gt_obj_set_pos(label_single, 0, y_pos);
    gt_obj_set_size(label_single, 280, 18);
    gt_label_set_single_line(label_single, true);
    y_pos += 28;

    /* 3. 单行省略（超出宽度末尾显示 "..."） */
    gt_obj_st * label_omit = gt_label_create(screen);
    gt_label_set_text(label_omit, "Omit mode: this text is too long to fit in the box.");
    gt_obj_set_pos(label_omit, 0, y_pos);
    gt_obj_set_size(label_omit, 280, 18);
    gt_label_set_omit_single_line(label_omit, true);
    y_pos += 28;

    /* 4. 单行自动滚动（超出宽度时自动向左滚动，总时长 3000ms） */
    gt_obj_st * label_scroll = gt_label_create(screen);
    gt_label_set_text(label_scroll, "Auto scroll: this text will scroll automatically.");
    gt_obj_set_pos(label_scroll, 0, y_pos);
    gt_obj_set_size(label_scroll, 280, 18);
    gt_label_set_auto_scroll_single_line(label_scroll, true);
    gt_label_set_auto_scroll_total_time(label_scroll, 3000);
    y_pos += 28;

    /* 5. 单行自动滚动（按单字符时长控制速度，每字符 100ms） */
    gt_obj_st * label_scroll2 = gt_label_create(screen);
    gt_label_set_text(label_scroll2, "Per-char scroll: each character takes 100ms to scroll.");
    gt_obj_set_pos(label_scroll2, 0, y_pos);
    gt_obj_set_size(label_scroll2, 280, 18);
    gt_label_set_auto_scroll_single_line(label_scroll2, true);
    gt_label_set_one_word_scroll_time(label_scroll2, 100);
    y_pos += 30;

    /* ---- Font Family 测试（gt_font_config.h 枚举调用 + scale_size 缩放）---- */

    /* === family_dot_16 === */
    /* 6. family_dot_16 原始 16pt */
    gt_obj_st * label_dot16 = gt_label_create(screen);
    gt_obj_set_pos(label_dot16, 0, y_pos);
    gt_obj_set_size(label_dot16, 780, 20);
    gt_label_set_font_family(label_dot16, family_dot_16);
    gt_label_set_text(label_dot16, "dot_16 纯点阵16pt ABC123");
    y_pos += 30;

    /* 6a. family_dot_16 16→24 放大1.5x */
    gt_obj_st * label_dot16_up = gt_label_create(screen);
    gt_obj_set_pos(label_dot16_up, 0, y_pos);
    gt_obj_set_size(label_dot16_up, 780, 28);
    gt_label_set_font_family(label_dot16_up, family_dot_16);
    gt_label_set_font_scale_size(label_dot16_up, 24);
    gt_label_set_text(label_dot16_up, "dot_16→24 放大1.5x ABC123");
    y_pos += 38;

    /* 6b. family_dot_16 16→12 缩小0.75x */
    gt_obj_st * label_dot16_down = gt_label_create(screen);
    gt_obj_set_pos(label_dot16_down, 0, y_pos);
    gt_obj_set_size(label_dot16_down, 780, 22);
    gt_label_set_font_family(label_dot16_down, family_dot_16);
    gt_label_set_font_scale_size(label_dot16_down, 12);
    gt_label_set_text(label_dot16_down, "dot_16→12 缩小0.75x ABC123");
    y_pos += 30;

    /* 6c. family_dot_16 16→32 放大2.0x */
    gt_obj_st * label_dot16_up2x = gt_label_create(screen);
    gt_obj_set_pos(label_dot16_up2x, 0, y_pos);
    gt_obj_set_size(label_dot16_up2x, 780, 36);
    gt_label_set_font_family(label_dot16_up2x, family_dot_16);
    gt_label_set_font_scale_size(label_dot16_up2x, 32);
    gt_label_set_text(label_dot16_up2x, "dot_16→32 放大2.0x ABC123");
    y_pos += 46;

    /* === family_dot_20 === */
    /* 7. family_dot_20 原始 20pt */
    gt_obj_st * label_dot20 = gt_label_create(screen);
    gt_obj_set_pos(label_dot20, 0, y_pos);
    gt_obj_set_size(label_dot20, 780, 24);
    gt_label_set_font_family(label_dot20, family_dot_20);
    gt_label_set_text(label_dot20, "dot_20 纯点阵20pt ABC123");
    y_pos += 34;

    /* 7a. family_dot_20 20→30 放大1.5x */
    gt_obj_st * label_dot20_up = gt_label_create(screen);
    gt_obj_set_pos(label_dot20_up, 0, y_pos);
    gt_obj_set_size(label_dot20_up, 780, 34);
    gt_label_set_font_family(label_dot20_up, family_dot_20);
    gt_label_set_font_scale_size(label_dot20_up, 30);
    gt_label_set_text(label_dot20_up, "dot_20→30 放大1.5x ABC123");
    y_pos += 44;

    /* 7b. family_dot_20 20→15 缩小0.75x */
    gt_obj_st * label_dot20_down = gt_label_create(screen);
    gt_obj_set_pos(label_dot20_down, 0, y_pos);
    gt_obj_set_size(label_dot20_down, 780, 24);
    gt_label_set_font_family(label_dot20_down, family_dot_20);
    gt_label_set_font_scale_size(label_dot20_down, 15);
    gt_label_set_text(label_dot20_down, "dot_20→15 缩小0.75x ABC123");
    y_pos += 32;

    /* === family_dot_24 === */
    /* 8. family_dot_24 原始 24pt */
    gt_obj_st * label_dot24 = gt_label_create(screen);
    gt_obj_set_pos(label_dot24, 0, y_pos);
    gt_obj_set_size(label_dot24, 780, 28);
    gt_label_set_font_family(label_dot24, family_dot_24);
    gt_label_set_text(label_dot24, "dot_24 纯点阵24pt ABC123");
    y_pos += 38;

    /* 8a. family_dot_24 24→32 放大1.33x */
    gt_obj_st * label_dot24_up = gt_label_create(screen);
    gt_obj_set_pos(label_dot24_up, 0, y_pos);
    gt_obj_set_size(label_dot24_up, 780, 36);
    gt_label_set_font_family(label_dot24_up, family_dot_24);
    gt_label_set_font_scale_size(label_dot24_up, 32);
    gt_label_set_text(label_dot24_up, "dot_24→32 放大1.33x ABC123");
    y_pos += 46;

    /* 8b. family_dot_24 24→18 缩小0.75x */
    gt_obj_st * label_dot24_down = gt_label_create(screen);
    gt_obj_set_pos(label_dot24_down, 0, y_pos);
    gt_obj_set_size(label_dot24_down, 780, 28);
    gt_label_set_font_family(label_dot24_down, family_dot_24);
    gt_label_set_font_scale_size(label_dot24_down, 18);
    gt_label_set_text(label_dot24_down, "dot_24→18 缩小0.75x ABC123");
    y_pos += 38;

    /* === family_gray_20 === */
    /* 9. family_gray_20 原始 20pt */
    gt_obj_st * label_gray20 = gt_label_create(screen);
    gt_obj_set_pos(label_gray20, 0, y_pos);
    gt_obj_set_size(label_gray20, 780, 24);
    gt_label_set_font_family(label_gray20, family_gray_20);
    gt_label_set_text(label_gray20, "gray_20 灰度4bit-20pt ABC123");
    y_pos += 34;

    /* 9a. family_gray_20 20→30 放大1.5x */
    gt_obj_st * label_gray20_up = gt_label_create(screen);
    gt_obj_set_pos(label_gray20_up, 0, y_pos);
    gt_obj_set_size(label_gray20_up, 780, 34);
    gt_label_set_font_family(label_gray20_up, family_gray_20);
    gt_label_set_font_scale_size(label_gray20_up, 30);
    gt_label_set_text(label_gray20_up, "gray_20→30 放大1.5x ABC123");
    y_pos += 44;

    /* 9b. family_gray_20 20→14 缩小0.7x */
    gt_obj_st * label_gray20_down = gt_label_create(screen);
    gt_obj_set_pos(label_gray20_down, 0, y_pos);
    gt_obj_set_size(label_gray20_down, 780, 24);
    gt_label_set_font_family(label_gray20_down, family_gray_20);
    gt_label_set_font_scale_size(label_gray20_down, 14);
    gt_label_set_text(label_gray20_down, "gray_20→14 缩小0.7x ABC123");
    y_pos += 32;

    /* === family_gray_24 === */
    /* 10. family_gray_24 原始 24pt */
    gt_obj_st * label_gray24 = gt_label_create(screen);
    gt_obj_set_pos(label_gray24, 0, y_pos);
    gt_obj_set_size(label_gray24, 780, 28);
    gt_label_set_font_family(label_gray24, family_gray_24);
    gt_label_set_text(label_gray24, "gray_24 灰度4bit-24pt ABC123");
    y_pos += 38;

    /* 10a. family_gray_24 24→16 缩小0.67x */
    gt_obj_st * label_gray24_down = gt_label_create(screen);
    gt_obj_set_pos(label_gray24_down, 0, y_pos);
    gt_obj_set_size(label_gray24_down, 780, 24);
    gt_label_set_font_family(label_gray24_down, family_gray_24);
    gt_label_set_font_scale_size(label_gray24_down, 16);
    gt_label_set_text(label_gray24_down, "gray_24→16 缩小0.67x ABC123");
    y_pos += 34;

    /* 10b. family_gray_24 24→36 放大1.5x */
    gt_obj_st * label_gray24_up = gt_label_create(screen);
    gt_obj_set_pos(label_gray24_up, 0, y_pos);
    gt_obj_set_size(label_gray24_up, 780, 40);
    gt_label_set_font_family(label_gray24_up, family_gray_24);
    gt_label_set_font_scale_size(label_gray24_up, 36);
    gt_label_set_text(label_gray24_up, "gray_24→36 放大1.5x ABC123");
    y_pos += 50;

    /* === family_dot_32 === */
    /* 11. family_dot_32 原始 32pt */
    gt_obj_st * label_dot32 = gt_label_create(screen);
    gt_obj_set_pos(label_dot32, 0, y_pos);
    gt_obj_set_size(label_dot32, 780, 36);
    gt_label_set_font_family(label_dot32, family_dot_32);
    gt_label_set_text(label_dot32, "dot_32 纯点阵32pt ABC123");
    y_pos += 46;

    /* 11a. family_dot_32 32→24 缩小0.75x */
    gt_obj_st * label_dot32_down = gt_label_create(screen);
    gt_obj_set_pos(label_dot32_down, 0, y_pos);
    gt_obj_set_size(label_dot32_down, 780, 32);
    gt_label_set_font_family(label_dot32_down, family_dot_32);
    gt_label_set_font_scale_size(label_dot32_down, 24);
    gt_label_set_text(label_dot32_down, "dot_32→24 缩小0.75x ABC123");
    y_pos += 42;

    /* 11b. family_dot_32 32→48 放大1.5x */
    gt_obj_st * label_dot32_up = gt_label_create(screen);
    gt_obj_set_pos(label_dot32_up, 0, y_pos);
    gt_obj_set_size(label_dot32_up, 780, 52);
    gt_label_set_font_family(label_dot32_up, family_dot_32);
    gt_label_set_font_scale_size(label_dot32_up, 48);
    gt_label_set_text(label_dot32_up, "dot_32→48 放大1.5x ABC123");
    y_pos += 62;

    /* === family_gray_16 === */
    /* 12. family_gray_16 原始 16pt */
    gt_obj_st * label_gray16 = gt_label_create(screen);
    gt_obj_set_pos(label_gray16, 0, y_pos);
    gt_obj_set_size(label_gray16, 780, 20);
    gt_label_set_font_family(label_gray16, family_gray_16);
    gt_label_set_text(label_gray16, "gray_16 灰度4bit-16pt ABC123");
    y_pos += 30;

    /* 12a. family_gray_16 16→12 缩小0.75x */
    gt_obj_st * label_gray16_down = gt_label_create(screen);
    gt_obj_set_pos(label_gray16_down, 0, y_pos);
    gt_obj_set_size(label_gray16_down, 780, 22);
    gt_label_set_font_family(label_gray16_down, family_gray_16);
    gt_label_set_font_scale_size(label_gray16_down, 12);
    gt_label_set_text(label_gray16_down, "gray_16→12 缩小0.75x ABC123");
    y_pos += 30;

    /* 12b. family_gray_16 16→24 放大1.5x */
    gt_obj_st * label_gray16_up = gt_label_create(screen);
    gt_obj_set_pos(label_gray16_up, 0, y_pos);
    gt_obj_set_size(label_gray16_up, 780, 28);
    gt_label_set_font_family(label_gray16_up, family_gray_16);
    gt_label_set_font_scale_size(label_gray16_up, 24);
    gt_label_set_text(label_gray16_up, "gray_16→24 放大1.5x ABC123");
    y_pos += 38;

    /* === family_gray_32 === */
    /* 13. family_gray_32 原始 32pt */
    gt_obj_st * label_gray32 = gt_label_create(screen);
    gt_obj_set_pos(label_gray32, 0, y_pos);
    gt_obj_set_size(label_gray32, 780, 36);
    gt_label_set_font_family(label_gray32, family_gray_32);
    gt_label_set_text(label_gray32, "gray_32 灰度4bit-32pt ABC123");
    y_pos += 46;

    /* 13a. family_gray_32 32→24 缩小0.75x */
    gt_obj_st * label_gray32_down = gt_label_create(screen);
    gt_obj_set_pos(label_gray32_down, 0, y_pos);
    gt_obj_set_size(label_gray32_down, 780, 32);
    gt_label_set_font_family(label_gray32_down, family_gray_32);
    gt_label_set_font_scale_size(label_gray32_down, 24);
    gt_label_set_text(label_gray32_down, "gray_32→24 缩小0.75x ABC123");
    y_pos += 42;

    /* 13b. family_gray_32 32→48 放大1.5x */
    gt_obj_st * label_gray32_up = gt_label_create(screen);
    gt_obj_set_pos(label_gray32_up, 0, y_pos);
    gt_obj_set_size(label_gray32_up, 780, 52);
    gt_label_set_font_family(label_gray32_up, family_gray_32);
    gt_label_set_font_scale_size(label_gray32_up, 48);
    gt_label_set_text(label_gray32_up, "gray_32→48 放大1.5x ABC123");
    y_pos += 62;

#if 01  //FIX Frozen
    /* === family_mix_16_cn_gray_ascii_dot === */
    /* 14. family_mix_16_cn_gray_ascii_dot 原始 16pt */
    gt_obj_st * label_mix16 = gt_label_create(screen);
    gt_obj_set_pos(label_mix16, 0, y_pos);
    gt_obj_set_size(label_mix16, 780, 20);
    gt_label_set_font_family(label_mix16, family_mix_16_cn_gray_ascii_dot);
    gt_label_set_text(label_mix16, "mix_16 中文4bit灰度+ASCII点阵16pt ABC123");
    y_pos += 30;

    /* 14a. family_mix_16_cn_gray_ascii_dot 16→12 缩小 */
    gt_obj_st * label_mix16_down = gt_label_create(screen);
    gt_obj_set_pos(label_mix16_down, 0, y_pos);
    gt_obj_set_size(label_mix16_down, 780, 22);
    gt_label_set_font_family(label_mix16_down, family_mix_16_cn_gray_ascii_dot);
    gt_label_set_font_scale_size(label_mix16_down, 12);
    gt_label_set_text(label_mix16_down, "mix_16→12 缩小0.75x 中文灰度+ASCII点阵 ABC123");
    y_pos += 28;

    /* 14b. family_mix_16_cn_gray_ascii_dot 16→24 放大 */
    gt_obj_st * label_mix16_up = gt_label_create(screen);
    gt_obj_set_pos(label_mix16_up, 0, y_pos);
    gt_obj_set_size(label_mix16_up, 780, 28);
    gt_label_set_font_family(label_mix16_up, family_mix_16_cn_gray_ascii_dot);
    gt_label_set_font_scale_size(label_mix16_up, 24);
    gt_label_set_text(label_mix16_up, "mix_16→24 放大1.5x 中文灰度+ASCII点阵 ABC123");
    y_pos += 38;

#endif
#if 01  //FIX Frozen
    /* ==================================================================== */
    /* ---- 矢量字体测试（family_vec, size=0, 需 gt_label_set_font_size）---- */
    /* ==================================================================== */

    /* 15. family_vec 12pt */
    gt_obj_st * label_vec_12 = gt_label_create(screen);
    gt_obj_set_pos(label_vec_12, 0, y_pos);
    gt_obj_set_size(label_vec_12, 780, 18);
    gt_label_set_font_family(label_vec_12, family_vec);
    gt_label_set_font_size(label_vec_12, 12);
    gt_label_set_text(label_vec_12, "vec 矢量12pt ABC123 中文黑体矢量");
    y_pos += 28;

    /* 16. family_vec 16pt */
    gt_obj_st * label_vec_16 = gt_label_create(screen);
    gt_obj_set_pos(label_vec_16, 0, y_pos);
    gt_obj_set_size(label_vec_16, 780, 22);
    gt_label_set_font_family(label_vec_16, family_vec);
    gt_label_set_font_size(label_vec_16, 16);
    gt_label_set_text(label_vec_16, "vec 矢量16pt ABC123 中文黑体矢量");
    y_pos += 32;

    /* 17. family_vec 20pt */
    gt_obj_st * label_vec_20 = gt_label_create(screen);
    gt_obj_set_pos(label_vec_20, 0, y_pos);
    gt_obj_set_size(label_vec_20, 780, 26);
    gt_label_set_font_family(label_vec_20, family_vec);
    gt_label_set_font_size(label_vec_20, 20);
    gt_label_set_text(label_vec_20, "vec 矢量20pt ABC123 中文黑体矢量");
    y_pos += 36;

    /* 18. family_vec 24pt */
    gt_obj_st * label_vec_24 = gt_label_create(screen);
    gt_obj_set_pos(label_vec_24, 0, y_pos);
    gt_obj_set_size(label_vec_24, 780, 30);
    gt_label_set_font_family(label_vec_24, family_vec);
    gt_label_set_font_size(label_vec_24, 24);
    gt_label_set_text(label_vec_24, "vec 矢量24pt ABC123 中文黑体矢量");
    y_pos += 40;

    /* 19. family_vec 32pt */
    gt_obj_st * label_vec_32 = gt_label_create(screen);
    gt_obj_set_pos(label_vec_32, 0, y_pos);
    gt_obj_set_size(label_vec_32, 780, 38);
    gt_label_set_font_family(label_vec_32, family_vec);
    gt_label_set_font_size(label_vec_32, 32);
    gt_label_set_text(label_vec_32, "vec 矢量32pt ABC123 中文黑体矢量");
    y_pos += 48;

    /* 20. family_vec 40pt */
    gt_obj_st * label_vec_40 = gt_label_create(screen);
    gt_obj_set_pos(label_vec_40, 0, y_pos);
    gt_obj_set_size(label_vec_40, 780, 46);
    gt_label_set_font_family(label_vec_40, family_vec);
    gt_label_set_font_size(label_vec_40, 40);
    gt_label_set_text(label_vec_40, "vec 矢量40pt ABC123 中文黑体矢量");
    y_pos += 56;

    /* 21. family_vec 48pt */
    gt_obj_st * label_vec_48 = gt_label_create(screen);
    gt_obj_set_pos(label_vec_48, 0, y_pos);
    gt_obj_set_size(label_vec_48, 780, 54);
    gt_label_set_font_family(label_vec_48, family_vec);
    gt_label_set_font_size(label_vec_48, 48);
    gt_label_set_text(label_vec_48, "vec 矢量48pt ABC123 中文黑体矢量");
    y_pos += 64;

#endif
#if 01  //FIX Frozen
    /* ==================================================================== */
    /* ---- 纯矢量无灰度（gray=1, 无抗锯齿, 纯黑白渲染）---- */
    /* ==================================================================== */

    /* 27. family_vec gray=1 12pt */
    gt_obj_st * label_vec_g1_12 = gt_label_create(screen);
    gt_obj_set_pos(label_vec_g1_12, 0, y_pos);
    gt_obj_set_size(label_vec_g1_12, 780, 18);
    gt_label_set_font_family(label_vec_g1_12, family_vec);
    gt_label_set_font_size(label_vec_g1_12, 12);
    gt_label_set_font_gray(label_vec_g1_12, 1);
    gt_label_set_text(label_vec_g1_12, "vec-gray1 矢量12pt 无抗锯齿 ABC123 中文黑体");
    y_pos += 28;

    /* 28. family_vec gray=1 16pt */
    gt_obj_st * label_vec_g1_16 = gt_label_create(screen);
    gt_obj_set_pos(label_vec_g1_16, 0, y_pos);
    gt_obj_set_size(label_vec_g1_16, 780, 22);
    gt_label_set_font_family(label_vec_g1_16, family_vec);
    gt_label_set_font_size(label_vec_g1_16, 16);
    gt_label_set_font_gray(label_vec_g1_16, 1);
    gt_label_set_text(label_vec_g1_16, "vec-gray1 矢量16pt 无抗锯齿 ABC123 中文黑体");
    y_pos += 32;

    /* 29. family_vec gray=1 20pt */
    gt_obj_st * label_vec_g1_20 = gt_label_create(screen);
    gt_obj_set_pos(label_vec_g1_20, 0, y_pos);
    gt_obj_set_size(label_vec_g1_20, 780, 26);
    gt_label_set_font_family(label_vec_g1_20, family_vec);
    gt_label_set_font_size(label_vec_g1_20, 20);
    gt_label_set_font_gray(label_vec_g1_20, 1);
    gt_label_set_text(label_vec_g1_20, "vec-gray1 矢量20pt 无抗锯齿 ABC123 中文黑体");
    y_pos += 36;

    /* 30. family_vec gray=1 24pt */
    gt_obj_st * label_vec_g1_24 = gt_label_create(screen);
    gt_obj_set_pos(label_vec_g1_24, 0, y_pos);
    gt_obj_set_size(label_vec_g1_24, 780, 30);
    gt_label_set_font_family(label_vec_g1_24, family_vec);
    gt_label_set_font_size(label_vec_g1_24, 24);
    gt_label_set_font_gray(label_vec_g1_24, 1);
    gt_label_set_text(label_vec_g1_24, "vec-gray1 矢量24pt 无抗锯齿 ABC123 中文黑体");
    y_pos += 40;

    /* 31. family_vec gray=1 32pt */
    gt_obj_st * label_vec_g1_32 = gt_label_create(screen);
    gt_obj_set_pos(label_vec_g1_32, 0, y_pos);
    gt_obj_set_size(label_vec_g1_32, 780, 38);
    gt_label_set_font_family(label_vec_g1_32, family_vec);
    gt_label_set_font_size(label_vec_g1_32, 32);
    gt_label_set_font_gray(label_vec_g1_32, 1);
    gt_label_set_text(label_vec_g1_32, "vec-gray1 矢量32pt 无抗锯齿 ABC123 中文黑体");
    y_pos += 48;

    /* 32. family_vec gray=1 48pt */
    gt_obj_st * label_vec_g1_48 = gt_label_create(screen);
    gt_obj_set_pos(label_vec_g1_48, 0, y_pos);
    gt_obj_set_size(label_vec_g1_48, 780, 54);
    gt_label_set_font_family(label_vec_g1_48, family_vec);
    gt_label_set_font_size(label_vec_g1_48, 48);
    gt_label_set_font_gray(label_vec_g1_48, 1);
    gt_label_set_text(label_vec_g1_48, "vec-gray1 矢量48pt 无抗锯齿 ABC 中文黑体");
    y_pos += 64;

    /* ---- gray=1 + scale_size 缩放 ---- */

    /* 33. family_vec gray=1 font_size=16 + scale_size=24 */
    gt_obj_st * label_vec_g1_16_24 = gt_label_create(screen);
    gt_obj_set_pos(label_vec_g1_16_24, 0, y_pos);
    gt_obj_set_size(label_vec_g1_16_24, 780, 30);
    gt_label_set_font_family(label_vec_g1_16_24, family_vec);
    gt_label_set_font_size(label_vec_g1_16_24, 16);
    gt_label_set_font_scale_size(label_vec_g1_16_24, 24);
    gt_label_set_font_gray(label_vec_g1_16_24, 1);
    gt_label_set_text(label_vec_g1_16_24, "vec-gray1 16→24 无抗锯齿 ABC123 中文黑体");
    y_pos += 40;

    /* 34. family_vec gray=1 font_size=24 + scale_size=36 */
    gt_obj_st * label_vec_g1_24_36 = gt_label_create(screen);
    gt_obj_set_pos(label_vec_g1_24_36, 0, y_pos);
    gt_obj_set_size(label_vec_g1_24_36, 780, 42);
    gt_label_set_font_family(label_vec_g1_24_36, family_vec);
    gt_label_set_font_size(label_vec_g1_24_36, 24);
    gt_label_set_font_scale_size(label_vec_g1_24_36, 36);
    gt_label_set_font_gray(label_vec_g1_24_36, 1);
    gt_label_set_text(label_vec_g1_24_36, "vec-gray1 24→36 无抗锯齿 ABC123 中文黑体");
    y_pos += 52;

    /* 35. family_vec gray=1 font_size=32 + scale_size=16 缩小 */
    gt_obj_st * label_vec_g1_32_16 = gt_label_create(screen);
    gt_obj_set_pos(label_vec_g1_32_16, 0, y_pos);
    gt_obj_set_size(label_vec_g1_32_16, 780, 26);
    gt_label_set_font_family(label_vec_g1_32_16, family_vec);
    gt_label_set_font_size(label_vec_g1_32_16, 32);
    gt_label_set_font_scale_size(label_vec_g1_32_16, 16);
    gt_label_set_font_gray(label_vec_g1_32_16, 1);
    gt_label_set_text(label_vec_g1_32_16, "vec-gray1 32→16 无抗锯齿 ABC123 中文黑体");
    y_pos += 36;

    /* ---- gray=1 + 笔画粗细控制 ---- */

    /* 36. family_vec gray=1 24pt thick_en=16 thick_cn=16 (最细) */
    gt_obj_st * label_vec_g1_thin = gt_label_create(screen);
    gt_obj_set_pos(label_vec_g1_thin, 0, y_pos);
    gt_obj_set_size(label_vec_g1_thin, 780, 34);
    gt_label_set_font_family(label_vec_g1_thin, family_vec);
    gt_label_set_font_size(label_vec_g1_thin, 24);
    gt_label_set_font_gray(label_vec_g1_thin, 1);
    gt_label_set_font_thick_en(label_vec_g1_thin, 16);
    gt_label_set_font_thick_cn(label_vec_g1_thin, 16);
    gt_label_set_text(label_vec_g1_thin, "vec-gray1 thick=16 最细 ABC123 中文黑体");
    y_pos += 44;

    /* 37. family_vec gray=1 24pt thick_en=24 thick_cn=24 (默认粗细) */
    gt_obj_st * label_vec_g1_normal = gt_label_create(screen);
    gt_obj_set_pos(label_vec_g1_normal, 0, y_pos);
    gt_obj_set_size(label_vec_g1_normal, 780, 30);
    gt_label_set_font_family(label_vec_g1_normal, family_vec);
    gt_label_set_font_size(label_vec_g1_normal, 24);
    gt_label_set_font_gray(label_vec_g1_normal, 1);
    gt_label_set_font_thick_en(label_vec_g1_normal, 24);
    gt_label_set_font_thick_cn(label_vec_g1_normal, 24);
    gt_label_set_text(label_vec_g1_normal, "vec-gray1 thick=24 默认粗细 ABC123 中文黑体");
    y_pos += 40;

    /* 38. family_vec gray=1 24pt thick_en=32 thick_cn=32 (加粗) */
    gt_obj_st * label_vec_g1_thick = gt_label_create(screen);
    gt_obj_set_pos(label_vec_g1_thick, 0, y_pos);
    gt_obj_set_size(label_vec_g1_thick, 780, 30);
    gt_label_set_font_family(label_vec_g1_thick, family_vec);
    gt_label_set_font_size(label_vec_g1_thick, 24);
    gt_label_set_font_gray(label_vec_g1_thick, 1);
    gt_label_set_font_thick_en(label_vec_g1_thick, 32);
    gt_label_set_font_thick_cn(label_vec_g1_thick, 32);
    gt_label_set_text(label_vec_g1_thick, "vec-gray1 thick=32 加粗 ABC123 中文黑体");
    y_pos += 40;

    /* 39. family_vec gray=1 24pt thick_en=48 thick_cn=48 (更粗) */
    gt_obj_st * label_vec_g1_bold = gt_label_create(screen);
    gt_obj_set_pos(label_vec_g1_bold, 0, y_pos);
    gt_obj_set_size(label_vec_g1_bold, 780, 30);
    gt_label_set_font_family(label_vec_g1_bold, family_vec);
    gt_label_set_font_size(label_vec_g1_bold, 24);
    gt_label_set_font_gray(label_vec_g1_bold, 1);
    gt_label_set_font_thick_en(label_vec_g1_bold, 48);
    gt_label_set_font_thick_cn(label_vec_g1_bold, 48);
    gt_label_set_text(label_vec_g1_bold, "vec-gray1 thick=48 更粗 ABC123 中文黑体");
    y_pos += 40;

    /* ==================================================================== */
    /* ---- 混合矢量 family（中文矢量 + 外文点阵/灰度）---- */
    /* ==================================================================== */

    /* === family_mix_24_cn_vec_ascii_gray: 中文GBK黑体矢量 + 外文24pt灰度 === */
    /* 40. family_mix_24_cn_vec_ascii_gray 原始 24pt */
    gt_obj_st * label_mix24v = gt_label_create(screen);
    gt_obj_set_pos(label_mix24v, 0, y_pos);
    gt_obj_set_size(label_mix24v, 780, 30);
    gt_label_set_font_family(label_mix24v, family_mix_24_cn_vec_ascii_gray);
    gt_label_set_text(label_mix24v, "mix_24v 中文GBK矢量+外文24pt灰度 ABC123");
    y_pos += 40;

    /* 40a. family_mix_24_cn_vec_ascii_gray 24→18 缩小0.75x */
    gt_obj_st * label_mix24v_down = gt_label_create(screen);
    gt_obj_set_pos(label_mix24v_down, 0, y_pos);
    gt_obj_set_size(label_mix24v_down, 780, 24);
    gt_label_set_font_family(label_mix24v_down, family_mix_24_cn_vec_ascii_gray);
    gt_label_set_font_scale_size(label_mix24v_down, 18);
    gt_label_set_text(label_mix24v_down, "mix_24v→18 缩小0.75x 中文矢量+外文灰度 ABC123");
    y_pos += 34;

    /* 40b. family_mix_24_cn_vec_ascii_gray 24→36 放大1.5x */
    gt_obj_st * label_mix24v_up = gt_label_create(screen);
    gt_obj_set_pos(label_mix24v_up, 0, y_pos);
    gt_obj_set_size(label_mix24v_up, 800, 42);
    gt_label_set_font_family(label_mix24v_up, family_mix_24_cn_vec_ascii_gray);
    gt_label_set_font_scale_size(label_mix24v_up, 36);
    gt_label_set_text(label_mix24v_up, "mix_24v→36 放大1.5x 中文矢量 ABC123");
    y_pos += 52;

    /* === family_mix_32_cn_vec_ascii_dot: 中文GB18030宋体矢量 + 外文32pt点阵 === */
    /* 41. family_mix_32_cn_vec_ascii_dot 原始 32pt */
    gt_obj_st * label_mix32v = gt_label_create(screen);
    gt_obj_set_pos(label_mix32v, 0, y_pos);
    gt_obj_set_size(label_mix32v, 800, 38);
    gt_label_set_font_family(label_mix32v, family_mix_32_cn_vec_ascii_dot);
    gt_label_set_text(label_mix32v, "mix^32v 中文GB18030宋体矢量+外文32pt点阵 ABC123");
    y_pos += 48;

    /* 41a. family_mix_32_cn_vec_ascii_dot 32→24 缩小0.75x */
    gt_obj_st * label_mix32v_down = gt_label_create(screen);
    gt_obj_set_pos(label_mix32v_down, 0, y_pos);
    gt_obj_set_size(label_mix32v_down, 800, 32);
    gt_label_set_font_family(label_mix32v_down, family_mix_32_cn_vec_ascii_dot);
    gt_label_set_font_scale_size(label_mix32v_down, 24);
    gt_label_set_text(label_mix32v_down, "mix^32v→24 缩小0.75x 中文矢量+外文点阵 ABC123");
    y_pos += 42;

    /* 41b. family_mix_32_cn_vec_ascii_dot 32→48 放大1.5x */
    gt_obj_st * label_mix32v_up = gt_label_create(screen);
    gt_obj_set_pos(label_mix32v_up, 0, y_pos);
    gt_obj_set_size(label_mix32v_up, 800, 58);
    gt_label_set_font_family(label_mix32v_up, family_mix_32_cn_vec_ascii_dot);
    gt_label_set_font_scale_size(label_mix32v_up, 48);
    gt_label_set_text(label_mix32v_up, "mix^32v→48 放大1.5x 中文矢量 ABC123");
    y_pos += 68;

    gt_obj_st * label_vec_64 = gt_label_create(screen);
    gt_obj_set_pos(label_vec_64, 0, y_pos);
    gt_obj_set_size(label_vec_64, 800, 64 * 3);
    gt_label_set_font_family(label_vec_64, family_vec);
    gt_label_set_font_size(label_vec_64, 64);
    gt_label_set_font_gray(label_vec_64, 1);
    gt_label_set_text(label_vec_64, "vec 64px ABC123 中文黑体");
    y_pos += 70;

    #if GT_MAX_FONT_SIZE >= 80
    gt_obj_st * label_vec_80 = gt_label_create(screen);
    gt_obj_set_pos(label_vec_80, 0, y_pos);
    gt_obj_set_size(label_vec_80, 800, 80 * 2);
    gt_label_set_font_family(label_vec_80, family_vec);
    gt_label_set_font_size(label_vec_80, 80);
    gt_label_set_font_gray(label_vec_80, 1);
    gt_label_set_text(label_vec_80, "vec 80px ABC123 中文黑体");
    y_pos += 80 * 2;
    #endif

    #if GT_MAX_FONT_SIZE >= 192
    gt_obj_st * label_vec_192 = gt_label_create(screen);
    gt_obj_set_pos(label_vec_192, 0, y_pos);
    gt_obj_set_size(label_vec_192, 800, 192 * 3);
    gt_label_set_font_family(label_vec_192, family_vec);
    gt_label_set_font_size(label_vec_192, 192);
    gt_label_set_font_gray(label_vec_192, 1);
    gt_label_set_text(label_vec_192, "vec 192px ABC123 中文黑体");
    y_pos += 192 * 3 + 50;
    #endif

    #if GT_MAX_FONT_SIZE >= 256
    gt_obj_st * label_vec_256 = gt_label_create(screen);
    gt_obj_set_pos(label_vec_256, 0, y_pos);
    gt_obj_set_size(label_vec_256, 800, 256 * 5);
    gt_label_set_font_family(label_vec_256, family_vec);
    gt_label_set_font_size(label_vec_256, 256);
    gt_label_set_font_gray(label_vec_256, 1);
    gt_label_set_text(label_vec_256, "vec 256px ABC123 中文黑体");
    y_pos += 256 * 5 + 50;
    #endif

    #if GT_MAX_FONT_SIZE >= 384
    gt_obj_st * label_vec_384 = gt_label_create(screen);
    gt_obj_set_pos(label_vec_384, 0, y_pos);
    gt_obj_set_size(label_vec_384, 800, 384 * 7);
    gt_label_set_font_family(label_vec_384, family_vec);
    gt_label_set_font_size(label_vec_384, 384);
    gt_label_set_font_gray(label_vec_384, 1);
    gt_label_set_text(label_vec_384, "vec 384px ABC123 中文黑体");
    y_pos += 384 * 7 + 50;
    #endif

    #if GT_MAX_FONT_SIZE >= 512
    gt_obj_st * label_vec_512 = gt_label_create(screen);
    gt_obj_set_pos(label_vec_512, 0, y_pos);
    gt_obj_set_size(label_vec_512, 800, 512 * 8);
    gt_label_set_font_family(label_vec_512, family_vec);
    gt_label_set_font_size(label_vec_512, 512);
    gt_label_set_font_gray(label_vec_512, 1);
    gt_label_set_text(label_vec_512, "vec 512px ABC123 中文黑体");
    y_pos += 512 * 8 + 50;

    #endif
#endif

    /* 备注：y_pos 当前累积约 1100+ px，测试内容超出 800×480 屏幕，请上下滚动查看 */

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}


#endif
/* end ------------------------------------------------------------------*/
