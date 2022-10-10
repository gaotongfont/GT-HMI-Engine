/**
 * @file gt_example_img.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2022-10-10 10:26:29
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/

#include "../gt_examples.h"
#include "../../src/widgets/gt_img.h"


/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES


/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

void gt_examples_img(void)
{
    gt_obj_st * screen;
    screen = gt_obj_create(NULL);
	gt_disp_set_scr(screen);

    gt_obj_st * image = gt_img_create(screen);
    gt_img_style_set_src(image, "./2.png");
	gt_obj_set_style_pos(image, 0, 0);

    gt_disp_load_scr(screen);
}


#endif
/* end ------------------------------------------------------------------*/
