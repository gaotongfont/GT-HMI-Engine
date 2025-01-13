/**
 * @file gt_cross_layout.c
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2024-12-23 16:02:11
 * @copyright Copyright (c) 2014-2024, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "./gt_cross_layout.h"
#if GT_CFG_ENABLE_CROSS_LAYOUT
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"
#include "../others/gt_anim.h"
#include "../utils/gt_vector.h"
#include "gt_rect.h"
/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_CROSS_LAYOUT
#define MY_CLASS    &gt_cross_layout_class

/* private typedef ------------------------------------------------------*/
typedef struct _gt_cross_layout_s {
    gt_obj_st obj;

    gt_obj_st* hor_obj;
    gt_obj_st* ver_obj;
    _gt_vector_st * hor_itmes;
    _gt_vector_st * ver_itmes;
    gt_size_t hor_move_max;
    gt_size_t ver_move_max;
    gt_size_t hor_offset;
    gt_size_t ver_offset;
    gt_cross_roll_mode_et roll_mode;

    uint8_t hor_gap;
    uint8_t ver_gap;

}_gt_cross_layout_st;


/* static prototypes ----------------------------------------------------*/



/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

static GT_ATTRIBUTE_RAM_DATA const gt_obj_class_st gt_cross_layout_class = {
    ._init_cb      = NULL,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_cross_layout_st)
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static bool _hor_itmes_free_cb(void * item)
{
    _gt_vector_free((_gt_vector_st*)item);
}
static GT_ATTRIBUTE_RAM_TEXT void _cross_hor_set_hidden(gt_obj_st* obj, uint16_t ver_index)
{
    _gt_cross_layout_st * style = (_gt_cross_layout_st *)obj;

    if(!style->hor_itmes){
        return ;
    }

    _gt_vector_st * hor_itme = _gt_vector_get_item(style->hor_itmes, ver_index);
    if(!hor_itme){
        return ;
    }

    gt_size_t count = _gt_vector_get_count(hor_itme);
    for(gt_size_t i = 0; i < count; ++i)
    {
        gt_obj_st* itme = (gt_obj_st*)_gt_vector_get_item(hor_itme, i);
        if(itme){
            gt_obj_set_visible(itme, GT_INVISIBLE);
        }
    }
}

static GT_ATTRIBUTE_RAM_TEXT void _cross_ver_handler(gt_obj_st* obj)
{
    _gt_cross_layout_st * style = (_gt_cross_layout_st *)obj;
    if(!style->ver_obj){
        return ;
    }
    gt_size_t x = gt_obj_get_x(style->ver_obj);
    gt_size_t y = gt_obj_get_y(style->ver_obj);
    gt_size_t count = _gt_vector_get_count(style->ver_itmes);
    style->ver_move_max = 0;
    if(GT_CROSS_ROLL_LOOP == style->roll_mode)
    {
        if(count <= 0){
            return ;
        }
        gt_size_t behind = gt_obj_get_y(style->ver_obj) + gt_obj_get_h(style->ver_obj);
        gt_size_t this_index = _gt_vector_get_index(style->ver_itmes);
        if(this_index < 0){
            return ;
        }
        gt_size_t i = this_index, i_count = 0;
        y = gt_obj_get_y(style->ver_obj) + style->ver_offset;
        while(i_count < count){
            if(y > behind){
                break;
            }
            gt_obj_st* itme = (gt_obj_st*)_gt_vector_get_item(style->ver_itmes, i);
            if(!itme){
                ++i_count;
                continue;
            }
            itme->visible = GT_VISIBLE;
            x = gt_obj_get_x(style->ver_obj) + ((gt_obj_get_w(style->ver_obj) - itme->area.w) >> 1);
            _gt_obj_move_child_by(itme, x - itme->area.x, y - itme->area.y);
            itme->area.x = x;
            itme->area.y = y;
            style->ver_move_max += gt_obj_get_h(itme) + style->ver_gap;
            y += gt_obj_get_h(itme) + style->ver_gap;
            ++i_count;
            i = (i + 1) % count;
        }
        i = (this_index + count - 1) % count;
        y = gt_obj_get_y(style->ver_obj) + style->ver_offset;
        while(i_count < count){
            gt_obj_st* itme = (gt_obj_st*)_gt_vector_get_item(style->ver_itmes, i);
            if(!itme){
                ++i_count;
                continue;
            }
            itme->visible = GT_VISIBLE;
            style->ver_move_max += gt_obj_get_h(itme) + style->ver_gap;
            y -= (gt_obj_get_h(itme) + style->ver_gap);
            x = gt_obj_get_x(style->ver_obj) + ((gt_obj_get_w(style->ver_obj) - itme->area.w) >> 1);
            _gt_obj_move_child_by(itme, x - itme->area.x, y - itme->area.y);
            itme->area.x = x;
            itme->area.y = y;
            ++i_count;
            i = (i + count - 1) % count;
        }
    }
    else{
        for(gt_size_t i = 0; i < count; ++i)
        {
            gt_obj_st* itme = (gt_obj_st*)_gt_vector_get_item(style->ver_itmes, i);
            if(itme){
                itme->visible = GT_VISIBLE;
                x = gt_obj_get_x(style->ver_obj) + ((gt_obj_get_w(style->ver_obj) - itme->area.w) >> 1);
                _gt_obj_move_child_by(itme, x - itme->area.x, y - itme->area.y);
                itme->area.x = x;
                itme->area.y = y;
                style->ver_move_max += gt_obj_get_h(itme) + style->ver_gap;
                y += gt_obj_get_h(itme) + style->ver_gap;
            }
        }

        style->ver_move_max -=style->ver_gap;
        if(style->ver_move_max < gt_obj_get_h(style->ver_obj)){
            style->ver_move_max = 0;
        }
        else{
            style->ver_move_max -= gt_obj_get_h(style->ver_obj);
        }
    }
_ret_handler:
    gt_disp_invalid_area(style->ver_obj);
}

static GT_ATTRIBUTE_RAM_TEXT void _cross_hor_handler(gt_obj_st* obj)
{
    _gt_cross_layout_st * style = (_gt_cross_layout_st *)obj;
    if(!style->hor_obj){
        return ;
    }

    gt_size_t x = gt_obj_get_x(style->hor_obj);
    gt_size_t y = gt_obj_get_y(style->hor_obj);

    if(0 == _gt_vector_get_count(style->ver_itmes)){
        return ;
    }

    gt_size_t index = _gt_vector_get_index(style->ver_itmes);
    if(-1 == index){
        return ;
    }

    _gt_vector_st * hor_itme = _gt_vector_get_item(style->hor_itmes, index);
    if(!hor_itme){
        return ;
    }
    gt_size_t count = _gt_vector_get_count(hor_itme);
    style->hor_move_max = 0;
    if(GT_CROSS_ROLL_LOOP == style->roll_mode){
        if(count <= 0){
            return ;
        }
        gt_size_t behind = gt_obj_get_x(style->hor_obj) + gt_obj_get_w(style->hor_obj);
        gt_size_t this_index = _gt_vector_get_index(hor_itme);
        if(this_index < 0){
            return ;
        }
        gt_size_t i = this_index, i_count = 0;

        x = gt_obj_get_x(style->hor_obj) + style->hor_offset;
        while(i_count < count){
            if(x > behind){
                break;
            }
            gt_obj_st* itme = (gt_obj_st*)_gt_vector_get_item(hor_itme, i);
            if(!itme){
                ++i_count;
                continue;
            }
            itme->visible = GT_VISIBLE;
            y = gt_obj_get_y(style->hor_obj) + ((gt_obj_get_h(style->hor_obj) - itme->area.h) >> 1);
            _gt_obj_move_child_by(itme, x - itme->area.x, y - itme->area.y);
            itme->area.x = x;
            itme->area.y = y;
            x += gt_obj_get_w(itme) + style->hor_gap;
            style->hor_move_max += gt_obj_get_w(itme) + style->hor_gap;
            ++i_count;
            i = (i + 1) % count;
        }

        i = (this_index + count - 1) % count;
        x = gt_obj_get_x(style->hor_obj) + style->hor_offset;
        while(i_count < count){
            gt_obj_st* itme = (gt_obj_st*)_gt_vector_get_item(hor_itme, i);
            if(!itme){
                ++i_count;
                continue;
            }
            itme->visible = GT_VISIBLE;
            x -= (gt_obj_get_w(itme) + style->hor_gap);
            style->hor_move_max += gt_obj_get_w(itme) + style->hor_gap;
            y = gt_obj_get_y(style->hor_obj) + ((gt_obj_get_h(style->hor_obj) - itme->area.h) >> 1);
            _gt_obj_move_child_by(itme, x - itme->area.x, y - itme->area.y);
            itme->area.x = x;
            itme->area.y = y;
            ++i_count;
            i = (i + count - 1) % count;
        }
    }
    else{
        for(gt_size_t i = 0; i < count; ++i)
        {
            gt_obj_st* itme = (gt_obj_st*)_gt_vector_get_item(hor_itme, i);
            if(itme){
                itme->visible = GT_VISIBLE;
                y = gt_obj_get_y(style->hor_obj) + ((gt_obj_get_h(style->hor_obj) - itme->area.h) >> 1);
                _gt_obj_move_child_by(itme, x - itme->area.x, y - itme->area.y);
                itme->area.x = x;
                itme->area.y = y;
                x += gt_obj_get_w(itme) + style->hor_gap;
                style->hor_move_max += gt_obj_get_w(itme) + style->hor_gap;
            }
        }
        style->hor_move_max -= style->hor_gap;
        if(style->hor_move_max < gt_obj_get_w(style->hor_obj)){
            style->hor_move_max = 0;
        }
        else{
            style->hor_move_max -= gt_obj_get_w(style->hor_obj);
        }
    }
_ret_handler:
    gt_disp_invalid_area(style->hor_obj);
}

static GT_ATTRIBUTE_RAM_TEXT void _cross_layout_handler(gt_obj_st* obj)
{
    _cross_ver_handler(obj);
    _cross_hor_handler(obj);
}

static GT_ATTRIBUTE_RAM_TEXT void _cross_hor_scroll_to(gt_obj_st* obj, gt_point_st dist)
{
    _gt_cross_layout_st * style = (_gt_cross_layout_st *)obj;

    if(!style->hor_obj){
        return ;
    }

    gt_size_t index = _gt_vector_get_index(style->ver_itmes);
    if(-1 == index){
        return ;
    }
    _gt_vector_st * hor_itme = _gt_vector_get_item(style->hor_itmes, index);
    if(!hor_itme){
        return ;
    }
    gt_size_t count = _gt_vector_get_count(hor_itme);
    if(count <= 0){
        return ;
    }

    gt_size_t tmp_x = 0;
    gt_size_t min = 0, min_idx = -1;
    uint16_t i = 0, i_count = 0;
    gt_size_t behind_s = gt_obj_get_x(style->hor_obj);
    gt_size_t behind_e = gt_obj_get_x(style->hor_obj) + gt_obj_get_w(style->hor_obj);
    if(GT_CROSS_ROLL_LOOP == style->roll_mode){
        // * find
        for(i = 0; i < count; ++i){
            gt_obj_st* itme = (gt_obj_st*)_gt_vector_get_item(hor_itme, i);
            if(!itme){
                continue;
            }
            tmp_x = itme->area.x + dist.x;
            if(tmp_x >= behind_s && tmp_x <= behind_e){
                min_idx = i;
                min = tmp_x;
                break;
            }
        }
        if(min_idx < 0){
            return ;
        }
        // * move
        tmp_x = min;
        i = min_idx % count;
        i_count = 0;
        while(i_count < count){
            if(tmp_x > behind_e){
                break;
            }
            gt_obj_st* itme = (gt_obj_st*)_gt_vector_get_item(hor_itme, i);
            if(!itme){
                ++i_count;
                continue;
            }
            _gt_obj_move_child_by(itme, tmp_x - itme->area.x, 0);
            itme->area.x = tmp_x;
            tmp_x += itme->area.w + style->hor_gap;
            i = (i + 1) % count;
            ++i_count;

        }

        tmp_x = min;
        i = (min_idx - 1 + count) % count;
        while(i_count < count){
            gt_obj_st* itme = (gt_obj_st*)_gt_vector_get_item(hor_itme, i);
            if(!itme){
                ++i_count;
                continue;
            }
            tmp_x -= (itme->area.w + style->hor_gap);
            _gt_obj_move_child_by(itme, tmp_x - itme->area.x, 0);
            itme->area.x = tmp_x;
            i = (i - 1 + count) % count;
            ++i_count;
        }
    }
    else{
        for(gt_size_t i = 0; i < count; ++i){
            gt_obj_st* itme = (gt_obj_st*)_gt_vector_get_item(hor_itme, i);
            if(!itme){
                continue;
            }
            itme->area.x += dist.x;
            _gt_obj_move_child_by(itme, dist.x, 0);
        }
    }
_ret_handler:
    gt_disp_invalid_area(style->hor_obj);
    return ;
}

static GT_ATTRIBUTE_RAM_TEXT void _cross_hor_set_index_scroll(gt_obj_st* obj, gt_size_t index)
{
    _gt_cross_layout_st * style = (_gt_cross_layout_st *)obj;

    if(!style->hor_obj){
        return ;
    }

    if(!style->hor_itmes && !style->ver_itmes){
        return ;
    }
    gt_size_t ver_index = _gt_vector_get_index(style->ver_itmes);
    if(-1 == ver_index){
        return ;
    }

    _gt_vector_st * hor_itme = (_gt_vector_st *)_gt_vector_get_item(style->hor_itmes, ver_index);
    if(!hor_itme){
        return ;
    }
    if(index > _gt_vector_get_count(hor_itme)){
        return ;
    }

    gt_size_t this_index = _gt_vector_get_index(hor_itme);
    if(this_index == index){
        return ;
    }

    gt_obj_st* itme = (gt_obj_st*)_gt_vector_get_item(hor_itme, index);
    if(!itme){
        return ;
    }
    gt_point_st dist = {0};
    dist.x = gt_obj_get_x(style->hor_obj) - gt_obj_get_x(itme);
    if(GT_CROSS_ROLL_LOOP == style->roll_mode){
        dist.x += style->hor_offset;
        _cross_hor_scroll_to(obj, dist);
    }
    else{
        style->hor_obj->process_attr.scroll = dist;
        gt_event_send(style->hor_obj, GT_EVENT_TYPE_INPUT_SCROLL, NULL);
    }
    _gt_vector_set_index(hor_itme, index);
}

static GT_ATTRIBUTE_RAM_TEXT void _cross_hor_scroll_handler(gt_event_st* e)
{
    gt_obj_st* obj = (gt_obj_st* )e->user_data;
    _gt_cross_layout_st * style = (_gt_cross_layout_st *)obj;

    if(style->hor_obj != e->target){
        return ;
    }

    gt_point_st *scroll = &e->target->process_attr.scroll;

    if(GT_CROSS_ROLL_LOOP == style->roll_mode){
        if(style->hor_move_max < gt_obj_get_w(style->hor_obj)){
            return ;
        }
    }
    else{
        if(scroll->x > 0){
            scroll->x = 0;
        }else if(scroll->x < 0 && scroll->x < -style->hor_move_max){
            scroll->x = -style->hor_move_max;
        }
    }

    gt_point_st offset;
    offset.x = scroll->x - e->target->process_attr.scroll_prev.x;
    _cross_hor_scroll_to(obj, offset);

    if(GT_CROSS_ROLL_LOOP == style->roll_mode){
        scroll->x = 0;
    }

    if(scroll->x != e->target->process_attr.scroll_prev.x){
        e->target->process_attr.scroll_prev.x = scroll->x;
    }
}

static GT_ATTRIBUTE_RAM_TEXT void _cross_ver_scroll_to(gt_obj_st* obj, gt_point_st dist)
{
    _gt_cross_layout_st * style = (_gt_cross_layout_st *)obj;
    gt_size_t count = _gt_vector_get_count(style->ver_itmes);
    if(count <= 0){
        return ;
    }
    gt_size_t tmp_y = 0;
    gt_size_t min = 0, min_idx = -1;
    uint16_t i = 0, i_count = 0;
    gt_size_t behind_s = gt_obj_get_y(style->ver_obj);
    gt_size_t behind_e = gt_obj_get_y(style->ver_obj) + gt_obj_get_h(style->ver_obj);

    if(GT_CROSS_ROLL_LOOP == style->roll_mode){
        // * find
        for(i = 0; i < count; ++i){
            gt_obj_st* itme = (gt_obj_st*)_gt_vector_get_item(style->ver_itmes, i);
            if(!itme){
                continue;
            }
            tmp_y = itme->area.y + dist.y;
            if(tmp_y >= behind_s && tmp_y < behind_e){
                min = tmp_y;
                min_idx = i;
                break;
            }
        }
        if(min_idx < 0){
            return ;
        }
        // * move
        tmp_y = min;
        i = min_idx % count;
        i_count = 0;
        while(i_count < count){
            if(tmp_y > behind_e){
                break;
            }
            gt_obj_st* itme = (gt_obj_st*)_gt_vector_get_item(style->ver_itmes, i);
            if(!itme){
                ++i_count;
                continue;
            }
            _gt_obj_move_child_by(itme, 0, tmp_y - itme->area.y);
            itme->area.y = tmp_y;
            tmp_y += itme->area.h + style->ver_gap;
            i = (i + 1) % count;
            ++i_count;
        }
        tmp_y = min;
        i = (min_idx + count - 1) % count;
        while(i_count < count){
            gt_obj_st* itme = (gt_obj_st*)_gt_vector_get_item(style->ver_itmes, i);
            if(!itme){
                ++i_count;
                continue;
            }
            tmp_y -= itme->area.h + style->ver_gap;
            _gt_obj_move_child_by(itme, 0, tmp_y - itme->area.y);
            itme->area.y = tmp_y;
            i = (i + count - 1) % count;
            ++i_count;
        }
    }
    else{
        _gt_obj_move_child_by(style->ver_obj, 0, dist.y);
    }

    gt_disp_invalid_area(style->ver_obj);
    return ;
}

static GT_ATTRIBUTE_RAM_TEXT void _cross_ver_set_index_scroll(gt_obj_st* obj, gt_size_t index)
{
    _gt_cross_layout_st * style = (_gt_cross_layout_st *)obj;
    gt_size_t count = _gt_vector_get_count(style->ver_itmes);
    if(index >= count){
        return ;
    }

    gt_size_t this_index = _gt_vector_get_index(style->ver_itmes);
    if(this_index == index){
        return ;
    }

    gt_obj_st* itme = (gt_obj_st*)_gt_vector_get_item(style->ver_itmes, index);
    if(!itme){
        return ;
    }
    gt_point_st dist = {0};
    dist.y = gt_obj_get_y(style->ver_obj) - gt_obj_get_y(itme);
    if(GT_CROSS_ROLL_LOOP == style->roll_mode){
        dist.y += style->ver_offset;
        _cross_ver_scroll_to(obj, dist);
    }
    else{
        style->ver_obj->process_attr.scroll = dist;
        gt_event_send(style->ver_obj, GT_EVENT_TYPE_INPUT_SCROLL, NULL);
    }
    _gt_vector_set_index(style->ver_itmes, index);
}

static GT_ATTRIBUTE_RAM_TEXT void _cross_ver_scroll_handler(gt_event_st* e)
{
    gt_obj_st* obj = (gt_obj_st* )e->user_data;
    _gt_cross_layout_st * style = (_gt_cross_layout_st *)obj;

    if(style->ver_obj != e->target){
        return ;
    }

    gt_point_st *scroll = &e->target->process_attr.scroll;

    if(GT_CROSS_ROLL_LOOP == style->roll_mode){
        if(style->ver_move_max < gt_obj_get_h(style->ver_obj)){
            return ;
        }
    }
    else{
        if(scroll->y > 0){
            scroll->y = 0;
        }else if(scroll->y < 0 && scroll->y < -style->ver_move_max){
            scroll->y = -style->ver_move_max;
        }
    }

    gt_point_st offset;
    offset.y = scroll->y - e->target->process_attr.scroll_prev.y;

    _cross_ver_scroll_to(obj, offset);
    if(GT_CROSS_ROLL_LOOP == style->roll_mode){
        scroll->y = 0;
    }

    if(scroll->y != e->target->process_attr.scroll_prev.y){
        e->target->process_attr.scroll_prev.y = scroll->y;
    }
}



static void _deinit_cb(gt_obj_st * obj)
{
    _gt_cross_layout_st * style = (_gt_cross_layout_st *)obj;
    if(style->hor_itmes){
        _gt_vector_free(style->hor_itmes);
        style->hor_itmes = NULL;
    }
    if(style->ver_itmes){
        _gt_vector_free(style->ver_itmes);
        style->ver_itmes = NULL;
    }
}

static void _event_cb(struct gt_obj_s * obj, gt_event_st * e)
{
    gt_event_type_et code_val = gt_event_get_code(e);
    switch(code_val){
        case GT_EVENT_TYPE_DRAW_START:
            gt_disp_invalid_area(obj);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;
        case GT_EVENT_TYPE_UPDATE_VALUE:
            _cross_layout_handler(obj);
            break;
        case GT_EVENT_TYPE_INPUT_SCROLL:
            break;
        default:
            break;
    }
}


/* global functions / API interface -------------------------------------*/

gt_obj_st * gt_cross_layout_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    if (NULL == obj) {
        return obj;
    }
    obj->focus_dis = GT_DISABLED;

    _gt_cross_layout_st * style = (_gt_cross_layout_st *)obj;

    style->hor_itmes = _gt_vector_create(_hor_itmes_free_cb, NULL);
    style->ver_itmes = _gt_vector_create(NULL, NULL);

    style->hor_obj = gt_obj_create(obj);
    GT_CHECK_BACK_VAL(style->hor_obj, obj);

    style->ver_obj = gt_obj_create(obj);
    GT_CHECK_BACK_VAL(style->ver_obj, obj);

    style->hor_obj->focus_dis = GT_DISABLED;
    style->hor_obj->scroll_dir = GT_SCROLL_HORIZONTAL;
    gt_obj_add_replace_core_event_cb(style->hor_obj, _cross_hor_scroll_handler, GT_EVENT_TYPE_INPUT_SCROLL, (void*)obj);


    style->ver_obj->focus_dis = GT_DISABLED;
    style->ver_obj->scroll_dir = GT_SCROLL_VERTICAL;
    gt_obj_add_replace_core_event_cb(style->ver_obj, _cross_ver_scroll_handler, GT_EVENT_TYPE_INPUT_SCROLL, (void*)obj);

    return obj;
}


void gt_cross_layout_set_gap(gt_obj_st* layout, uint8_t hor_gap, uint8_t ver_gap)
{
    GT_CHECK_BACK(layout);

    if(false == gt_obj_is_type(layout, OBJ_TYPE)){
        return;
    }
    _gt_cross_layout_st * style = (_gt_cross_layout_st *)layout;
    style->hor_gap = hor_gap;
    style->ver_gap = ver_gap;
}


gt_obj_st* gt_cross_layout_get_hor_obj(gt_obj_st* layout)
{
    GT_CHECK_BACK_VAL(layout, NULL);
    _gt_cross_layout_st * style = (_gt_cross_layout_st *)layout;
    return style->hor_obj;
}

gt_obj_st* gt_cross_layout_get_ver_obj(gt_obj_st* layout)
{
    GT_CHECK_BACK_VAL(layout, NULL);
    _gt_cross_layout_st * style = (_gt_cross_layout_st *)layout;
    return style->ver_obj;
}

void gt_cross_layout_set_roll_mode(gt_obj_st* layout, gt_cross_roll_mode_et roll_mode)
{
    GT_CHECK_BACK(layout);

    if(false == gt_obj_is_type(layout, OBJ_TYPE)){
        return;
    }
    _gt_cross_layout_st * style = (_gt_cross_layout_st *)layout;
    style->roll_mode = roll_mode;
}

void gt_cross_layout_set_hor_obj_area(gt_obj_st* layout, gt_size_t x, gt_size_t y, uint16_t w, uint16_t h)
{
    GT_CHECK_BACK(layout);

    if(false == gt_obj_is_type(layout, OBJ_TYPE)){
        return;
    }
    _gt_cross_layout_st * style = (_gt_cross_layout_st *)layout;
    GT_CHECK_BACK(style->hor_obj);

    gt_area_st area = {x, y, w, h};
    gt_obj_set_area(style->hor_obj, area);
}

void gt_cross_layout_set_ver_obj_area(gt_obj_st* layout, gt_size_t x, gt_size_t y, uint16_t w, uint16_t h)
{
    GT_CHECK_BACK(layout);

    if(false == gt_obj_is_type(layout, OBJ_TYPE)){
        return;
    }
    _gt_cross_layout_st * style = (_gt_cross_layout_st *)layout;
    GT_CHECK_BACK(style->ver_obj);
    gt_area_st area = {x, y, w, h};
    gt_obj_set_area(style->ver_obj, area);
}

void gt_cross_layout_set_hor_offset(gt_obj_st* layout, gt_size_t offset)
{
    GT_CHECK_BACK(layout);

    if(false == gt_obj_is_type(layout, OBJ_TYPE)){
        return;
    }
    _gt_cross_layout_st * style = (_gt_cross_layout_st *)layout;
    style->hor_offset = offset;
}

void gt_cross_layout_set_ver_offset(gt_obj_st* layout, gt_size_t offset)
{
    GT_CHECK_BACK(layout);

    if(false == gt_obj_is_type(layout, OBJ_TYPE)){
        return;
    }
    _gt_cross_layout_st * style = (_gt_cross_layout_st *)layout;
    style->ver_offset = offset;
}

void gt_cross_layout_hor_add_itme(gt_obj_st* layout, gt_obj_st* obj, uint8_t ver_index)
{
    GT_CHECK_BACK(layout);
    GT_CHECK_BACK(obj);
    GT_CHECK_BACK(obj->parent);

    if(false == gt_obj_is_type(layout, OBJ_TYPE)){
        return;
    }

    _gt_cross_layout_st * style = (_gt_cross_layout_st *)layout;

    if(obj->parent != style->hor_obj){
        return;
    }

    GT_CHECK_BACK(style->ver_itmes);
    GT_CHECK_BACK(style->hor_itmes);

    gt_size_t count = _gt_vector_get_count(style->ver_itmes);
    if(ver_index >= count){
        return ;
    }

    _gt_vector_st* hor_itme = (_gt_vector_st *)_gt_vector_get_item(style->hor_itmes, ver_index);
    if(!hor_itme){
        hor_itme = _gt_vector_create(NULL, NULL);
        GT_CHECK_BACK(hor_itme);
        if (false == _gt_vector_add_item(style->hor_itmes, (void * )hor_itme)) {
            _gt_vector_free(hor_itme);
            return ;
        }
    }

    if (false == _gt_vector_add_item(hor_itme, (void * )obj)) {
        return ;
    }

    gt_obj_set_visible(obj, GT_INVISIBLE);
    gt_event_send(layout, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}


void gt_cross_layout_ver_add_item(gt_obj_st* layout, gt_obj_st* obj)
{
    GT_CHECK_BACK(layout);
    GT_CHECK_BACK(obj);
    GT_CHECK_BACK(obj->parent);

    if(false == gt_obj_is_type(layout, OBJ_TYPE)){
        return;
    }

    _gt_cross_layout_st * style = (_gt_cross_layout_st *)layout;

    if(obj->parent != style->ver_obj){
        return;
    }

    GT_CHECK_BACK(style->ver_itmes);

    if (false == _gt_vector_add_item(style->ver_itmes, (void * )obj)) {
        return ;
    }

    gt_obj_set_visible(obj, GT_INVISIBLE);
    gt_event_send(layout, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}


void gt_cross_layout_set_ver_index(gt_obj_st* layout, uint8_t ver_index)
{
    GT_CHECK_BACK(layout);

    if(false == gt_obj_is_type(layout, OBJ_TYPE)){
        return;
    }

    _gt_cross_layout_st * style = (_gt_cross_layout_st *)layout;

    GT_CHECK_BACK(style->ver_itmes);

    gt_size_t this_index = _gt_vector_get_index(style->ver_itmes);
    gt_size_t count = _gt_vector_get_count(style->ver_itmes);

    if(ver_index >= count || this_index == ver_index){
        return;
    }

    _cross_hor_set_hidden(layout, this_index);
    style->hor_obj->process_attr.scroll.x = 0;
    style->hor_obj->process_attr.scroll_prev.x = 0;
    // _gt_vector_set_index(style->ver_itmes, ver_index);
    _cross_ver_set_index_scroll(layout, ver_index);
    _cross_hor_handler(layout);
}

void gt_cross_layout_set_hor_index(gt_obj_st* layout, uint8_t hor_index)
{
    GT_CHECK_BACK(layout);

    if(false == gt_obj_is_type(layout, OBJ_TYPE)){
        return;
    }

    _gt_cross_layout_st * style = (_gt_cross_layout_st *)layout;

    _cross_hor_set_index_scroll(layout, hor_index);
}

gt_size_t gt_cross_layout_get_hor_count(gt_obj_st* layout)
{
    GT_CHECK_BACK_VAL(layout, -1);

    if(false == gt_obj_is_type(layout, OBJ_TYPE)){
        return -1;
    }

    _gt_cross_layout_st * style = (_gt_cross_layout_st *)layout;

    if(!style->hor_itmes && !style->ver_itmes){
        return -1;
    }

    gt_size_t ver_index = _gt_vector_get_index(style->ver_itmes);
    if(-1 == ver_index){
        return -1;
    }

    _gt_vector_st * hor_itme = (_gt_vector_st *)_gt_vector_get_item(style->hor_itmes, ver_index);
    if(!hor_itme){
        return -1;
    }

    return _gt_vector_get_count(hor_itme);
}

gt_size_t gt_cross_layout_get_ver_count(gt_obj_st* layout)
{
    GT_CHECK_BACK_VAL(layout, -1);

    if(false == gt_obj_is_type(layout, OBJ_TYPE)){
        return -1;
    }

    _gt_cross_layout_st * style = (_gt_cross_layout_st *)layout;

    if(!style->ver_itmes){
        return -1;
    }

    return _gt_vector_get_count(style->ver_itmes);
}

gt_size_t gt_cross_layout_get_hor_index(gt_obj_st* layout)
{
    GT_CHECK_BACK_VAL(layout, -1);

    if(false == gt_obj_is_type(layout, OBJ_TYPE)){
        return -1;
    }

    _gt_cross_layout_st * style = (_gt_cross_layout_st *)layout;

    if(!style->hor_itmes && !style->ver_itmes){
        return -1;
    }

    gt_size_t ver_index = _gt_vector_get_index(style->ver_itmes);
    if(-1 == ver_index){
        return -1;
    }

    _gt_vector_st * hor_itme = (_gt_vector_st *)_gt_vector_get_item(style->hor_itmes, ver_index);
    if(!hor_itme){
        return -1;
    }

    return _gt_vector_get_index(hor_itme);
}

gt_size_t gt_cross_layout_get_ver_index(gt_obj_st* layout)
{
    GT_CHECK_BACK_VAL(layout, -1);

    if(false == gt_obj_is_type(layout, OBJ_TYPE)){
        return -1;
    }

    _gt_cross_layout_st * style = (_gt_cross_layout_st *)layout;

    if(!style->ver_itmes){
        return -1;
    }
    return _gt_vector_get_index(style->ver_itmes);
}

gt_obj_st* gt_cross_layout_get_hor_select_obj(gt_obj_st* layout)
{
    GT_CHECK_BACK_VAL(layout, NULL);

    if(false == gt_obj_is_type(layout, OBJ_TYPE)){
        return NULL;
    }

    _gt_cross_layout_st * style = (_gt_cross_layout_st *)layout;

    if(!style->hor_itmes && !style->ver_itmes){
        return NULL;
    }

    gt_size_t ver_index = _gt_vector_get_index(style->ver_itmes);
    if(-1 == ver_index){
        return NULL;
    }

    _gt_vector_st * hor_itme = (_gt_vector_st *)_gt_vector_get_item(style->hor_itmes, ver_index);
    if(!hor_itme){
        return NULL;
    }

    return _gt_vector_get_item(hor_itme, _gt_vector_get_index(hor_itme));
}

gt_obj_st* gt_cross_layout_get_ver_select_obj(gt_obj_st* layout)
{
    GT_CHECK_BACK_VAL(layout, NULL);

    if(false == gt_obj_is_type(layout, OBJ_TYPE)){
        return NULL;
    }

    _gt_cross_layout_st * style = (_gt_cross_layout_st *)layout;

    if(!style->ver_itmes){
        return NULL;
    }
    return _gt_vector_get_item(style->ver_itmes, _gt_vector_get_index(style->ver_itmes));
}
/* end of file ----------------------------------------------------------*/
#endif //!GT_CFG_ENABLE_CROSS_LAYOUT

