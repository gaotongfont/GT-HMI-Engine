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
/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/

typedef enum{
    GT_NOT_BUSY,
    GT_BUSY,
}gt_busy_et;



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
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

static void gt_check_obj_visible_and_copy(gt_obj_st * obj)
{
    gt_disp_st * disp = gt_disp_get_default();
    if( !gt_obj_is_visible(obj) ){
        // GT_LOGD(GT_LOG_TAG_GUI, "obj not visible");
        return;
    }

    GT_LOGV(GT_LOG_TAG_GUI, "obj visible");

    // GT_LOGV(GT_LOG_TAG_GUI, "obj->x:%d,y:%d,w:%d,h:%d====disp->x:%d,y:%d,w:%d,h:%d",
    //     obj->area.x, obj->area.y, obj->area.w, obj->area.h,
    //     disp->area_disp.x, disp->area_disp.y, disp->area_disp.w, disp->area_disp.h);

    gt_draw_ctx_t draw_ctx = {
        .buf        = disp->vbd_color,
        .buf_area   = &disp->area_disp,
    };
    obj->draw_ctx = &draw_ctx;
    obj->class->_init_cb(obj, obj->class);
}

static void _gt_disp_check_and_copy_foreach(gt_obj_st * obj)
{
    int16_t idx = 0;
    while( idx < obj->cnt_child ) {
        if( obj->child[idx]->cnt_child != 0 ){
            _gt_disp_check_and_copy_foreach( obj->child[idx] );
        }

        gt_check_obj_visible_and_copy(obj->child[idx]);
        idx++;
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

gt_obj_st * gt_disp_get_scr() {
    gt_disp_st * disp = gt_disp_get_default();
    if( NULL == disp ){
        return NULL;
    }
    return disp->scr_act;
}


void gt_disp_load_scr(gt_obj_st * scr)
{
    gt_obj_st * scr_old = gt_disp_get_scr();
    gt_disp_st * disp = gt_disp_get_default();
    extern void gt_event_clr_mask(gt_obj_st * parent);
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
        disp->area_act.x = 0;
        disp->area_act.y = 0;
        gt_event_clr_mask(scr_old);
        gt_event_clr_mask(scr);
    }
    gt_disp_set_scr(scr);

    // _gt_disp_send_draw_event_foreach(scr);

    disp->area_act.h = disp->drv->res_ver;
    disp->area_act.w = disp->drv->res_hor;

    #if 0
    uint32_t len = disp->drv->res_hor * GT_REFRESH_FLUSH_LINE_PRE_TIME;
    disp->area_disp.y = disp->area_act.y;
    disp->area_disp.x = disp->area_act.x;
    disp->area_disp.w = disp->drv->res_hor;
    disp->area_disp.h = GT_REFRESH_FLUSH_LINE_PRE_TIME;

    uint16_t y = 0;
	gt_color_t fill_color;
	fill_color = gt_screen_get_bgcolor(scr);
    while( y < disp->drv->res_ver ){

        gt_color_fill(disp->vbd_color, len, fill_color);

        // use virt screen check obj visible area
        disp->area_disp.y = disp->area_act.y + y;
        disp->area_disp.x = disp->area_act.x;
        _gt_disp_check_and_copy_foreach(scr);

        // phy screen use area is from W x H
        disp->area_disp.y = y;
        disp->area_disp.x = 0;
        disp->drv->flush_cb(disp->drv, &disp->area_disp, disp->vbd_color);
        y += GT_REFRESH_FLUSH_LINE_PRE_TIME;
    }
    #else
    _gt_disp_refr_append_area(&disp->area_act);
    #endif
    _gt_disp_set_state(disp, GT_NOT_BUSY);
    // here must clr all obj pre event_mask---->if not clr event_mask,obj will be next time handler the event
    // if( scr_old != scr ){
    //     disp->area_act.x = 0;
    //     disp->area_act.y = 0;
    //     gt_event_clr_mask(scr);
    // }
}

void gt_disp_ref_area(const gt_area_st * coords)
{
    gt_disp_st * disp = gt_disp_get_default();
	gt_obj_st * scr = gt_disp_get_scr();
    gt_area_st area;
    if( scr == NULL ){
        return;
    }
    if( _gt_disp_get_state(disp) == GT_BUSY ){
        GT_LOGD(GT_LOG_TAG_GUI, "disp is busy");
        return;
    }
    _gt_disp_set_state(disp, GT_BUSY);

    gt_area_copy(&area, (gt_area_st *)coords);
    uint16_t dist = 0;
    if( disp->area_act.y > area.y ){
        dist = disp->area_act.y - area.y;
        area.y += dist;
        area.h -= dist;
    }
    if( disp->area_act.x > area.x ){
        dist = disp->area_act.x - area.x;
        area.x += dist;
        area.w -= dist;
    }

    uint16_t line = GT_REFRESH_FLUSH_LINE_PRE_TIME;
    if( (area.w * area.h) <= (line * GT_SCREEN_WIDTH) ){
        line = area.h;
    }else{
        line = (line * GT_SCREEN_WIDTH)/area.w;
    }

    uint16_t y = 0;
    uint32_t len = area.w * line;

    disp->area_disp.x = area.x;
    disp->area_disp.y = area.y;
    disp->area_disp.w = area.w;
    disp->area_disp.h = line;

    gt_color_t color_fill;
    color_fill = gt_screen_get_bgcolor(scr);

    while( y < area.h ){

        gt_color_fill(disp->vbd_color, len, color_fill);

        // use virt screen check obj visible area
        disp->area_disp.y = area.y + y;
        disp->area_disp.x = area.x;
        _gt_disp_check_and_copy_foreach(scr);

        // phy screen use area is from W x H
        disp->area_disp.y = disp->area_disp.y - disp->area_act.y;
        disp->area_disp.x = area.x - disp->area_act.x;
        disp->drv->flush_cb(disp->drv, &disp->area_disp, disp->vbd_color);
        y += line;
        if( (y) > (area.y + area.h) ){
            disp->area_disp.h = area.h - y;
        }
    }

    _gt_disp_set_state(disp, GT_NOT_BUSY);
}


void gt_disp_set_area_act(gt_area_st * area){
    gt_disp_st * disp_dev = gt_disp_get_default();
    gt_area_copy(&disp_dev->area_act, area);
}

gt_area_st * gt_disp_get_area_act(){
    gt_disp_st * disp_dev = gt_disp_get_default();
    return &disp_dev->area_act;
}

void gt_disp_scroll_area_act(uint16_t dist_x, int16_t dist_y)
{
    gt_area_st * area = gt_disp_get_area_act();

    area->x += dist_x;
    area->y += dist_y;

    if(area->x < 0){
        area->x = 0;
    }
    if(area->y < 0){
        area->y = 0;
    }

    if( area->x > (GT_SCREEN_WIDTH << 1) ){
        area->x = GT_SCREEN_WIDTH << 1;
    }
    if( area->y > GT_SCREEN_HEIGHT << 1 ){
        area->y = GT_SCREEN_HEIGHT << 1;
    }
    gt_disp_load_scr(gt_disp_get_scr());
}

void gt_disp_obj(gt_obj_st * obj){
    gt_area_st * area = gt_disp_get_area_act();
    gt_area_st area_valid;
    gt_obj_get_valid_area(obj, area, &area_valid);

    gt_disp_st * disp_dev = gt_disp_get_default();

    if( 1 ){
        gt_area_st area_src = {
            .x = area_valid.x,
            .y = area_valid.y,
            .w = obj->area.w,
            .h = obj->area.h
        }, area_dst = {
            .x = 0,
            .y = 0,//area_valid.y,
            .w = area_valid.w,//obj->area.w,
            .h = area_valid.h//obj->area.h
        }, area_show = {
            .w = area_valid.w,
            .h = area_valid.h
        };

        if( obj->area.y < disp_dev->area_act.y ){
            area_show.y = 0;
        }else{
            area_show.y = obj->area.y - disp_dev->area_act.y;
        }

        if( obj->area.x < disp_dev->area_act.x ){
            area_show.x = 0;
        }else{
            area_show.x = obj->area.x - disp_dev->area_act.x;
        }
//        gt_color_t * color_p = (gt_color_t *)obj->cache.cache_color;
//        gt_color_area_copy(disp_dev->vbd_color, area_dst,  color_p, area_src, area_valid.w, area_valid.h);
        disp_dev->drv->flush_cb(disp_dev->drv, &area_show, disp_dev->vbd_color);
    }
}


/* end ------------------------------------------------------------------*/
