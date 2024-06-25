/**
 * @file gt_btnmap.c
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2024-03-04 11:11:07
 * @copyright Copyright (c) 2014-2024, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_btnmap.h"

#if GT_CFG_ENABLE_BTNMAP
#include "../others/gt_log.h"
#include "../core/gt_mem.h"
#include "../core/gt_draw.h"
#include "../core/gt_obj_pos.h"
#include "../core/gt_disp.h"
#include "./gt_input.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_BTNMAP
#define MY_CLASS    &gt_btnmap_class

#define _DEFAULT_SPACE  5

/* private typedef ------------------------------------------------------*/
typedef struct _btnmap_btn_param_s {
    gt_color_t bg_color;
    gt_color_t border_color;
    gt_color_t font_color;
    gt_size_t border_width;
}_btnmap_btn_param_st;


typedef struct _gt_btnmap_s {
    gt_obj_st obj;
    gt_obj_st* _input;
    gt_map_st* map;
    gt_py_input_method_st* py_input_method;
    char* press_btn;

    uint16_t* _line_w_list;
    uint16_t* _line_numb_list;      /** array of line number @ref _max_line */

    gt_btnmap_disp_special_btn_cb_t _disp_special_btn_cb;
    gt_btnmap_push_btn_kv_cb_t _push_btn_kv_cb;

    gt_font_info_st font_info;

    _btnmap_btn_param_st btn_param;
    _btnmap_btn_param_st special_btn_param;

    uint16_t btn_height;
    uint16_t _max_line;
    uint16_t map_type;

    uint8_t btn_x_space;
    uint8_t btn_y_space;

    uint8_t auto_fill_h : 1;
}_gt_btnmap_st;


/* static prototypes ----------------------------------------------------*/
static void _init_cb(struct gt_obj_s *);
static void _deinit_cb(struct gt_obj_s *);
static void _event_cb(struct gt_obj_s *, struct _gt_event_s *);
static bool _gt_btn_cmp(const char* key1, const char* key2);
static void _gt_auto_btn_height(gt_obj_st* btnmap);
static uint16_t _gt_get_map_max_line(const gt_map_st *map);
static void _gt_get_line_list(const gt_map_st* map, uint16_t* w_list, uint16_t* numb_list);
static void _gt_update_map(gt_obj_st* btnmap);
static gt_area_st _gt_get_btn_area_idx(gt_obj_st* btnmap, uint16_t idx, uint16_t line, uint16_t col, uint16_t sum_w);
static void _gt_set_press_btn(gt_obj_st* obj);
static char* _gt_get_press_btn(gt_obj_st* obj);
static void _gt_clean_press_btn(gt_obj_st* obj);
static void _gt_push_btn_kv(gt_obj_st* obj);

/* static variables -----------------------------------------------------*/
const gt_obj_class_st gt_btnmap_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_btnmap_st)
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static void _init_cb(struct gt_obj_s * obj) {
    _gt_btnmap_st * style = (_gt_btnmap_st * )obj;

    _gt_auto_btn_height(obj);

    if(obj->area.w == 0 || obj->area.h == 0) return ;
    if(NULL == style->map || style->btn_height <= 0) return ;

    gt_attr_rect_st rect_attr = {0};
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.reg.is_fill = 1;
    rect_attr.bg_opa = obj->opa;
    rect_attr.bg_color = style->btn_param.bg_color;
    rect_attr.border_color = style->btn_param.border_color;
    rect_attr.border_width = style->btn_param.border_width;
    rect_attr.radius = obj->radius;
    //
    gt_font_st font = {0};
    font.info = style->font_info;
    font.res = NULL;
    gt_font_info_update_font_thick(&font.info);
    gt_attr_font_st font_attr = {
        .font = &font,
        .align = GT_ALIGN_CENTER_MID,
        .font_color = style->btn_param.font_color,
        .opa = obj->opa,
    };
    gt_map_st* map = style->map;
    gt_area_st btn_area;
    uint16_t map_idx = 0, line = 0, col = 0, sum_w = 0;
    uint8_t r, g, b;

    while (map[map_idx].kv || map[map_idx].w > 0) {
        rect_attr.bg_color = style->btn_param.bg_color;
        rect_attr.border_color = style->btn_param.border_color;
        rect_attr.border_width = style->btn_param.border_width;
        font_attr.font_color = style->btn_param.font_color;

        if(!map[map_idx].kv){
            goto g_NEXT;
        }

        if(_gt_btn_cmp(map[map_idx].kv, GT_BTNMAP_NEW_LINE)){
            line++;
            map_idx++;
            col = 0;
            sum_w = 0;
            continue;
        }

        font.utf8 = (char*)map[map_idx].kv;
        font.len = strlen(map[map_idx].kv);

        if(style->_disp_special_btn_cb){
            // Attribute change
            if(style->_disp_special_btn_cb(obj, map[map_idx].kv, &font_attr)){
                rect_attr.bg_color = style->special_btn_param.bg_color;
                rect_attr.border_color = style->special_btn_param.border_color;
                rect_attr.border_width = style->special_btn_param.border_width;
                font_attr.font_color = style->special_btn_param.font_color;
            }
        }

        if(_gt_get_press_btn(obj) && _gt_btn_cmp(map[map_idx].kv, _gt_get_press_btn(obj))){
            r = GT_COLOR_GET_R(rect_attr.bg_color) >> 1;
            g = GT_COLOR_GET_G(rect_attr.bg_color) >> 1;
            b = GT_COLOR_GET_B(rect_attr.bg_color) >> 1;
            GT_COLOR_SET_RGB(rect_attr.bg_color, r, g, b);
        }

        btn_area = _gt_get_btn_area_idx(obj, map_idx, line, col, sum_w);
        draw_bg(obj->draw_ctx, &rect_attr, &btn_area);
        // draw text
        font_attr.logical_area = btn_area;
        draw_text(obj->draw_ctx, &font_attr, &btn_area);

g_NEXT:
        sum_w += map[map_idx].w;
        col++;
        map_idx++;
    }
}

static void _deinit_cb(struct gt_obj_s *obj) {
    _gt_btnmap_st * style = (_gt_btnmap_st * )obj;
    if(style->_line_w_list){
        gt_mem_free(style->_line_w_list);
        style->_line_w_list = NULL;
    }

}

static void _event_cb(struct gt_obj_s *obj, struct _gt_event_s *e) {
    gt_event_type_et code = gt_event_get_code(e);

    switch (code) {
    case GT_EVENT_TYPE_DRAW_START:
        GT_LOGV(GT_LOG_TAG_GUI, "start draw");
        gt_disp_invalid_area(obj);
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
        break;

    case GT_EVENT_TYPE_DRAW_END:
        GT_LOGV(GT_LOG_TAG_GUI, "end draw");
        break;

    case GT_EVENT_TYPE_CHANGE_CHILD_REMOVE: /* remove child from screen but not delete */
        GT_LOGV(GT_LOG_TAG_GUI, "child remove");
        break;

    case GT_EVENT_TYPE_CHANGE_CHILD_DELETE: /* delete child */
        GT_LOGV(GT_LOG_TAG_GUI, "child delete");
        break;

    case GT_EVENT_TYPE_INPUT_PRESSING:   /* add clicking style and process clicking event */
        GT_LOGV(GT_LOG_TAG_GUI, "PRESSING");
        break;
    case GT_EVENT_TYPE_INPUT_PRESSED:
        GT_LOGV(GT_LOG_TAG_GUI, "PRESSED");
        _gt_set_press_btn(obj);
        break;
    case GT_EVENT_TYPE_DRAW_REDRAW: {
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
        break;
    }
    case GT_EVENT_TYPE_INPUT_SCROLL:
        GT_LOGV(GT_LOG_TAG_GUI, "scroll");
        _gt_clean_press_btn(obj);
        break;

    case GT_EVENT_TYPE_INPUT_PRESS_LOST:
    case GT_EVENT_TYPE_INPUT_RELEASED: /* click event finish */
        GT_LOGV(GT_LOG_TAG_GUI, "processed");
        _gt_push_btn_kv(obj);
        _gt_clean_press_btn(obj);
        break;

    default:
        break;
    }
}

static bool _gt_btn_cmp(const char* key1, const char* key2) {
    if(!key1 || !key2) return false;
    else if(!key1 && !key2) true;

    if(0 == strcmp(key1, key2)){
        return true;
    }
    return false;
}

static void _gt_auto_btn_height(gt_obj_st* btnmap) {
    _gt_btnmap_st * style = (_gt_btnmap_st * )btnmap;

    if(!style->auto_fill_h) {
        btnmap->area.h = style->btn_height * style->_max_line + style->btn_y_space * (style->_max_line + 1);
        return ;
    }

    style->btn_height = (btnmap->area.h - ((style->_max_line+1)*style->btn_y_space)) / style->_max_line;
}

static void _gt_update_map(gt_obj_st* btnmap) {
    _gt_btnmap_st * style = (_gt_btnmap_st * )btnmap;
    static uint16_t buf_max = 0;
    if(!style->map) return ;

    style->_max_line = _gt_get_map_max_line((const gt_map_st*)style->map);

    if(style->_max_line > buf_max && style->_line_w_list){
        gt_mem_free(style->_line_w_list);
        style->_line_w_list = NULL;
    }
    //
    if(!style->_line_w_list) {
        style->_line_w_list = gt_mem_malloc(style->_max_line * sizeof(uint16_t) * 2);
        if(NULL == style->_line_w_list) {
            GT_LOGE(GT_LOG_TAG_GUI, "btnmap malloc failed!!! size = %d", style->_max_line * sizeof(uint16_t) * 2);
            return ;
        }
        buf_max = style->_max_line;
    }
    gt_memset_0(style->_line_w_list, style->_max_line * sizeof(uint16_t) * 2);
    style->_line_numb_list = &style->_line_w_list[style->_max_line];

    _gt_get_line_list((const gt_map_st*)style->map, style->_line_w_list, style->_line_numb_list);

    _gt_auto_btn_height(btnmap);
}

static uint16_t _gt_get_map_max_line(const gt_map_st *map) {
    uint16_t max_line = 1;
    uint16_t map_idx = 0;

    while (map[map_idx].kv || map[map_idx].w > 0)
    {
        if(_gt_btn_cmp(map[map_idx].kv, GT_BTNMAP_NEW_LINE)){
            max_line++;
        }
        map_idx++;
    }

    return max_line;
}

static void _gt_get_line_list(const gt_map_st* map, uint16_t* w_list, uint16_t* numb_list) {
    if(!map || !w_list) return ;
    uint16_t map_idx = 0;
    uint16_t max_line = 0;

    while (map[map_idx].kv || map[map_idx].w > 0)
    {
        w_list[max_line] += map[map_idx].w;

        numb_list[max_line] += (map[map_idx].w ? 1 : 0);

        if(_gt_btn_cmp(map[map_idx].kv, GT_BTNMAP_NEW_LINE)){
            max_line++;
        }
        map_idx++;
    }
}

static gt_area_st _gt_get_btn_area_idx(gt_obj_st* btnmap, uint16_t idx, uint16_t line, uint16_t col, uint16_t sum_w) {
    gt_area_st btn_area = {0};
    _gt_btnmap_st * style = (_gt_btnmap_st * )btnmap;
    if(!style->_line_w_list || !style->_line_numb_list){
        return btn_area;
    }

    uint16_t line_numb = style->_line_numb_list[line];
    uint16_t line_w = style->_line_w_list[line];

    gt_area_copy(&btn_area, &btnmap->area);

    btn_area.y += style->btn_height * line + style->btn_y_space * (line+1);
    btn_area.h = style->btn_height;

    btn_area.x += sum_w * (btnmap->area.w - ((line_numb+1)*style->btn_x_space)) / line_w + style->btn_x_space * (col+1);

    btn_area.w = style->map[idx].w * (btnmap->area.w - ((line_numb+1)*style->btn_x_space)) / line_w;

    return btn_area;
}

static void _gt_set_press_btn(gt_obj_st* obj) {
    _gt_btnmap_st * style = (_gt_btnmap_st * )obj;
    const gt_map_st* map = (const gt_map_st*)style->map;

    const char* const press_old_btn = style->press_btn;
    uint16_t map_idx = 0, line = 0, col = 0, sum_w = 0;
    gt_area_st btn_area = {0}, area_tmp = {0};
    while (map[map_idx].kv || map[map_idx].w > 0)
    {
        if(!map[map_idx].kv){
            goto g_NEXT;
        }
        //
        if(_gt_btn_cmp(map[map_idx].kv, GT_BTNMAP_NEW_LINE)){
            line++;
            map_idx++;
            col = 0;
            sum_w = 0;
            continue;
        }
        btn_area = _gt_get_btn_area_idx(obj, map_idx, line, col, sum_w);

        if ((btn_area.x < obj->process_attr.point.x + obj->area.x) &&
            ((btn_area.x + btn_area.w) > obj->process_attr.point.x + obj->area.x) &&
            (btn_area.y < obj->process_attr.point.y + obj->area.y) &&
            ((btn_area.y + btn_area.h) > obj->process_attr.point.y + obj->area.y)) {

            style->press_btn = (char*)map[map_idx].kv;
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);

            // if(press_old_btn != style->press_btn){
            //     gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            // }

            // if(style->press_btn){
            //     _gt_disp_refr_append_area(&btn_area);
            // }
            return;
        }
g_NEXT:
        sum_w += map[map_idx].w;
        col++;
        map_idx++;
    }
    _gt_clean_press_btn(obj);
}

static char * _gt_get_press_btn(gt_obj_st* obj) {
    _gt_btnmap_st * style = (_gt_btnmap_st * )obj;
    return style->press_btn;
}

static void _gt_clean_press_btn(gt_obj_st* obj) {
    _gt_btnmap_st * style = (_gt_btnmap_st * )obj;
    style->press_btn = NULL;
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

static void _gt_push_btn_kv(gt_obj_st* obj) {
    _gt_btnmap_st * style = (_gt_btnmap_st * )obj;
    if(!style->press_btn) return ;

    if(style->_push_btn_kv_cb){
        style->_push_btn_kv_cb( obj, style->_input, style->press_btn);
    }
    else if(style->_input){
        gt_input_append_value(style->_input, style->press_btn);
    }
}

/* global functions / API interface -------------------------------------*/
/**
 * @brief create a btnmap obj
 *
 * @param parent btnmap's parent element
 * @return gt_obj_st* btnmap obj
 */
gt_obj_st * gt_btnmap_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    if (NULL == obj) {
        return obj;
    }
    obj->radius = 4;
    _gt_btnmap_st * style = (_gt_btnmap_st * )obj;
    style->btn_x_space = _DEFAULT_SPACE;
    style->btn_y_space = _DEFAULT_SPACE;
    gt_font_info_init(&style->font_info);

    _gt_update_map(obj);
    return obj;
}

void gt_btnmap_set_map(gt_obj_st* btnmap, gt_map_st* map, uint16_t map_type)
{
    if (false == gt_obj_is_type(btnmap, OBJ_TYPE)) {
        return;
    }
    _gt_btnmap_st * style = (_gt_btnmap_st * )btnmap;
    style->map = map;
    style->map_type = map_type;
    _gt_update_map(btnmap);

    gt_event_send(btnmap, GT_EVENT_TYPE_DRAW_START, NULL);
}

uint16_t gt_btnmap_get_map_type(gt_obj_st* btnmap)
{
    if (false == gt_obj_is_type(btnmap, OBJ_TYPE)) {
        return 0;
    }
    _gt_btnmap_st * style = (_gt_btnmap_st * )btnmap;
    return style->map_type;
}

void gt_btnmap_set_input(gt_obj_st * btnmap, gt_obj_st * input)
{
    if (false == gt_obj_is_type(btnmap, OBJ_TYPE)) {
        return;
    }
    if (NULL == input) {
        return;
    }
    _gt_btnmap_st * style = (_gt_btnmap_st * )btnmap;
    style->_input = input;
}

void gt_btnmap_set_radius(gt_obj_st* btnmap, uint8_t radius)
{
    if (false == gt_obj_is_type(btnmap, OBJ_TYPE)) {
        return;
    }
    btnmap->radius = radius;
    gt_event_send(btnmap, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_btnmap_set_btn_height(gt_obj_st* btnmap, uint16_t height)
{
    if (false == gt_obj_is_type(btnmap, OBJ_TYPE)) {
        return;
    }
    _gt_btnmap_st * style = (_gt_btnmap_st * )btnmap;
    style->btn_height = height;
    gt_event_send(btnmap, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_btnmap_set_btn_height_auto_fill(gt_obj_st* btnmap, bool auto_fill)
{
    if (false == gt_obj_is_type(btnmap, OBJ_TYPE)) {
        return;
    }
    _gt_btnmap_st * style = (_gt_btnmap_st * )btnmap;
    style->auto_fill_h = auto_fill;
    gt_event_send(btnmap, GT_EVENT_TYPE_DRAW_START, NULL);
}

bool gt_btnmap_get_btn_height_auto_fill(gt_obj_st* btnmap)
{
    if (false == gt_obj_is_type(btnmap, OBJ_TYPE)) {
        return false;
    }
    _gt_btnmap_st * style = (_gt_btnmap_st * )btnmap;
    return style->auto_fill_h;
}

void gt_btnmap_set_btn_xy_space(gt_obj_st* btnmap, uint8_t x_space, uint8_t y_space)
{
    if (false == gt_obj_is_type(btnmap, OBJ_TYPE)) {
        return;
    }
    _gt_btnmap_st * style = (_gt_btnmap_st * )btnmap;
    style->btn_x_space = x_space;
    style->btn_y_space = y_space;
    gt_event_send(btnmap, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_btnmap_set_color_background(gt_obj_st * btnmap, gt_color_t color)
{
    if (false == gt_obj_is_type(btnmap, OBJ_TYPE)) {
        return;
    }
    _gt_btnmap_st* style = (_gt_btnmap_st*)btnmap;
    style->btn_param.bg_color = color;
    gt_event_send(btnmap, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_btnmap_set_border_width_and_color(gt_obj_st * btnmap, gt_size_t width, gt_color_t color)
{
    if (false == gt_obj_is_type(btnmap, OBJ_TYPE)) {
        return;
    }
    _gt_btnmap_st* style = (_gt_btnmap_st*)btnmap;
    style->btn_param.border_width = width;
    style->btn_param.border_color = color;
    gt_event_send(btnmap, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_btnmap_set_font_color(gt_obj_st * btnmap, gt_color_t color)
{
    if (false == gt_obj_is_type(btnmap, OBJ_TYPE)) {
        return;
    }
    _gt_btnmap_st* style = (_gt_btnmap_st*)btnmap;
    style->btn_param.font_color = color;
    gt_event_send(btnmap, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_btnmap_set_font_size(gt_obj_st * btnmap, uint8_t size)
{
    if (false == gt_obj_is_type(btnmap, OBJ_TYPE)) {
        return;
    }
    _gt_btnmap_st * style = (_gt_btnmap_st * )btnmap;
    style->font_info.size = size;
    gt_event_send(btnmap, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_btnmap_set_font_gray(gt_obj_st * btnmap, uint8_t gray)
{
    if (false == gt_obj_is_type(btnmap, OBJ_TYPE)) {
        return;
    }
    _gt_btnmap_st * style = (_gt_btnmap_st * )btnmap;
    style->font_info.gray = gray;
    gt_event_send(btnmap, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_btnmap_set_font_family_cn(gt_obj_st * btnmap, gt_family_t family)
{
    if (false == gt_obj_is_type(btnmap, OBJ_TYPE)) {
        return;
    }
    _gt_btnmap_st * style = (_gt_btnmap_st * )btnmap;
    style->font_info.style_cn = family;
    gt_event_send(btnmap, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_btnmap_set_font_family_en(gt_obj_st * btnmap, gt_family_t family)
{
    if (false == gt_obj_is_type(btnmap, OBJ_TYPE)) {
        return;
    }
    _gt_btnmap_st * style = (_gt_btnmap_st * )btnmap;
    style->font_info.style_en = family;
    gt_event_send(btnmap, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_btnmap_set_font_family_fl(gt_obj_st * btnmap, gt_family_t family)
{
    if (false == gt_obj_is_type(btnmap, OBJ_TYPE)) {
        return;
    }
    _gt_btnmap_st * style = (_gt_btnmap_st * )btnmap;
    style->font_info.style_fl = family;
    gt_event_send(btnmap, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_btnmap_set_font_family_numb(gt_obj_st * btnmap, gt_family_t family)
{
    if (false == gt_obj_is_type(btnmap, OBJ_TYPE)) {
        return;
    }
    _gt_btnmap_st * style = (_gt_btnmap_st * )btnmap;
    style->font_info.style_numb = family;
    gt_event_send(btnmap, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_btnmap_set_font_thick_en(gt_obj_st * btnmap, uint8_t thick)
{
    if (false == gt_obj_is_type(btnmap, OBJ_TYPE)) {
        return;
    }
    _gt_btnmap_st * style = (_gt_btnmap_st * )btnmap;
    style->font_info.thick_en = thick;
    gt_event_send(btnmap, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_btnmap_set_font_thick_cn(gt_obj_st * btnmap, uint8_t thick)
{
    if (false == gt_obj_is_type(btnmap, OBJ_TYPE)) {
        return;
    }
    _gt_btnmap_st * style = (_gt_btnmap_st * )btnmap;
    style->font_info.thick_cn = thick;
    gt_event_send(btnmap, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_btnmap_set_font_encoding(gt_obj_st * btnmap, gt_encoding_et encoding)
{
    if (false == gt_obj_is_type(btnmap, OBJ_TYPE)) {
        return;
    }
    _gt_btnmap_st * style = (_gt_btnmap_st * )btnmap;
    style->font_info.encoding = encoding;
}

void gt_btnmap_set_push_btn_kv_handler(gt_obj_st * btnmap, gt_btnmap_push_btn_kv_cb_t push_btn_kv_cb)
{
    if (false == gt_obj_is_type(btnmap, OBJ_TYPE)) {
        return;
    }
    _gt_btnmap_st * style = (_gt_btnmap_st * )btnmap;
    style->_push_btn_kv_cb = push_btn_kv_cb;
}
//
void gt_btnmap_set_disp_special_btn_handler(gt_obj_st * btnmap, gt_btnmap_disp_special_btn_cb_t disp_special_btn_cb)
{
    if (false == gt_obj_is_type(btnmap, OBJ_TYPE)) {
        return;
    }
    _gt_btnmap_st * style = (_gt_btnmap_st * )btnmap;
    style->_disp_special_btn_cb = disp_special_btn_cb;
}
void gt_btnmap_set_special_btn_color_background(gt_obj_st * btnmap, gt_color_t color)
{
    if (false == gt_obj_is_type(btnmap, OBJ_TYPE)) {
        return;
    }
    _gt_btnmap_st* style = (_gt_btnmap_st*)btnmap;
    style->special_btn_param.bg_color = color;
    gt_event_send(btnmap, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_btnmap_set_special_btn_border_width_and_color(gt_obj_st * btnmap, gt_size_t width, gt_color_t color)
{
    if (false == gt_obj_is_type(btnmap, OBJ_TYPE)) {
        return;
    }
    _gt_btnmap_st* style = (_gt_btnmap_st*)btnmap;
    style->special_btn_param.border_width = width;
    style->special_btn_param.border_color = color;
    gt_event_send(btnmap, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_btnmap_set_special_btn_font_color(gt_obj_st * btnmap, gt_color_t color)
{
    if (false == gt_obj_is_type(btnmap, OBJ_TYPE)) {
        return;
    }
    _gt_btnmap_st* style = (_gt_btnmap_st*)btnmap;
    style->special_btn_param.font_color = color;
}

//
void gt_btnmap_set_py_input_method(gt_obj_st * btnmap, gt_py_input_method_st* py_input_method)
{
    if (false == gt_obj_is_type(btnmap, OBJ_TYPE)) {
        return;
    }
    _gt_btnmap_st* style = (_gt_btnmap_st*)btnmap;
    style->py_input_method = py_input_method;
}

gt_py_input_method_st* gt_btnmap_get_py_input_method(gt_obj_st * btnmap)
{
    if (false == gt_obj_is_type(btnmap, OBJ_TYPE)) {
        return NULL;
    }
    _gt_btnmap_st* style = (_gt_btnmap_st*)btnmap;
    return style->py_input_method;
}

uint16_t gt_btnmap_get_max_key_count_of_lines(gt_obj_st * btnmap)
{
    if (false == gt_obj_is_type(btnmap, OBJ_TYPE)) {
        return 0;
    }
    _gt_btnmap_st* style = (_gt_btnmap_st*)btnmap;
    uint16_t max_key_count = 0;
    for (uint16_t i = 0; i < style->_max_line; ++i) {
        if (style->_line_numb_list[i] > max_key_count) {
            max_key_count = style->_line_numb_list[i];
        }
    }
    return max_key_count;
}

uint16_t gt_btnmap_get_line_count(gt_obj_st * btnmap)
{
    if (false == gt_obj_is_type(btnmap, OBJ_TYPE)) {
        return 0;
    }
    _gt_btnmap_st* style = (_gt_btnmap_st*)btnmap;
    return style->_max_line;
}

uint16_t gt_btnmap_get_calc_minimum_width(gt_obj_st * btnmap)
{
    if (false == gt_obj_is_type(btnmap, OBJ_TYPE)) {
        return 0;
    }
    _gt_btnmap_st* style = (_gt_btnmap_st*)btnmap;
    return (6 + style->font_info.size) * gt_btnmap_get_max_key_count_of_lines(btnmap);
}

#endif  /** GT_CFG_ENABLE_BTNMAP */
/* end of file ----------------------------------------------------------*/


