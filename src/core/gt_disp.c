/**
 * @file gt_disp.c
 * @author yongg
 * @brief Display implementation for the screen.
 * @version 0.1
 * @date 2022-05-11 14:59:50
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

 /* include --------------------------------------------------------------*/
#include "../gt_conf.h"
#include "../widgets/gt_obj.h"
#include "../others/gt_log.h"
#include "gt_obj_pos.h"
#include "../hal/gt_hal_disp.h"
#include "gt_disp.h"
#include "../extra/draw/gt_draw_blend.h"
#include "../core/gt_draw.h"
#include "../core/gt_obj_pos.h"
#include "../others/gt_area.h"
#include "../others/gt_anim.h"
#include "../core/gt_mem.h"
#include "../core/gt_indev.h"
#include "../others/gt_gc.h"

#include "./gt_obj_scroll.h"

/* private define -------------------------------------------------------*/

#ifndef _GT_STACK_USE_LOADING_LOG
    /**
     * @brief 0[defalut]: Close the page stack log
     */
    #define _GT_STACK_USE_LOADING_LOG   0
#endif


/* private typedef ------------------------------------------------------*/

typedef enum{
    GT_NOT_BUSY,
    GT_BUSY,
}gt_busy_et;

/**
 * @brief flush screen by animation or direct param
 */
typedef struct _flush_scr_param_s {
    gt_disp_st * disp;
    gt_area_st area_flush;         // The buffer or area of display flush area.
#if GT_USE_SCREEN_ANIM
    gt_area_st view_scr_abs;       // The new interface displays the absolute extent of the area on the physical screen
    gt_area_st view_scr_prev_abs;  // The previous interface displays the absolute extent within the area on the physical screen
#endif
    _gt_draw_valid_st valid;       // Interface intersection results and screen display start offset
    uint16_t line;                 // Number of rows per refresh
}_flush_scr_param_st;

typedef struct _bg_abs_area_st {
    gt_area_st area;
    gt_color_t color;
}_bg_abs_area_st;

/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
/**
 * @brief Is display busy
 *
 * @param disp
 * @return true busy now
 * @return false
 */
static bool _gt_disp_get_state(gt_disp_st * disp)
{
    if( disp == NULL ){
        return GT_BUSY;
    }
    if( disp->drv == NULL ){
        return GT_BUSY;
    }
    return disp->drv->busy;
}

static void _gt_disp_set_state(gt_disp_st * disp, uint8_t state)
{
    if( disp == NULL ){
        return;
    }
    if( disp->drv == NULL ){
        return;
    }
    disp->drv->busy = state;
}

/**
 * @brief foreach obj send draw start event
 *
 * @param obj parent
 */
static void _gt_disp_send_draw_event_foreach(gt_obj_st * obj)
{
    uint16_t idx = 0;
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
    while( idx < obj->cnt_child ) {
        _gt_disp_send_draw_event_foreach( obj->child[idx] );
        idx++;
    }
}

/**
 * @brief Checked the obj is display area visible
 *
 * @param obj The object to be checked and copied
 * @param param
 * @param area_parent All parent displayable areas
 * @return true: visible or area need to refresh, false: invisible or area not need to refresh
 */
static inline bool gt_check_obj_visible_and_copy(gt_obj_st * obj, _flush_scr_param_st * param, gt_area_st area_parent)
{
    if( GT_INVISIBLE == gt_obj_get_visible(obj) ){
        return false;
    }
    if (false == gt_area_is_intersect_screen(&param->disp->area_disp, &obj->area)) {
        return false;
    }

    _gt_draw_ctx_st draw_ctx = {
        /** Page change animation */
        .valid    = param->disp->scr_prev ? &param->valid : NULL,
        .parent_area = obj->inside ? &area_parent : NULL,
        .buf      = param->disp->vbd_color,
        .buf_area = param->disp->area_disp,
    };

    obj->draw_ctx = (struct _gt_draw_ctx_s * )&draw_ctx;
    obj->class->_init_cb(obj);

    return true;
}

/**
 * @brief
 *
 * @param obj The object to be checked and copied
 * @param param
 * @param area_parent All parent displayable areas
 */
static void _check_and_copy_foreach(gt_obj_st * obj, _flush_scr_param_st * param, gt_area_st area_parent)
{
    uint16_t idx = 0;
    gt_obj_st * child_p = NULL;
    gt_area_st area_cross = area_parent;
    gt_area_st reduce = obj->area;

    if (GT_TYPE_SCREEN != gt_obj_class_get_type(obj)) {
        reduce = gt_area_reduce(obj->area, gt_obj_get_reduce(obj));
        if (false == gt_area_cover_screen(&area_parent, &reduce, &area_cross)) {
            if (GT_TYPE_GROUP != gt_obj_class_get_type(obj)) {
                /** The Group ignores the area effects */
                return ;
            }
        }
    } else {
        /** screen */
        if (GT_INVISIBLE == gt_obj_get_visible(obj)) {
            return ;
        }
    }

    while( idx < obj->cnt_child ) {
        child_p = obj->child[idx++];
        if (false == gt_check_obj_visible_and_copy(child_p, param, area_cross)) {
            /** No need to iterate over child widgets */
            continue;
        }

        if( child_p->cnt_child != 0 ){
            _check_and_copy_foreach(child_p, param, area_cross);
        }
    }
}

static inline void _gt_disp_check_and_copy_foreach(gt_obj_st * obj, _flush_scr_param_st * param) {
    if (NULL == obj) {
        return ;
    }
    /** The screen display area */
    _check_and_copy_foreach(obj, param, param->disp->area_disp);
}

static void _scr_anim_exec_x_cb(gt_obj_st * obj, int32_t x) {
    gt_obj_set_x(obj, (gt_size_t)x);
}

static void _scr_anim_exec_y_cb(gt_obj_st * obj, int32_t y) {
    gt_obj_set_y(obj, (gt_size_t)y);
}

static void _scr_anim_del_ready_cb(struct gt_anim_s * anim) {
    anim->target->using = 0;
    gt_obj_destroy(anim->target);
}

#if GT_USE_SCREEN_ANIM
static void _scr_anim_start_cb(struct gt_anim_s * anim) {
    gt_disp_st * disp = gt_disp_get_default();
    disp->scr_prev         = disp->scr_act;
    disp->scr_act          = anim->target;

    /** Remark the begin status prev and scr position */
    disp->anim_scr_remark.x = disp->scr_act->area.x;
    disp->anim_scr_remark.y = disp->scr_act->area.y;

    /** Can not calling other event by all of input device */
    gt_indev_set_disabled(true);
}

/**
 * @brief Reset old screen display position
 *
 * @param anim
 */
static void _old_scr_anim_ready_cb(struct gt_anim_s * anim) {
    gt_area_st * area = (gt_area_st * )anim->data;
    gt_obj_st * old_scr = anim->target;
    if (NULL == old_scr) {
        return ;
    }
    if (old_scr->delate) {
        return ;
    }
    if (false == old_scr->using) {
        return ;
    }
    gt_area_copy(&old_scr->area, area);
}

static void _scr_anim_ready_cb(struct gt_anim_s * anim) {
    gt_disp_st * disp = gt_disp_get_default();
    if (NULL == disp) {
        return;
    }
    if (disp->scr_prev && disp->scr_prev->delate) {
        disp->scr_prev->using = 0;
        gt_obj_destroy(disp->scr_prev);
    }

    disp->scr_prev      = NULL;
    disp->area_act.x    = disp->scr_act->area.x;
    disp->area_act.y    = disp->scr_act->area.y;

    disp->scr_act->area.w = gt_disp_get_res_hor(NULL);
    disp->scr_act->area.h = gt_disp_get_res_ver(NULL);

    _gt_disp_reload_max_area(anim->target);
    gt_disp_invalid_area(disp->scr_act);

    /** Enabled all of input device event */
    gt_indev_set_disabled(false);
}

static bool _is_anim_type_hor(gt_scr_anim_type_et anim_type) {
    if (GT_SCR_ANIM_TYPE_MOVE_LEFT == anim_type) {
        return true;
    }
    if (GT_SCR_ANIM_TYPE_MOVE_RIGHT == anim_type) {
        return true;
    }
    if (GT_SCR_ANIM_TYPE_COVER_LEFT == anim_type) {
        return true;
    }
    if (GT_SCR_ANIM_TYPE_COVER_RIGHT == anim_type) {
        return true;
    }
    return false;
}

static bool _is_anim_type_ver(gt_scr_anim_type_et anim_type) {
    if (GT_SCR_ANIM_TYPE_MOVE_UP == anim_type) {
        return true;
    }
    if (GT_SCR_ANIM_TYPE_MOVE_DOWN == anim_type) {
        return true;
    }
    if (GT_SCR_ANIM_TYPE_COVER_UP == anim_type) {
        return true;
    }
    if (GT_SCR_ANIM_TYPE_COVER_DOWN == anim_type) {
        return true;
    }
    return false;
}

static void _fill_color_hor(_flush_scr_param_st * param, _bg_abs_area_st * left, _bg_abs_area_st * right, bool is_left) {
    gt_disp_st * disp = param->disp;
    uint32_t cnt = 0;
    uint16_t row = 0;
    uint16_t line = GT_REFRESH_FLUSH_LINE_PRE_TIME;
    uint16_t len_pre_line = disp->area_act.w * sizeof(gt_color_t);

    if (is_left) {
        gt_color_fill(&disp->vbd_color[cnt], left->area.w, left->color);
        len_pre_line = left->area.w * sizeof(gt_color_t);
        for (row = 1; row < line; row++) {
            gt_memmove(&disp->vbd_color[cnt + disp->area_act.w], &disp->vbd_color[cnt], len_pre_line);
            cnt += disp->area_act.w;
        }
        return;
    }

    /** right */
    uint16_t right_length = 0;
    if (left->area.w == disp->area_act.w) {
        /** new screen cover into old screen from right side */
        cnt = gt_abs(right->area.x);
    } else {
        cnt = left->area.w;
    }
    right_length = disp->area_act.w - cnt;
    gt_color_fill(&disp->vbd_color[cnt], right_length, right->color);
    len_pre_line = right_length * sizeof(gt_color_t);

    for (row = 1; row < line; row++) {
        gt_memmove(&disp->vbd_color[cnt + disp->area_act.w], &disp->vbd_color[cnt], len_pre_line);
        cnt += disp->area_act.w;
    }
}

static void _fill_color_ver(_flush_scr_param_st * param, _bg_abs_area_st * top, _bg_abs_area_st * bottom, gt_size_t cur_row, bool is_top) {
     gt_disp_st * disp = param->disp;
    uint16_t line = GT_REFRESH_FLUSH_LINE_PRE_TIME;
    uint32_t len = line * disp->area_act.w;
    uint32_t top_len = 0;

    if (cur_row < top->area.h) {
        // top
        if (cur_row + line > top->area.h) {
            top_len = (top->area.h - cur_row) * disp->area_act.w;
            if (is_top) {
                gt_color_fill(disp->vbd_color, top_len, top->color);
            } else {
                gt_color_fill(&disp->vbd_color[top_len], len - top_len, bottom->color);
            }
        } else {
            if (is_top) {
                gt_color_fill(disp->vbd_color, len, top->color);
            }
        }
    } else if (cur_row >= top->area.h) {
        if (false == is_top) {
            gt_color_fill(disp->vbd_color, len, bottom->color);
        }
    }
}

/**
 * @brief Horizontal animation direction, adapt to the display range of the interface
 *
 * @param param
 */
static inline void _adapt_area_flush_hor(_flush_scr_param_st * param) {
    gt_obj_st * prev = param->disp->scr_prev;
    gt_obj_st * scr = param->disp->scr_act;

    param->valid.is_hor = true;

    scr->area.w = gt_abs(scr->area.x - param->disp->anim_scr_remark.x);
    prev->area.w = gt_disp_get_res_hor(param->disp) - scr->area.w;
    param->view_scr_abs.w = scr->area.w;
    param->view_scr_prev_abs.w = prev->area.w;

    switch (param->disp->scr_anim_type) {
        case GT_SCR_ANIM_TYPE_COVER_LEFT:
        case GT_SCR_ANIM_TYPE_MOVE_LEFT: {
            param->view_scr_abs.x = prev->area.w;
            param->view_scr_prev_abs.x = 0;
            break;
        }
        case GT_SCR_ANIM_TYPE_COVER_RIGHT:
        case GT_SCR_ANIM_TYPE_MOVE_RIGHT: {
            param->view_scr_abs.x = 0;
            param->view_scr_prev_abs.x = scr->area.w;
            break;
        }
        default:
            break;
    }
}

/**
 * @brief In the direction of vertical animation, adapt to the display range of the interface
 *
 * @param param
 */
static inline void _adapt_area_flush_ver(_flush_scr_param_st * param) {
    gt_obj_st * prev = param->disp->scr_prev;
    gt_obj_st * scr = param->disp->scr_act;

    param->valid.is_hor = false;

    scr->area.h = gt_abs(scr->area.y - param->disp->anim_scr_remark.y);
    prev->area.h = gt_disp_get_res_ver(param->disp) - scr->area.h;
    param->view_scr_abs.h = scr->area.h;
    param->view_scr_prev_abs.h = prev->area.h;

    switch (param->disp->scr_anim_type) {
        case GT_SCR_ANIM_TYPE_COVER_UP:
        case GT_SCR_ANIM_TYPE_MOVE_UP: {
            param->view_scr_abs.y = prev->area.h;
            param->view_scr_prev_abs.y = 0;
            break;
        }
        case GT_SCR_ANIM_TYPE_COVER_DOWN:
        case GT_SCR_ANIM_TYPE_MOVE_DOWN: {
            param->view_scr_abs.y = 0;
            param->view_scr_prev_abs.y = scr->area.h;
            break;
        }
        default:
            break;
    }
}

static inline void _clear_buffer(_flush_scr_param_st * param, gt_color_t prev_c, gt_color_t cur_c, gt_size_t cur_row, bool is_prev) {
    gt_disp_st * disp = param->disp;
    _bg_abs_area_st prev = {
        .area = param->view_scr_prev_abs,
        .color = prev_c,
    };
    _bg_abs_area_st next = {
        .area = param->view_scr_abs,
        .color = cur_c,
    };

    switch (disp->scr_anim_type) {
        case GT_SCR_ANIM_TYPE_MOVE_LEFT:
        case GT_SCR_ANIM_TYPE_COVER_LEFT: {
            _fill_color_hor(param, &prev, &next, is_prev);
            break;
        }
        case GT_SCR_ANIM_TYPE_MOVE_RIGHT:
        case GT_SCR_ANIM_TYPE_COVER_RIGHT: {
            _fill_color_hor(param, &next, &prev, !is_prev);
            break;
        }
        case GT_SCR_ANIM_TYPE_MOVE_UP:
        case GT_SCR_ANIM_TYPE_COVER_UP: {
            _fill_color_ver(param, &prev, &next, cur_row, is_prev);
            break;
        }
        case GT_SCR_ANIM_TYPE_MOVE_DOWN:
        case GT_SCR_ANIM_TYPE_COVER_DOWN: {
            _fill_color_ver(param, &next, &prev, cur_row, !is_prev);
            break;
        }
        default:
            break;
    }
}

static inline void _adjust_clip_area_and_flush(_flush_scr_param_st * param, gt_obj_st * target_scr) {
    param->disp->area_disp.x = target_scr->area.x + param->valid.area_clip.x;
    param->disp->area_disp.y = target_scr->area.y + param->valid.area_clip.y;
    param->disp->area_disp.h = param->valid.area_clip.h;

    if (param->area_flush.y == param->valid.area_clip.y) {
        param->valid.area_clip.y = 0;
    } else {
        param->valid.area_clip.y = param->valid.area_clip.y - param->area_flush.y;
    }
    _gt_disp_check_and_copy_foreach(target_scr, param);
}

/**
 * @brief flush screen by animation
 *
 * @param param The package of all of param, using such as: disp, area_flush, scr, scr_prev, view_scr_prev_abs, valid, line.
 * @param disp
 * @param area_flush The buffer or area of display flush area.
 * @param scr       The current screen absolute display area
 * @param scr_prev  The previous screen absolute display area
 * @param view_scr_abs The next screen in the display area of the physical window
 * @param view_scr_prev_abs The previous screen in the display area of the physical window
 * @param valid Interface intersection results and screen display start offset
 * @param line Number of rows per refresh
 */
static inline void _flush_scr_by_anim(_flush_scr_param_st * param) {
    gt_obj_st * prev = param->disp->scr_prev;
    gt_obj_st * scr = param->disp->scr_act;
    bool is_cover = false;

    // 计算scr 在屏幕的显示区域
    if (_is_anim_type_hor((gt_scr_anim_type_et)param->disp->scr_anim_type)) {
        _adapt_area_flush_hor(param);
    }
    if (_is_anim_type_ver((gt_scr_anim_type_et)param->disp->scr_anim_type)) {
        _adapt_area_flush_ver(param);
    }

    while(param->area_flush.y < param->disp->area_act.h) {
        /** prev screen */
        _clear_buffer(param, prev->bgcolor, scr->bgcolor, param->area_flush.y, true);

        is_cover = gt_area_cover_screen(&param->area_flush, &param->view_scr_prev_abs, &param->valid.area_clip);
        if (is_cover) {
            _adjust_clip_area_and_flush(param, prev);
        }

        /** new screen */
        _clear_buffer(param, prev->bgcolor, scr->bgcolor, param->area_flush.y, false);

        is_cover = gt_area_cover_screen(&param->area_flush, &param->view_scr_abs, &param->valid.area_clip);
        if (is_cover) {
            _adjust_clip_area_and_flush(param, scr);
        }

#if GT_USE_LAYER_TOP
        // flush top layer
        is_cover = gt_area_cover_screen(&param->area_flush, &param->disp->layer_top->area, &param->valid.area_clip);
        if (is_cover) {
            param->valid.layer_top = true;
            param->valid.area_clip.x = 0;
            param->valid.area_clip.y = 0;
            param->disp->area_disp.x = param->area_flush.x;
            param->disp->area_disp.y = param->area_flush.y;
            param->disp->area_disp.h = param->line;
            _gt_disp_check_and_copy_foreach(param->disp->layer_top, param);
            param->valid.layer_top = false;
        }
#endif

        /** flush display by buffer area */
        param->disp->drv->flush_cb(param->disp->drv, &param->area_flush, param->disp->vbd_color);
        param->area_flush.y += param->line;
    }
}
#endif  /** GT_USE_SCREEN_ANIM */

/**
 * @brief flush screen by direct or full screen refresh
 *
 * @param param The package of all of param, using such as: disp, area_flush, scr, line.
 * @param disp
 * @param area_flush The buffer or area of display flush area.
 * @param scr       The current screen absolute display area
 * @param line Number of rows per refresh
 */
static inline void _flush_scr_by_direct(_flush_scr_param_st * param) {
    gt_obj_st * scr = param->disp->scr_act;
    gt_color_t color_fill = gt_screen_get_bgcolor(scr);
    uint16_t width = gt_disp_get_res_hor(param->disp);
    uint16_t height = gt_disp_get_res_ver(param->disp);

    scr->area.x = param->disp->area_act.x;
    scr->area.y = param->disp->area_act.y;

    if (false == gt_area_is_intersect_screen(&scr->area, &param->area_flush)) {
        return ;
    }

    if(width == param->area_flush.w && height == param->area_flush.h){
        if(param->area_flush.x < 0 || (param->area_flush.x + param->area_flush.w > width)){
            param->area_flush.x = 0;
        }
        if(param->area_flush.y < 0 || (param->area_flush.y + param->area_flush.h > height)){
            param->area_flush.y = 0;
        }
    }
    else{
        // Calculate the area_flush x coordinates and width
        if (scr->area.x == 0) {
            if(param->area_flush.x < 0){
                param->area_flush.w = param->area_flush.w + param->area_flush.x;
                param->area_flush.x = 0;
            }

            if ((param->area_flush.x % width) + param->area_flush.w > width) {
                param->area_flush.w = width - (param->area_flush.x % width);
            }
        }
        else if (scr->area.x > 0) {
            if (param->area_flush.x < 0) {
                param->area_flush.w = param->area_flush.w + param->area_flush.x;
                param->area_flush.x = 0;
            }

            if (param->area_flush.x > scr->area.x) {
                param->area_flush.x = param->area_flush.x - scr->area.x;
            } else {
                param->area_flush.w = (param->area_flush.w + param->area_flush.x) - scr->area.x;
                param->area_flush.x = 0;
            }

            if ((param->area_flush.x % width) + param->area_flush.w > width) {
                param->area_flush.w = width - (param->area_flush.x % width);
            }
        }
        else if (scr->area.x < 0) {
            if (param->area_flush.x < 0) {
                param->area_flush.w = param->area_flush.w + param->area_flush.x - scr->area.x;
                param->area_flush.x = 0;
            }

            if (param->area_flush.x > scr->area.x) {
                param->area_flush.x = param->area_flush.x - scr->area.x;
            } else if (param->area_flush.x > 0) {
                param->area_flush.x = param->area_flush.x - scr->area.x;
            }

            if ((param->area_flush.x % width) + param->area_flush.w > width) {
                param->area_flush.w = width - (param->area_flush.x % width);
            }
        }

        // Calculate the area_flush y coordinates and height
        if (scr->area.y == 0) {
            if (param->area_flush.y < 0) {
                param->area_flush.h = param->area_flush.h + param->area_flush.y;
                param->area_flush.y = 0;
            }

            if ((param->area_flush.y % height) + param->area_flush.h > height) {
                param->area_flush.h = height - (param->area_flush.y % height);
            }
        }
        else if (scr->area.y > 0) {
            if (param->area_flush.y < 0) {
                param->area_flush.h = param->area_flush.h + param->area_flush.y;
                param->area_flush.y = 0;
            }

            if (param->area_flush.y > scr->area.y) {
                param->area_flush.y = param->area_flush.y - scr->area.y;
            } else {
                param->area_flush.h = (param->area_flush.h + param->area_flush.y) - scr->area.y;
                param->area_flush.y = 0;
            }

            if ((param->area_flush.y % height) + param->area_flush.h > height) {
                param->area_flush.h = height - (param->area_flush.y % height);
            }
        }
        else if (scr->area.y < 0) {
            if (param->area_flush.y <= 0) {
                param->area_flush.h = param->area_flush.h + param->area_flush.y - scr->area.y;
                param->area_flush.y = 0;
            }

            if (param->area_flush.y > 0) {
                param->area_flush.y = param->area_flush.y - scr->area.y;
            }

            if ((param->area_flush.y % height) + param->area_flush.h > height) {
                param->area_flush.h = height - (param->area_flush.y % height);
            }
        }
    }

    uint16_t end_y = param->area_flush.y + param->area_flush.h;
    param->disp->area_disp.y = param->area_flush.y;
    param->disp->area_disp.w = param->area_flush.w;
    if (param->area_flush.h > param->line) {
        param->area_flush.h = param->line;
    }
    param->disp->area_disp.h = param->area_flush.h;

    uint32_t len = param->disp->area_disp.w * param->disp->area_disp.h;

    while (param->area_flush.y < end_y) {
        gt_color_fill(param->disp->vbd_color, len, color_fill);

        param->disp->area_disp.x = param->area_flush.x + scr->area.x;
        param->disp->area_disp.y = param->area_flush.y + scr->area.y;

        _gt_disp_check_and_copy_foreach(scr, param);

#if GT_USE_LAYER_TOP
        // flush top layer
        param->disp->area_disp.x = param->area_flush.x;
        param->disp->area_disp.y = param->area_flush.y;
        _gt_disp_check_and_copy_foreach(param->disp->layer_top, param);
#endif

        /** flush display by buffer area */
        param->disp->drv->flush_cb(param->disp->drv, &param->area_flush, param->disp->vbd_color);
        param->area_flush.y += param->line;

        if( end_y - param->area_flush.y < param->line){
            param->area_flush.h = end_y - param->area_flush.y;
            param->disp->area_disp.h = param->area_flush.h;
        }
    }
}

static inline gt_scr_anim_type_et _get_anti_anim_type(gt_scr_anim_type_et type) {
#if GT_USE_SCREEN_ANIM
    if (GT_SCR_ANIM_TYPE_MOVE_LEFT == type) { type = GT_SCR_ANIM_TYPE_MOVE_RIGHT; }
    else if (GT_SCR_ANIM_TYPE_MOVE_RIGHT == type) { type = GT_SCR_ANIM_TYPE_MOVE_LEFT; }
    else if (GT_SCR_ANIM_TYPE_MOVE_UP == type) { type = GT_SCR_ANIM_TYPE_MOVE_DOWN; }
    else if (GT_SCR_ANIM_TYPE_MOVE_DOWN == type) { type =  GT_SCR_ANIM_TYPE_MOVE_UP; }
    else if (GT_SCR_ANIM_TYPE_COVER_LEFT == type) { type = GT_SCR_ANIM_TYPE_COVER_RIGHT; }
    else if (GT_SCR_ANIM_TYPE_COVER_RIGHT == type) { type = GT_SCR_ANIM_TYPE_COVER_LEFT; }
    else if (GT_SCR_ANIM_TYPE_COVER_UP == type) { type = GT_SCR_ANIM_TYPE_COVER_DOWN; }
    else if (GT_SCR_ANIM_TYPE_COVER_DOWN == type) { type = GT_SCR_ANIM_TYPE_COVER_UP; }
    return type;
#else
    return GT_SCR_ANIM_TYPE_NONE;
#endif
}

static gt_obj_st * _create_scr_by_id(gt_scr_id_t scr_id) {
    gt_scr_init_func_cb_t scr_init_cb = gt_scr_stack_get_init_func(scr_id);
    if (NULL == scr_init_cb) {
        GT_LOGE(GT_LOG_TAG_GUI, "Screen[0x%X = %d] init callback is NULL", scr_id, scr_id);
        return NULL;
    }
    return scr_init_cb();
}

/* global functions / API interface -------------------------------------*/
gt_scr_id_t gt_disp_stack_go_back(gt_stack_size_t step)
{
    gt_scr_stack_item_st * target = NULL;
    gt_disp_st * disp = gt_disp_get_default();
    gt_obj_st * scr_old = gt_disp_get_scr();

    if (!disp) {
        return -1;
    }

    if (step <= 0) {
        target = gt_scr_stack_peek();
        return target ? target->prev_scr_id : -1;
    }

    target = gt_scr_stack_pop(step);
    if (NULL == target) {
        return -1;
    }

    gt_scr_stack_item_st new_item = {
        .current_scr = target->prev_scr_alive,
        .current_scr_id = target->prev_scr_id,
        .prev_scr_alive = NULL,
        .prev_scr_id = -1,
        .anim_type = target->anim_type,
        .time = target->time,
        .delay = target->delay,
    };
    new_item.anim_type = _get_anti_anim_type(new_item.anim_type);

    gt_scr_id_t home_id = gt_scr_stack_get_home_scr_id();
    if (-1 == new_item.current_scr_id) {
        if (false == gt_scr_stack_is_empty()) {
            gt_scr_stack_clear();
        }
        if (-1 != home_id) {
            new_item.current_scr_id = home_id;
            if (gt_scr_stack_is_home_scr_alive()) {
                new_item.current_scr = gt_scr_stack_get_home_scr();
            }
        }
        if (-1 == new_item.current_scr_id) {
            /** set bottom stack screen */
            new_item.current_scr_id = target->current_scr_id;
            new_item.current_scr = target->current_scr;
        }
    } else if (home_id == new_item.current_scr_id) {
        if (false == gt_scr_stack_is_empty()) {
            gt_scr_stack_clear();
        }
        if (gt_scr_stack_is_home_scr_alive()) {
            new_item.current_scr = gt_scr_stack_get_home_scr();
        }
    }

    if (NULL == new_item.current_scr) {
        new_item.current_scr = _create_scr_by_id(new_item.current_scr_id);
        if (NULL == new_item.current_scr) {
            return -1;
        }
        /** update prev current screen object */
        gt_scr_stack_item_st * prev = gt_scr_stack_peek();
        if (prev && prev->current_scr_id == new_item.current_scr_id) {
            prev->current_scr = new_item.current_scr;
        }
    }

    if (gt_scr_stack_get_home_scr_id() == new_item.current_scr_id ||
        gt_scr_stack_is_empty()) {
        if (false == gt_scr_stack_push(&new_item)) {
            return -1;
        }
    }

    if (scr_old != new_item.current_scr) {
#if _GT_STACK_USE_LOADING_LOG
        GT_LOGD(GT_LOG_TAG_GUI, "layer[%d] = new_scr[0x%X = %d]", gt_scr_stack_get_count(), new_item.current_scr_id, new_item.current_scr_id);
#endif
        gt_disp_load_scr_anim(new_item.current_scr, new_item.anim_type, new_item.time, new_item.delay, true);
    }

    return new_item.current_scr_id;
}

void gt_disp_stack_load_scr(gt_scr_id_t scr_id)
{
    gt_disp_stack_param_st param = {
        .scr_id = scr_id,
        .type = GT_SCR_ANIM_TYPE_NONE,
        .time = 300,
        .delay = 0,
        .del_prev_scr = true,
#if GT_DISP_STACK_IGNORE_REPEAT_SCREEN_ID
        .ignore_repeat_screen_id = false,
#endif
    };
    gt_disp_stack_load_scr_anim_st(&param);
}

void gt_disp_stack_load_scr_anim(gt_scr_id_t scr_id, gt_scr_anim_type_et type, uint32_t time, uint32_t delay, bool del_prev_scr)
{
    gt_disp_stack_param_st param = {
        .scr_id = scr_id,
        .type = type,
        .time = time,
        .delay = delay,
        .del_prev_scr = del_prev_scr,
#if GT_DISP_STACK_IGNORE_REPEAT_SCREEN_ID
        .ignore_repeat_screen_id = false,
#endif
    };
    gt_disp_stack_load_scr_anim_st(&param);
}

void gt_disp_stack_load_scr_anim_st(gt_disp_stack_param_st const * const param)
{
    if (NULL == param) {
        return;
    }
    gt_obj_st * scr_old = gt_disp_get_scr();
    bool del_prev_scr = param->del_prev_scr;

    gt_scr_stack_item_st new_item = {
        .current_scr_id = param->scr_id,
        .prev_scr_id = -1,
        .prev_scr_alive = NULL,
        .current_scr = NULL,
        .time = param->time,
        .delay = param->delay,
        .anim_type = param->type,
    };
    gt_scr_id_t home_id = gt_scr_stack_get_home_scr_id();

    /** Has the same page before and pop unused stack item */
    gt_stack_item_st result = gt_scr_stack_has_before(&new_item);
#if GT_DISP_STACK_IGNORE_REPEAT_SCREEN_ID
    if (param->ignore_repeat_screen_id) {
        result.data = NULL; /** Need to create a new screen to load */
    }
#endif
    if (result.data) {
        gt_scr_stack_item_st * same_scr = gt_scr_stack_pop(result.index_from_top);
        new_item.prev_scr_id = same_scr->prev_scr_id;
        new_item.prev_scr_alive = same_scr->prev_scr_alive;
        new_item.current_scr = same_scr->current_scr;
        del_prev_scr = true;    /** Must free old screen memory */
    } else {
        gt_scr_stack_item_st * prev = gt_scr_stack_peek();
        if (prev) {
            new_item.prev_scr_id = prev->current_scr_id;
            if (false == del_prev_scr) {
                new_item.prev_scr_alive = prev->current_scr;
            }
            if (home_id == new_item.prev_scr_id && gt_scr_stack_is_home_scr_alive()) {
                new_item.prev_scr_alive = NULL;
                del_prev_scr = false;
            }
        }
    }
    /** Home page reset stack */
    if (-1 != home_id && new_item.current_scr_id == home_id) {
        if (false == gt_scr_stack_is_empty()) {
            gt_scr_stack_clear();
        }
        new_item.prev_scr_id = -1;
        new_item.prev_scr_alive = NULL;
    }

    /** ready to init and load screen */
    if (NULL == new_item.current_scr) {
        new_item.current_scr = _create_scr_by_id(new_item.current_scr_id);
        if (NULL == new_item.current_scr) {
            return ;
        }
        if (home_id == new_item.current_scr_id) {
            gt_scr_stack_set_home_scr(new_item.current_scr);
        }
    }
    if (false == gt_scr_stack_push(&new_item)) {
        GT_LOGE(GT_LOG_TAG_GUI, "Push stack failed");
        return ;
    }
    if (scr_old == new_item.current_scr) {
        gt_disp_invalid_area(new_item.current_scr);
        return;
    }
#if _GT_STACK_USE_LOADING_LOG
    GT_LOGD(GT_LOG_TAG_GUI, "layer[%d] = new_scr[0x%X = %d]", gt_scr_stack_get_count(), new_item.current_scr_id, new_item.current_scr_id);
#endif
    gt_disp_load_scr_anim(new_item.current_scr, new_item.anim_type, new_item.time, new_item.delay, del_prev_scr);
}

void gt_disp_load_scr(gt_obj_st * scr)
{
    gt_disp_load_scr_anim(scr, GT_SCR_ANIM_TYPE_NONE, 300, 0, true);
}

void gt_disp_load_scr_anim(gt_obj_st * scr, gt_scr_anim_type_et type, uint32_t time, uint32_t delay, bool del_prev_scr)
{
    gt_disp_st * disp = gt_disp_get_default();
    gt_obj_st * scr_old = gt_disp_get_scr();

    if (!disp) {
        return;
    }
#if !GT_USE_SCREEN_ANIM
    type = GT_SCR_ANIM_TYPE_NONE;
#endif

    _gt_disp_refr_reset_areas();
    if (NULL == scr_old) {
        if (GT_SCR_ANIM_TYPE_NONE != type) {
            type = GT_SCR_ANIM_TYPE_NONE;
        }
    }

    scr->using = true;
    disp->scr_anim_type = type;

    if (GT_SCR_ANIM_TYPE_NONE == type) {
        if( scr_old != scr ){
            disp->area_act.x = scr->area.x;
            disp->area_act.y = scr->area.y;

            /** reset screen range of activity */
            _gt_disp_reload_max_area(scr);
        }
        gt_disp_set_scr(scr);

        // _gt_disp_send_draw_event_foreach(scr);

        disp->area_act.h = disp->drv->res_ver;
        disp->area_act.w = disp->drv->res_hor;

        gt_disp_ref_area(&disp->area_act);

        if (del_prev_scr && scr_old && scr_old != scr) {
            gt_anim_st anim_del;
            gt_anim_init(&anim_del);
            gt_anim_set_time(&anim_del, time);
            gt_anim_set_time_delay_start(&anim_del, delay);
            gt_anim_set_target(&anim_del, scr_old);
            gt_anim_set_ready_cb(&anim_del, _scr_anim_del_ready_cb);
            gt_anim_start(&anim_del);
        }
    }
#if GT_USE_SCREEN_ANIM
    else {
        gt_anim_st anim_old;
        gt_area_st area = scr_old->area;
        gt_anim_init(&anim_old);
        gt_anim_set_time(&anim_old, time);
        gt_anim_set_data(&anim_old, &area, sizeof(gt_area_st));
        gt_anim_set_time_delay_start(&anim_old, delay);
        gt_anim_set_ready_cb(&anim_old, _old_scr_anim_ready_cb);
        gt_anim_set_target(&anim_old, scr_old);

        gt_anim_st anim_new;
        gt_anim_init(&anim_new);
        gt_anim_set_time(&anim_new, time);
        gt_anim_set_time_delay_start(&anim_new, delay);
        gt_anim_set_target(&anim_new, scr);
        gt_anim_set_start_cb(&anim_new, _scr_anim_start_cb);
        gt_anim_set_ready_cb(&anim_new, _scr_anim_ready_cb);
        gt_anim_set_path_type(&anim_old, GT_ANIM_PATH_TYPE_EASE_IN_OUT);
        gt_anim_set_path_type(&anim_new, GT_ANIM_PATH_TYPE_EASE_IN_OUT);

        switch (disp->scr_anim_type)
        {
        case GT_SCR_ANIM_TYPE_MOVE_LEFT: {
            gt_anim_set_exec_cb(&anim_old, _scr_anim_exec_x_cb);
            gt_anim_set_value(&anim_old, area.x, area.x + disp->drv->res_hor);

            gt_anim_set_exec_cb(&anim_new, _scr_anim_exec_x_cb);
            gt_anim_set_value(&anim_new, scr->area.x - disp->drv->res_hor, scr->area.x);
            gt_obj_set_x(scr, scr->area.x - disp->drv->res_hor);
            break;
        }
        case GT_SCR_ANIM_TYPE_MOVE_RIGHT: {
            gt_anim_set_exec_cb(&anim_old, _scr_anim_exec_x_cb);
            gt_anim_set_value(&anim_old, area.x, area.x - disp->drv->res_hor);

            gt_anim_set_exec_cb(&anim_new, _scr_anim_exec_x_cb);
            gt_anim_set_value(&anim_new, scr->area.x + disp->drv->res_hor, scr->area.x);
            gt_obj_set_x(scr, scr->area.x + disp->drv->res_hor);
            break;
        }
        case GT_SCR_ANIM_TYPE_MOVE_UP: {
            gt_anim_set_exec_cb(&anim_old, _scr_anim_exec_y_cb);
            gt_anim_set_value(&anim_old, area.y, area.y + disp->drv->res_ver);

            gt_anim_set_exec_cb(&anim_new, _scr_anim_exec_y_cb);
            gt_anim_set_value(&anim_new, scr->area.y - disp->drv->res_ver, scr->area.y);
            gt_obj_set_y(scr, scr->area.y - disp->drv->res_ver);
            break;
        }
        case GT_SCR_ANIM_TYPE_MOVE_DOWN: {
            gt_anim_set_exec_cb(&anim_old, _scr_anim_exec_y_cb);
            gt_anim_set_value(&anim_old, area.y, area.y - disp->drv->res_ver);

            gt_anim_set_exec_cb(&anim_new, _scr_anim_exec_y_cb);
            gt_anim_set_value(&anim_new, scr->area.y + disp->drv->res_ver, scr->area.y);
            gt_obj_set_y(scr, scr->area.y + disp->drv->res_ver);
            break;
        }

        case GT_SCR_ANIM_TYPE_COVER_LEFT: {
            gt_anim_set_exec_cb(&anim_new, _scr_anim_exec_x_cb);
            gt_anim_set_value(&anim_new, scr->area.x - disp->drv->res_hor, scr->area.x);
            gt_obj_set_x(scr, scr->area.x - disp->drv->res_hor);
            break;
        }
        case GT_SCR_ANIM_TYPE_COVER_RIGHT: {
            gt_anim_set_exec_cb(&anim_new, _scr_anim_exec_x_cb);
            gt_anim_set_value(&anim_new, scr->area.x + disp->drv->res_hor, scr->area.x);
            gt_obj_set_x(scr, scr->area.x + disp->drv->res_hor);
            break;
        }
        case GT_SCR_ANIM_TYPE_COVER_UP: {
            gt_anim_set_exec_cb(&anim_new, _scr_anim_exec_y_cb);
            gt_anim_set_value(&anim_new, scr->area.y - disp->drv->res_ver, scr->area.y);
            gt_obj_set_y(scr, scr->area.y - disp->drv->res_ver);
            break;
        }
        case GT_SCR_ANIM_TYPE_COVER_DOWN: {
            gt_anim_set_exec_cb(&anim_new, _scr_anim_exec_y_cb);
            gt_anim_set_value(&anim_new, scr->area.y + disp->drv->res_ver, scr->area.y);
            gt_obj_set_y(scr, scr->area.y + disp->drv->res_ver);
            break;
        }
        default:
            break;
        }

        gt_anim_start(&anim_new);
        gt_anim_start(&anim_old);

        if (del_prev_scr) {
            /** When the animation is finished, the memory is freed */
            scr_old->delate = true;
        }
    }
#endif
}

void gt_disp_set_scr(gt_obj_st * scr)
{
    gt_disp_st * disp = gt_disp_get_default();
    if( NULL == disp ){
        return;
    }
    disp->scr_act = scr;
}

gt_obj_st * gt_disp_get_scr(void)
{
    gt_disp_st * disp = gt_disp_get_default();
    if( NULL == disp ){
        return NULL;
    }
    return disp->scr_act;
}

#if GT_USE_LAYER_TOP
gt_obj_st * gt_disp_get_layer_top(void)
{
    gt_disp_st * disp = gt_disp_get_default();
    if( NULL == disp ){
        return NULL;
    }
    return disp->layer_top;
}
#endif

void gt_disp_ref_area(const gt_area_st * coords)
{
    uint16_t scr_width = gt_disp_get_res_hor(NULL);
    uint16_t scr_height = gt_disp_get_res_ver(NULL);
    _flush_scr_param_st param = {
        .disp = gt_disp_get_default(),
        .area_flush = *coords,
#if GT_USE_SCREEN_ANIM
        .view_scr_abs = {0, 0, scr_width, scr_height},
        .view_scr_prev_abs = {0, 0, scr_width, scr_height},
#endif
        .valid = {
#if GT_USE_SCREEN_ANIM
            .area_clip = { 0, 0, 0, 0 },
#endif
            .is_hor    = false,
            .layer_top = false,
        },
        .line = GT_REFRESH_FLUSH_LINE_PRE_TIME,
    };

    if ( param.disp->scr_act == NULL ) {
        return;
    }
    if ( _gt_disp_get_state(param.disp) == GT_BUSY ) {
        GT_LOGD(GT_LOG_TAG_GUI, "disp is busy");
        return;
    }
    _gt_disp_set_state(param.disp, GT_BUSY);

    param.disp->area_disp.w = scr_width;
    param.disp->area_disp.h = param.line;

#if GT_USE_SCREEN_ANIM
    if (param.disp->scr_prev) {
        param.area_flush.x = 0;
        param.area_flush.y = 0;
        param.area_flush.w = scr_width;
        param.area_flush.h = GT_REFRESH_FLUSH_LINE_PRE_TIME;
        _flush_scr_by_anim(&param);
    } else {
        _flush_scr_by_direct(&param);
    }
#else
    _flush_scr_by_direct(&param);
#endif

    _gt_disp_set_state(param.disp, GT_NOT_BUSY);
}

gt_area_st * gt_disp_get_area_act(void)
{
    gt_disp_st * disp_dev = gt_disp_get_default();
    return &disp_dev->area_act;
}

gt_area_abs_st * gt_disp_get_area_max(void)
{
    gt_disp_st * disp_dev = gt_disp_get_default();
    return &disp_dev->area_max;
}

void gt_disp_scroll_area_act(int16_t dist_x, int16_t dist_y)
{
    gt_area_st * area = gt_disp_get_area_act();
    gt_area_abs_st * max_area = gt_disp_get_area_max();

    area->x = dist_x;
    area->y = dist_y;

    /** Limit the range where the display can move */
    if (area->x < max_area->left) {
        area->x = max_area->left;
    } else if (area->x + area->w > max_area->right) {
        if (area->w < max_area->right) {
            area->x = max_area->right - area->w;
        } else if (area->x > 0) {
            area->x = 0;
        }
    }

    /** Limit the range where the display can move */
    if (area->y < max_area->top) {
        area->y = max_area->top;
    } else if (area->y + area->h > max_area->bottom) {
        if (area->h < max_area->bottom) {
            area->y = max_area->bottom - area->h;
        } else if (area->y > 0) {
            area->y = 0;
        }
    }

    _gt_disp_refr_reset_areas();
    _gt_disp_refr_append_area(area);
}

void gt_disp_invalid_area(gt_obj_st * obj)
{
    gt_obj_st * scr = gt_disp_get_scr();
    if (NULL == obj) {
        /** full screen refresh */
        obj = scr;
        obj->area.w = gt_disp_get_res_hor(NULL);
        obj->area.h = gt_disp_get_res_ver(NULL);
    }
    gt_area_st invalid = obj->area;
#if GT_USE_LAYER_TOP
    gt_obj_st * top = gt_disp_get_layer_top();

    if (top && gt_obj_is_child(obj, top)) {
        invalid.x += gt_obj_get_x(scr);
        invalid.y += gt_obj_get_y(scr);
    }
    else if( !gt_obj_check_scr(obj)){
        return;
    }
#else
    if( !gt_obj_check_scr(obj)){
        return;
    }
#endif
    _gt_disp_refr_append_area(&invalid);
}

/* end ------------------------------------------------------------------*/
