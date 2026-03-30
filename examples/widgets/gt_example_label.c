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

    /* 1. 普通多行文本 */
    gt_obj_st * label = gt_label_create(screen);
    gt_obj_set_pos(label, 0, 0);
    gt_obj_set_size(label, 60, 30);
    gt_label_set_text(label, "This is label test.");

    /* 2. 单行显示（超出宽度直接截断，不换行，不省略） */
    gt_obj_st * label_single = gt_label_create(screen);
    gt_label_set_text(label_single, "Single line: long text will be clipped without ellipsis.");
    gt_obj_set_pos(label_single, 0, 30);
    gt_obj_set_size(label_single, 200, 20);
    gt_label_set_single_line(label_single, true);

    /* 3. 单行省略（超出宽度末尾显示 "..."） */
    gt_obj_st * label_omit = gt_label_create(screen);
    gt_label_set_text(label_omit, "Omit mode: this text is too long to fit in the box.");
    gt_obj_set_pos(label_omit, 0, 60);
    gt_obj_set_size(label_omit, 200, 20);
    gt_label_set_omit_single_line(label_omit, true);

    /* 4. 单行自动滚动（超出宽度时自动向左滚动，总时长 3000ms） */
    gt_obj_st * label_scroll = gt_label_create(screen);
    gt_label_set_text(label_scroll, "Auto scroll: this text will scroll automatically.");
    gt_obj_set_pos(label_scroll, 0, 90);
    gt_obj_set_size(label_scroll, 200, 20);
    gt_label_set_auto_scroll_single_line(label_scroll, true);
    gt_label_set_auto_scroll_total_time(label_scroll, 3000);

    /* 5. 单行自动滚动（按单字符时长控制速度，每字符 100ms） */
    gt_obj_st * label_scroll2 = gt_label_create(screen);
    gt_label_set_text(label_scroll2, "Per-char scroll: each character takes 100ms to scroll.");
    gt_obj_set_pos(label_scroll2, 0, 120);
    gt_obj_set_size(label_scroll2, 200, 20);
    gt_label_set_auto_scroll_single_line(label_scroll2, true);
    gt_label_set_one_word_scroll_time(label_scroll2, 100);

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}


#endif
/* end ------------------------------------------------------------------*/
