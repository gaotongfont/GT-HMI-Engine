/**
 * @file gt_example_player.c
 * @author Feyoung
 * @brief Player widget example
 * @version 0.1
 * @date 2026-03-27
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_player.h"
#include "../../src/widgets/gt_btn.h"

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/
static gt_obj_st * _player = NULL;

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/
static void _btn_play_cb(gt_event_st * e)
{
    if (_player) {
        gt_player_toggle(_player);
    }
}

static void _btn_next_cb(gt_event_st * e)
{
    if (_player) {
        gt_player_turn_next(_player);
    }
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief Create a player with play/next controls.
 */
gt_obj_st * gt_examples_player(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);

    _player = gt_player_create(screen);
    gt_obj_set_pos(_player, 50, 10);
    gt_obj_set_size(_player, 200, 150);
    gt_player_set_auto_play_period(_player, 500);

    gt_obj_st * btn_play = gt_btn_create(screen);
    gt_obj_set_pos(btn_play, 50, 180);
    gt_obj_set_size(btn_play, 80, 36);
    gt_btn_set_text(btn_play, "Play");
    gt_obj_add_event_cb(btn_play, _btn_play_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    gt_obj_st * btn_next = gt_btn_create(screen);
    gt_obj_set_pos(btn_next, 150, 180);
    gt_obj_set_size(btn_next, 80, 36);
    gt_btn_set_text(btn_next, "Next");
    gt_obj_add_event_cb(btn_next, _btn_next_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
