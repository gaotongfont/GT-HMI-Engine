/**
 * @file gt_example_barcode.c
 * @author Feyoung
 * @brief Barcode widget example
 * @version 0.1
 * @date 2026-03-27
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_barcode.h"

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/

/* global functions / API interface -------------------------------------*/

/**
 * @brief Create a barcode widget.
 */
gt_obj_st * gt_examples_barcode(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);

#if GT_CFG_ENABLE_BARCODE && defined(__GT_BARCODE_EN_)
    gt_obj_st * barcode = gt_barcode_create(screen);
    gt_obj_set_pos(barcode, 50, 50);
    gt_obj_set_size(barcode, 250, 80);
    gt_barcode_set_code_text(barcode, "1234567890");
    gt_barcode_set_mode_w(barcode, 2);
    gt_barcode_set_mode_h(barcode, 60);
#endif

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
