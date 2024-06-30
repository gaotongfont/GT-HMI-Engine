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

#if GT_CFG_ENABLE_SLIDER
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
/**
 * @brief AB repeat mode
 */
typedef struct _repeat_mode_s {
    gt_size_t begin;
    gt_size_t stop;
    gt_color_t color;   /** The color of the repeat area */
}_repeat_mode_st;


typedef struct _gt_slider_s {
    gt_obj_st obj;

    gt_obj_st *tag;
    gt_obj_st *img;         /** The img object within tag area */

    _repeat_mode_st repeat_info;    /** AB repeat mode */

    gt_color_t color_act;   /** The active color of the slider */
    gt_color_t color_ina;   /** The inactive color of the slider */

    gt_size_t start;    /** The starting value of the range */
    gt_size_t end;      /** The end value of the range */
    gt_size_t pos;      /** The current value of the range */
    uint16_t step;      /** Increments/decreases in units each time */

    uint16_t offset;    /** Offset pixels from the start pixel */
    uint16_t thickness;
    uint16_t tag_size;
    gt_bar_dir_et dir;

    uint8_t mode        : 2;    /** @ref gt_slider_mode_et */
    uint8_t tag_visible : 1;
    uint8_t repeat_mode : 1;
    uint8_t reserved : 4;
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

    if(style->thickness <= 0){
        style->thickness =  max >> 1;
    }
    else if(style->thickness > max){
        style->thickness = max;
    }

    return style->thickness;
}

/**
 * @brief limit total slider widget range
 *
 * @param style
 * @param pos need to limit pos
 * @return gt_size_t reasonable pos
 */
static gt_size_t _limit_total_range_pos(_gt_slider_st * style, gt_size_t pos) {
    if (pos < style->start) {
        pos = style->start;
    }
    else if (pos > style->end) {
        pos = style->end;
    }
    return pos;
}

static inline void _reset_repeat_mode(_gt_slider_st * style) {
    style->repeat_mode = false;
    style->repeat_info.begin = 0;
    style->repeat_info.stop = 0;
}

/**
 * @brief limit AB repeat mode range
 *
 * @param style
 * @param pos need to limit pos
 * @return gt_size_t reasonable pos
 */
static gt_size_t _limit_repeat_range_pos(_gt_slider_st * style, gt_size_t pos) {
    if (GT_SLIDER_MODE_DEFAULT != style->mode) {
        return pos;
    }
    if (false == style->repeat_mode) {
        /** return itself value */
        return pos;
    }
    if (pos < style->repeat_info.begin) {
        pos = style->repeat_info.begin;
    }
    else if (style->repeat_info.begin < style->repeat_info.stop &&
        pos > style->repeat_info.stop) {
        pos = style->repeat_info.stop;
    }
    return pos;
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

static gt_size_t _calc_pixel_offset_by_value(gt_obj_st * slider, gt_size_t value) {
    _gt_slider_st * style = (_gt_slider_st * )slider;
    uint16_t max = 0;

    if( (style->dir == GT_BAR_DIR_HOR_L2R || style->dir == GT_BAR_DIR_HOR_R2L) ){
        max = slider->area.w - (_gt_slider_get_tag_size(slider));
    }
    else if( (style->dir == GT_BAR_DIR_VER_U2D || style->dir == GT_BAR_DIR_VER_D2U) ){
        max = slider->area.h - (_gt_slider_get_tag_size(slider));
    }

    return (value - style->start) * max / (style->end - style->start);
}

static void _gt_slider_set_offset_by_pos(gt_obj_st * slider, gt_size_t pos)
{
    _gt_slider_st * style = (_gt_slider_st * )slider;

    style->pos = _limit_total_range_pos(style, pos);

    style->pos = _limit_repeat_range_pos(style, style->pos);

    style->offset = _calc_pixel_offset_by_value(slider, style->pos);
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
    } else {
        style->offset = offset;
    }

    gt_size_t tmp_pos = style->offset * (style->end - style->start) / max_offset + style->start;
    if(GT_SLIDER_MODE_STEP == style->mode){
        tmp_pos = tmp_pos / style->step * style->step;
        _gt_slider_set_offset_by_pos(slider , tmp_pos);
        return ;
    }

    if (style->repeat_mode) {
        gt_size_t pixel = _calc_pixel_offset_by_value(slider, style->repeat_info.begin);
        if (style->offset < pixel) {
            style->offset = pixel;
            tmp_pos = style->repeat_info.begin;
        }
        pixel = _calc_pixel_offset_by_value(slider, style->repeat_info.stop);
        if (style->repeat_info.stop > style->repeat_info.begin &&
            style->offset > pixel) {
            style->offset = pixel;
            tmp_pos = style->repeat_info.stop;
        }
    }

    style->pos = tmp_pos;
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

static void _calc_all_layer_area(gt_obj_st * slider, gt_area_st * base, gt_area_st * val, gt_area_st * tag) {
    _gt_slider_st * style = (_gt_slider_st * )slider;
    uint16_t size = _gt_slider_get_size(slider);
    uint16_t tag_size = _gt_slider_get_tag_size(slider);
    gt_size_t dist = style->offset;

    if( style->dir == GT_BAR_DIR_HOR_L2R || style->dir == GT_BAR_DIR_HOR_R2L ){
        base->x = slider->area.x + (tag_size >> 1);
        base->w = slider->area.w - (tag_size);
        base->y = slider->area.y + ((slider->area.h-size)>>1);
        base->h = size;

        tag->y = slider->area.y + ((slider->area.h-tag_size)>>1);

        gt_area_copy(val, base);
        val->w = dist;
        if(style->dir == GT_BAR_DIR_HOR_R2L){
            val->x += (base->w - dist);
            tag->x = slider->area.x + slider->area.w - dist - tag_size;
        }
        else{
            tag->x = slider->area.x + dist;
        }

    }else if( style->dir == GT_BAR_DIR_VER_U2D || style->dir == GT_BAR_DIR_VER_D2U ){
        base->y = slider->area.y + (tag_size >> 1);
        base->h = slider->area.h - (tag_size);
        base->x = slider->area.x + ((slider->area.w-size)>>1);
        base->w = size;

        tag->x = slider->area.x + ((slider->area.w-tag_size)>>1);

        gt_area_copy(val, base);
        val->h = dist;
        if (style->dir == GT_BAR_DIR_VER_D2U){
            val->y += (base->h - dist);
            tag->y = slider->area.y + slider->area.h - dist - tag_size;
        }
        else{
            tag->y = slider->area.y + dist;
        }
    }
}

static gt_area_st _calc_repeat_mode_area(gt_obj_st * slider, gt_area_st const * const val) {
    _gt_slider_st * style = (_gt_slider_st * )slider;
    gt_area_st repeat_area = *val;

    gt_size_t begin_pixel = _calc_pixel_offset_by_value(slider, style->repeat_info.begin);
    /** if stop have not set, using local pos */
    gt_size_t stop_pixel = _calc_pixel_offset_by_value(slider,
        style->repeat_info.stop ? style->repeat_info.stop : style->pos);

    if (begin_pixel > stop_pixel) {
        /** have no end remark point, do not display area */
        repeat_area.w = 0;
        repeat_area.h = 0;
        return repeat_area;
    }

    if (GT_BAR_DIR_HOR_L2R == style->dir) {
        repeat_area.x += begin_pixel;
        repeat_area.w = stop_pixel - begin_pixel;
    }
    else if (GT_BAR_DIR_HOR_R2L == style->dir) {
        repeat_area.x += (val->w - stop_pixel);
        repeat_area.w = stop_pixel - begin_pixel;
    }
    else if (GT_BAR_DIR_VER_U2D == style->dir) {
        repeat_area.y += begin_pixel;
        repeat_area.h = stop_pixel - begin_pixel;
    }
    else if (GT_BAR_DIR_VER_D2U == style->dir) {
        repeat_area.y += (val->h - stop_pixel);
        repeat_area.h = stop_pixel - begin_pixel;
    }

    return repeat_area;
}

/**
 * @brief obj init slider widget call back
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj) {
    _gt_slider_st * style = (_gt_slider_st * )obj;
    gt_size_t dist_min = 0;

    // set default size
    if( !obj->area.w || !obj->area.h ){
        obj->area.w = 100;
        obj->area.h = 16;
    };

    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.reg.is_fill    = 1;
    rect_attr.bg_opa         = obj->opa;
    rect_attr.bg_color       = style->color_ina;
    //
    gt_area_st area_base = {0}, area_val = {0}, tag_area = {0};

    rect_attr.radius = _gt_slider_get_size(obj) >> 1;
    _calc_all_layer_area(obj, &area_base, &area_val, &tag_area);
    if(style->pos < style->end){
        /** background area */
        draw_bg(obj->draw_ctx, &rect_attr, &area_base);
    }

    rect_attr.bg_color = style->color_act;
    dist_min = rect_attr.radius << 1;
    /* When style->offset = 1, area_val is plotted as a line */
    if(style->offset > 1){
        if(style->offset < dist_min){
            rect_attr.base_area = &area_base;
        }
        /** active area */
        draw_bg(obj->draw_ctx, &rect_attr, &area_val);

        if (style->repeat_mode) {
            gt_attr_rect_st repeat_attr = rect_attr;
            repeat_attr.bg_color = style->repeat_info.color;
            gt_area_st repeat_area = _calc_repeat_mode_area(obj, &area_val);
            /** AB repeat area */
            draw_bg(obj->draw_ctx, &repeat_attr, &repeat_area);
        }
        rect_attr.base_area = NULL;
    }

    /* visible or not */
    if( !style->tag_visible ){
        goto draw_focus;
    }

    uint16_t tag_size = _gt_slider_get_tag_size(obj);
    tag_area.w = tag_size;
    tag_area.h = tag_size;
    rect_attr.radius = tag_size >> 1;

    // draw img tag
    if(style->tag && style->img){
        uint16_t _w = 0,_h = 0;
        if( GT_FS_RES_OK == gt_fs_read_img_wh(gt_img_get_src(style->img), &_w, &_h) ) {
            if (0 == _w || 0 == _h) {
                goto draw_focus;
            }
            tag_area.w = _w > tag_size ? tag_size : _w;
            tag_area.h = _h > tag_size ? tag_size : _h;
            tag_area.x += (tag_size - tag_area.w) >> 1;
            tag_area.y += (tag_size - tag_area.h) >> 1;
        } else {
            goto draw_focus;
        }
        gt_obj_set_area(style->tag, tag_area);
        gt_obj_set_area(style->img , tag_area);
        goto draw_focus;
    } else {
        rect_attr.bg_color = style->repeat_mode ? style->repeat_info.color : style->color_act;
    }

    // draw tag
    draw_bg(obj->draw_ctx, &rect_attr, &tag_area);

draw_focus:
    // focus
    draw_focus(obj , 0);
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
            gt_disp_invalid_area(obj);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_PRESSING:   /* add clicking style and process clicking event */
            gt_event_send(obj, GT_EVENT_TYPE_INPUT_RELEASED, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_SCROLL:
            _gt_slider_set_offset(obj, _calc_touch_point_pos(obj));
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_RELEASED: /* click event finish */
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
    if (NULL == obj) {
        return obj;
    }
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
    obj->fixed = false;

    /** AB repeat mode setting */
    style->repeat_info.color = gt_color_hex(0xff6b81);

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
    if (false == gt_obj_is_type(slider, OBJ_TYPE)) {
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
    if (false == gt_obj_is_type(slider, OBJ_TYPE)) {
        return;
    }
    _gt_slider_st * style = (_gt_slider_st *)slider;
    style->start = start;
    style->end = end;
}

gt_size_t gt_slider_get_pos(gt_obj_st * slider)
{
    if (false == gt_obj_is_type(slider, OBJ_TYPE)) {
        return 0;
    }
    _gt_slider_st * style = (_gt_slider_st *)slider;
    return style->pos;
}

gt_size_t gt_slider_get_start(gt_obj_st * slider)
{
    if (false == gt_obj_is_type(slider, OBJ_TYPE)) {
        return 0;
    }
    _gt_slider_st * style = (_gt_slider_st *)slider;
    return style->start;
}

gt_size_t gt_slider_get_end(gt_obj_st * slider)
{
    if (false == gt_obj_is_type(slider, OBJ_TYPE)) {
        return 0;
    }
    _gt_slider_st * style = (_gt_slider_st *)slider;
    return style->end;
}

gt_size_t gt_slider_get_total(gt_obj_st * slider)
{
    if (false == gt_obj_is_type(slider, OBJ_TYPE)) {
        return 0;
    }
    _gt_slider_st * style = (_gt_slider_st *)slider;
    return ( style->end - style->start);
}

void gt_slider_set_color_act(gt_obj_st * slider, gt_color_t color)
{
    if (false == gt_obj_is_type(slider, OBJ_TYPE)) {
        return;
    }
    _gt_slider_st * style = (_gt_slider_st *)slider;
    style->color_act = color;
    gt_event_send(slider, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_slider_set_color_ina(gt_obj_st * slider, gt_color_t color)
{
    if (false == gt_obj_is_type(slider, OBJ_TYPE)) {
        return;
    }
    _gt_slider_st * style = (_gt_slider_st *)slider;
    style->color_ina = color;
    gt_event_send(slider, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_slider_set_dir(gt_obj_st * slider, gt_bar_dir_et dir)
{
    if (false == gt_obj_is_type(slider, OBJ_TYPE)) {
        return;
    }
    _gt_slider_st * style = (_gt_slider_st *)slider;

    style->dir = dir;
    // gt_event_send(slider, GT_EVENT_TYPE_DRAW_START, NULL);
    gt_slider_set_pos(slider, style->pos);
}

gt_bar_dir_et gt_slider_get_dir(gt_obj_st * slider)
{
    if (false == gt_obj_is_type(slider, OBJ_TYPE)) {
        return (gt_bar_dir_et)0;
    }
    _gt_slider_st * style = (_gt_slider_st *)slider;
    return style->dir;
}

uint16_t gt_slider_get_step(gt_obj_st * slider)
{
    if (false == gt_obj_is_type(slider, OBJ_TYPE)) {
        return 0;
    }
    _gt_slider_st * style = (_gt_slider_st *)slider;

    return style->step;
}

void gt_slider_set_step(gt_obj_st * slider, uint16_t step)
{
    if (false == gt_obj_is_type(slider, OBJ_TYPE)) {
        return;
    }
    _gt_slider_st * style = (_gt_slider_st *)slider;

    style->step = step;
}

void gt_slider_set_tag(gt_obj_st * slider, char * src)
{
    if (false == gt_obj_is_type(slider, OBJ_TYPE)) {
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
    if (false == gt_obj_is_type(slider, OBJ_TYPE)) {
        return;
    }
    _gt_slider_st * style = (_gt_slider_st *)slider;
    style->tag_visible = visible;
    gt_event_send(slider, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_slider_set_tag_size(gt_obj_st * slider , uint16_t size)
{
    if (false == gt_obj_is_type(slider, OBJ_TYPE)) {
        return;
    }
    _gt_slider_st * style = (_gt_slider_st * )slider;
    style->tag_size = size;
    gt_event_send(slider, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_slider_set_thickness(gt_obj_st * slider , uint16_t thickness)
{
    if (false == gt_obj_is_type(slider, OBJ_TYPE)) {
        return;
    }
    _gt_slider_st * style = (_gt_slider_st * )slider;
    style->thickness = thickness;
    gt_event_send(slider, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_slider_set_mode(gt_obj_st * slider , gt_slider_mode_et mode)
{
    if (false == gt_obj_is_type(slider, OBJ_TYPE)) {
        return;
    }
    _gt_slider_st * style = (_gt_slider_st * )slider;
    style->mode = mode;
    gt_event_send(slider, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_slider_set_AB_repeat_color(gt_obj_st * slider, gt_color_t color)
{
    if (false == gt_obj_is_type(slider, OBJ_TYPE)) {
        return;
    }
    _gt_slider_st * style = (_gt_slider_st * )slider;
    style->repeat_info.color = color;

    gt_event_send(slider, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_slider_set_AB_repeat_remark_point(gt_obj_st * slider)
{
    if (false == gt_obj_is_type(slider, OBJ_TYPE)) {
        return;
    }
    _gt_slider_st * style = (_gt_slider_st * )slider;
    if (GT_SLIDER_MODE_DEFAULT != style->mode) {
        _reset_repeat_mode(style);
        return;
    }

    if (false == style->repeat_mode) {
        style->repeat_mode = true;
    }

    if (style->repeat_info.stop && style->repeat_info.begin <= style->repeat_info.stop) {
        _reset_repeat_mode(style);
    } else if (0 == style->repeat_info.begin && 0 == style->repeat_info.stop) {
        style->repeat_info.begin = style->pos;
        style->repeat_info.stop = 0;
    } else {
        style->repeat_info.stop = style->pos;
    }
    gt_event_send(slider, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_slider_set_AB_repeat_start_by(gt_obj_st * slider, gt_size_t start_value)
{
    if (false == gt_obj_is_type(slider, OBJ_TYPE)) {
        return;
    }
    _gt_slider_st * style = (_gt_slider_st * )slider;
    // TODO
}

void gt_slider_set_AB_repeat_stop_by(gt_obj_st * slider, gt_size_t stop_value)
{
    if (false == gt_obj_is_type(slider, OBJ_TYPE)) {
        return;
    }
    _gt_slider_st * style = (_gt_slider_st * )slider;
    // TODO

}

void gt_slider_unset_AB_repeat(gt_obj_st * slider)
{
    if (false == gt_obj_is_type(slider, OBJ_TYPE)) {
        return;
    }
    _gt_slider_st * style = (_gt_slider_st * )slider;

    _reset_repeat_mode(style);
    gt_event_send(slider, GT_EVENT_TYPE_DRAW_START, NULL);
}


#endif  /** GT_CFG_ENABLE_SLIDER */
/* end ------------------------------------------------------------------*/
