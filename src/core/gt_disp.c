/**
 * @file gt_disp.c
 * @author yongg
 * @brief Display implementation for the screen.
 * @version 0.1
 * @date 2022-05-11 14:59:50
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
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

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/

typedef enum{
    GT_NOT_BUSY,
    GT_BUSY,
}gt_busy_et;



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
    int16_t idx = 0;
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
    while( idx < obj->cnt_child ) {
        _gt_disp_send_draw_event_foreach( obj->child[idx] );
        idx++;
    }
}

static inline void gt_check_obj_visible_and_copy(gt_obj_st * obj)
{
    if( !gt_obj_is_visible(obj) ){
        return;
    }
    gt_disp_st * disp = gt_disp_get_default();

    // GT_LOGV(GT_LOG_TAG_GUI, "obj->x:%d, y:%d, w:%d, h:%d ==> disp->x:%d, y:%d, w:%d, h:%d",
    //     obj->area.x, obj->area.y, obj->area.w, obj->area.h,
    //     disp->area_disp.x, disp->area_disp.y, disp->area_disp.w, disp->area_disp.h);

    gt_draw_ctx_t draw_ctx = {
        .buf        = disp->vbd_color,
        .buf_area   = disp->area_disp,
    };
    obj->draw_ctx = (struct _gt_draw_ctx_t * )&draw_ctx;
    obj->class->_init_cb(obj);
}

static void _gt_disp_check_and_copy_foreach(gt_obj_st * obj)
{
    int16_t idx = 0;
    gt_obj_st * child_p = NULL;

    while( idx < obj->cnt_child ) {
        child_p = obj->child[idx];
        gt_check_obj_visible_and_copy(child_p);

        if( child_p->cnt_child != 0 ){
            _gt_disp_check_and_copy_foreach(child_p);
        }

        ++idx;
    }
}

/* global functions / API interface -------------------------------------*/

void gt_disp_set_scr(gt_obj_st * scr) {
    gt_disp_st * disp = gt_disp_get_default();
    if( NULL == disp ){
        return;
    }
    disp->scr_act = scr;
}

gt_obj_st * gt_disp_get_scr(void) {
    gt_disp_st * disp = gt_disp_get_default();
    if( NULL == disp ){
        return NULL;
    }
    return disp->scr_act;
}

uint16_t gt_disp_get_ver_pixel(void)
{
    gt_disp_st * disp = gt_disp_get_default();
    if( NULL == disp ){
        return 0;
    }
    return disp->drv->res_ver;
}

uint16_t gt_disp_get_hor_pixel(void)
{
    gt_disp_st * disp = gt_disp_get_default();
    if( NULL == disp ){
        return 0;
    }
    return disp->drv->res_hor;
}


void gt_disp_load_scr(gt_obj_st * scr)
{
    gt_obj_st * scr_old = gt_disp_get_scr();
    gt_disp_st * disp = gt_disp_get_default();

    if( !disp ){
        return;
    }

    if( _gt_disp_get_state(disp) == GT_BUSY ){
        GT_LOGD(GT_LOG_TAG_GUI, "disp is busy");
        return;
    }

    _gt_disp_set_state(disp, GT_BUSY);
    _gt_disp_refr_reset_areas();

    if( scr_old != scr ){
        disp->area_act.x = scr->area.x;
        disp->area_act.y = scr->area.y;
        gt_event_clr_all_event(scr_old);
        gt_event_clr_all_event(scr);
    }
    gt_disp_set_scr(scr);

    // _gt_disp_send_draw_event_foreach(scr);

    disp->area_act.h = disp->drv->res_ver;
    disp->area_act.w = disp->drv->res_hor;

    _gt_disp_refr_append_area(&disp->area_act);
    _gt_disp_set_state(disp, GT_NOT_BUSY);
}

void gt_disp_load_scr_anim(gt_obj_st * src, gt_scr_anim_type_et type, uint32_t time, uint32_t delay, bool auto_del)
{

}

void gt_disp_ref_area(const gt_area_st * coords)
{
    gt_disp_st * disp = gt_disp_get_default();
	gt_obj_st * scr = gt_disp_get_scr();
    gt_area_st area;
    uint16_t dist = 0;
    if ( scr == NULL ) {
        return;
    }
    if ( _gt_disp_get_state(disp) == GT_BUSY ) {
        GT_LOGD(GT_LOG_TAG_GUI, "disp is busy");
        return;
    }
    _gt_disp_set_state(disp, GT_BUSY);

    gt_area_copy(&area, (gt_area_st *)coords);
    if ( disp->area_act.y > area.y ) {
        dist = disp->area_act.y - area.y;
        area.y += dist;
        area.h -= dist;
    }
    if ( disp->area_act.x > area.x ) {
        dist = disp->area_act.x - area.x;
        area.x += dist;
        area.w -= dist;
    } else if (disp->area_act.x + GT_SCREEN_WIDTH < area.x + area.w) {
        if (disp->area_act.x > 0) {
            area.x -= disp->area_act.x;
        } else {
            /** screen x < 0 */
            area.x += disp->area_act.x;
        }
        area.w -= area.x + area.w - (disp->area_act.x + GT_SCREEN_WIDTH);
    }

    uint16_t line = GT_REFRESH_FLUSH_LINE_PRE_TIME;
    if ( (area.w * area.h) > (line * GT_SCREEN_WIDTH) ) {
        line = (line * GT_SCREEN_WIDTH) / area.w;
    } else {
        line = area.h;
    }

    gt_size_t y = 0;
    uint32_t len = area.w * line;

    disp->area_disp.x = area.x;
    disp->area_disp.y = area.y;
    disp->area_disp.w = area.w;
    disp->area_disp.h = line;

    gt_color_t color_fill = gt_screen_get_bgcolor(scr);

    while( y < area.h ) {

        gt_color_fill(disp->vbd_color, len, color_fill);

        disp->area_disp.y = area.y + y;
        disp->area_disp.x = area.x;
        // use virt screen check obj visible area
        _gt_disp_check_and_copy_foreach(scr);

        // phy screen use area is from W x H
        disp->area_disp.y -= disp->area_act.y;
        disp->area_disp.x = area.x - disp->area_act.x;
        disp->drv->flush_cb(disp->drv, &disp->area_disp, disp->vbd_color);
        y += line;
        if( y > (area.y + area.h) ){
            disp->area_disp.h = area.h - y;
        }
    }

    _gt_disp_set_state(disp, GT_NOT_BUSY);
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

    area->x += dist_x;
    area->y += dist_y;

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

    gt_disp_load_scr(gt_disp_get_scr());
}

void gt_disp_invalid_area(gt_obj_st * obj)
{
    if( !gt_obj_check_scr(obj) ){
        return;
    }
    _gt_disp_refr_append_area(&obj->area);
}

/* end ------------------------------------------------------------------*/
