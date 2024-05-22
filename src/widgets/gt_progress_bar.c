/**
 * @file gt_progress_bar.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-07-22 14:13:10
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_progress_bar.h"

#if GT_CFG_ENABLE_PROGRESS_BAR
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../font/gt_font.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_PROCESS_BAR
#define MY_CLASS    &gt_progress_bar_class

/* private typedef ------------------------------------------------------*/
typedef struct _gt_progress_bar_s {
    gt_obj_st obj;
    gt_color_t color_act;
    gt_color_t color_ina;
    gt_size_t start;
    gt_size_t end;
    gt_size_t pos;
    gt_bar_dir_et dir;
}_gt_progress_bar_st;


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_progress_bar_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_progress_bar_st)
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

/**
 * @brief obj init progress_bar widget call back
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * progress_bar) {
    _gt_progress_bar_st * style = (_gt_progress_bar_st * )progress_bar;
    gt_size_t w, h;
    int dist = 0, dist_min = 0;
    int all_pos = style->end - style->start;
    int current_pos = style->pos - style->start;

    // set default size
    if( !progress_bar->area.w || !progress_bar->area.h ){
        progress_bar->area.w = 100;
        progress_bar->area.h = 16;
    };

    w = progress_bar->area.w;
    h = progress_bar->area.h;

    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.reg.is_fill    = 1;
    rect_attr.bg_color       = style->color_ina;
    rect_attr.border_color   = style->color_ina;
    rect_attr.radius         = h >> 1;
    rect_attr.bg_opa         = progress_bar->opa;


    gt_area_st area_base = progress_bar->area;
    gt_area_st area_val = progress_bar->area;

    if( style->dir == GT_BAR_DIR_HOR_L2R || style->dir == GT_BAR_DIR_HOR_R2L ){
        dist = ((float)current_pos/(float)all_pos) * (float)w;
        area_val.w = dist;
        // area_base.w -= 2;
        if(style->dir == GT_BAR_DIR_HOR_L2R){
            // area_base.x += 2;
        }else{
            area_val.x += (w - dist);
        }
    }else if( style->dir == GT_BAR_DIR_VER_U2D || style->dir == GT_BAR_DIR_VER_D2U ){
        dist = ((float)current_pos/(float)all_pos) * (float)h;
        area_val.h = dist;
        // area_base.h -= 2;
        rect_attr.radius = w >> 1;

        if (style->dir == GT_BAR_DIR_VER_U2D){
            // area_base.y += 2;
        }else{
            area_val.y += (h - dist);
        }
    }
    if(style->pos < style->end){
        draw_bg(progress_bar->draw_ctx, &rect_attr, &area_base);
    }
    rect_attr.bg_color = style->color_act;

    dist_min = rect_attr.radius << 1;
    // When dist = 1, area_val is plotted as a line
    if(dist > 1){
        if(dist < dist_min){
            rect_attr.base_area = &area_base;
        }
        draw_bg(progress_bar->draw_ctx, &rect_attr, &area_val);
    }
    // focus
    draw_focus(progress_bar , 0);
}

/**
 * @brief obj deinit call back
 *
 * @param obj
 */
static void _deinit_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "start deinit_cb");

    if (NULL == obj) {
        return ;
    }

}

static void _scroll_dir_ver(gt_obj_st * obj, bool is_up) {
    _gt_progress_bar_st * style = (_gt_progress_bar_st * )obj;
    gt_size_t val = is_up ? 1 : -1;

    if (GT_BAR_DIR_VER_U2D == style->dir) {
        gt_progress_bar_set_pos(obj, gt_progress_bar_get_pos(obj) + val);
    } else if (GT_BAR_DIR_VER_D2U == style->dir) {
        gt_progress_bar_set_pos(obj, gt_progress_bar_get_pos(obj) - val);
    }
}

static void _scroll_dir_hor(gt_obj_st * obj, bool is_right) {
    _gt_progress_bar_st * style = (_gt_progress_bar_st * )obj;
    gt_size_t val = is_right ? 1 : -1;

    if (GT_BAR_DIR_HOR_L2R == style->dir) {
        gt_progress_bar_set_pos(obj, gt_progress_bar_get_pos(obj) + val);
    } else if (GT_BAR_DIR_HOR_R2L == style->dir) {
        gt_progress_bar_set_pos(obj, gt_progress_bar_get_pos(obj) - val);
    }
}

/**
 * @brief obj event handler call back
 *
 * @param obj
 * @param e event
 */
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code = gt_event_get_code(e);
    switch(code) {
        case GT_EVENT_TYPE_DRAW_START:
            GT_LOGV(GT_LOG_TAG_GUI, "start draw");
            gt_disp_invalid_area(obj);
            break;

        case GT_EVENT_TYPE_INPUT_RELEASED: /* click event finish */
            GT_LOGV(GT_LOG_TAG_GUI, "processed");
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_SCROLL_UP:
            _scroll_dir_ver(obj, true);
            break;

        case GT_EVENT_TYPE_INPUT_SCROLL_DOWN:
            _scroll_dir_ver(obj, false);
            break;

        case GT_EVENT_TYPE_INPUT_SCROLL_LEFT:
            _scroll_dir_hor(obj, false);
            break;

        case GT_EVENT_TYPE_INPUT_SCROLL_RIGHT:
            _scroll_dir_hor(obj, true);
            break;

        default:
            break;
    }
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief create a progress_bar obj
 *
 * @param parent progress_bar's parent element
 * @return gt_obj_st* progress_bar obj
 */
gt_obj_st * gt_progress_bar_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    if (NULL == obj) {
        return obj;
    }

    _gt_progress_bar_st * style = (_gt_progress_bar_st * )obj;

    style->color_act = gt_color_hex(0x409eff);
    style->color_ina = gt_color_hex(0xebeef5);
    style->start = 0;
    style->end = 100;
    style->pos = 50;
    style->dir = GT_BAR_DIR_HOR_L2R;

    return obj;
}


/**
 * @brief set progress_bar pos
 *
 * @param progress_bar progress_bar
 * @param pos pos
 */
void gt_progress_bar_set_pos(gt_obj_st * progress_bar, gt_size_t pos)
{
    if (false == gt_obj_is_type(progress_bar, OBJ_TYPE)) {
        return;
    }
    _gt_progress_bar_st * style = (_gt_progress_bar_st * )progress_bar;
    if( pos < style->start ){
        style->pos = style->start;
    }else if( pos > style->end ){
        style->pos = style->end;
    }else{
        style->pos = pos;
    }
    gt_event_send(progress_bar, GT_EVENT_TYPE_DRAW_START, NULL);
    gt_event_send(progress_bar, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

void gt_progress_bar_set_start_end(gt_obj_st * progress_bar, gt_size_t start, gt_size_t end)
{
    if (false == gt_obj_is_type(progress_bar, OBJ_TYPE)) {
        return;
    }
    _gt_progress_bar_st * style = (_gt_progress_bar_st * )progress_bar;
    style->start = start;
    style->end = end;
}

gt_size_t gt_progress_bar_get_pos(gt_obj_st * progress_bar)
{
    if (false == gt_obj_is_type(progress_bar, OBJ_TYPE)) {
        return 0;
    }
    _gt_progress_bar_st * style = (_gt_progress_bar_st * )progress_bar;
    return style->pos;
}

gt_size_t gt_progress_bar_get_start(gt_obj_st * progress_bar)
{
    if (false == gt_obj_is_type(progress_bar, OBJ_TYPE)) {
        return 0;
    }
    _gt_progress_bar_st * style = (_gt_progress_bar_st * )progress_bar;
    return style->start;
}

gt_size_t gt_progress_bar_get_end(gt_obj_st * progress_bar)
{
    if (false == gt_obj_is_type(progress_bar, OBJ_TYPE)) {
        return 0;
    }
    _gt_progress_bar_st * style = (_gt_progress_bar_st * )progress_bar;
    return style->end;
}

gt_size_t gt_progress_bar_get_total(gt_obj_st * progress_bar)
{
    if (false == gt_obj_is_type(progress_bar, OBJ_TYPE)) {
        return 0;
    }
    _gt_progress_bar_st * style = (_gt_progress_bar_st * )progress_bar;
    return ( style->end - style->start);
}

void gt_progress_bar_set_color_act(gt_obj_st * progress_bar, gt_color_t color)
{
    if (false == gt_obj_is_type(progress_bar, OBJ_TYPE)) {
        return;
    }
    _gt_progress_bar_st * style = (_gt_progress_bar_st * )progress_bar;
    style->color_act = color;
    gt_event_send(progress_bar, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_progress_bar_set_color_ina(gt_obj_st * progress_bar, gt_color_t color)
{
    if (false == gt_obj_is_type(progress_bar, OBJ_TYPE)) {
        return;
    }
    _gt_progress_bar_st * style = (_gt_progress_bar_st * )progress_bar;
    style->color_ina = color;
    gt_event_send(progress_bar, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_progress_bar_set_dir(gt_obj_st * progress_bar, gt_bar_dir_et dir)
{
    if (false == gt_obj_is_type(progress_bar, OBJ_TYPE)) {
        return;
    }
    _gt_progress_bar_st * style = (_gt_progress_bar_st * )progress_bar;
    style->dir = dir;
    gt_event_send(progress_bar, GT_EVENT_TYPE_DRAW_START, NULL);
}

gt_bar_dir_et gt_progress_bar_get_dir(gt_obj_st * progress_bar)
{
    if (false == gt_obj_is_type(progress_bar, OBJ_TYPE)) {
        return (gt_bar_dir_et)0;
    }
    _gt_progress_bar_st * style = (_gt_progress_bar_st * )progress_bar;
    return style->dir;
}

#endif  /** GT_CFG_ENABLE_PROGRESS_BAR */
/* end ------------------------------------------------------------------*/
