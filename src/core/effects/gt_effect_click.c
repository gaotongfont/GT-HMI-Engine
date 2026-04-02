/**
 * @file gt_effect_click.c
 * @author Feyoung
 * @brief Generic click feedback effect implementation.
 *        Press: instant opacity dim. Release: animated restore.
 *        Zero static RAM — effect data stored in event user_data per widget.
 * @version 0.2
 * @date 2026-04-02
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_effect_click.h"

#if GT_USE_CLICK_EFFECT

#include "../../core/gt_mem.h"
#include "../../core/gt_disp.h"
#include "../../core/gt_event.h"
#include "../../others/gt_log.h"

/* private typedef ------------------------------------------------------*/
struct _gt_effect_click_s {
    gt_anim_st * anim;
    uint16_t time_ms;
    uint8_t opa_origin;
    uint8_t opa_pressed;
    gt_anim_path_type_em path_type;
    uint8_t active : 1;
    uint8_t reserved : 7;
};

/* static functions -----------------------------------------------------*/

static void _stop_anim(gt_obj_st * obj, gt_effect_click_st * eff) {
    if (eff->anim) {
        gt_anim_del_safe_by(obj);
        eff->anim = NULL;
    }
}

/* --- restore animation callbacks --- */

static void _restore_exec_cb(void * tar, int32_t value) {
    gt_obj_st * obj = (gt_obj_st *)tar;
    gt_effect_click_st ** pp = (gt_effect_click_st **)gt_anim_get_data();
    if (NULL == pp || NULL == *pp) { return; }
    gt_effect_click_st * eff = *pp;
    int32_t opa = (int32_t)eff->opa_pressed +
        ((int32_t)eff->opa_origin - eff->opa_pressed) * value / 1000;
    obj->opa = (uint8_t)opa;
    gt_disp_invalid_area(obj);
}

static void _restore_ready_cb(struct gt_anim_s * anim) {
    gt_obj_st * obj = (gt_obj_st *)anim->tar;
    gt_effect_click_st ** pp = (gt_effect_click_st **)anim->data;
    if (NULL == pp || NULL == *pp) { return; }
    gt_effect_click_st * eff = *pp;
    obj->opa = eff->opa_origin;
    eff->anim = NULL;
    eff->active = 0;
    gt_disp_invalid_area(obj);
}

/* --- event callbacks --- */

static void _pressed_event_cb(gt_event_st * e) {
    gt_obj_st * obj = e->origin;
    gt_effect_click_st * eff = (gt_effect_click_st *)e->user_data;
    if (NULL == eff) { return; }
    _stop_anim(obj, eff);
    if (0 == eff->active) {
        eff->opa_origin = obj->opa;
    }
    eff->active = 1;
    obj->opa = eff->opa_pressed;
    gt_disp_invalid_area(obj);
}

static void _released_event_cb(gt_event_st * e) {
    gt_obj_st * obj = e->origin;
    gt_effect_click_st * eff = (gt_effect_click_st *)e->user_data;
    if (NULL == eff || 0 == eff->active) { return; }
    _stop_anim(obj, eff);

    gt_anim_st anim;
    gt_anim_init(&anim);
    gt_anim_set_target(&anim, obj);
    gt_anim_set_time(&anim, eff->time_ms);
    gt_anim_set_value(&anim, 0, 1000);
    gt_anim_set_exec_cb(&anim, _restore_exec_cb);
    gt_anim_set_ready_cb(&anim, _restore_ready_cb);
    gt_anim_set_path_type(&anim, eff->path_type);
    gt_anim_set_data(&anim, &eff, sizeof(eff));
    eff->anim = gt_anim_start(&anim);
}

static void _deleted_event_cb(gt_event_st * e) {
    gt_effect_click_st * eff = (gt_effect_click_st *)e->user_data;
    if (NULL == eff) { return; }
    if (eff->anim) {
        gt_anim_del_safe_by(e->origin);
        eff->anim = NULL;
    }
    gt_mem_free(eff);
}

/* public functions ---------------------------------------------------------*/

gt_effect_click_st * gt_effect_click_register(gt_obj_st * obj) {
    if (NULL == obj) { return NULL; }

    gt_effect_click_st * eff = (gt_effect_click_st *)gt_mem_malloc(sizeof(gt_effect_click_st));
    if (NULL == eff) { return NULL; }

    eff->anim        = NULL;
    eff->time_ms     = GT_EFFECT_CLICK_TIME_DEFAULT;
    eff->opa_origin  = obj->opa;
    eff->opa_pressed = GT_EFFECT_CLICK_OPA_DEFAULT;
    eff->path_type   = GT_ANIM_PATH_TYPE_EASE_OUT;
    eff->active      = 0;
    eff->reserved    = 0;

    gt_obj_add_event_cb(obj, _pressed_event_cb,  GT_EVENT_TYPE_INPUT_PRESSED,    eff);
    gt_obj_add_event_cb(obj, _released_event_cb, GT_EVENT_TYPE_INPUT_RELEASED,   eff);
    gt_obj_add_event_cb(obj, _released_event_cb, GT_EVENT_TYPE_INPUT_PRESS_LOST, eff);
    gt_obj_add_event_cb(obj, _deleted_event_cb,  GT_EVENT_TYPE_CHANGE_DELETED,   eff);
    return eff;
}

void gt_effect_click_set_opa(gt_effect_click_st * eff, uint8_t opa) {
    if (NULL == eff) { return; }
    eff->opa_pressed = opa;
}

void gt_effect_click_set_time(gt_effect_click_st * eff, uint16_t time_ms) {
    if (NULL == eff) { return; }
    eff->time_ms = time_ms;
}

void gt_effect_click_set_path_type(gt_effect_click_st * eff, gt_anim_path_type_em type) {
    if (NULL == eff) { return; }
    eff->path_type = type;
}

#endif /* GT_USE_CLICK_EFFECT */
