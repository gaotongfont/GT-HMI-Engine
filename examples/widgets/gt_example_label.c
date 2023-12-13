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
void gt_examples_label(void)
{
    gt_obj_st * screen;
    screen = gt_obj_create(NULL);

    gt_obj_st * label = gt_label_create(screen);
    gt_label_set_text(label, "This is label test.");
	gt_obj_set_pos(label, 0, 0);

    gt_disp_load_scr(screen);
}


#endif
/* end ------------------------------------------------------------------*/
