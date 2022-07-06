/**
 * @file gt_disp.c
 * @author yongg
 * @brief
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
/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/





/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



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

/**
 * @brief foreach obj send draw start event
 *
 * @param obj parent
 */
void _gt_disp_send_draw_event_foreach(gt_obj_st * obj)
{
    int idx = 0;
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
    while( idx < obj->cnt_child ) {
        _gt_disp_send_draw_event_foreach( obj->child[idx] );
        idx++;
    }
}


void gt_check_obj_visible_and_copy(gt_obj_st * obj)
{
    gt_area_st area_valib;
    gt_disp_st * disp = gt_disp_get_default();
    if( !gt_obj_is_visible(obj) ){
        // GT_LOGD(GT_LOG_TAG_GUI, "obj not visible");
        return;
    }
    GT_LOGD(GT_LOG_TAG_GUI, "obj visible");
    obj->class->_init_cb(obj, obj->class);

    GT_LOGD(GT_LOG_TAG_GUI, "obj->x:%d,y:%d,w:%d,h:%d====disp->x:%d,y:%d,w:%d,h:%d",
        obj->area.x, obj->area.y, obj->area.w, obj->area.h,
        disp->area_disp.x, disp->area_disp.y, disp->area_disp.w, disp->area_disp.h);

    gt_obj_get_valid_area(obj, &disp->area_disp, &area_valib);

    GT_LOGD(GT_LOG_TAG_GUI, "area_valib:x:%d,y:%d,w:%d,h:%d", area_valib.x, area_valib.y, area_valib.w, area_valib.h);

    gt_area_st area_cpy_dst = {
        .x = obj->area.x,
        .y = 0,
        .w = disp->area_disp.w,
        .h = disp->area_disp.h
    }, area_cpy_src = {
        .x = area_valib.x,
        .y = area_valib.y,
        .w = obj->area.w,
        .h = obj->area.h
    };

    if( obj->area.y > disp->area_disp.y ){
        area_cpy_dst.y = obj->area.y - disp->area_disp.y;
    }

    gt_color_area_copy(disp->vbd_color, area_cpy_dst, obj->cache.cache_color, area_cpy_src, area_valib.w, area_valib.h);
    obj->class->_deinit_cb(obj, obj->class);
}

void _gt_disp_check_and_copy_foreach(gt_obj_st * obj)
{
    int idx = 0;
    while( idx < obj->cnt_child ) {
        if( obj->child[idx]->cnt_child != 0 ){
            _gt_disp_check_and_copy_foreach( obj->child[idx] );
        }
        
        gt_check_obj_visible_and_copy(obj->child[idx]);
        idx++;
    }
}

/**
 * @brief display screen
 *
 * @param scr screen obj
 */
void gt_disp_load_scr(gt_obj_st * scr)
{
    gt_disp_set_scr(scr);
    gt_disp_st * disp = gt_disp_get_default();
    if( !disp ){
        return;
    }
    // _gt_disp_send_draw_event_foreach(scr);

    uint16_t y = 0;
    uint32_t idx = 0;
    uint32_t len = GT_SCREEN_WIDTH * GT_REFRESH_FLUSH_LINE_PRE_TIME;

    disp->area_act.x = 0;
    disp->area_act.y = 0;
    disp->area_act.h = GT_SCREEN_HEIGHT;
    disp->area_act.w = GT_SCREEN_WIDTH;

    disp->area_disp.x = 0;
    disp->area_disp.w = GT_SCREEN_WIDTH;
    disp->area_disp.h = GT_REFRESH_FLUSH_LINE_PRE_TIME;

    while( y < GT_SCREEN_HEIGHT ){
        y+=GT_REFRESH_FLUSH_LINE_PRE_TIME;
        idx = 0;
        while( idx < len ){
            GT_COLOR_SET(disp->vbd_color[idx], 0xFFFF);
            idx++;
        }
        _gt_disp_check_and_copy_foreach(scr);
        disp->drv->flush_cb(disp->drv, &disp->area_disp, disp->vbd_color);
        disp->area_disp.y += GT_REFRESH_FLUSH_LINE_PRE_TIME;
    }
}



void gt_disp_obj_by_10_lines( gt_obj_st * obj ){
    
    gt_disp_st * disp = gt_disp_get_default();

    if( obj->class->type == 0 ){
        /* if obj type is screen */
        // disp->area_act
    }

    disp->area_disp.x = disp->area_act.x;
    disp->area_disp.y = disp->area_act.y;
    disp->area_disp.w = disp->area_act.w;
    disp->area_disp.h = GT_REFRESH_FLUSH_LINE_PRE_TIME;

    int idx = 0;
    gt_obj_st * scr = obj;
    while( disp->area_disp.y < (disp->area_act.y + disp->area_act.h) ){
        if( (disp->area_disp.y + disp->area_disp.h) > (disp->area_act.y + disp->area_act.h) ){
            disp->area_disp.h = (disp->area_act.y + disp->area_act.h) - disp->area_disp.y;
        }

        /* foreach all obj */
        while( idx < scr->cnt_child ){
            /* 1.check obj is visible */

            /* 2.copy obj color to disp area color */

            /* 3.disp 10 lines to screen */
        }
    }
}


/* end ------------------------------------------------------------------*/
