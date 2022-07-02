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
void _gt_disp_load_obj_foreach(gt_obj_st * obj)
{
    int idx = 0;
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
    while( idx < obj->cnt_child ) {
        _gt_disp_load_obj_foreach( obj->child[idx] );
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
    _gt_disp_load_obj_foreach(scr);
}

void gt_disp_scr_by_10_lines(){
    static uint16_t y_offset = 0;

    while( y_offset < GT_SCREEN_HEIGHT ){
        /* 1.check obj is visible */

        /* 2.copy obj color to disp area color */

        /* 3.disp 10 lines to screen */

    }
}



/* end ------------------------------------------------------------------*/
