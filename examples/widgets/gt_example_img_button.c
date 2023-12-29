/**
 * @file gt_example_img_button.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2022-10-10 10:51:58
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
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

    gt_obj_st * image_btn = gt_imgbtn_create(screen);
    gt_imgbtn_set_src(image_btn, "./2.png");
    gt_imgbtn_set_src_press(image_btn, "./1.bmp");
    gt_imgbtn_set_src_release(image_btn, "./2.png");
	gt_obj_set_pos(image_btn, 100, 100);

    gt_disp_load_scr(screen);
}


#endif
/* end ------------------------------------------------------------------*/
