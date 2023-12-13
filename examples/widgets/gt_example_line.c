/**
 * @file gt_example_line.c
 * @author Li
 * @brief
 * @version 0.1
 * @date 2023-01-10 11:41:32
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/core/gt_event.h"
#include "../../src/widgets/gt_btn.h"
#include "../../src/widgets/gt_label.h"
#include "../../src/widgets/gt_line.h"


/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/


/* global functions / API interface -------------------------------------*/

/**
 * @brief create a line.
 */
void gt_examples_line(void)
{
    gt_obj_st * screen;
    screen = gt_obj_create(NULL);

    gt_obj_st * line = gt_line_create(screen);

	gt_line_set_color(line,gt_color_blue());
	gt_line_set_line_width(line,1);
	gt_line_set_start_point(line,100,100);
	gt_line_set_end_point(line,400,133);
    gt_disp_load_scr(screen);
}

#endif
/* end ------------------------------------------------------------------*/
