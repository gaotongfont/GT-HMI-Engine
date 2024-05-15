/**
 * @file gt_obj_pos.c
 * @author yongg
 * @brief Set object position function
 * @version 0.1
 * @date 2022-06-15 14:34:36
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_obj_pos.h"
#include "../hal/gt_hal_disp.h"
#include "../widgets/gt_obj.h"
#include "../others/gt_log.h"
#include "gt_disp.h"
#include "gt_indev.h"
#include "gt_style.h"
/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/
/**
 * @brief The object click area check state
 */
typedef enum _check_clicked_state_e {
    _CHECK_CLICKED_STATE_FAIL = 0,
    _CHECK_CLICKED_STATE_CONTINUE,
    _CHECK_CLICKED_STATE_OK,
}_check_clicked_state_em;

/**
 * @brief The click object return struct
 */
typedef struct {
    gt_obj_st * obj;
    _check_clicked_state_em state;
}_click_obj_ret_st;


/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

/**
 * @brief check obj was clicked by point
 *
 * @param obj check obj
 * @param point click point
 * @return true clicked
 * @return false not clicked
 */
static _check_clicked_state_em gt_obj_check_is_clicked( gt_obj_st * obj, gt_point_st * point ) {
    if (point->x < obj->area.x || point->x > (obj->area.x + obj->area.w)) {
        return _CHECK_CLICKED_STATE_FAIL;
    }
    if (point->y < obj->area.y || point->y > (obj->area.y + obj->area.h)) {
        return _CHECK_CLICKED_STATE_FAIL;
    }
    if (GT_INVISIBLE == gt_obj_get_visible(obj)) {
        return _CHECK_CLICKED_STATE_FAIL;
    }
    if (gt_obj_get_virtual(obj)) {
        return _CHECK_CLICKED_STATE_CONTINUE;
    }
    return _CHECK_CLICKED_STATE_OK;
}

/**
 * @brief According to the point, find the clicked object from the
 *      newer layer to the first layer.
 *
 * @param parent
 * @param point
 * @return gt_obj_st* The clicked object
 */
static _click_obj_ret_st _gt_obj_foreach_clicked(gt_obj_st * parent, gt_point_st * point) {
    gt_size_t idx = parent->cnt_child - 1;
    _click_obj_ret_st obj_clicked = {
        .obj = NULL,
        .state = _CHECK_CLICKED_STATE_CONTINUE,
    };
    _click_obj_ret_st ret;

    obj_clicked.state = gt_obj_check_is_clicked(parent, point);
    if (_CHECK_CLICKED_STATE_FAIL == obj_clicked.state) {
        return obj_clicked;
    }
    else if (_CHECK_CLICKED_STATE_OK == obj_clicked.state) {
        if (gt_obj_get_touch_parent(parent)) {
            /** Selected parent object */
            return obj_clicked;
        }
        if (0 == parent->cnt_child) {
            /** The deepest one */
            obj_clicked.obj = parent;
            return obj_clicked;
        }
    }

    /** Continue to find out children which is clicked */
    while (idx > -1) {
        ret = _gt_obj_foreach_clicked(parent->child[idx], point);
        if (_CHECK_CLICKED_STATE_OK == ret.state) {
            if (ret.obj && GT_VISIBLE == gt_obj_get_visible(ret.obj)) {
                return ret;
            }
        }
        --idx;
    }

    if (_CHECK_CLICKED_STATE_OK == obj_clicked.state) {
        obj_clicked.obj = parent;
    }

    return obj_clicked;
}

static gt_obj_st* _gt_obj_next_focus_get( gt_obj_st * parent, const gt_obj_st * cur_obj, bool flag)
{
    gt_obj_st * next = NULL;
    int idx = 0;
    bool tmp = flag;

    tmp = parent == cur_obj ? true : false;

    while (idx < parent->cnt_child)
    {
        if(tmp){
            if(GT_ENABLED == parent->child[idx]->focus_dis){
                return parent->child[idx];
            }
        }

        next = _gt_obj_next_focus_get(parent->child[idx], cur_obj, tmp);

        if(cur_obj == parent->child[idx]){
            tmp = true;
        }

        idx++;
    }

    return next;
}

static gt_obj_st* _gt_obj_prev_focus_get( const gt_obj_st * cur_obj)
{
    gt_obj_st * prev = NULL;
    gt_obj_st *  parent = NULL;
    bool flag = false;

    parent = cur_obj->parent;
    if(NULL == parent){
        return NULL;
    }

    int idx = parent->cnt_child;
    prev = cur_obj->parent;

    while (idx >= 0)
    {
        if(flag){
            if(GT_ENABLED == parent->child[idx]->focus_dis){
                prev = parent->child[idx];
                break;
            }
        }

        if(cur_obj == parent->child[idx])
        {
            flag = true;
        }
        --idx;
    }

    if(idx < 0){
        prev =  _gt_obj_prev_focus_get(parent);
    }
    return prev;
}



/* global functions / API interface -------------------------------------*/

void gt_area_copy(gt_area_st * dst, gt_area_st const * const src)
{
    dst->x = src->x;
    dst->y = src->y;
    dst->w = src->w;
    dst->h = src->h;
}

gt_obj_st * gt_find_clicked_obj_by_point(gt_obj_st * scr_or_top, gt_point_st * point) {
    gt_obj_st * obj_clicked = NULL;
    _click_obj_ret_st obj_temp = {
        .obj = NULL,
        .state = _CHECK_CLICKED_STATE_FAIL
    };

    gt_point_st _point = {
        .x = point->x + scr_or_top->area.x,
        .y = point->y + scr_or_top->area.y
    };
    obj_temp = _gt_obj_foreach_clicked(scr_or_top, &_point);
    if ( obj_temp.obj ) {
        obj_clicked = obj_temp.obj;
    }
    return obj_clicked;
}

gt_obj_st* _gt_obj_focus_clicked(gt_obj_st * parent)
{
    int idx = 0;
    gt_obj_st * obj_clicked = NULL, * obj_temp = NULL;
    while( idx < parent->cnt_child ){
        if( 0 != parent->child[idx]->cnt_child ){
            obj_temp = _gt_obj_focus_clicked(parent->child[idx]);
            if(obj_temp){
                obj_clicked = obj_temp;
                return obj_clicked;
            }
        }

        if(GT_ENABLED == parent->child[idx]->focus_dis && parent->child[idx]->focus){
            obj_clicked = parent->child[idx];
        }
        idx++;
    }
    return obj_clicked;
}

gt_obj_st* gt_find_clicked_obj_by_focus(gt_obj_st * parent)
{
    gt_obj_st * obj_clicked = NULL;

    obj_clicked = _gt_obj_focus_clicked(parent);

    return obj_clicked ? obj_clicked : parent;
}

void gt_obj_next_focus_change(gt_obj_st * cur_obj)
{
    if(NULL == cur_obj){
        return;
    }

    gt_obj_st* parent = gt_disp_get_scr();

    gt_obj_st* next = _gt_obj_next_focus_get(parent, cur_obj, false);

    if(!next){
        next = parent;
    }

    gt_obj_set_focus(cur_obj, false);
    gt_obj_set_focus(next, true);
}

void gt_obj_prev_focus_change(gt_obj_st * cur_obj)
{
    if(NULL == cur_obj){
        return;
    }

    gt_obj_st* parent = gt_disp_get_scr();
    gt_obj_st* prev = NULL;
    if(cur_obj == parent && parent->cnt_child > 0)
    {
        if(parent->cnt_child == 0 && GT_ENABLED == parent->child[0]->focus_dis)
        {
            prev = parent->child[0];
        }
        else if(parent->cnt_child > 1)
        {
            if(GT_ENABLED == parent->child[parent->cnt_child-1]->focus_dis)
            {
                prev = parent->child[parent->cnt_child-1];
            }
            else if(parent->cnt_child > 2){
                prev =  _gt_obj_prev_focus_get(parent->child[parent->cnt_child-2]);
            }
        }
    }
    else{
        prev = _gt_obj_prev_focus_get(cur_obj);
    }

    if(!prev){
        prev = parent;
    }

    gt_obj_set_focus(cur_obj, false);
    gt_obj_set_focus(prev, true);
}



bool gt_obj_check_scr(gt_obj_st * obj)
{
    gt_obj_st * scr_now = gt_disp_get_scr();
    if (NULL == scr_now) {
        /** first time load screen */
        if (GT_TYPE_SCREEN == gt_obj_class_get_type(obj)) {
            return true;
        }
    }
    if (scr_now == obj) {
        return true;
    }

    gt_obj_st * scr_obj = obj->parent;
    while( scr_obj->parent ){
        scr_obj = scr_obj->parent;
    }
    if( scr_now == scr_obj ){
        return true;
    }
    return false;
}

/**
 * @brief Get the point of the clicked obj
 *
 * @param obj click obj
 * @param point_phy point data
 * @param point_ret point data
 */
void gt_obj_get_click_point_by_phy_point(gt_obj_st * obj, gt_point_st * point_phy, gt_point_st * point_ret)
{
    gt_point_st point;
    point.x = point_phy->x;
    point.y = point_phy->y;

    // cal last point in virt screen pos
    gt_area_st * area_act_p = gt_disp_get_area_act();
    gt_area_st area_act;
    gt_area_copy(&area_act, area_act_p);

    point.x += area_act.x;
    point.y += area_act.y;

    point_ret->x = point.x - obj->area.x;
    point_ret->y = point.y - obj->area.y;
}

/* end ------------------------------------------------------------------*/
