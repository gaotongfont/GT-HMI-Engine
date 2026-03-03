/**
 * @file gt_roll_anim.c
 * @author xiaoliu17
 * @brief
 * @version 0.1
 * @date 2025-08-19
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/

#include "gt_roll_anim.h"

#include "../core/gt_disp.h"
#include "../widgets/gt_label.h"
#include "../widgets/gt_btn.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static _auto_scroll_st * GT_ATTRIBUTE_RAM_TEXT
_create_auto_scroll_st(gt_obj_st * obj) {
    _auto_scroll_st * auto_scroll_p = NULL;
    if (NULL == auto_scroll_p) {
        auto_scroll_p = gt_mem_malloc(sizeof(_auto_scroll_st));
        GT_CHECK_BACK_VAL(auto_scroll_p, NULL);
        _roll_anim_auto_scroll_init(obj, auto_scroll_p);
    }

    return _roll_anim_reset_auto_scroll_ctrl(auto_scroll_p);
}

static void GT_ATTRIBUTE_RAM_TEXT
_auto_scroll_exec_cb(void * obj, int32_t value) {
    _auto_scroll_st * auto_scroll_p = (_auto_scroll_st *)obj;
    if (NULL == auto_scroll_p) {
        return;
    }
    if (auto_scroll_p->paused) {
        return;
    }
    if (false == gt_obj_is_child(auto_scroll_p->parent, gt_disp_get_scr())
#if GT_USE_LAYER_TOP
        || false == gt_obj_is_child(auto_scroll_p->parent, gt_disp_get_layer_top())
#endif
    ) {
        return;
    }
    auto_scroll_p->area.x = value;
    if (GT_INVISIBLE == gt_obj_get_visible(auto_scroll_p->parent)) {
        return;
    }
    gt_event_send(auto_scroll_p->parent, GT_EVENT_TYPE_DRAW_START, NULL);
}

/* global functions / API interface -------------------------------------*/

void GT_ATTRIBUTE_RAM_TEXT
_roll_anim_free_auto_ctrl(_auto_scroll_st ** auto_scroll)
{
    _auto_scroll_st * auto_scroll_p = *auto_scroll;
    if (NULL == auto_scroll_p) {
        return ;
    }

    if (auto_scroll_p->anim) {
        gt_anim_del_by(auto_scroll_p->anim);
        auto_scroll_p->anim = NULL;
    }
    gt_mem_free(auto_scroll_p);
    auto_scroll_p = NULL;
    *auto_scroll = auto_scroll_p;
}

_auto_scroll_st * GT_ATTRIBUTE_RAM_TEXT
_roll_anim_reset_auto_scroll_ctrl(_auto_scroll_st * as)
{
    if (NULL == as) {
        return NULL;
    }
    if (as->anim) {
        gt_anim_restart(as->anim);
        if (false == gt_anim_is_paused(as->anim)) {
            gt_anim_set_paused(as->anim, true);
        }
        gt_anim_set_value(as->anim, 0, 0);
    }
    as->area.x = 0;
    as->area.y = 0;
    as->area.w = 0xffff;
    as->area.h = 0;
    return as;
}

void GT_ATTRIBUTE_RAM_TEXT
_roll_anim_auto_scroll_init(gt_obj_st * obj, _auto_scroll_st * const auto_scroll_p)
{
    if (NULL == auto_scroll_p) {
        return ;
    }
    gt_memset(auto_scroll_p, 0, sizeof(_auto_scroll_st));
    auto_scroll_p->parent = obj;
    auto_scroll_p->total_time = 2000;   /** 2s */
    auto_scroll_p->area.w = 0xffff;
}

_auto_scroll_st * GT_ATTRIBUTE_RAM_TEXT
_roll_anim_create_auto_scroll_anim(gt_obj_st * obj, _auto_scroll_st * auto_scroll)
{
    _auto_scroll_st * auto_scroll_p = auto_scroll;
    if (NULL == auto_scroll_p) {
        auto_scroll_p = _create_auto_scroll_st(obj);
        GT_CHECK_BACK_VAL(auto_scroll_p, NULL);
    }
    if (auto_scroll_p->anim) {
        _roll_anim_reset_auto_scroll_ctrl(auto_scroll_p);
        return auto_scroll_p;
    }
    gt_anim_st anim;
    gt_anim_init(&anim);
    gt_anim_set_target(&anim, auto_scroll_p);
    gt_anim_set_value(&anim, 0, 0);
    gt_anim_set_time_delay_start(&anim, 500);
    gt_anim_set_repeat_delay(&anim, 500);
    gt_anim_set_exec_cb(&anim, _auto_scroll_exec_cb);
    gt_anim_set_playback(&anim, true);
    gt_anim_set_repeat_count(&anim, GT_ANIM_REPEAT_INFINITE);
    auto_scroll_p->anim = gt_anim_start(&anim);
    gt_anim_set_paused(auto_scroll_p->anim, true);

    return auto_scroll_p;
}


void GT_ATTRIBUTE_RAM_TEXT
_roll_anim_first_restart_scroll(_gt_text_style_st text_style, _auto_scroll_st * auto_scroll,
    gt_area_st box_area, _gt_draw_font_res_st font_res)
{
    if (!text_style.single_line || NULL == auto_scroll) {
        return ;
    }

    /** first time to calc label widget need to scroll and begin to scroll */
    if (auto_scroll->paused) {
        return;
    }
    if (font_res.size.x > box_area.w && gt_anim_is_paused(auto_scroll->anim)) {
        auto_scroll->area.w = font_res.size.x;
        gt_anim_set_time(auto_scroll->anim, auto_scroll->total_time);
        gt_anim_set_value(auto_scroll->anim, 0, box_area.w - auto_scroll->area.w);
        gt_anim_restart(auto_scroll->anim);
    }
}

void GT_ATTRIBUTE_RAM_TEXT
_roll_anim_paused(_auto_scroll_st * auto_scroll, bool paused)
{
    if (NULL == auto_scroll) {
        return ;
    }
    auto_scroll->paused = paused;
}
/* end ------------------------------------------------------------------*/
