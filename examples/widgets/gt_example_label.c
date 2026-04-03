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
    y_pos += 30;

    /* 2. 单行显示（超出宽度直接截断，不换行，不省略） */
    gt_obj_st * label_single = gt_label_create(screen);
    gt_label_set_text(label_single, "Single line: long text will be clipped without ellipsis.");
    gt_obj_set_pos(label_single, 0, y_pos);
    gt_obj_set_size(label_single, 280, 18);
    gt_label_set_single_line(label_single, true);
    y_pos += 20;

    /* 3. 单行省略（超出宽度末尾显示 "..."） */
    gt_obj_st * label_omit = gt_label_create(screen);
    gt_label_set_text(label_omit, "Omit mode: this text is too long to fit in the box.");
    gt_obj_set_pos(label_omit, 0, y_pos);
    gt_obj_set_size(label_omit, 280, 18);
    gt_label_set_omit_single_line(label_omit, true);
    y_pos += 20;

    /* 4. 单行自动滚动（超出宽度时自动向左滚动，总时长 3000ms） */
    gt_obj_st * label_scroll = gt_label_create(screen);
    gt_label_set_text(label_scroll, "Auto scroll: this text will scroll automatically.");
    gt_obj_set_pos(label_scroll, 0, y_pos);
    gt_obj_set_size(label_scroll, 280, 18);
    gt_label_set_auto_scroll_single_line(label_scroll, true);
    gt_label_set_auto_scroll_total_time(label_scroll, 3000);
    y_pos += 20;

    /* 5. 单行自动滚动（按单字符时长控制速度，每字符 100ms） */
    gt_obj_st * label_scroll2 = gt_label_create(screen);
    gt_label_set_text(label_scroll2, "Per-char scroll: each character takes 100ms to scroll.");
    gt_obj_set_pos(label_scroll2, 0, y_pos);
    gt_obj_set_size(label_scroll2, 280, 18);
    gt_label_set_auto_scroll_single_line(label_scroll2, true);
    gt_label_set_one_word_scroll_time(label_scroll2, 100);
    y_pos += 22;

    /* ---- Font Family 测试（gt_font_config.h 枚举调用 + scale_size 缩放）---- */

    /* === family_dot_16 === */
    /* 6. family_dot_16 原始 16pt */
    gt_obj_st * label_dot16 = gt_label_create(screen);
    gt_obj_set_pos(label_dot16, 0, y_pos);
    gt_obj_set_size(label_dot16, 780, 20);
    gt_label_set_font_family(label_dot16, family_dot_16);
    gt_label_set_text(label_dot16, "dot_16 纯点阵16pt ABC123");
    y_pos += 22;

    /* 6a. family_dot_16 16→24 放大1.5x */
    gt_obj_st * label_dot16_up = gt_label_create(screen);
    gt_obj_set_pos(label_dot16_up, 0, y_pos);
    gt_obj_set_size(label_dot16_up, 780, 28);
    gt_label_set_font_family(label_dot16_up, family_dot_16);
    gt_label_set_font_scale_size(label_dot16_up, 24);
    gt_label_set_text(label_dot16_up, "dot_16→24 放大1.5x ABC123");
    y_pos += 30;

    /* 6b. family_dot_16 16→12 缩小0.75x */
    gt_obj_st * label_dot16_down = gt_label_create(screen);
    gt_obj_set_pos(label_dot16_down, 0, y_pos);
    gt_obj_set_size(label_dot16_down, 780, 16);
    gt_label_set_font_family(label_dot16_down, family_dot_16);
    gt_label_set_font_scale_size(label_dot16_down, 12);
    gt_label_set_text(label_dot16_down, "dot_16→12 缩小0.75x ABC123");
    y_pos += 18;

    /* 6c. family_dot_16 16→32 放大2.0x */
    gt_obj_st * label_dot16_up2x = gt_label_create(screen);
    gt_obj_set_pos(label_dot16_up2x, 0, y_pos);
    gt_obj_set_size(label_dot16_up2x, 780, 36);
    gt_label_set_font_family(label_dot16_up2x, family_dot_16);
    gt_label_set_font_scale_size(label_dot16_up2x, 32);
    gt_label_set_text(label_dot16_up2x, "dot_16→32 放大2.0x ABC123");
    y_pos += 38;

    /* === family_dot_20 === */
    /* 7. family_dot_20 原始 20pt */
    gt_obj_st * label_dot20 = gt_label_create(screen);
    gt_obj_set_pos(label_dot20, 0, y_pos);
    gt_obj_set_size(label_dot20, 780, 24);
    gt_label_set_font_family(label_dot20, family_dot_20);
    gt_label_set_text(label_dot20, "dot_20 纯点阵20pt ABC123");
    y_pos += 26;

    /* 7a. family_dot_20 20→30 放大1.5x */
    gt_obj_st * label_dot20_up = gt_label_create(screen);
    gt_obj_set_pos(label_dot20_up, 0, y_pos);
    gt_obj_set_size(label_dot20_up, 780, 34);
    gt_label_set_font_family(label_dot20_up, family_dot_20);
    gt_label_set_font_scale_size(label_dot20_up, 30);
    gt_label_set_text(label_dot20_up, "dot_20→30 放大1.5x ABC123");
    y_pos += 36;

    /* 7b. family_dot_20 20→15 缩小0.75x */
    gt_obj_st * label_dot20_down = gt_label_create(screen);
    gt_obj_set_pos(label_dot20_down, 0, y_pos);
    gt_obj_set_size(label_dot20_down, 780, 18);
    gt_label_set_font_family(label_dot20_down, family_dot_20);
    gt_label_set_font_scale_size(label_dot20_down, 15);
    gt_label_set_text(label_dot20_down, "dot_20→15 缩小0.75x ABC123");
    y_pos += 20;

    /* === family_dot_24 === */
    /* 8. family_dot_24 原始 24pt */
    gt_obj_st * label_dot24 = gt_label_create(screen);
    gt_obj_set_pos(label_dot24, 0, y_pos);
    gt_obj_set_size(label_dot24, 780, 28);
    gt_label_set_font_family(label_dot24, family_dot_24);
    gt_label_set_text(label_dot24, "dot_24 纯点阵24pt ABC123");
    y_pos += 30;

    /* 8a. family_dot_24 24→32 放大1.33x */
    gt_obj_st * label_dot24_up = gt_label_create(screen);
    gt_obj_set_pos(label_dot24_up, 0, y_pos);
    gt_obj_set_size(label_dot24_up, 780, 36);
    gt_label_set_font_family(label_dot24_up, family_dot_24);
    gt_label_set_font_scale_size(label_dot24_up, 32);
    gt_label_set_text(label_dot24_up, "dot_24→32 放大1.33x ABC123");
    y_pos += 38;

    /* 8b. family_dot_24 24→18 缩小0.75x */
    gt_obj_st * label_dot24_down = gt_label_create(screen);
    gt_obj_set_pos(label_dot24_down, 0, y_pos);
    gt_obj_set_size(label_dot24_down, 780, 22);
    gt_label_set_font_family(label_dot24_down, family_dot_24);
    gt_label_set_font_scale_size(label_dot24_down, 18);
    gt_label_set_text(label_dot24_down, "dot_24→18 缩小0.75x ABC123");
    y_pos += 24;

    /* === family_gray_20 === */
    /* 9. family_gray_20 原始 20pt */
    gt_obj_st * label_gray20 = gt_label_create(screen);
    gt_obj_set_pos(label_gray20, 0, y_pos);
    gt_obj_set_size(label_gray20, 780, 24);
    gt_label_set_font_family(label_gray20, family_gray_20);
    gt_label_set_text(label_gray20, "gray_20 灰度4bit-20pt ABC123");
    y_pos += 26;

    /* 9a. family_gray_20 20→30 放大1.5x */
    gt_obj_st * label_gray20_up = gt_label_create(screen);
    gt_obj_set_pos(label_gray20_up, 0, y_pos);
    gt_obj_set_size(label_gray20_up, 780, 34);
    gt_label_set_font_family(label_gray20_up, family_gray_20);
    gt_label_set_font_scale_size(label_gray20_up, 30);
    gt_label_set_text(label_gray20_up, "gray_20→30 放大1.5x ABC123");
    y_pos += 36;

    /* 9b. family_gray_20 20→14 缩小0.7x */
    gt_obj_st * label_gray20_down = gt_label_create(screen);
    gt_obj_set_pos(label_gray20_down, 0, y_pos);
    gt_obj_set_size(label_gray20_down, 780, 18);
    gt_label_set_font_family(label_gray20_down, family_gray_20);
    gt_label_set_font_scale_size(label_gray20_down, 14);
    gt_label_set_text(label_gray20_down, "gray_20→14 缩小0.7x ABC123");
    y_pos += 20;

    /* === family_gray_24 === */
    /* 10. family_gray_24 原始 24pt */
    gt_obj_st * label_gray24 = gt_label_create(screen);
    gt_obj_set_pos(label_gray24, 0, y_pos);
    gt_obj_set_size(label_gray24, 780, 28);
    gt_label_set_font_family(label_gray24, family_gray_24);
    gt_label_set_text(label_gray24, "gray_24 灰度4bit-24pt ABC123");
    y_pos += 30;

    /* 10a. family_gray_24 24→16 缩小0.67x */
    gt_obj_st * label_gray24_down = gt_label_create(screen);
    gt_obj_set_pos(label_gray24_down, 0, y_pos);
    gt_obj_set_size(label_gray24_down, 780, 20);
    gt_label_set_font_family(label_gray24_down, family_gray_24);
    gt_label_set_font_scale_size(label_gray24_down, 16);
    gt_label_set_text(label_gray24_down, "gray_24→16 缩小0.67x ABC123");
    y_pos += 22;

    /* 10b. family_gray_24 24→36 放大1.5x */
    gt_obj_st * label_gray24_up = gt_label_create(screen);
    gt_obj_set_pos(label_gray24_up, 0, y_pos);
    gt_obj_set_size(label_gray24_up, 780, 40);
    gt_label_set_font_family(label_gray24_up, family_gray_24);
    gt_label_set_font_scale_size(label_gray24_up, 36);
    gt_label_set_text(label_gray24_up, "gray_24→36 放大1.5x ABC123");
    y_pos += 42;

    /* 备注：y_pos 当前累积到约 450px 左右，适配 800×480 屏幕 */

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}


#endif
/* end ------------------------------------------------------------------*/
