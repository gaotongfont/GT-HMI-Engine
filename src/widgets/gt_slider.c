/**
 * @file gt_slider.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-07-21 19:53:32
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_slider.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../font/gt_font.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"
#include "../core/gt_fs.h"
#include "../core/gt_indev.h"
/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_SLIDER
#define MY_CLASS    &gt_slider_class

/* private typedef ------------------------------------------------------*/
typedef struct _gt_slider_s
{
    char * tag;             //img src
    gt_area_st area_base;

    gt_color_t color_act;
    gt_color_t color_ina;

    gt_size_t start;
    gt_size_t end;
    gt_size_t pos;

    uint16_t offset;
    gt_bar_dir_et dir;
    uint8_t step;

    bool tag_visible;
}_gt_slider_st;


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_slider_class = {
    ._init_cb       = _init_cb,
    ._deinit_cb     = _deinit_cb,
    ._event_cb      = _event_cb,
    .type           = OBJ_TYPE,
    .size_style     = sizeof(_gt_slider_st)
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static uint16_t _gt_slider_get_offset(gt_obj_st * slider)
{
    _gt_slider_st * style = slider->style;

    return style->offset;
}

static gt_size_t _calc_touch_point_pos(gt_obj_st * obj) {
    _gt_slider_st * style = obj->style;
    gt_obj_st * scr = gt_disp_get_scr();
    gt_point_st point_click = gt_indev_get_point();
    gt_size_t pos = 0;

    if(NULL == scr){
        return style->offset;
    }

    point_click.x += scr->area.x;
    point_click.y += scr->area.y;

    if( gt_slider_get_dir(obj) == GT_BAR_DIR_HOR_L2R ){
        pos = point_click.x - obj->area.x;
    }else if( gt_slider_get_dir(obj) == GT_BAR_DIR_HOR_R2L ){
        pos = obj->area.w - (point_click.x - obj->area.x);
    }else if( gt_slider_get_dir(obj) == GT_BAR_DIR_VER_U2D ){
        pos = point_click.y - obj->area.y;
    }else if( gt_slider_get_dir(obj) == GT_BAR_DIR_VER_D2U ){
        pos = obj->area.h - (point_click.y - obj->area.y);
    }
    return pos;
}

static void _gt_slider_set_offset(gt_obj_st * slider , int offset)
{
    _gt_slider_st * style = slider->style;

    if( (offset > slider->area.w) && (style->dir == GT_BAR_DIR_HOR_L2R || style->dir == GT_BAR_DIR_HOR_R2L) ){
        style->offset = slider->area.w;
    }
    else if( (offset > slider->area.h) && (style->dir == GT_BAR_DIR_VER_U2D || style->dir == GT_BAR_DIR_VER_D2U) ){
        style->offset = slider->area.h;
    }
    else if(offset < 0){
        style->offset = 0;
    }
    else{
        style->offset = offset;
    }
}

static void _scroll_dir_ver(gt_obj_st * obj, bool is_up) {
    _gt_slider_st * style = obj->style;
    gt_size_t val = is_up ? 1 : -1;

    if (GT_BAR_DIR_VER_U2D == style->dir) {
        _gt_slider_set_offset(obj, _gt_slider_get_offset(obj) + val);
    } else if (GT_BAR_DIR_VER_D2U == style->dir) {
        _gt_slider_set_offset(obj, _gt_slider_get_offset(obj) - val);
    }
}

static void _scroll_dir_hor(gt_obj_st * obj, bool is_right) {
    _gt_slider_st * style = obj->style;
    gt_size_t val = is_right ? 1 : -1;

    if (GT_BAR_DIR_HOR_L2R == style->dir) {
        _gt_slider_set_offset(obj, _gt_slider_get_offset(obj) + val);
    } else if (GT_BAR_DIR_HOR_R2L == style->dir) {
        _gt_slider_set_offset(obj, _gt_slider_get_offset(obj) - val);
    }
}

static inline void _gt_slider_init_widget(gt_obj_st * slider) {
    _gt_slider_st * style = slider->style;
    gt_size_t w, h;
    // int all_pos = style->end - style->start;
    float _tmp_pos;
    int dist = 0, dist_min;
    // set default size
    if( !slider->area.w || !slider->area.h ){
        slider->area.w = 100;
        slider->area.h = 20;
    };

    if( style->tag_visible && style->tag && style->area_base.w != 0 && style->area_base.h != 0){
        slider->area = style->area_base;
    }
    w = slider->area.w;
    h = slider->area.h;

    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.reg.is_fill    = 1;
    rect_attr.radius         = h>>2;
    rect_attr.bg_color       = style->color_ina;
    rect_attr.border_color   = style->color_ina;
    rect_attr.bg_opa         = slider->opa;

    gt_area_st area_base = slider->area;
    gt_area_st area_val = slider->area;
    /* 1 base shape */
    if( style->dir == GT_BAR_DIR_HOR_L2R || style->dir == GT_BAR_DIR_HOR_R2L ){
        area_base.y += rect_attr.radius;
        area_base.w -= 2;
        area_base.h -= h>>1;
        dist = style->offset; // ((float)current_pos/(float)all_pos) * (float)w;
        area_val.y = area_base.y;
        area_val.w = dist;
        area_val.h = area_base.h;
        if(style->dir == GT_BAR_DIR_HOR_L2R){
            area_base.x += 2;
        }else{
            area_val.x += (w - dist);
        }
        _tmp_pos = slider->area.w;
    }else if( style->dir == GT_BAR_DIR_VER_U2D || style->dir == GT_BAR_DIR_VER_D2U ){
        rect_attr.radius = w >> 2;
        area_base.x += rect_attr.radius;
        area_base.w -= w >> 1;
        area_base.h -= 2;
        dist = style->offset; // ((float)style->pos/(float)all_pos) * (float)h;
        area_val.x = area_base.x;
        area_val.w = area_base.w;
        area_val.h = dist;
        if (style->dir == GT_BAR_DIR_VER_U2D){
            area_base.y += 2;
        }else{
            area_val.y += (h - dist);
        }
        _tmp_pos = slider->area.h;
    }
    /* 1 draw base shape */
    // if(style->pos < style->end){
    //     draw_bg(slider->draw_ctx, &rect_attr, &area_base);
    // }

    draw_bg(slider->draw_ctx, &rect_attr, &area_base);

    /* 2 draw val shape */
    rect_attr.bg_color = style->color_act;
    dist_min = rect_attr.radius;
    bool isTag = false;

    if( style->tag_visible ){
        uint16_t _w,_h;
        if( style->tag != NULL ){
            if( !gt_fs_read_img_wh(style->tag, &_w, &_h) ){
                w = _w;
                h = _h;
                isTag = true;
            }
        }
        if( style->dir == GT_BAR_DIR_HOR_L2R || style->dir == GT_BAR_DIR_HOR_R2L ){
            dist_min = (h>>1);
        }else if( style->dir == GT_BAR_DIR_VER_D2U || style->dir == GT_BAR_DIR_VER_U2D ){
            dist_min = (w>>1);
        }
    }

    if( dist >= dist_min ){
        draw_bg(slider->draw_ctx, &rect_attr, &area_val);
    }

    //
    _tmp_pos = (float)style->offset / _tmp_pos;
    style->pos = (gt_size_t)(_tmp_pos * (style->end-style->start)) + style->start;

    /* visible or not */
    if( !style->tag_visible ){
        return;
    }
    gt_area_st area_big_circle = slider->area;

    /*get scale indication circle info*/
    if( style->dir == GT_BAR_DIR_HOR_L2R || style->dir == GT_BAR_DIR_HOR_R2L ){
        area_big_circle.y += (slider->area.h - h) >> 1;
        area_big_circle.h = h;
        area_big_circle.w = area_big_circle.h;
        rect_attr.radius = area_big_circle.h >> 1;
        if( style->dir == GT_BAR_DIR_HOR_L2R ){
            area_big_circle.x += (dist - rect_attr.radius);
        }else{
            area_big_circle.x = (area_val.x - rect_attr.radius);
        }
        if( area_big_circle.x < slider->area.x ){
            area_big_circle.x = slider->area.x;
        }
        if( area_big_circle.x > (slider->area.x + slider->area.w - area_big_circle.w) ){
            area_big_circle.x = slider->area.x + slider->area.w - area_big_circle.w;
        }
    }else if( style->dir == GT_BAR_DIR_VER_U2D || style->dir == GT_BAR_DIR_VER_D2U ){
        area_big_circle.x += (slider->area.w - w) >> 1;
        area_big_circle.w = w;
        area_big_circle.h = area_big_circle.w;
        rect_attr.radius = area_big_circle.w >> 1;
        if( style->dir == GT_BAR_DIR_VER_U2D ){
            area_big_circle.y += (dist - rect_attr.radius);
        }else{
            area_big_circle.y = (area_val.y - rect_attr.radius);
        }
        if( area_big_circle.y < slider->area.y ){
            area_big_circle.y = slider->area.y;
        }
        if( area_big_circle.y > (slider->area.y + slider->area.h - area_big_circle.h) ){
            area_big_circle.y = slider->area.y + slider->area.h - area_big_circle.h;
        }
    }
    /*draw scale indication circle or image*/
    if( isTag == false ){
        draw_bg(slider->draw_ctx, &rect_attr, &area_big_circle);
    }else{
        rect_attr.bg_img_src = style->tag;
        // area_big_circle.w = w;
        // area_big_circle.h = h;
        style->area_base = slider->area;
        if( slider->area.w < w ){
            slider->area.w = w;
            slider->area.x = area_big_circle.x;
        }
        if( slider->area.h < h ){
            slider->area.h = h;
            slider->area.y  = area_big_circle.y;
        }
        draw_bg_img(slider->draw_ctx, &rect_attr, &area_big_circle);
    }

    // focus
    draw_focus(slider , 0);
}

/**
 * @brief obj init slider widget call back
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");

    _gt_slider_init_widget(obj);
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

    _gt_slider_st ** style_p = (_gt_slider_st ** )&obj->style;
    if (NULL == *style_p) {
        return ;
    }

    if (NULL != (*style_p)->tag) {
        gt_mem_free((*style_p)->tag);
        (*style_p)->tag = NULL;
    }

    gt_mem_free(*style_p);
    *style_p = NULL;
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
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_PRESSING:   /* add clicking style and process clicking event */
            GT_LOGV(GT_LOG_TAG_GUI, "clicking");
            gt_event_send(obj, GT_EVENT_TYPE_INPUT_RELEASED, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_SCROLL:
            GT_LOGV(GT_LOG_TAG_GUI, "scroll");
            _gt_slider_set_offset(obj, _calc_touch_point_pos(obj));
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_RELEASED: /* click event finish */
            GT_LOGV(GT_LOG_TAG_GUI, "processed");
            _gt_slider_set_offset(obj, _calc_touch_point_pos(obj));
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
 * @brief create a slider obj
 *
 * @param parent slider's parent element
 * @return gt_obj_st* slider obj
 */
gt_obj_st * gt_slider_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    _gt_slider_st * style = (_gt_slider_st *)obj->style;

    gt_memset(style,0,sizeof(_gt_slider_st));

    style->color_act = gt_color_hex(0x409eff);
    style->color_ina = gt_color_hex(0xebeef5);
    style->start = 0;
    style->end = 100;
    style->pos = 50;
    style->dir = GT_BAR_DIR_HOR_L2R;
    style->step = 1;
    style->tag_visible = true;
    style->offset = 0;

    return obj;
}


/**
 * @brief set slider pos
 *
 * @param slider object
 * @param pos pos
 */
void gt_slider_set_pos(gt_obj_st * slider, gt_size_t pos)
{
    if (NULL == slider) {
        return;
    }
    if (GT_TYPE_SLIDER != gt_obj_class_get_type(slider)) {
        return;
    }
    _gt_slider_st * style = (_gt_slider_st *)slider->style;
    float max = 0 , offset = 0;
    if( pos < style->start ){
        style->pos = style->start;
        offset = 0;
    }else if( pos > style->end ){
        style->pos = style->end;
        offset = GT_MAX(slider->area.w , slider->area.h);
    }else{
        style->pos = ( (int)(pos / style->step) * ((int)(style->step)) );
        if( (style->dir == GT_BAR_DIR_HOR_L2R || style->dir == GT_BAR_DIR_HOR_R2L) ){
            max = slider->area.w;

        }
        else if( (style->dir == GT_BAR_DIR_VER_U2D || style->dir == GT_BAR_DIR_VER_D2U) ){
            max = slider->area.h;
        }

        offset = (float)(pos - style->start) / (style->end - style->start);
        offset = max * offset;
    }
    _gt_slider_set_offset(slider , offset);

    gt_event_send(slider, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_slider_set_start_end(gt_obj_st * slider, gt_size_t start, gt_size_t end)
{
    if (NULL == slider) {
        return;
    }
    if (GT_TYPE_SLIDER != gt_obj_class_get_type(slider)) {
        return;
    }
    _gt_slider_st * style = (_gt_slider_st *)slider->style;
    style->start = start;
    style->end = end;
}

gt_size_t gt_slider_get_pos(gt_obj_st * slider)
{
    if (NULL == slider) {
        return 0;
    }
    if (GT_TYPE_SLIDER != gt_obj_class_get_type(slider)) {
        return 0;
    }
    _gt_slider_st * style = (_gt_slider_st *)slider->style;
    return style->pos;
}

gt_size_t gt_slider_get_start(gt_obj_st * slider)
{
    if (NULL == slider) {
        return 0;
    }
    if (GT_TYPE_SLIDER != gt_obj_class_get_type(slider)) {
        return 0;
    }
    _gt_slider_st * style = (_gt_slider_st *)slider->style;
    return style->start;
}

gt_size_t gt_slider_get_end(gt_obj_st * slider)
{
    if (NULL == slider) {
        return 0;
    }
    if (GT_TYPE_SLIDER != gt_obj_class_get_type(slider)) {
        return 0;
    }
    _gt_slider_st * style = (_gt_slider_st *)slider->style;
    return style->end;
}

gt_size_t gt_slider_get_total(gt_obj_st * slider)
{
    if (NULL == slider) {
        return 0;
    }
    if (GT_TYPE_SLIDER != gt_obj_class_get_type(slider)) {
        return 0;
    }
    _gt_slider_st * style = (_gt_slider_st *)slider->style;
    return ( style->end - style->start);
}

void gt_slider_set_color_act(gt_obj_st * slider, gt_color_t color)
{
    if (NULL == slider) {
        return;
    }
    if (GT_TYPE_SLIDER != gt_obj_class_get_type(slider)) {
        return;
    }
    _gt_slider_st * style = slider->style;
    style->color_act = color;
    gt_event_send(slider, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_slider_set_color_ina(gt_obj_st * slider, gt_color_t color)
{
    if (NULL == slider) {
        return;
    }
    if (GT_TYPE_SLIDER != gt_obj_class_get_type(slider)) {
        return;
    }
    _gt_slider_st * style = slider->style;
    style->color_ina = color;
    gt_event_send(slider, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_slider_set_dir(gt_obj_st * slider, gt_bar_dir_et dir)
{
    if (NULL == slider) {
        return;
    }
    if (GT_TYPE_SLIDER != gt_obj_class_get_type(slider)) {
        return;
    }
    _gt_slider_st * style = (_gt_slider_st *)slider->style;

    style->dir = dir;
    // gt_event_send(slider, GT_EVENT_TYPE_DRAW_START, NULL);
    gt_slider_set_pos(slider, style->pos);
}

gt_bar_dir_et gt_slider_get_dir(gt_obj_st * slider)
{
    if (NULL == slider) {
        return 0;
    }
    if (GT_TYPE_SLIDER != gt_obj_class_get_type(slider)) {
        return 0;
    }
    _gt_slider_st * style = (_gt_slider_st *)slider->style;
    return style->dir;
}

void gt_slider_set_step(gt_obj_st * slider, uint8_t step)
{
    if (NULL == slider) {
        return;
    }
    if (GT_TYPE_SLIDER != gt_obj_class_get_type(slider)) {
        return;
    }
    _gt_slider_st * style = (_gt_slider_st *)slider->style;
    style->step = step;
}

uint8_t gt_slider_get_step(gt_obj_st * slider)
{
    if (NULL == slider) {
        return 0;
    }
    if (GT_TYPE_SLIDER != gt_obj_class_get_type(slider)) {
        return 0;
    }
    _gt_slider_st * style = (_gt_slider_st *)slider->style;
    return ( style->step);
}

void gt_slider_set_tag(gt_obj_st * slider, char * src)
{
    if (NULL == slider) {
        return;
    }
    if (GT_TYPE_SLIDER != gt_obj_class_get_type(slider)) {
        return;
    }
    _gt_slider_st * style = (_gt_slider_st *)slider->style;
    if( style->tag != NULL ){
        gt_mem_free(style->tag);
    }
    style->tag = gt_mem_malloc(strlen(src) + 1);
    gt_memcpy(style->tag, src, strlen(src));
    style->tag[strlen(src)] = 0;
    gt_event_send(slider, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_slider_set_tag_visible(gt_obj_st * slider, bool visible)
{
    if (NULL == slider) {
        return;
    }
    if (GT_TYPE_SLIDER != gt_obj_class_get_type(slider)) {
        return;
    }
    _gt_slider_st * style = (_gt_slider_st *)slider->style;
    style->tag_visible = visible;
    gt_event_send(slider, GT_EVENT_TYPE_DRAW_START, NULL);
}
/* end ------------------------------------------------------------------*/
