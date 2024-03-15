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
#include "gt_img.h"
/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_SLIDER
#define MY_CLASS    &gt_slider_class

/* private typedef ------------------------------------------------------*/
typedef struct _gt_slider_s {
    gt_obj_st obj;

    gt_obj_st *tag;
    gt_obj_st *img;         /** The img object within tag area */

    gt_color_t color_act;   /** The active color of the slider */
    gt_color_t color_ina;   /** The inactive color of the slider */

    gt_size_t start;    /** The starting value of the range */
    gt_size_t end;      /** The end value of the range */
    gt_size_t pos;      /** The current value of the range */
    uint16_t step;      /** Increments/decreases in units each time */

    uint16_t offset;    /** Offset pixels from the start pixel */
    uint16_t size;
    uint16_t tag_size;
    gt_bar_dir_et dir;

    uint8_t mode;

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
static uint16_t _gt_slider_get_tag_size(gt_obj_st * slider)
{
    _gt_slider_st * style = (_gt_slider_st * )slider;
    uint16_t max = 0;
    if(style->dir == GT_BAR_DIR_HOR_L2R || style->dir == GT_BAR_DIR_HOR_R2L){
        max = slider->area.h;
    }
    else if(style->dir == GT_BAR_DIR_VER_U2D || style->dir == GT_BAR_DIR_VER_D2U){
        max = slider->area.w;
    }

    return ((style->tag_size > 0 && style->tag_size <= max) ? style->tag_size : max);
}

static uint16_t _gt_slider_get_size(gt_obj_st * slider)
{
    _gt_slider_st * style = (_gt_slider_st * )slider;
    uint16_t max = 0;
    if(style->dir == GT_BAR_DIR_HOR_L2R || style->dir == GT_BAR_DIR_HOR_R2L){
        max = slider->area.h;
    }
    else if(style->dir == GT_BAR_DIR_VER_U2D || style->dir == GT_BAR_DIR_VER_D2U){
        max = slider->area.w;
    }

    if(style->size <= 0){
        style->size =  max >> 1;
    }
    else if(style->size > max){
        style->size = max;
    }

    return style->size;
}

static gt_size_t _calc_touch_point_pos(gt_obj_st * obj) {
    _gt_slider_st * style = (_gt_slider_st * )obj;
    gt_obj_st * scr = gt_disp_get_scr();
    gt_point_st point_click = gt_indev_get_point();
    gt_size_t temp_offset = 0;

    if(NULL == scr){
        return style->offset;
    }

    point_click.x += scr->area.x;
    point_click.y += scr->area.y;

    if( gt_slider_get_dir(obj) == GT_BAR_DIR_HOR_L2R ){
        temp_offset = point_click.x - obj->area.x;
    }else if( gt_slider_get_dir(obj) == GT_BAR_DIR_HOR_R2L ){
        temp_offset = obj->area.w - (point_click.x - obj->area.x);
    }else if( gt_slider_get_dir(obj) == GT_BAR_DIR_VER_U2D ){
        temp_offset = point_click.y - obj->area.y;
    }else if( gt_slider_get_dir(obj) == GT_BAR_DIR_VER_D2U ){
        temp_offset = obj->area.h - (point_click.y - obj->area.y);
    }
    return temp_offset;
}

static void _gt_slider_set_offset_by_pos(gt_obj_st * slider, gt_size_t pos)
{
    _gt_slider_st * style = (_gt_slider_st * )slider;

    if (pos < style->start) {
        pos = style->start;
    } else if (pos > style->end) {
        pos = style->end;
    }
    style->pos = pos;

    uint16_t max = 0;

    if( (style->dir == GT_BAR_DIR_HOR_L2R || style->dir == GT_BAR_DIR_HOR_R2L) ){
        max = slider->area.w - (_gt_slider_get_tag_size(slider));
    }
    else if( (style->dir == GT_BAR_DIR_VER_U2D || style->dir == GT_BAR_DIR_VER_D2U) ){
        max = slider->area.h - (_gt_slider_get_tag_size(slider));
    }

    style->offset = (style->pos - style->start) * max / (style->end - style->start);
}

static void _gt_slider_set_offset(gt_obj_st * slider , gt_size_t offset)
{
    _gt_slider_st * style = (_gt_slider_st * )slider;
    uint16_t max_offset = 0;
    uint16_t tag_size = _gt_slider_get_tag_size(slider);
    if(style->dir == GT_BAR_DIR_HOR_L2R || style->dir == GT_BAR_DIR_HOR_R2L){
        max_offset = slider->area.w - tag_size;
    }
    else if(style->dir == GT_BAR_DIR_VER_U2D || style->dir == GT_BAR_DIR_VER_D2U){
        max_offset = slider->area.h - tag_size;
    }

    offset -= (tag_size >> 1);

    if(offset > max_offset){
        style->offset = max_offset;
    }
    else if(offset < 0){
        style->offset = 0;
    }
    else{
        style->offset = offset;
    }

    gt_size_t tmp_pos = style->offset * (style->end - style->start) / max_offset + style->start;
    if(GT_SLIDER_MODE_STEP == style->mode){
        tmp_pos = tmp_pos / style->step * style->step;
        _gt_slider_set_offset_by_pos(slider , tmp_pos);
        return ;
    }

    style->pos = tmp_pos;

    // style->pos = style->offset * (style->end - style->start) / max_offset + style->start;
}

static void _scroll_dir_ver(gt_obj_st * obj, bool is_up) {
    _gt_slider_st * style = (_gt_slider_st * )obj;
    gt_size_t val = is_up ? style->step : -style->step;

    if (GT_BAR_DIR_VER_U2D == style->dir) {
        _gt_slider_set_offset_by_pos(obj, style->pos + val);
    } else if (GT_BAR_DIR_VER_D2U == style->dir) {
        _gt_slider_set_offset_by_pos(obj, style->pos - val);
    }
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

static void _scroll_dir_hor(gt_obj_st * obj, bool is_right) {
    _gt_slider_st * style = (_gt_slider_st * )obj;
    gt_size_t val = is_right ? style->step : -style->step;

    if (GT_BAR_DIR_HOR_L2R == style->dir) {
        _gt_slider_set_offset_by_pos(obj, style->pos + val);
    } else if (GT_BAR_DIR_HOR_R2L == style->dir) {
        _gt_slider_set_offset_by_pos(obj, style->pos - val);
    }
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

static inline void _gt_slider_init_widget(gt_obj_st * slider) {
    _gt_slider_st * style = (_gt_slider_st * )slider;
    int dist = 0, dist_min = 0;

    // set default size
    if( !slider->area.w || !slider->area.h ){
        slider->area.w = 100;
        slider->area.h = 16;
    };

    uint16_t size = _gt_slider_get_size(slider);
    uint16_t tag_size = _gt_slider_get_tag_size(slider);

    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.reg.is_fill    = 1;
    rect_attr.bg_opa         = slider->opa;
    rect_attr.bg_color       = style->color_ina;
    //
    gt_area_st area_base = {0}, area_val = {0} , tag_area = {0};

    dist = style->offset;
    rect_attr.radius = size >> 1;
    if( style->dir == GT_BAR_DIR_HOR_L2R || style->dir == GT_BAR_DIR_HOR_R2L ){
        area_base.x = slider->area.x + (tag_size >> 1);
        area_base.w = slider->area.w - (tag_size);
        area_base.y = slider->area.y + ((slider->area.h-size)>>1);
        area_base.h = size;
        //
        tag_area.y = slider->area.y + ((slider->area.h-tag_size)>>1);
        //
        gt_area_copy(&area_val, &area_base);
        area_val.w = dist;
        if(style->dir == GT_BAR_DIR_HOR_R2L){
            area_val.x += (area_base.w - dist);
            tag_area.x = slider->area.x + slider->area.w - dist - tag_size;
        }
        else{
            tag_area.x = slider->area.x + dist;
        }

    }else if( style->dir == GT_BAR_DIR_VER_U2D || style->dir == GT_BAR_DIR_VER_D2U ){
        area_base.y = slider->area.y + (tag_size >> 1);
        area_base.h = slider->area.h - (tag_size);
        area_base.x = slider->area.x + ((slider->area.w-size)>>1);
        area_base.w = size;
        //
        tag_area.x = slider->area.x + ((slider->area.w-tag_size)>>1);
        //
        gt_area_copy(&area_val, &area_base);
        area_val.h = dist;
        if (style->dir == GT_BAR_DIR_VER_D2U){
            area_val.y += (area_base.h - dist);
            tag_area.y = slider->area.y + slider->area.h - dist - tag_size;
        }
        else{
            tag_area.y = slider->area.y + dist;
        }
    }
    if(style->pos < style->end){
        draw_bg(slider->draw_ctx, &rect_attr, &area_base);
    }

    rect_attr.bg_color = style->color_act;
    dist_min = rect_attr.radius << 1;
    /* When dist = 1, area_val is plotted as a line */
    if(dist > 1){
        if(dist < dist_min){
            rect_attr.base_area = &area_base;
        }
        draw_bg(slider->draw_ctx, &rect_attr, &area_val);
        rect_attr.base_area = NULL;
    }

    /* visible or not */
    if( !style->tag_visible ){
        goto draw_focus;
    }

    tag_area.w = tag_size;
    tag_area.h = tag_size;
    rect_attr.radius = tag_size >> 1;

    // draw img tag
    if(style->tag && style->img){
        uint16_t _w = 0,_h = 0;
        if( !gt_fs_read_img_wh(gt_img_get_src(style->img), &_w, &_h) ){
            tag_area.w = _w > tag_size ? tag_size : _w;
            tag_area.h = _h > tag_size ? tag_size : _h;
            tag_area.x += (tag_size - tag_area.w) >> 1;
            tag_area.y += (tag_size - tag_area.h) >> 1;
        }
        gt_obj_set_area(style->tag, tag_area);
        gt_obj_set_area(style->img , tag_area);
        goto draw_focus;
    }

    // draw tag
    draw_bg(slider->draw_ctx, &rect_attr, &tag_area);

draw_focus:
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

    _gt_slider_st * style_p = (_gt_slider_st * )obj;
    if (NULL != style_p->tag) {
        gt_mem_free(style_p->tag);
        style_p->tag = NULL;
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
    _gt_slider_st * style = (_gt_slider_st *)obj;

    style->color_act = gt_color_hex(0x409eff);
    style->color_ina = gt_color_hex(0xebeef5);
    style->step = 1;
    style->start = 0;
    style->end = 100;
    style->pos = 0;
    style->dir = GT_BAR_DIR_HOR_L2R;
    style->tag_visible = true;
    style->offset = 0;
    style->mode = GT_SLIDER_MODE_DEFAULT;
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
    _gt_slider_st * style = (_gt_slider_st * )slider;

    // limit end and start
    if (style->end < style->start) {
        style->end = style->start;
    }

    _gt_slider_set_offset_by_pos(slider , pos);

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
    _gt_slider_st * style = (_gt_slider_st *)slider;
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
    _gt_slider_st * style = (_gt_slider_st *)slider;
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
    _gt_slider_st * style = (_gt_slider_st *)slider;
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
    _gt_slider_st * style = (_gt_slider_st *)slider;
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
    _gt_slider_st * style = (_gt_slider_st *)slider;
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
    _gt_slider_st * style = (_gt_slider_st *)slider;
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
    _gt_slider_st * style = (_gt_slider_st *)slider;
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
    _gt_slider_st * style = (_gt_slider_st *)slider;

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
    _gt_slider_st * style = (_gt_slider_st *)slider;
    return style->dir;
}

uint16_t gt_slider_get_step(gt_obj_st * slider)
{
    if (NULL == slider) {
        return 0;
    }
    if (GT_TYPE_SLIDER != gt_obj_class_get_type(slider)) {
        return 0;
    }
    _gt_slider_st * style = (_gt_slider_st *)slider;

    return style->step;
}

void gt_slider_set_step(gt_obj_st * slider, uint16_t step)
{
    if (NULL == slider) {
        return;
    }
    if (GT_TYPE_SLIDER != gt_obj_class_get_type(slider)) {
        return;
    }
    _gt_slider_st * style = (_gt_slider_st *)slider;

    style->step = step;
}

void gt_slider_set_tag(gt_obj_st * slider, char * src)
{
    if (NULL == slider) {
        return;
    }
    if (GT_TYPE_SLIDER != gt_obj_class_get_type(slider)) {
        return;
    }
    _gt_slider_st * style = (_gt_slider_st *)slider;
    if(NULL == style->tag){
        style->tag = gt_obj_create(slider);
        gt_obj_set_touch_parent(style->tag, true);
        gt_obj_set_bubble_notify(style->tag, true);
        gt_obj_set_inside(style->tag, true);
    }

    if(style->tag && NULL == style->img){
        style->img = gt_img_create(style->tag);
        gt_obj_set_touch_parent(style->img, true);
        gt_obj_set_bubble_notify(style->img, true);
        gt_obj_set_inside(style->img, true);
    }

    if(style->img){
        gt_img_set_src(style->img, src);
        gt_event_send(slider, GT_EVENT_TYPE_DRAW_START, NULL);
    }
}

void gt_slider_set_tag_visible(gt_obj_st * slider, bool visible)
{
    if (NULL == slider) {
        return;
    }
    if (GT_TYPE_SLIDER != gt_obj_class_get_type(slider)) {
        return;
    }
    _gt_slider_st * style = (_gt_slider_st *)slider;
    style->tag_visible = visible;
    gt_event_send(slider, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_slider_set_tag_size(gt_obj_st * slider , uint16_t size)
{
    if (NULL == slider) {
        return;
    }
    if (GT_TYPE_SLIDER != gt_obj_class_get_type(slider)) {
        return;
    }
    _gt_slider_st * style = (_gt_slider_st * )slider;
    style->tag_size = size;
    gt_event_send(slider, GT_EVENT_TYPE_DRAW_START, NULL);
}
void gt_slider_set_size(gt_obj_st * slider , uint16_t size)
{
    if (NULL == slider) {
        return;
    }
    if (GT_TYPE_SLIDER != gt_obj_class_get_type(slider)) {
        return;
    }
    _gt_slider_st * style = (_gt_slider_st * )slider;
    style->size = size;
    gt_event_send(slider, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_slider_set_mode(gt_obj_st * slider , gt_slider_mode_e mode)
{
    if (NULL == slider) {
        return;
    }
    if (GT_TYPE_SLIDER != gt_obj_class_get_type(slider)) {
        return;
    }
    _gt_slider_st * style = (_gt_slider_st * )slider;
    style->mode = mode;
    gt_event_send(slider, GT_EVENT_TYPE_DRAW_START, NULL);
}
/* end ------------------------------------------------------------------*/
