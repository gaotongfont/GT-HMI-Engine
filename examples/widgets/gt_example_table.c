/**
 * @file gt_example_table.c
 * @author Feyoung
 * @brief Table widget example
 * @version 0.1
 * @date 2026-03-27
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_table.h"

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/

/* global functions / API interface -------------------------------------*/

/**
 * @brief Create a table widget with header and data rows.
 */
gt_obj_st * gt_examples_table(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);

    gt_obj_st * table = gt_table_create(screen);
    gt_obj_set_pos(table, 10, 10);
    gt_obj_set_size(table, 350, 200);
    gt_table_set_font_size(table, 14);
    gt_table_set_font_color(table, gt_color_black());

    gt_table_add_header_text_by_len(table, "Name", 4, GT_ALIGN_LEFT);
    gt_table_add_header_text_by_len(table, "Value", 5, GT_ALIGN_CENTER);
    gt_table_add_header_text_by_len(table, "Unit", 4, GT_ALIGN_RIGHT);

    gt_table_add_content_text_by_len(table, "Temp", 4);
    gt_table_add_content_text_by_len(table, "25.6", 4);
    gt_table_add_content_text_by_len(table, "C", 1);

    gt_table_add_content_text_by_len(table, "Humidity", 8);
    gt_table_add_content_text_by_len(table, "60", 2);
    gt_table_add_content_text_by_len(table, "%", 1);

    gt_table_refresh(table);

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
