/**
 * @file gt_example_graphs.c
 * @author Feyoung
 * @brief Graphs widget example
 * @version 0.1
 * @date 2026-03-27
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_graphs.h"

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/

/* global functions / API interface -------------------------------------*/

/**
 * @brief Create a line chart with sample data.
 */
gt_obj_st * gt_examples_graphs(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);

    gt_obj_st * graph = gt_graphs_create(screen);
    gt_obj_set_pos(graph, 10, 10);
    gt_obj_set_size(graph, 350, 220);
    gt_graphs_set_show_ruler(graph, true);
    gt_graphs_set_font_size(graph, 12);
    gt_graphs_set_font_color(graph, gt_color_black());

    gt_axis_st axis = {
        .hor = { .start = 0, .end = 10 },
        .ver = { .start = 0, .end = 100 },
        .hor_unit = 2,
        .ver_unit = 20,
    };
    gt_graphs_set_axis(graph, &axis);

    gt_line_st line_style = {
        .color = gt_color_hex(0x2196F3),
        .width = 2,
        .type = GT_GRAPHS_TYPE_LINE,
    };
    gt_series_points_st * series = gt_graphs_add_series_line(graph, &line_style);

    gt_point_f_st points[] = {
        {0, 10}, {2, 45}, {4, 30}, {6, 80}, {8, 55}, {10, 90}
    };
    gt_graphs_series_add_point_list(graph, series, points, NULL, 6);

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
