/**
 * @file gt_scroll_tip.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2025-12-18
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "./gt_scroll_tip.h"
#include "../core/gt_graph_base.h"
#include "./gt_draw.h"



/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/
#define _IS_SCROLL_TIP_MODE(val, em)        (em & val)

#define _CALC_TIP_NEARBY_POS(p_v, p_size, size)         (p_v + (size >> 1))
#define _CALC_TIP_FAR_POS(p_v, p_size, size)            (p_v + p_size - ((size >> 1) + size))

/**
 * @brief calc tip bar display length
 *      (tip_size / window_size) = (window_size / scroll_size), calc "tip_size".
 */
#define _CALC_TIP_BAR_LENGTH(win_size, scroll_size)     (win_size * win_size / scroll_size)

/**
 * @brief calc tip bar move position
 *      (cur_pos / window_size) = (scroll_pos / scroll_range), calc "cur_pos".
 */
#define _CALC_TIP_BAR_MOVE_POS(pos, range, win_val, parent_p)   ((gt_abs(pos) * win_val / gt_abs(range)) + parent_p)

/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/


static gt_area_st GT_ATTRIBUTE_RAM_TEXT
_calc_scroll_tip_bar_area(gt_obj_st * obj, gt_scroll_tip_et type) {
    gt_area_st ret = obj->area;
    gt_point_st size = gt_obj_get_childs_max_size(obj);

    if (GT_SCROLL_TIP_RIGHT == type || GT_SCROLL_TIP_LEFT == type) {
        ret.w = 4;
        ret.x = GT_SCROLL_TIP_LEFT == type ?
            _CALC_TIP_NEARBY_POS(obj->area.x, obj->area.w, ret.w) :
            _CALC_TIP_FAR_POS(obj->area.x, obj->area.w, ret.w);

        ret.h = size.y > ret.h ? _CALC_TIP_BAR_LENGTH(ret.h, size.y) : 0;
        if (0 == ret.h) { return ret; }

        ret.y = _CALC_TIP_BAR_MOVE_POS(obj->process_attr.scroll.y, size.y, obj->area.h, obj->area.y);
    }

    if (GT_SCROLL_TIP_TOP == type || GT_SCROLL_TIP_BOTTOM == type) {
        ret.h = 4;
        ret.y = GT_SCROLL_TIP_TOP == type ?
            _CALC_TIP_NEARBY_POS(obj->area.y, obj->area.h, ret.h) :
            _CALC_TIP_FAR_POS(obj->area.y, obj->area.h, ret.h);

        ret.w = size.x > ret.w ? _CALC_TIP_BAR_LENGTH(ret.w, size.x) : 0;
        if (0 == ret.w) { return ret; }

        ret.x = _CALC_TIP_BAR_MOVE_POS(obj->process_attr.scroll.x, size.x, obj->area.w, obj->area.x);
    }
    return ret;
}

static void GT_ATTRIBUTE_RAM_TEXT
_tip_bar_init(gt_obj_st * obj) {
    gt_area_st area_base = obj->area;
    gt_attr_rect_st rect_attr = {0};
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.reg.is_fill    = true;
    rect_attr.border_width   = 0;
    rect_attr.radius         = 2;
    rect_attr.border_color   = gt_color_hex(GT_COLOR_DARK_GRAY);
    rect_attr.bg_color       = gt_color_hex(GT_COLOR_DARK_GRAY);
    rect_attr.bg_opa         = GT_OPA_60;

    if (_IS_SCROLL_TIP_MODE(obj->scroll_tip, GT_SCROLL_TIP_RIGHT)) {
        area_base = _calc_scroll_tip_bar_area(obj, GT_SCROLL_TIP_RIGHT);
    } else if (_IS_SCROLL_TIP_MODE(obj->scroll_tip, GT_SCROLL_TIP_LEFT)) {
        area_base = _calc_scroll_tip_bar_area(obj, GT_SCROLL_TIP_LEFT);
    }
    if (area_base.w && area_base.h) {
        draw_bg(obj->draw_ctx, &rect_attr, &area_base);
    }

    if (_IS_SCROLL_TIP_MODE(obj->scroll_tip, GT_SCROLL_TIP_BOTTOM)) {
        area_base = _calc_scroll_tip_bar_area(obj, GT_SCROLL_TIP_BOTTOM);
        draw_bg(obj->draw_ctx, &rect_attr, &area_base);
    } else if (_IS_SCROLL_TIP_MODE(obj->scroll_tip, GT_SCROLL_TIP_TOP)) {
        area_base = _calc_scroll_tip_bar_area(obj, GT_SCROLL_TIP_TOP);
        draw_bg(obj->draw_ctx, &rect_attr, &area_base);
    }
    if (area_base.w && area_base.h) {
        draw_bg(obj->draw_ctx, &rect_attr, &area_base);
    }
}



/* global functions / API interface -------------------------------------*/
gt_res_t gt_scroll_tip_draw(gt_obj_st * obj)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return GT_RES_FAIL;
    }
    if (GT_SCROLL_TIP_NONE == obj->scroll_tip) {
        return GT_RES_INV;
    }
    if (obj->scroll_tip > _GT_SCROLL_TIP_MAX) {
        return GT_RES_INV;
    }

    _tip_bar_init(obj);

    return GT_RES_OK;
}


/* end ------------------------------------------------------------------*/
