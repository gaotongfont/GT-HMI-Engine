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
    gt_area_st area_dirty;         // The area of display dirty area.
    gt_area_st area_flush;         // The buffer or area of display flush area.
    gt_area_st view_scr_abs;       // The new interface displays the absolute extent of the area on the physical screen
    gt_area_st view_scr_prev_abs;  // The previous interface displays the absolute extent within the area on the physical screen
    _gt_draw_valid_st valid;       // Interface intersection results and screen display start offset
    uint16_t line;                 // Number of rows per refresh
}_flush_scr_param_st;

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

static gt_obj_st * _get_scr_prev(void)
{
    gt_disp_st * disp = gt_disp_get_default();
    if( NULL == disp ){
        return NULL;
    }
    return disp->scr_prev;
}

/**
 * @brief foreach obj send draw start event
 *
 * @param obj parent
 */
static void _gt_disp_send_draw_event_foreach(gt_obj_st * obj)
{
    int16_t idx = 0;
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
    if (false == gt_area_is_intersect_screen(&param->disp->area_disp, &obj->area)) {
        return false;
    }

    if( !_gt_obj_is_disp_area_visible(obj) ){
        return false;
    }
    gt_disp_st * disp = param->disp;

    gt_draw_ctx_t draw_ctx = {
        /** Page change animation */
        .valid    = param->disp->scr_prev ? &param->valid : NULL,
        .parent_area = obj->inside ? &area_parent : NULL,
        .buf      = disp->vbd_color,
        .buf_area = disp->area_disp,
    };

    obj->draw_ctx = (struct _gt_draw_ctx_t * )&draw_ctx;
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
    int16_t idx = 0;
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

static void _scr_anim_start_cb(struct gt_anim_s * anim) {
    gt_disp_st * disp = gt_disp_get_default();
    disp->scr_prev         = disp->scr_act;
    disp->scr_act          = anim->target;
    disp->scr_prev->area.x = 0;
    disp->scr_prev->area.y = 0;

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

static void _reset_layer_top_area(gt_disp_st * disp) {
    if (NULL == disp->layer_top) {
        return ;
    }
    _gt_obj_move_child_by(disp->layer_top,
        disp->area_act.x - disp->layer_top->area.x, disp->area_act.y - disp->layer_top->area.y);
    disp->layer_top->area.x = disp->area_act.x;
    disp->layer_top->area.y = disp->area_act.y;
}

static void _scr_anim_ready_cb(struct gt_anim_s * anim) {
    gt_disp_st * disp = gt_disp_get_default();
    if (NULL == disp) {
        return;
    }
    if (disp->stack && disp->stack->need_backoff_scr == disp->scr_prev) {
        gt_mem_free(disp->stack);
        disp->stack = NULL;
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

    _reset_layer_top_area(disp);
    _gt_disp_reload_max_area(anim->target);
    gt_disp_invalid_area(disp->scr_act);

    /** Enabled all of input device event */
    gt_indev_set_disabled(false);
}

static void _scr_anim_del_ready_cb(struct gt_anim_s * anim) {
    gt_disp_st * disp = gt_disp_get_default();
    if (disp && disp->stack && disp->stack->need_backoff_scr == anim->target) {
        gt_mem_free(disp->stack);
        disp->stack = NULL;
    }
    anim->target->using = 0;
    gt_obj_destroy(anim->target);
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

static void _fill_color_hor(gt_disp_st * disp, gt_obj_st * left_scr, gt_obj_st * right_scr, bool is_left) {
    uint16_t row = 0;
    uint16_t line = GT_REFRESH_FLUSH_LINE_PRE_TIME;
    uint32_t cnt = 0;
    gt_color_t left_color = gt_screen_get_bgcolor(left_scr);
    gt_color_t right_color = gt_screen_get_bgcolor(right_scr);
    uint16_t len_pre_line = disp->area_act.w * sizeof(gt_color_t);

    if (is_left) {
        gt_color_fill(&disp->vbd_color[cnt], left_scr->area.w, left_color);
        len_pre_line = left_scr->area.w * sizeof(gt_color_t);
        for (row = 1; row < line; row++) {
            gt_memmove(&disp->vbd_color[cnt + disp->area_act.w], &disp->vbd_color[cnt], len_pre_line);
            cnt += disp->area_act.w;
        }
        return;
    }

    /** right */
    uint16_t right_length = 0;
    if (left_scr->area.w == disp->area_act.w) {
        /** new screen cover into old screen from right side */
        cnt = gt_abs(right_scr->area.x);
    } else {
        cnt = left_scr->area.w;
    }
    right_length = disp->area_act.w - cnt;
    gt_color_fill(&disp->vbd_color[cnt], right_length, right_color);
    len_pre_line = right_length * sizeof(gt_color_t);

    for (row = 1; row < line; row++) {
        gt_memmove(&disp->vbd_color[cnt + disp->area_act.w], &disp->vbd_color[cnt], len_pre_line);
        cnt += disp->area_act.w;
    }
}

static void _fill_color_ver(gt_disp_st * disp, gt_obj_st * top_scr, gt_obj_st * bottom_scr, gt_size_t cur_row, bool is_top) {
    gt_color_t top_color = gt_screen_get_bgcolor(top_scr);
    gt_color_t bottom_color = gt_screen_get_bgcolor(bottom_scr);
    uint16_t line = GT_REFRESH_FLUSH_LINE_PRE_TIME;
    uint32_t len = line * disp->area_act.w;
    uint32_t top_len = 0;

    if (cur_row < top_scr->area.h) {
        // top
        if (cur_row + line > top_scr->area.h) {
            top_len = (top_scr->area.h - cur_row) * disp->area_act.w;
            if (is_top) {
                gt_color_fill(disp->vbd_color, top_len, top_color);
            } else {
                gt_color_fill(&disp->vbd_color[top_len], len - top_len, bottom_color);
            }
        } else {
            if (is_top) {
                gt_color_fill(disp->vbd_color, len, top_color);
            }
        }
    } else if (cur_row >= top_scr->area.h) {
        if (false == is_top) {
            gt_color_fill(disp->vbd_color, len, bottom_color);
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
    prev->area.w = param->disp->area_act.w - gt_abs(prev->area.x);
    switch (param->disp->scr_anim_type) {
        case GT_SCR_ANIM_TYPE_MOVE_LEFT: {
            /** new screen */
            param->valid.offset_scr.x = prev->area.w;
            param->view_scr_abs.w = param->disp->area_act.w - prev->area.w;
            break;
        }
        case GT_SCR_ANIM_TYPE_COVER_RIGHT: {
            /** prev screen, don't move */
            param->view_scr_prev_abs.x = param->disp->area_act.x;
            param->valid.offset_prev.x = scr->area.w;
            /** new screen */
            param->view_scr_abs.w = scr->area.w;
            break;
        }
        case GT_SCR_ANIM_TYPE_COVER_LEFT: {
            /** prev screen, don't move */
            param->view_scr_prev_abs.w = param->disp->area_act.w - scr->area.w;
            /** new screen */
            param->view_scr_abs.w = scr->area.w;
            param->valid.offset_scr.x = gt_abs(scr->area.x);
            break;
        }
        case GT_SCR_ANIM_TYPE_MOVE_RIGHT: {
            /** prev screen */
            param->view_scr_prev_abs.x = param->disp->area_act.x + prev->area.x;
            /** new screen */
            param->view_scr_abs.w = gt_abs(prev->area.x);
            param->valid.offset_prev.x = gt_abs(prev->area.x);
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
    prev->area.h = param->disp->area_act.h - gt_abs(prev->area.y);
    param->view_scr_prev_abs.h = prev->area.h;
    switch (param->disp->scr_anim_type) {
        case GT_SCR_ANIM_TYPE_MOVE_UP: {
            param->valid.is_over_top = true;
            /** prev screen */
            param->view_scr_prev_abs.y = param->disp->area_act.y + prev->area.y; // area_act.y:原界面的偏移
            /** new screen */
            param->view_scr_abs.h = scr->area.h;
            param->valid.offset_scr.y = scr->area.y;
            break;
        }
        case GT_SCR_ANIM_TYPE_MOVE_DOWN: {
            param->valid.is_over_top = true;
            /** prev screen */
            param->view_scr_prev_abs.y = param->disp->area_act.y + prev->area.y + scr->area.h;
            param->view_scr_prev_abs.h = param->disp->area_act.h;
            if (param->disp->area_act.y > 0) {
                param->valid.offset_prev.y = param->disp->area_act.y;
            }
            /** new screen */
            param->view_scr_abs.h = scr->area.h;
            break;
        }
        case GT_SCR_ANIM_TYPE_COVER_UP: {
            /** prev screen, don't move */
            param->view_scr_prev_abs.y = param->disp->area_act.y;
            param->view_scr_prev_abs.h = param->disp->area_act.h - scr->area.h;
            prev->area.h = param->view_scr_prev_abs.h;
            /** new screen */
            param->view_scr_abs.h = scr->area.h;
            param->valid.offset_scr.y = scr->area.y;
            break;
        }
        case GT_SCR_ANIM_TYPE_COVER_DOWN: {
            /** prev screen, don't move */
            param->view_scr_prev_abs.y = param->disp->area_act.y + prev->area.y + scr->area.h;
            param->valid.offset_prev.y = scr->area.h;
            /** new screen */
            param->view_scr_abs.h = param->disp->area_act.h - gt_abs(scr->area.y);
            break;
        }
        default:
            break;
    }
}

static inline void _clear_buffer(gt_disp_st * disp, gt_obj_st * prev_scr, gt_obj_st * cur_scr, gt_size_t cur_row, bool is_prev) {

    if (NULL == prev_scr) {
        goto def_lb;    /** default */
    }

    switch (disp->scr_anim_type) {
        case GT_SCR_ANIM_TYPE_MOVE_LEFT:
        case GT_SCR_ANIM_TYPE_COVER_LEFT: {
            _fill_color_hor(disp, prev_scr, cur_scr, is_prev);
            break;
        }
        case GT_SCR_ANIM_TYPE_MOVE_RIGHT:
        case GT_SCR_ANIM_TYPE_COVER_RIGHT: {
            _fill_color_hor(disp, cur_scr, prev_scr, !is_prev);
            break;
        }
        case GT_SCR_ANIM_TYPE_MOVE_UP:
        case GT_SCR_ANIM_TYPE_COVER_UP: {
            _fill_color_ver(disp, prev_scr, cur_scr, cur_row, is_prev);
            break;
        }
        case GT_SCR_ANIM_TYPE_MOVE_DOWN:
        case GT_SCR_ANIM_TYPE_COVER_DOWN: {
            _fill_color_ver(disp, cur_scr, prev_scr, cur_row, !is_prev);
            break;
        }
        default:
            goto def_lb;
    }

    return;
def_lb:
    gt_color_fill(disp->vbd_color, GT_REFRESH_FLUSH_LINE_PRE_TIME * disp->area_act.w, gt_screen_get_bgcolor(cur_scr));
    return;
}

/**
 * @brief flush screen by animation
 *
 * @param param The package of all of param, using such as: disp, area_flush, scr, scr_prev, view_scr_prev_abs, valid, line.
 * @param disp
 * @param area_flush The buffer or area of display flush area.
 * @param scr       The current screen absolute display area
 * @param scr_prev  The previous screen absolute display area
 * @param view_scr_prev_abs The previous screen in the display area of the window
 * @param valid Interface intersection results and screen display start offset
 * @param line Number of rows per refresh
 */
static inline void _flush_scr_by_anim(_flush_scr_param_st * param) {
    gt_obj_st * prev = param->disp->scr_prev;
    gt_obj_st * scr = param->disp->scr_act;

    // 计算scr 在屏幕的显示区域
    if (_is_anim_type_hor(param->disp->scr_anim_type)) {
        scr->area.w = param->disp->area_act.w - gt_abs(scr->area.x);
        _adapt_area_flush_hor(param);
    }
    if (_is_anim_type_ver(param->disp->scr_anim_type)) {
        scr->area.h = param->disp->area_act.h - gt_abs(scr->area.y);
        _adapt_area_flush_ver(param);
    }

    while(param->area_flush.y < param->disp->area_act.h) {
        _clear_buffer(param->disp, prev, scr, param->area_flush.y, true);

        /** prev screen */
        param->valid.is_prev = true;
        param->disp->area_disp.x = param->disp->area_act.x + prev->area.x;
        param->disp->area_disp.y = param->disp->area_act.y + prev->area.y + param->area_flush.y;
        param->disp->area_disp.h = param->disp->area_act.h + param->line;

#if GT_REFRESH_FLUSH_LINE_PRE_TIME != GT_SCREEN_HEIGHT
        if (GT_SCR_ANIM_TYPE_MOVE_UP == param->disp->scr_anim_type || GT_SCR_ANIM_TYPE_COVER_UP == param->disp->scr_anim_type) {
            if (param->area_flush.y + param->line > prev->area.h) {
                param->disp->area_disp.h = prev->area.h % param->line;
            } else {
                param->disp->area_disp.h = param->line;
            }
        }
        else if (GT_SCR_ANIM_TYPE_COVER_DOWN == param->disp->scr_anim_type) {
            param->valid.offset_prev.y = 0;
            param->view_scr_prev_abs.y = scr->area.h;
            param->disp->area_disp.h = param->disp->area_act.h;
            if (param->area_flush.y + param->line > scr->area.h) {
                /*
                 *  FIX: The GT_SCR_ANIM_TYPE_COVER_DOWN mode filled last area will display error when open it,
                 *       close is ok but lose a little performance, such as code:
                 *  // param->valid.offset_prev.y = scr->area.h % param->line;
                 */
                param->view_scr_prev_abs.y = param->area_flush.y;
            }
        }
#endif  /** GT_REFRESH_FLUSH_LINE_PRE_TIME != GT_SCREEN_HEIGHT */

        gt_area_cover_screen(&param->disp->area_disp, &param->view_scr_prev_abs, &param->valid.area_prev);
        _gt_disp_check_and_copy_foreach(prev, param);

        /** new screen */
        _clear_buffer(param->disp, prev, scr, param->area_flush.y, false);
        param->valid.is_prev = false;
        param->disp->area_disp.x = scr->area.x;
        param->disp->area_disp.y = scr->area.y + param->area_flush.y;

#if GT_REFRESH_FLUSH_LINE_PRE_TIME != GT_SCREEN_HEIGHT
        if (GT_SCR_ANIM_TYPE_MOVE_UP == param->disp->scr_anim_type || GT_SCR_ANIM_TYPE_COVER_UP == param->disp->scr_anim_type) {
            param->disp->area_disp.h = param->area_flush.y + param->line;   // param->disp->area_act.h;
            param->valid.offset_scr.y = 0;

            if (param->area_flush.y + param->line < prev->area.h) {
                /** Previous area, do not display current screen area */
                param->disp->area_disp.h = 0;
            } else {
                /** The area between the previous screen area and the new screen area, or the only current screen area */
                param->view_scr_abs.h = param->disp->area_act.h;
                if (param->disp->area_disp.y + param->line < param->line) {
                    param->valid.offset_scr.y = param->disp->area_disp.y;
                }
            }
        }
        else if (GT_SCR_ANIM_TYPE_COVER_DOWN == param->disp->scr_anim_type) {
            param->disp->area_disp.h = param->area_flush.y + param->line;
        }
#endif  /** GT_REFRESH_FLUSH_LINE_PRE_TIME != GT_SCREEN_HEIGHT */

        gt_area_cover_screen(&param->disp->area_disp, &param->view_scr_abs, &param->valid.area_scr);
        _gt_disp_check_and_copy_foreach(scr, param);

        /** flush display by buffer area */
        param->disp->drv->flush_cb(param->disp->drv, &param->area_flush, param->disp->vbd_color);
        param->area_flush.y += param->line;
    }
}

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

    scr->area.x = param->disp->area_act.x;
    scr->area.y = param->disp->area_act.y;

    if(gt_disp_get_res_hor(NULL) == param->area_flush.w && gt_disp_get_res_ver(NULL) == param->area_flush.h){
        if(param->area_flush.x < 0 || (param->area_flush.x + param->area_flush.w > gt_disp_get_res_hor(NULL))){
            param->area_flush.x = 0;
        }
        if(param->area_flush.y < 0 || (param->area_flush.y + param->area_flush.h > gt_disp_get_res_ver(NULL))){
            param->area_flush.y = 0;
        }
    }
    else{
        // Calculate the area_flush x coordinates and width
        if(param->disp->area_act.x == 0){
            if(param->area_flush.x < 0){
                param->area_flush.w = param->area_flush.w + param->area_flush.x;
                param->area_flush.x = 0;
            }

            if((param->area_flush.x % gt_disp_get_res_hor(NULL)) + param->area_flush.w > gt_disp_get_res_hor(NULL)){
                param->area_flush.w = gt_disp_get_res_hor(NULL) - (param->area_flush.x % gt_disp_get_res_hor(NULL));
            }
        }
        else if(param->disp->area_act.x > 0){
            if(param->area_flush.x < 0){
                param->area_flush.w = param->area_flush.w + param->area_flush.x;
                param->area_flush.x = 0;
            }

            if(param->area_flush.x > param->disp->area_act.x){
                param->area_flush.x = param->area_flush.x - param->disp->area_act.x;
            }
            else{
                param->area_flush.w = (param->area_flush.w + param->area_flush.x) - param->disp->area_act.x;
                param->area_flush.x = 0;
            }

            if((param->area_flush.x % gt_disp_get_res_hor(NULL)) + param->area_flush.w > gt_disp_get_res_hor(NULL)){
                param->area_flush.w = gt_disp_get_res_hor(NULL) - (param->area_flush.x % gt_disp_get_res_hor(NULL));
            }

        }
        else if(param->disp->area_act.x < 0)
        {
            if(param->area_flush.x < 0){
                param->area_flush.w = param->area_flush.w + param->area_flush.x - param->disp->area_act.x;
                param->area_flush.x = 0;
            }

            if(param->area_flush.x > 0){
                param->area_flush.x = param->area_flush.x - param->disp->area_act.x;
            }

            if((param->area_flush.x % gt_disp_get_res_hor(NULL)) + param->area_flush.w > gt_disp_get_res_hor(NULL)){
                param->area_flush.w = gt_disp_get_res_hor(NULL) - (param->area_flush.x % gt_disp_get_res_hor(NULL));
            }

        }

        // Calculate the area_flush y coordinates and height
        if(param->disp->area_act.y == 0){
            if(param->area_flush.y < 0){
                param->area_flush.h = param->area_flush.h + param->area_flush.y;
                param->area_flush.y = 0;
            }

            if((param->area_flush.y % gt_disp_get_res_ver(NULL)) + param->area_flush.h > gt_disp_get_res_ver(NULL)){
                param->area_flush.h = gt_disp_get_res_ver(NULL) - (param->area_flush.y % gt_disp_get_res_ver(NULL));
            }

        }
        else if(param->disp->area_act.y > 0){
            if(param->area_flush.y < 0){
                param->area_flush.h = param->area_flush.h + param->area_flush.y;
                param->area_flush.y = 0;
            }

            if(param->area_flush.y > param->disp->area_act.y){
                param->area_flush.y = param->area_flush.y - param->disp->area_act.y;
            }
            else{
                param->area_flush.h = (param->area_flush.h + param->area_flush.y) - param->disp->area_act.y;
                param->area_flush.y = 0;
            }

            if((param->area_flush.y % gt_disp_get_res_ver(NULL)) + param->area_flush.h > gt_disp_get_res_ver(NULL)){
                param->area_flush.h = gt_disp_get_res_ver(NULL) - (param->area_flush.y % gt_disp_get_res_ver(NULL));
            }

        }
        else if(param->disp->area_act.y < 0)
        {
            if(param->area_flush.y < 0){
                param->area_flush.h = param->area_flush.h + param->area_flush.y - param->disp->area_act.y;
                param->area_flush.y = 0;
            }

            if(param->area_flush.y > 0){
                param->area_flush.y = param->area_flush.y - param->disp->area_act.y;
            }

            if((param->area_flush.y % gt_disp_get_res_ver(NULL)) + param->area_flush.h > gt_disp_get_res_ver(NULL)){
                param->area_flush.h = gt_disp_get_res_ver(NULL) - (param->area_flush.y % gt_disp_get_res_ver(NULL));
            }
        }
    }
    //
    uint16_t end_y = param->area_flush.y + param->area_flush.h;
    param->disp->area_disp.x = param->area_flush.x + param->disp->area_act.x;
    param->disp->area_disp.y = param->area_flush.y;
    param->disp->area_disp.w = param->area_flush.w;
    if(param->area_flush.h > param->line){
        param->area_flush.h = param->line;
    }
    param->disp->area_disp.h = param->area_flush.h;

    uint32_t len = param->disp->area_disp.w * param->disp->area_disp.h;

    while (param->area_flush.y < end_y)
    {
        gt_color_fill(param->disp->vbd_color, len, color_fill);

        param->disp->area_disp.y = param->area_flush.y + param->disp->area_act.y;

        _gt_disp_check_and_copy_foreach(scr, param);

        // flush top layer
        _gt_disp_check_and_copy_foreach(param->disp->layer_top, param);

        /** flush display by buffer area */
        param->disp->drv->flush_cb(param->disp->drv, &param->area_flush, param->disp->vbd_color);
        param->area_flush.y += param->line;

        if( end_y - param->area_flush.y < param->line){
            param->area_flush.h = end_y - param->area_flush.y;
            param->disp->area_disp.h = param->area_flush.h;
        }

    }
}

static gt_scr_anim_type_et _get_anti_anim_type(gt_scr_anim_type_et type) {
    if (GT_SCR_ANIM_TYPE_MOVE_LEFT == type) { type = GT_SCR_ANIM_TYPE_MOVE_RIGHT; }
    else if (GT_SCR_ANIM_TYPE_MOVE_RIGHT == type) { type = GT_SCR_ANIM_TYPE_MOVE_LEFT; }
    else if (GT_SCR_ANIM_TYPE_MOVE_UP == type) { type = GT_SCR_ANIM_TYPE_MOVE_DOWN; }
    else if (GT_SCR_ANIM_TYPE_MOVE_DOWN == type) { type =  GT_SCR_ANIM_TYPE_MOVE_UP; }
    else if (GT_SCR_ANIM_TYPE_COVER_LEFT == type) { type = GT_SCR_ANIM_TYPE_COVER_RIGHT; }
    else if (GT_SCR_ANIM_TYPE_COVER_RIGHT == type) { type = GT_SCR_ANIM_TYPE_COVER_LEFT; }
    else if (GT_SCR_ANIM_TYPE_COVER_UP == type) { type = GT_SCR_ANIM_TYPE_COVER_DOWN; }
    else if (GT_SCR_ANIM_TYPE_COVER_DOWN == type) { type = GT_SCR_ANIM_TYPE_COVER_UP; }
    return type;
}

static gt_obj_st * _create_scr_by_id(gt_scr_id_t scr_id) {
    gt_scr_init_func_cb_t scr_init_cb = gt_scr_stack_get_init_func(scr_id);
    if (NULL == scr_init_cb) {
        GT_LOGE(GT_LOG_TAG_GUI, "Screen init callback is NULL");
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

    if (0 == step) {
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
    }

    if (NULL == new_item.current_scr) {
        new_item.current_scr = _create_scr_by_id(new_item.current_scr_id);
        if (NULL == new_item.current_scr) {
            return -1;
        }
    }

    if (gt_scr_stack_get_home_scr_id() == new_item.current_scr_id ||
        gt_scr_stack_is_empty()) {
        if (false == gt_scr_stack_push(&new_item)) {
            return -1;
        }
    }

    if (scr_old != new_item.current_scr) {
        gt_disp_load_scr_anim(new_item.current_scr, new_item.anim_type, new_item.time, new_item.delay, true);
    }

    return new_item.current_scr_id;
}

void gt_disp_stack_load_scr(gt_scr_id_t scr_id)
{
    gt_disp_stack_load_scr_anim(scr_id, GT_SCR_ANIM_TYPE_NONE, 300, 0, true);
}

void gt_disp_stack_load_scr_anim(gt_scr_id_t scr_id, gt_scr_anim_type_et type, uint32_t time, uint32_t delay, bool del_prev_scr)
{
    gt_obj_st * scr_old = gt_disp_get_scr();

    gt_scr_stack_item_st new_item = {
        .current_scr_id = scr_id,
        .prev_scr_id = -1,
        .prev_scr_alive = NULL,
        .current_scr = NULL,
        .time = time,
        .delay = delay,
        .anim_type = type,
    };
    gt_scr_id_t home_id = gt_scr_stack_get_home_scr_id();

    /** Has the same page before and pop unused stack item */
    gt_stack_item_st result = gt_scr_stack_has_before(&new_item);
    if (result.index_from_top) {
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
        return ;
    }
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

    _gt_disp_refr_reset_areas();
    if (NULL == scr_old) {
        if (GT_SCR_ANIM_TYPE_NONE != type) {
            type = GT_SCR_ANIM_TYPE_NONE;
        }
    } else if (scr_old != scr) {
        gt_event_clr_all_event(scr_old);
        // gt_event_clr_all_event(scr);     // TODO: why clear event?
    }

    scr->using = true;
    disp->scr_anim_type = type;

    if (GT_SCR_ANIM_TYPE_NONE == type) {
        if( scr_old != scr ){
            disp->area_act.x = scr->area.x;
            disp->area_act.y = scr->area.y;

            _reset_layer_top_area(disp);
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
    } else {
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
            gt_anim_set_value(&anim_old, 0, disp->drv->res_hor);

            gt_anim_set_exec_cb(&anim_new, _scr_anim_exec_x_cb);
            gt_anim_set_value(&anim_new, -disp->drv->res_hor, 0);
            break;
        }
        case GT_SCR_ANIM_TYPE_MOVE_RIGHT: {
            gt_anim_set_exec_cb(&anim_old, _scr_anim_exec_x_cb);
            gt_anim_set_value(&anim_old, 0, -disp->drv->res_hor);

            gt_anim_set_exec_cb(&anim_new, _scr_anim_exec_x_cb);
            gt_anim_set_value(&anim_new, disp->drv->res_hor, 0);
            break;
        }
        case GT_SCR_ANIM_TYPE_MOVE_UP: {
            gt_anim_set_exec_cb(&anim_old, _scr_anim_exec_y_cb);
            gt_anim_set_value(&anim_old, 0, disp->drv->res_ver);

            gt_anim_set_exec_cb(&anim_new, _scr_anim_exec_y_cb);
            gt_anim_set_value(&anim_new, -disp->drv->res_ver, 0);
            break;
        }
        case GT_SCR_ANIM_TYPE_MOVE_DOWN: {
            gt_anim_set_exec_cb(&anim_old, _scr_anim_exec_y_cb);
            gt_anim_set_value(&anim_old, 0, -disp->drv->res_ver);

            gt_anim_set_exec_cb(&anim_new, _scr_anim_exec_y_cb);
            gt_anim_set_value(&anim_new, disp->drv->res_ver, 0);
            break;
        }

        case GT_SCR_ANIM_TYPE_COVER_LEFT: {
            gt_anim_set_exec_cb(&anim_new, _scr_anim_exec_x_cb);
            gt_anim_set_value(&anim_new, -disp->drv->res_hor, 0);
            break;
        }
        case GT_SCR_ANIM_TYPE_COVER_RIGHT: {
            gt_anim_set_exec_cb(&anim_new, _scr_anim_exec_x_cb);
            gt_anim_set_value(&anim_new, disp->drv->res_hor, 0);
            break;
        }
        case GT_SCR_ANIM_TYPE_COVER_UP: {
            gt_anim_set_exec_cb(&anim_new, _scr_anim_exec_y_cb);
            gt_anim_set_value(&anim_new, -disp->drv->res_ver, 0);
            break;
        }
        case GT_SCR_ANIM_TYPE_COVER_DOWN: {
            gt_anim_set_exec_cb(&anim_new, _scr_anim_exec_y_cb);
            gt_anim_set_value(&anim_new, disp->drv->res_ver, 0);
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

gt_obj_st * gt_disp_get_layer_top(void)
{
    gt_disp_st * disp = gt_disp_get_default();
    if( NULL == disp ){
        return NULL;
    }
    return disp->layer_top;
}

void gt_disp_ref_area(const gt_area_st * coords)
{
    uint16_t scr_width = gt_disp_get_res_hor(NULL);
    _flush_scr_param_st param = {
        .disp = gt_disp_get_default(),
        .area_dirty = *coords,
        .area_flush = {0, 0, scr_width, GT_REFRESH_FLUSH_LINE_PRE_TIME},
        /** 旧界面在可视窗口中的区域 */
        .view_scr_abs = {0, 0, 0, 0},
        .view_scr_prev_abs = {0, 0, 0, 0},
        /** 活动窗口的偏移量 */
        .valid = {
            .area_prev   = {0, 0, 0, 0},
            .offset_prev = {0, 0},
            .area_scr    = {0, 0, 0, 0},
            .offset_scr  = {0, 0},
            .is_hor      = false,
            .is_prev     = false,
        },
        .line = GT_REFRESH_FLUSH_LINE_PRE_TIME,
    };
    param.view_scr_abs = param.disp->scr_act->area;
    param.view_scr_prev_abs = param.disp->area_act;

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

    if (param.disp->scr_prev) {
        _flush_scr_by_anim(&param);
    } else {
        param.area_flush = param.area_dirty;
        _flush_scr_by_direct(&param);
    }

    _gt_disp_set_state(param.disp, GT_NOT_BUSY);
}

void gt_disp_set_area_act(gt_area_st * area)
{
    gt_disp_st * disp_dev = gt_disp_get_default();
    gt_area_copy(&disp_dev->area_act, area);
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
    gt_disp_st * disp = gt_disp_get_default();

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

    if (disp->layer_top) {
        if (disp->layer_top->cnt_child) {
            /** Keep float layer top widgets stay within display screen area */
            _gt_obj_move_child_by(disp->layer_top,
                area->x - gt_obj_get_x(disp->layer_top),
                area->y - gt_obj_get_y(disp->layer_top));
        }
        disp->layer_top->area.x = area->x;
        disp->layer_top->area.y = area->y;
    }

    _gt_disp_refr_reset_areas();
    _gt_disp_refr_append_area(area);
}

void gt_disp_invalid_area(gt_obj_st * obj)
{
    if (NULL == obj) {
        /** full screen refresh */
        obj = gt_disp_get_scr();
        obj->area.w = gt_disp_get_res_hor(NULL);
        obj->area.h = gt_disp_get_res_ver(NULL);
    }
    if( !gt_obj_check_scr(obj)){
        return;
    }
    _gt_disp_refr_append_area(&obj->area);
}

void gt_disp_set_backoff_scr(gt_obj_st * need_backoff_scr, gt_scr_init_func_cb_t init_cb, uint32_t time, uint32_t delay)
{
    gt_disp_st * disp = gt_disp_get_default();
    if (NULL == disp) {
        return;
    }
    if (NULL == disp->stack) {
        disp->stack = gt_mem_malloc(sizeof(_gt_disp_stack_st));
        if (NULL == disp->stack) {
            return;
        }
    }
    disp->stack->need_backoff_scr = need_backoff_scr;
    disp->stack->init_func_cb = init_cb;
    disp->stack->time = time;
    disp->stack->delay = delay;
}

void gt_disp_go_backoff_scr(void)
{
    gt_disp_st * disp = gt_disp_get_default();
    if (NULL == disp) {
        return;
    }
    _gt_disp_stack_st * prev_stack = disp->stack;
    if (NULL == prev_stack) {
        return;
    }
    gt_obj_st * step_back_scr = prev_stack->init_func_cb();
    if (NULL == step_back_scr) {
        return;
    }
    gt_scr_anim_type_et type = _get_anti_anim_type(disp->scr_anim_type);

    gt_disp_load_scr_anim(step_back_scr, type, prev_stack->time, prev_stack->delay, true);
}

/* end ------------------------------------------------------------------*/
