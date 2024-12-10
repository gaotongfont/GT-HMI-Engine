/**
 * @file gt_arc.c
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2024-11-14 14:51:51
 * @copyright Copyright (c) 2014-2024, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "./gt_arc.h"

#if GT_CFG_ENABLE_ARC
#include "../core/gt_mem.h"
#include "../hal/gt_hal_disp.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"
#include "../core/gt_event.h"
#include "../core/gt_fs.h"
#include "../others/gt_area.h"
#include "../others/gt_anim.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_ARC
#define MY_CLASS    &gt_arc_class


/* private typedef ------------------------------------------------------*/
typedef struct {
    char * src;             // img src, It can be a virt file name/path or an array of C
#if GT_USE_FILE_HEADER
    gt_file_header_param_st fh;
#endif
#if GT_USE_DIRECT_ADDR
    gt_addr_t addr;
#endif
#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
    gt_direct_addr_custom_size_st custom_addr;
#endif
}_gt_img_element_st;

typedef struct {

    _gt_img_element_st img;

    uint16_t start_angle;
    uint16_t end_angle;
    uint16_t width;

}_gt_arc_element_st;

typedef struct _gt_arc_s {
    gt_obj_st obj;

    gt_anim_st anim;

    _gt_arc_element_st bg;
    _gt_arc_element_st act;
    _gt_img_element_st knob_img;

    gt_color_t act_color;
    gt_color_t knob_color;

    uint16_t knob_width;

    int16_t value;
    int16_t value_min;
    int16_t value_max;
    int16_t value_step;

    uint8_t rounded : 1;
    uint8_t anim_state: 1;
} _gt_arc_st;


/* static prototypes ----------------------------------------------------*/



/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

static GT_ATTRIBUTE_RAM_DATA const gt_obj_class_st gt_arc_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_arc_st)
};



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj)
{
    _gt_arc_st * style = (_gt_arc_st * )obj;

    gt_attr_arc_st arc_attr;
    gt_memset_0(&arc_attr, sizeof(gt_attr_arc_st));
    arc_attr.color = obj->bgcolor;
    arc_attr.opa = obj->opa;
    arc_attr.rounded = style->rounded;
    // bg
    arc_attr.start_angle = style->bg.start_angle;
    arc_attr.end_angle = style->bg.end_angle;
    arc_attr.width = style->bg.width;
    arc_attr.img = style->bg.img.src;
#if GT_USE_FILE_HEADER
    arc_attr.file_header = gt_file_header_param_check_valid(&style->bg.img.fh);
#endif
#if GT_USE_DIRECT_ADDR
    arc_attr.addr = style->bg.img.addr;
#endif
#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
    arc_attr.custom_addr = &style->bg.img.custom_addr;
#endif

    gt_area_st area;
    uint16_t max_w = GT_MAX(style->knob_width, GT_MAX(style->bg.width, style->act.width));

    uint8_t reduce = (max_w - style->bg.width) >> 1;
    area = gt_area_reduce(obj->area, reduce);
    gt_draw_arc(obj->draw_ctx, &arc_attr, &area);

    // act
    arc_attr.color = style->act_color;
    arc_attr.start_angle = style->act.start_angle;
    arc_attr.end_angle = style->act.end_angle;
    arc_attr.width = style->act.width;
    arc_attr.img = style->act.img.src;
#if GT_USE_FILE_HEADER
    arc_attr.file_header = gt_file_header_param_check_valid(&style->act.img.fh);
#endif
#if GT_USE_DIRECT_ADDR
    arc_attr.addr = style->act.img.addr;
#endif
#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
    arc_attr.custom_addr = &style->act.img.custom_addr;
#endif

    reduce = (max_w - style->act.width) >> 1;
    area = gt_area_reduce(obj->area, reduce);
    gt_draw_arc(obj->draw_ctx, &arc_attr, &area);

    // knob
    arc_attr.color = style->knob_color;
    arc_attr.start_angle = 0;
    arc_attr.end_angle = 360;
    arc_attr.width = 0xFFFF;
    arc_attr.img = style->knob_img.src;
#if GT_USE_FILE_HEADER
    arc_attr.file_header = gt_file_header_param_check_valid(&style->knob_img.fh);
#endif
#if GT_USE_DIRECT_ADDR
    arc_attr.addr = style->knob_img.addr;
#endif
#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
    arc_attr.custom_addr = &style->knob_img.custom_addr;
#endif


    reduce = (max_w - style->knob_width) >> 1;
    area = gt_area_reduce(obj->area, reduce);
    uint16_t radius = GT_MIN(area.w, area.h) >> 1;
    gt_point_st center;
    center.x = area.x + radius;
    center.y = area.y + radius;
    uint16_t angle = style->act.end_angle;

    while(angle > 360) { angle -= 360; }

    gt_rounded_area_get(angle, radius, style->knob_width, &area);
    area.x += center.x;
    area.y += center.y;

    arc_attr.rounded = false;
    gt_draw_arc(obj->draw_ctx, &arc_attr, &area);

    //
    draw_focus(obj, 0);

}
static void _deinit_cb(gt_obj_st * obj)
{
    GT_CHECK_BACK(obj);

    _gt_arc_st * style_p = (_gt_arc_st * )obj;
    if (NULL != style_p->bg.img.src) {
        gt_mem_free(style_p->bg.img.src);
        style_p->bg.img.src = NULL;
    }

    if(NULL != style_p->act.img.src) {
        gt_mem_free(style_p->act.img.src);
        style_p->act.img.src = NULL;
    }

    if(NULL != style_p->knob_img.src) {
        gt_mem_free(style_p->knob_img.src);
        style_p->knob_img.src = NULL;
    }

}

static void _knob_scroll_headler(gt_obj_st * obj, bool flag)
{
    _gt_arc_st * style = (_gt_arc_st *)obj;

    if(style->anim_state) {
        return ;
    }

    gt_point_st point = obj->process_attr.point;
    uint16_t radius = GT_MIN(obj->area.w, obj->area.h) >> 1;
    gt_point_st center;
    center.x = radius;
    center.y = radius;
    point.x -= center.x;
    point.y -= center.y;

    uint16_t end_angle = (style->bg.start_angle < style->bg.end_angle) ? style->bg.end_angle : (style->bg.end_angle + 360);
    int16_t angle = gt_atan2(point.y, point.x);

    angle -= style->bg.start_angle;
    while(angle < 0) { angle += 360; }
    angle += style->bg.start_angle;

    if(angle < style->bg.start_angle || angle > end_angle) {
        return;
    }

    // ? movement Angle check
    int16_t diff_angle = gt_abs(angle - style->act.end_angle);
    if(!flag && diff_angle > 280){
        return ;
    }

    // get value
    int16_t value = gt_map(angle, style->bg.start_angle, end_angle, style->value_min, style->value_max);

    if(diff_angle > 90) {
        gt_anim_set_time(&style->anim, 500);
        gt_anim_set_value(&style->anim, style->value, value);
        gt_anim_start(&style->anim);
    }
    else{
        gt_arc_set_value(obj, value);
    }

}

static void _event_cb(struct gt_obj_s * obj, gt_event_st * e)
{
    gt_event_type_et code_val = gt_event_get_code(e);

    switch(code_val) {
        case GT_EVENT_TYPE_DRAW_START:
            gt_disp_invalid_area(obj);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;

        case GT_EVENT_TYPE_DRAW_REDRAW:
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_PRESSED:
            _knob_scroll_headler(obj, true);
            break;
        case GT_EVENT_TYPE_INPUT_SCROLL:
        case GT_EVENT_TYPE_INPUT_PRESSING:
            _knob_scroll_headler(obj, false);
            break;
        default:
            break;
    }
}

static void _updata_value(gt_obj_st* obj)
{
    _gt_arc_st * style = (_gt_arc_st *)obj;
    uint16_t end_angle = (style->bg.start_angle < style->bg.end_angle) ? style->bg.end_angle : (style->bg.end_angle + 360);

    int16_t angle = gt_map(style->value, style->value_min, style->value_max, style->bg.start_angle, end_angle);

    gt_arc_set_act_start_end_angle(obj, style->bg.start_angle, angle);
}

static GT_ATTRIBUTE_RAM_TEXT void _arc_anim_exec_cb(void * obj, int32_t count)
{
    gt_arc_set_value(obj, count);
}

static GT_ATTRIBUTE_RAM_TEXT void _arc_anim_start_cb(struct gt_anim_s * anim)
{
    _gt_arc_st * style = (_gt_arc_st * )anim->tar;
    style->anim_state = true;

}
static GT_ATTRIBUTE_RAM_TEXT void _arc_anim_ready_cb(struct gt_anim_s * anim)
{
    _gt_arc_st * style = (_gt_arc_st * )anim->tar;
    style->anim_state = false;
}

/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_arc_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    GT_CHECK_BACK_VAL(obj, NULL);

    obj->fixed = false;
    _gt_arc_st * style = (_gt_arc_st *)obj;

    gt_anim_init(&style->anim);
    gt_anim_set_target(&style->anim, obj);
    gt_anim_set_exec_cb(&style->anim, _arc_anim_exec_cb);
    gt_anim_set_start_cb(&style->anim, _arc_anim_start_cb);
    gt_anim_set_ready_cb(&style->anim, _arc_anim_ready_cb);

#if GT_USE_FILE_HEADER
    gt_file_header_param_init(&style->bg.img.fh);
    gt_file_header_param_init(&style->act.img.fh);
    gt_file_header_param_init(&style->knob_img.fh);
#endif

#if GT_USE_DIRECT_ADDR
    gt_hal_direct_addr_init(&style->bg.img.addr);
    gt_hal_direct_addr_init(&style->act.img.addr);
    gt_hal_direct_addr_init(&style->knob_img.addr);
#endif

#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
    gt_hal_custom_size_addr_init(&style->bg.img.custom_addr);
    gt_hal_custom_size_addr_init(&style->act.img.custom_addr);
    gt_hal_custom_size_addr_init(&style->knob_img.custom_addr);
#endif

    return obj;
}

void gt_arc_set_bg_img(gt_obj_st * obj, char * src)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    if (NULL == src) {
        return ;
    }
    _gt_arc_st * style = (_gt_arc_st *)obj;
    if (NULL != style->bg.img.src && !strcmp(src, style->bg.img.src)) {
        return ;
    }

    uint16_t len = src == NULL ? 0 : strlen(src);
    style->bg.img.src = style->bg.img.src ? gt_mem_realloc(style->bg.img.src, len + 1) : gt_mem_malloc(len + 1);
    if (NULL == style->bg.img.src) {
        return ;
    }
    gt_memcpy(style->bg.img.src, src, len);
    style->bg.img.src[len] = '\0';

#if GT_USE_FILE_HEADER
    gt_file_header_param_init(&style->bg.img.fh);
#endif

#if GT_USE_DIRECT_ADDR
    gt_hal_direct_addr_init(&style->bg.img.addr);
#endif

#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
    gt_hal_custom_size_addr_init(&style->bg.img.custom_addr);
#endif

    gt_area_st area;
    gt_area_copy(&area, &obj->area);
    if(GT_FS_RES_OK == gt_fs_read_img_wh(style->bg.img.src, &area.w, &area.h)){
        if(area.w != 0 && area.h != 0){
            gt_obj_size_change(obj, &area);
        }
    }

    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_arc_set_act_img(gt_obj_st * obj, char * src)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    if (NULL == src) {
        return ;
    }
    _gt_arc_st * style = (_gt_arc_st *)obj;
    if (NULL != style->act.img.src && !strcmp(src, style->act.img.src)) {
        return ;
    }

    uint16_t len = src == NULL ? 0 : strlen(src);
    style->act.img.src = style->act.img.src ? gt_mem_realloc(style->act.img.src, len + 1) : gt_mem_malloc(len + 1);
    if (NULL == style->act.img.src) {
        return ;
    }
    gt_memcpy(style->act.img.src, src, len);
    style->act.img.src[len] = '\0';

#if GT_USE_FILE_HEADER
    gt_file_header_param_init(&style->act.img.fh);
#endif

#if GT_USE_DIRECT_ADDR
    gt_hal_direct_addr_init(&style->act.img.addr);
#endif

#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
    gt_hal_custom_size_addr_init(&style->act.img.custom_addr);
#endif

    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

#if GT_USE_FILE_HEADER
void gt_arc_set_bg_by_file_header(gt_obj_st * obj, gt_file_header_param_st * fh)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    if (NULL == fh) {
        return ;
    }
    _gt_arc_st * style = (_gt_arc_st *)obj;
    style->bg.img.fh = *fh;

    if (NULL != style->bg.img.src) {
        gt_mem_free(style->bg.img.src);
        style->bg.img.src = NULL;
    }

#if GT_USE_DIRECT_ADDR
    gt_hal_direct_addr_init(&style->bg.img.addr);
#endif
#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
    gt_hal_custom_size_addr_init(&style->bg.img.custom_addr);
#endif

    gt_area_st area;
    gt_area_copy(&area, &obj->area);
    if(GT_RES_OK == gt_fs_fh_read_img_wh(&style->bg.img.fh, &area.w, &area.h)){
        if(area.w != 0 && area.h != 0){
            gt_obj_size_change(obj, &area);
        }
    }

    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_arc_set_act_by_file_header(gt_obj_st * obj, gt_file_header_param_st * fh)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    if (NULL == fh) {
        return ;
    }
    _gt_arc_st * style = (_gt_arc_st *)obj;
    style->act.img.fh = *fh;

    if (NULL != style->act.img.src) {
        gt_mem_free(style->act.img.src);
        style->act.img.src = NULL;
    }

#if GT_USE_DIRECT_ADDR
    gt_hal_direct_addr_init(&style->act.img.addr);
#endif
#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
    gt_hal_custom_size_addr_init(&style->act.img.custom_addr);
#endif

    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_arc_set_knob_by_file_header(gt_obj_st * obj, gt_file_header_param_st * fh)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    if (NULL == fh) {
        return ;
    }
    _gt_arc_st * style = (_gt_arc_st *)obj;
    style->knob_img.fh = *fh;

    if (NULL != style->knob_img.src) {
        gt_mem_free(style->knob_img.src);
        style->knob_img.src = NULL;
    }

#if GT_USE_DIRECT_ADDR
    gt_hal_direct_addr_init(&style->knob_img.addr);
#endif
#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
    gt_hal_custom_size_addr_init(&style->knob_img.custom_addr);
#endif

    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

#endif

#if GT_USE_DIRECT_ADDR
void gt_arc_set_bg_by_direct_addr(gt_obj_st * obj, gt_addr_t addr)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_arc_st * style = (_gt_arc_st *)obj;
    style->bg.img.addr = addr;

    if (NULL != style->bg.img.src) {
        gt_mem_free(style->bg.img.src);
        style->bg.img.src = NULL;
    }

#if GT_USE_FILE_HEADER
    gt_file_header_param_init(&style->bg.img.fh);
#endif

#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
    gt_hal_custom_size_addr_init(&style->bg.img.custom_addr);
#endif

    gt_area_st area;
    gt_area_copy(&area, &obj->area);
    if(GT_RES_OK == gt_fs_direct_addr_read_img_wh(style->bg.img.addr, &area.w, &area.h)){
        if(area.w != 0 && area.h != 0){
            gt_obj_size_change(obj, &area);
        }
    }

    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_arc_set_act_by_direct_addr(gt_obj_st * obj, gt_addr_t addr)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_arc_st * style = (_gt_arc_st *)obj;
    style->act.img.addr = addr;

    if (NULL != style->act.img.src) {
        gt_mem_free(style->act.img.src);
        style->act.img.src = NULL;
    }

#if GT_USE_FILE_HEADER
    gt_file_header_param_init(&style->act.img.fh);
#endif

#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
    gt_hal_custom_size_addr_init(&style->act.img.custom_addr);
#endif

    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_arc_set_knob_by_direct_addr(gt_obj_st * obj, gt_addr_t addr)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_arc_st * style = (_gt_arc_st *)obj;
    style->knob_img.addr = addr;

    if (NULL != style->knob_img.src) {
        gt_mem_free(style->knob_img.src);
        style->knob_img.src = NULL;
    }

#if GT_USE_FILE_HEADER
    gt_file_header_param_init(&style->knob_img.fh);
#endif

#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
    gt_hal_custom_size_addr_init(&style->knob_img.custom_addr);
#endif

    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}
#endif

#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
void gt_arc_set_bg_by_custom_size_addr(gt_obj_st * obj, gt_direct_addr_custom_size_st * dac)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_arc_st * style = (_gt_arc_st *)obj;
    style->bg.img.custom_addr = *dac;

    if (NULL != style->bg.img.src) {
        gt_mem_free(style->bg.img.src);
        style->bg.img.src = NULL;
    }

#if GT_USE_FILE_HEADER
    gt_file_header_param_init(&style->bg.img.fh);
#endif

#if GT_USE_DIRECT_ADDR
    gt_hal_direct_addr_init(&style->bg.img.addr);
#endif

    gt_area_st area;
    gt_area_copy(&area, &obj->area);
    area.w = style->bg.img.custom_addr.w;
    area.h = style->bg.img.custom_addr.h;
    if(area.w != 0 && area.h != 0){
        gt_obj_size_change(obj, &area);
    }
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_arc_set_act_by_custom_size_addr(gt_obj_st * obj, gt_direct_addr_custom_size_st * dac)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_arc_st * style = (_gt_arc_st *)obj;
    style->act.img.custom_addr = *dac;

    if (NULL != style->act.img.src) {
        gt_mem_free(style->act.img.src);
        style->act.img.src = NULL;
    }

#if GT_USE_FILE_HEADER
    gt_file_header_param_init(&style->act.img.fh);
#endif

#if GT_USE_DIRECT_ADDR
    gt_hal_direct_addr_init(&style->act.img.addr);
#endif

    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_arc_set_knob_by_custom_size_addr(gt_obj_st * obj, gt_direct_addr_custom_size_st * dac)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_arc_st * style = (_gt_arc_st *)obj;
    style->knob_img.custom_addr = *dac;

    if (NULL != style->knob_img.src) {
        gt_mem_free(style->knob_img.src);
        style->knob_img.src = NULL;
    }

#if GT_USE_FILE_HEADER
    gt_file_header_param_init(&style->knob_img.fh);
#endif

#if GT_USE_DIRECT_ADDR
    gt_hal_direct_addr_init(&style->knob_img.addr);
#endif

    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}
#endif

void gt_arc_set_bg_start_angle(gt_obj_st* obj, uint16_t angle)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_arc_st * style = (_gt_arc_st * )obj;
    while(angle > 360) { angle -= 360; }

    if(style->bg.start_angle == angle) {
        return ;
    }

    style->bg.start_angle = angle;
    _updata_value(obj);
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_arc_set_bg_end_angle(gt_obj_st* obj, uint16_t angle)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_arc_st * style = (_gt_arc_st * )obj;

    if(style->bg.end_angle == angle) {
        return ;
    }

    style->bg.end_angle = angle;
    _updata_value(obj);
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_arc_set_bg_start_end_angle(gt_obj_st* obj, uint16_t start_angle, uint16_t end_angle)
{
    gt_arc_set_bg_start_angle(obj, start_angle);
    gt_arc_set_bg_end_angle(obj, end_angle);
}

void gt_arc_set_act_start_angle(gt_obj_st* obj, uint16_t angle)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_arc_st * style = (_gt_arc_st * )obj;
    while(angle > 360) { angle -= 360; }

    if(style->act.start_angle == angle) {
        return ;
    }

    style->act.start_angle = angle;
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_arc_set_act_end_angle(gt_obj_st* obj, uint16_t angle)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_arc_st * style = (_gt_arc_st * )obj;

    if(style->act.end_angle == angle) {
        return ;
    }

    style->act.end_angle = angle;
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_arc_set_act_start_end_angle(gt_obj_st* obj, uint16_t start_angle, uint16_t end_angle)
{
    gt_arc_set_act_start_angle(obj, start_angle);
    gt_arc_set_act_end_angle(obj, end_angle);
}

void gt_arc_set_bg_width(gt_obj_st* obj, uint16_t width)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_arc_st * style = (_gt_arc_st * )obj;

    if(style->bg.width == width) {
        return ;
    }

    style->bg.width = width;
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_arc_set_act_width(gt_obj_st* obj, uint16_t width)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_arc_st * style = (_gt_arc_st * )obj;

    if(style->act.width == width) {
        return ;
    }

    style->act.width = width;
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_arc_set_bg_color(gt_obj_st* obj, gt_color_t color)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    gt_obj_set_bgcolor(obj, color);
}

void gt_arc_set_act_color(gt_obj_st* obj, gt_color_t color)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_arc_st * style = (_gt_arc_st * )obj;

    style->act_color = color;
}

void gt_arc_set_value(gt_obj_st* obj, int16_t value)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_arc_st * style = (_gt_arc_st * )obj;
    if(style->value == value) {
        return ;
    }

    int16_t new_value = 0;

    if(style->value_max < style->value_min){
        new_value = value > style->value_min ? style->value_min : value;
        new_value = new_value < style->value_max ? style->value_max : new_value;
    }
    else{
        new_value = value > style->value_max ? style->value_max : value;
        new_value = new_value < style->value_min ? style->value_min : new_value;
    }

    if(new_value == style->value) {
        return ;
    }

    style->value = new_value;
    _updata_value(obj);
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_arc_set_value_min_max(gt_obj_st* obj, int16_t min, int16_t max)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_arc_st * style = (_gt_arc_st * )obj;
    if(min == style->value_min && max == style->value_max) {
        return ;
    }

    style->value_min = min;
    style->value_max = max;

    if(max < min){
        if(style->value < max){
            style->value = max;
        }

        if(style->value > min){
            style->value = min;
        }
    }
    else{
        if(style->value > max){
            style->value = max;
        }

        if(style->value < min){
            style->value = min;
        }
    }



    _updata_value(obj);
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_arc_set_value_step(gt_obj_st* obj, int16_t step)
{
    if( false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_arc_st * style = (_gt_arc_st * )obj;
    style->value_step = step;
}

void gt_arc_set_value_increase(gt_obj_st* obj)
{
    if( false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_arc_st * style = (_gt_arc_st * )obj;
    gt_arc_set_value(obj, style->value + style->value_step);
}

void gt_arc_set_value_decrease(gt_obj_st* obj)
{
    if( false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_arc_st * style = (_gt_arc_st * )obj;
    gt_arc_set_value(obj, style->value - style->value_step);
}

void gt_arc_set_rounded(gt_obj_st* obj, bool rounded)
{
    if( false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_arc_st * style = (_gt_arc_st * )obj;
    style->rounded = rounded;
}


void gt_arc_set_knob_color(gt_obj_st* obj, gt_color_t color)
{
    if( false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_arc_st * style = (_gt_arc_st * )obj;
    style->knob_color = color;
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_arc_set_knob_width(gt_obj_st* obj, uint16_t width)
{
    if( false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_arc_st * style = (_gt_arc_st * )obj;
    style->knob_width = width;
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

/* end of file ----------------------------------------------------------*/
#endif /* GT_CFG_ENABLE_BTN */

