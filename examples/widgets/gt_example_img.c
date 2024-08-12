/**
 * @file gt_example_img.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2022-10-10 10:26:29
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/

#include "../gt_examples.h"
#include "../../src/widgets/gt_img.h"
#include "../../src/core/gt_event.h"


/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES


/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/

static gt_obj_st * _screen = NULL;
static gt_obj_st * image = NULL;
static gt_obj_st * image2 = NULL;

/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static void _image_one_cb(gt_event_st * e) {
    GT_LOGD(GT_LOG_TAG_EXTRA, "image one call back");
    static int _cnt_1 = 0;
    gt_img_set_src(image, (_cnt_1++ % 2) ? "./2.png" : "./1.png");
    gt_disp_load_scr(_screen);
}

static void _image_two_cb(gt_event_st * e) {
    GT_LOGD(GT_LOG_TAG_EXTRA, "image two call back");
    static int _cnt_2 = 0;
    gt_img_set_src(image2, (_cnt_2++ % 2) ? "./1.png" : "./2.png");
    gt_disp_load_scr(_screen);
}
/* global functions / API interface -------------------------------------*/

void gt_examples_img(void)
{
    _screen = gt_obj_create(NULL);

    image = gt_img_create(_screen);
    gt_img_set_src(image, "./2.png");
	gt_obj_set_pos(image, 0, 0);
    gt_obj_add_event_cb(image, _image_one_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    image2 = gt_img_create(_screen);
    gt_img_set_src(image2, "./1.png");
	gt_obj_set_pos(image2, 0, 100);
    gt_obj_add_event_cb(image2, _image_two_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    gt_disp_load_scr(_screen);
}


#endif
/* end ------------------------------------------------------------------*/
