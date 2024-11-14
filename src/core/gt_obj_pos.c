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
#include "./gt_obj_scroll.h"
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
#if GT_USE_CUSTOM_TOUCH_EXPAND_SIZE
    if (point->x + obj->touch_expand_size.x < obj->area.x ||
        point->x > (obj->area.x + obj->area.w + obj->touch_expand_size.x)) {
        return _CHECK_CLICKED_STATE_FAIL;
    }
    if (point->y + obj->touch_expand_size.y < obj->area.y ||
        point->y > (obj->area.y + obj->area.h + obj->touch_expand_size.y)) {
        return _CHECK_CLICKED_STATE_FAIL;
    }
#else
    if (point->x < obj->area.x || point->x > (obj->area.x + obj->area.w)) {
        return _CHECK_CLICKED_STATE_FAIL;
    }
    if (point->y < obj->area.y || point->y > (obj->area.y + obj->area.h)) {
        return _CHECK_CLICKED_STATE_FAIL;
    }
#endif
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

static gt_obj_st* _gt_obj_next_focus_get(const gt_obj_st * cur_obj, bool flag)
{
    if(!cur_obj){
        return NULL;
    }

    if(cur_obj == gt_disp_get_scr() && cur_obj->cnt_child > 0){
        return cur_obj->child[0];
    }

    gt_obj_st* parent = cur_obj->parent;

    if(parent == NULL){
        return NULL;
    }

    int idx = 0, mim_idx = parent->cnt_child-1;
    bool is_find = false;
    gt_obj_st* next = NULL;

    for(idx = 0; idx < parent->cnt_child; ++idx){

        if(GT_ENABLED != parent->child[idx]->focus_dis){
            continue;
        }

        mim_idx = GT_MIN(mim_idx, idx);

        if(flag) {
            return parent->child[idx];
        }

        if(parent->child[idx] != cur_obj){
            continue;
        }

        // virtuality
        if(parent->child[idx]->virtuality){
            if(parent->child[idx]->cnt_child > 0){
                next = _gt_obj_next_focus_get(parent->child[idx]->child[0], true);
                if(next){
                    return next;
                }
            }
            continue;
        }

        return  ((idx + 1) < parent->cnt_child) ? parent->child[idx + 1] : parent->child[mim_idx];
    }

    return parent->child[mim_idx];
}

static gt_obj_st* _gt_obj_prev_focus_get(const gt_obj_st * cur_obj, bool flag)
{
    if(!cur_obj){
        return NULL;
    }

    if(cur_obj == gt_disp_get_scr() && cur_obj->cnt_child > 0){
        return cur_obj->child[cur_obj->cnt_child-1];
    }

    gt_obj_st* parent = cur_obj->parent;

    if(parent == NULL){
        return NULL;
    }

    int idx = 0, max_idx = 0;
    bool is_find = false;
    gt_obj_st* prev = NULL;

    for(idx = parent->cnt_child - 1; idx >= 0; --idx){

        if(GT_ENABLED != parent->child[idx]->focus_dis){
            continue;
        }

        max_idx = GT_MAX(max_idx, idx);

        if(flag) {
            return parent->child[idx];
        }

        if(parent->child[idx] != cur_obj){
            continue;
        }

        // virtuality
        if(parent->child[idx]->virtuality){
            if(parent->child[idx]->cnt_child > 0){
                prev = _gt_obj_next_focus_get(parent->child[idx]->child[parent->child[idx]->cnt_child - 1], true);
                if(prev){
                    return prev;
                }
            }
            continue;
        }

        return  ((idx - 1) >= 0) ? parent->child[idx - 1] : parent->child[max_idx];
    }

    return parent->child[max_idx];
}

static gt_obj_st* _gt_obj_into_focus_get(const gt_obj_st * cur_obj)
{
    if(!cur_obj){
        return NULL;
    }

    if(GT_ENABLED != cur_obj->focus_dis){
        return NULL;
    }

    int idx = 0;

    for(idx = 0; idx < cur_obj->cnt_child; ++idx)
    {
        if(GT_ENABLED != cur_obj->child[idx]->focus_dis){
            continue;
        }

        return cur_obj->child[idx];
    }

    return NULL;
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
            break;
        }
        idx++;
    }
    return obj_clicked;
}

gt_obj_st * gt_find_clicked_obj_by_focus(gt_obj_st * parent)
{
    gt_obj_st * obj_clicked = NULL;

    obj_clicked = _gt_obj_focus_clicked(parent);

    return obj_clicked ? obj_clicked : parent;
}

gt_obj_change_st gt_obj_into_focus_change(gt_obj_st * cur_obj)
{
    gt_obj_change_st ret = {
        .src = cur_obj,
        .dst = NULL,
    };

    if (NULL == cur_obj) {
        return ret;
    }

    if(0 == cur_obj->cnt_child){
        return ret;
    }

    ret.dst = _gt_obj_into_focus_get(cur_obj);
    if(!ret.dst){
        return ret;
    }

    gt_obj_set_focus(cur_obj, false);
    gt_obj_set_focus(ret.dst, true);
    return ret;
}

gt_obj_change_st gt_obj_out_focus_change(gt_obj_st * cur_obj)
{
    gt_obj_change_st ret = {
        .src = cur_obj,
        .dst = NULL,
    };

    if (NULL == cur_obj) {
        return ret;
    }

    if(cur_obj->parent == NULL){
        return ret;
    }

    ret.dst = _gt_obj_next_focus_get(cur_obj->parent, true);

    if(!ret.dst){
        return ret;
    }


    gt_obj_set_focus(cur_obj, false);
    gt_obj_set_focus(ret.dst, true);

    return ret;
}

gt_obj_change_st gt_obj_next_focus_change(gt_obj_st * cur_obj)
{
    gt_obj_change_st ret = {
        .src = cur_obj,
        .dst = NULL,
    };
    if (NULL == cur_obj) {
        return ret;
    }

    ret.dst = _gt_obj_next_focus_get(cur_obj, false);
    if (!ret.dst) {
        ret.dst = gt_disp_get_scr();
    }

    gt_obj_set_focus(cur_obj, false);
    gt_obj_set_focus(ret.dst, true);
    return ret;
}

gt_obj_change_st gt_obj_prev_focus_change(gt_obj_st * cur_obj)
{
    gt_obj_change_st ret = {
        .src = cur_obj,
        .dst = NULL,
    };
    if (NULL == cur_obj) {
        return ret;
    }

    ret.dst = _gt_obj_prev_focus_get(cur_obj, false);
    //
    if (!ret.dst) {
        ret.dst = gt_disp_get_scr();
    }

    gt_obj_set_focus(cur_obj, false);
    gt_obj_set_focus(ret.dst, true);
    return ret;
}

void gt_obj_focus_change_display(gt_obj_change_st * chg)
{
    if (NULL == chg->dst) {
        return;
    }
    gt_obj_st * parent = chg->dst->parent;
    switch (gt_obj_class_get_type(parent)) {
        case GT_TYPE_SCREEN: {
            GT_LOG_A("", "screen display");
            break;
        }
        case GT_TYPE_LISTVIEW: {
            gt_obj_scroll_to_y(parent, chg->src->area.y - chg->dst->area.y, GT_ANIM_OFF);
            break;
        }
        case GT_TYPE_VIEW_PAGER: {
            GT_LOG_A("", "vp display");
            break;
        }
        default:
            break;
    }
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
