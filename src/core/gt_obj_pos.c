/**
 * @file gt_obj_pos.c
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-06-15 14:34:36
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_obj_pos.h"
#include "../hal/gt_hal_disp.h"
#include "../widgets/gt_obj.h"
#include "../others/gt_log.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

void gt_area_copy(gt_area_st * dst, gt_area_st * src){
    dst->x = src->x;
    dst->y = src->y;
    dst->w = src->w;
    dst->h = src->h;
}

bool gt_obj_is_visible(gt_obj_st * obj){
    bool res = true;
    gt_disp_st * disp = gt_disp_get_default();
    if( !disp ){
        GT_LOGW( GT_LOG_TAG_GUI, "disp is null" );
    }

    gt_area_st area_obj, area_disp;
    
    gt_area_copy( &area_disp, &disp->area_act );
    gt_area_copy( &area_obj, &obj->area );

    if( (area_obj.x + area_obj.w) < area_disp.x ){
        return false;
    }

    if( (area_obj.y + area_obj.h) < area_disp.y ){
        return false;
    }

    if( area_obj.x > (area_disp.x + area_disp.w) ){
        return false;
    }
    if( area_obj.y > (area_disp.y + area_disp.h) ){
        return false;
    }
    return true;
}

void gt_obj_get_valid_area(gt_obj_st * obj, gt_area_st * area_act, gt_area_st *area_valid){
    gt_disp_st * disp = gt_disp_get_default();
    gt_area_st area_disp, area_obj;
    
    gt_area_copy(&area_disp, area_act);
    gt_area_copy(&area_obj, &obj->area);
    
    #define __X1(area)    (area.x)
    #define __X2(area)    (area.x+area.w)
    #define __Y1(area)    (area.y)
    #define __Y2(area)    (area.y+area.h)

    if( __X1(area_disp) >= __X1(area_obj) ){
        area_valid->x = __X1(area_disp) - __X1(area_obj);
    }else{
        area_valid->x = 0;
    }

    if( __Y1(area_disp) >= __Y1(area_obj) ){
        area_valid->y = __Y1(area_disp) - __Y1(area_obj);
    }else{
        area_valid->y = 0;
    }

    if( __X2(area_disp) > __X2(area_obj) ){
        area_valid->w = area_obj.w;
    }else{
        GT_LOGV(GT_LOG_TAG_GUI, "%d,%d,%d", __X2(area_disp), __X1(area_obj), area_valid->x);
        area_valid->w = __X2(area_disp) - __X1(area_obj) - area_valid->x;
    }
    
    if( __Y2(area_disp) > __Y2(area_obj) ){
        area_valid->h = area_obj.h;
    }else{
        area_valid->h = __Y2(area_disp) - __Y2(area_obj) - area_valid->y;
    }

}


/* end ------------------------------------------------------------------*/
