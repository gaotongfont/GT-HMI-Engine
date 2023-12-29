/**
 * @file gt_example_keyboard.c
 * @author Li
 * @brief
 * @version 0.1
 * @date 2023-03-14 16:57:32
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/core/gt_event.h"
#include "../../src/widgets/gt_keypad.h"
#include "../../src/widgets/gt_input.h"


/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/


/* global functions / API interface -------------------------------------*/

/**
 * @brief create a keyboard.
 */
void gt_examples_keyboard(void)
{
    gt_obj_st * screen;
    screen = gt_obj_create(NULL);

    gt_obj_st * keyboard = gt_keypad_create(screen);//Ä¬ÈÏÎª¿í300 ¸ß170
    gt_obj_set_pos(keyboard , 0 , 50);

    gt_obj_st* input = gt_input_create(screen);
    gt_obj_set_pos(input , 0 , 55);
    gt_obj_set_size(input , 300 , 25);
    gt_keypad_set_target(keyboard , input);


    gt_disp_load_scr(screen);

}

#endif
/* end ------------------------------------------------------------------*/
