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
    
    gt_area_copy( &area_disp, &disp->area_disp );
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
    int x1 = 0, y1 = 0, x2 = 0, y2 = 0;
		
	gt_area_st area_disp, area_obj;
    
    gt_area_copy(&area_disp, area_act);
    gt_area_copy(&area_obj, &obj->area);
    
    #define __X1(area)    (area.x)
    #define __X2(area)    (area.x+area.w)
    #define __Y1(area)    (area.y)
    #define __Y2(area)    (area.y+area.h)


    if( __X1(area_disp) <= __X1(area_obj) ){
        area_valid->x = 0;
    }else{
        area_valid->x = __X1(area_disp) - __X1(area_obj);
    }

    if( __Y1(area_disp) <= __Y1(area_obj) ){
        area_valid->y = 0;
    }else{
        area_valid->y = __Y1(area_disp) - __Y1(area_obj);
    }

    if( __X2(area_disp) >= __X2(area_obj) ){
        if( __X1(area_disp) <= __X1(area_obj) ){
            area_valid->w = area_obj.w;
        }else{
            area_valid->w = __X2(area_obj) - __X1(area_disp);
        }
    }else{
        if( __X1(area_disp) <= __X1(area_obj) ){
            area_valid->w = __X2(area_disp) - __X1(area_obj);
        }else{
            area_valid->w = area_disp.w;
        }
    }

    if( __Y2(area_disp) >= __Y2(area_obj) ){
        if( __Y1(area_disp) <= __Y1(area_obj) ){
            area_valid->h = area_obj.h;
        }else{
            area_valid->h = __Y2(area_obj) - __Y1(area_disp);
        }
    }else{
			if( __Y1(area_disp) <= __Y1(area_obj) ){
            area_valid->h = __Y2(area_disp) - __Y1(area_obj);
        }else{
            area_valid->h = area_disp.h;
        }
		}
		
		

    return;

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
        area_valid->w = __X2(area_disp) - __X1(area_obj) - area_valid->x;
    }
    
    if( __Y2(area_disp) > __Y2(area_obj) ){
        area_valid->h = area_obj.h;
    }else{
        area_valid->h = __Y2(area_obj) - __Y2(area_disp) - area_valid->y;
    }
}


/**
 * @brief check obj was clicked
 * 
 * @param obj check obj
 * @param point click point
 * @return true clicked
 * @return false not clicked
 */
bool gt_obj_check_is_clicked( gt_obj_st * obj, gt_point_st * point ){
    bool ret = false;
    if( (point->x >= obj->area.x) && (point->x <= (obj->area.x + obj->area.w)) ){
        if( (point->y >= obj->area.y) && (point->y <= (obj->area.y + obj->area.h)) ){
            ret = true;
        }
    }
    return ret;
}

/**
 * @brief find smallest clicked obj from parent
 * 
 * @param parent parent obj
 * @param point click point
 * @return gt_obj_st* smallest and clicked obj in parent
 */
gt_obj_st * gt_find_clicked_obj_by_point(gt_obj_st * parent, gt_point_st * point){
    uint8_t idx = 0;
    gt_obj_st * obj_clicked = parent;
    while( idx < parent->cnt_child ){
        if( gt_obj_check_is_clicked(parent->child[idx], point) ){
            if( parent->child[idx] ){
                obj_clicked = parent->child[idx];
            }
        }
        idx++;
    }
    return obj_clicked;
}

/* end ------------------------------------------------------------------*/
