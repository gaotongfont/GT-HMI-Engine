/**
 * @file gt_example_img_button.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2022-10-10 10:51:58
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_imgbtn.h"


/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES


/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

void gt_examples_img_button(void)
{
    gt_obj_st * screen;
    screen = gt_obj_create(NULL);
	gt_disp_set_scr(screen);

    gt_obj_st * image_btn = gt_imgbtn_create(screen);
    gt_imgbtn_style_set_src(image_btn, "./2.png");
    gt_imgbtn_style_set_src_press(image_btn, "./50x50.png");
    gt_imgbtn_style_set_src_release(image_btn, "./2.png");
	gt_obj_style_set_pos(image_btn, 0, 0);

    gt_disp_load_scr(screen);
}


#endif
/* end ------------------------------------------------------------------*/
