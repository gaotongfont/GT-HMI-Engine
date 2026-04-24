/**
 * @file gt_formula.c
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2026-03-24 15:00:37
 * @copyright Copyright (c) 2014-2026, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_formula.h"
#if GT_CFG_ENABLE_FORMULA
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

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_FORMULA
#define MY_CLASS    &gt_formula_class


/* private typedef ------------------------------------------------------*/
typedef struct _gt_formula_s {
    gt_obj_st obj;
    char * text;
    gt_formula_font_st* font_cfg;
    gt_color_t font_color;

    gt_latex_off_t latex_off;

    uint16_t max_width;
    uint16_t text_len;
}_gt_formula_st;


/* static prototypes ----------------------------------------------------*/
GT_ATTRIBUTE_RAM_TEXT
static void _formula_init_cb(gt_obj_st * obj);
GT_ATTRIBUTE_RAM_TEXT
static void _formula_deinit_cb(gt_obj_st * obj);
GT_ATTRIBUTE_RAM_TEXT
static void _formula_event_cb(struct gt_obj_s * obj, gt_event_st * e);


/* static variables -----------------------------------------------------*/
static GT_ATTRIBUTE_RAM_DATA const gt_obj_class_st gt_formula_class = {
    ._init_cb      = _formula_init_cb,
    ._deinit_cb    = _formula_deinit_cb,
    ._event_cb     = _formula_event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_formula_st)
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
GT_ATTRIBUTE_RAM_TEXT
static void _formula_set_font_cfg(gt_obj_st * obj)
{
    _gt_formula_st* style = (_gt_formula_st*)obj;

    if(NULL == style->font_cfg) {
        return ;
    }

    gt_latex_font_set_temp_user_data((void*)style->font_cfg->info);
    gt_latex_font_set_temp_mode(style->font_cfg->font_mode);

    for(int i = 0; i < GT_FORMULA_FONT_MAX_LEVEL; i++) {
        gt_latex_font_set_temp_level(i, style->font_cfg->info[i].size, style->font_cfg->info[i].family);
    }
}

GT_ATTRIBUTE_RAM_TEXT
static void _formula_clear_font_cfg(gt_obj_st * obj)
{
    _gt_formula_st* style = (_gt_formula_st*)obj;

    if(NULL == style->font_cfg) {
        return ;
    }

    gt_latex_font_clear_temp_user_data();
    gt_latex_font_clear_temp();
}

GT_ATTRIBUTE_RAM_TEXT
static void _formula_bg_show(gt_obj_st * obj) {
    _gt_formula_st* style = (_gt_formula_st*)obj;

    if (obj->show_bg) {
        gt_attr_rect_st rect_attr;
        gt_graph_init_rect_attr(&rect_attr);
        rect_attr.radius = obj->radius;
        rect_attr.border_width = 0;
        rect_attr.reg.is_fill = 1;
        rect_attr.bg_color = obj->bgcolor;
        rect_attr.bg_opa = obj->opa;
        draw_bg(obj->draw_ctx, &rect_attr, &obj->area);
    }
}

static gt_font_info_st _formula_get_font_info(const gt_latex_font_level_t *level_cfg, const void* user_data)
{
    gt_font_info_st info = {0};
    gt_font_info_init(&info);

    if(user_data) {
        gt_font_info_st* font_info_buf = (gt_font_info_st*)user_data;
        for(int i = 0; i < GT_FORMULA_FONT_MAX_LEVEL; i++) {
            if(level_cfg->size == font_info_buf[i].size && level_cfg->font_type == font_info_buf[i].family) {
                gt_memcpy(&info, &font_info_buf[i], sizeof(gt_font_info_st));
                break;
            }
        }
    }
    else{
        gt_font_set_family(&info, level_cfg->font_type);
    }
    return info;
}
GT_ATTRIBUTE_RAM_TEXT
static int _formula_get_metrics_cb(uint32_t code,
                                const gt_latex_font_level_t *level_cfg,
                                gt_latex_glyph_metrics_t    *metrics,
                                void* user_data)
{
    if (NULL == level_cfg || NULL == metrics) { return -1; }
    gt_font_info_st info = _formula_get_font_info(level_cfg, user_data);
    gt_font_metrics_st m = gt_font_get_metrics_by_info(code, &info);
    metrics->font_w = m.font_w;
    metrics->height = m.font_h;
    metrics->axis = (m.font_h >> 1);
    return 0;
}
GT_ATTRIBUTE_RAM_TEXT
static void _formula_draw_glyph_cb(int16_t x, int16_t y,
                                uint32_t code,
                                const gt_latex_font_level_t *level_cfg,
                                uint32_t color, void *user_data)
{
    if (NULL == level_cfg || NULL == user_data) { return; }
    gt_obj_st* obj = (gt_obj_st*)user_data;
    _gt_formula_st* style = (_gt_formula_st*)obj;
    char utf8[8] = {0};

    gt_font_st font = {
        .res = NULL,
        .utf8 = utf8,
        .len = 0,
    };
    font.info = _formula_get_font_info(level_cfg, gt_latex_font_get_user_data());
    font.len = gt_unicode_to_utf8((uint8_t*)font.utf8, code);
    font.utf8[font.len] = '\0';
    gt_font_info_update_font_thick(&font.info);

    gt_attr_font_st font_attr = {
        .font       = &font,
        .font_color = style->font_color,
        .space_x    = 0,
        .space_y    = 0,
        .align      = GT_ALIGN_LEFT,
        .opa        = obj->opa,
        .reg        = 0,
        .reg.enabled_start = true,
        .reg.single_line = true,
        .logical_area = obj->area,
        .indent     = 0,
        .start_x    = x,
        .start_y    = y,
    };
    draw_text(obj->draw_ctx, &font_attr, &obj->area);
}

GT_ATTRIBUTE_RAM_TEXT
static void _formula_draw_line(gt_obj_st* obj, gt_point_st start, gt_point_st end, uint8_t thickness)
{
    _gt_formula_st* style = (_gt_formula_st*)obj;
    gt_attr_line_st line_attr;
    line_attr.line.opa = obj->opa;
    line_attr.line.color = style->font_color;
    line_attr.line.brush = GT_BRUSH_TYPE_ROUND;
    line_attr.line.type =  GT_GRAPHS_TYPE_LINE;
    line_attr.line.width = thickness;

    line_attr.start.x = start.x;
    line_attr.start.y = start.y;

    line_attr.end.x = end.x;
    line_attr.end.y = end.y;
    gt_draw_line(obj->draw_ctx, &line_attr, &obj->area);
}

GT_ATTRIBUTE_RAM_TEXT
static void _formula_draw_hline_cb(int16_t x, int16_t y,
                                uint16_t w, uint8_t thickness,
                                uint32_t color, void *user_data)
{
    if (NULL == user_data || 0 == w || 0 == thickness) { return; }
    gt_obj_st* obj = (gt_obj_st*)user_data;

    gt_point_st start = {0};
    gt_point_st end = {0};

    start.x = x;
    start.y = y;

    end.x = x + w;
    end.y = y;

    _formula_draw_line(obj, start, end, thickness);
}

GT_ATTRIBUTE_RAM_TEXT
static void _formula_draw_line_cb(int16_t x0, int16_t y0,
                            int16_t x1, int16_t y1,
                            uint8_t thickness,
                            uint32_t color, void *user_data)
{
    if (NULL == user_data) { return; }
    gt_obj_st* obj = (gt_obj_st*)user_data;

    thickness = thickness > 1 ? thickness : 2;
    gt_point_st start = {0};
    gt_point_st end = {0};

    start.x = x0;
    start.y = y0;

    end.x = x1;
    end.y = y1;

    _formula_draw_line(obj, start, end, thickness);

}

GT_ATTRIBUTE_RAM_TEXT
static void _formula_draw_arc_cb(int16_t cx, int16_t cy,
                            uint16_t r,
                            uint16_t start_deg, uint16_t end_deg,
                            uint8_t thickness,
                            uint32_t color, void *user_data)
{
    if (NULL == user_data) { return; }
    gt_obj_st* obj = (gt_obj_st*)user_data;
    _gt_formula_st* style = (_gt_formula_st*)obj;

    gt_attr_arc_st arc_attr = {
        .color = style->font_color,
        .opa = obj->opa,
        .width = thickness,
        .start_angle = start_deg,
        .end_angle = end_deg,
        .rounded = false,
    };

    gt_area_st coords = {
        .x = cx - r,
        .y = cy - r,
        .w = r * 2,
        .h = r * 2,
    };

    gt_draw_arc(obj->draw_ctx, &arc_attr, &coords);
}

GT_ATTRIBUTE_RAM_TEXT
static void _formula_init_cb(gt_obj_st * obj)
{
    _gt_formula_st * style = (_gt_formula_st * )obj;

    if(NULL == style->text) {
        return ;
    }
    gt_latex_render_register(_formula_draw_glyph_cb,
                            _formula_draw_hline_cb,
                            _formula_draw_line_cb,
                             _formula_draw_arc_cb);

    gt_latex_font_set_callback(_formula_get_metrics_cb);
    _formula_set_font_cfg(obj);


    gt_latex_lexer_t  lexer  = { 0 };
    gt_latex_parser_t parser = { 0 };
    gt_latex_layout_t layout = { 0 };

    int ret = gt_latex_parse(style->text, style->text_len, &parser, &lexer);
    if(GT_LATEX_OK != ret) {
        goto _deinit_ret;
    }

    uint16_t w = 0, h = 0;
    ret = gt_latex_layout(&parser, &layout, style->max_width, style->latex_off);
    if(GT_LATEX_OK != ret) {
        GT_LOGE(GT_LOG_TAG_GUI, "Failed to layout latex formula, ret=%d", ret);
        goto _deinit_ret;
    }
    gt_latex_layout_get_size(&layout, &w, &h);
    if(0 == w && 0 == h) {
        GT_LOGE(GT_LOG_TAG_GUI, "Failed to layout latex formula, w=%d, h=%d", w, h);
        goto _deinit_ret;
    }

    obj->area.w = w > style->max_width ? w : style->max_width; // 4 is padding
    obj->area.h = h; // 4 is padding
    _formula_bg_show(obj);

    gt_latex_canvas_t cv = { 0 };
    cv.x = obj->area.x;
    cv.y = obj->area.y;
    cv.fg_color = gt_color_to32(style->font_color);
    cv.user_data = (void*)obj;

    ret = gt_latex_render_draw(&layout, &cv);
    if(GT_LATEX_OK != ret) {
        GT_LOGE(GT_LOG_TAG_GUI, "Failed to render latex formula, ret=%d", ret);
    }

_deinit_ret:
    gt_latex_layout_deinit(&layout);
    gt_latex_parse_free(&parser, &lexer);
    _formula_clear_font_cfg(obj);
}

GT_ATTRIBUTE_RAM_TEXT
static void _formula_deinit_cb(gt_obj_st * obj)
{
    GT_LOGV(GT_LOG_TAG_GUI, "formula deinit");
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }

    _gt_formula_st * style = (_gt_formula_st * )obj;
    if (style->text) {
        gt_mem_free(style->text);
        style->text = NULL;
    }

    if(style->font_cfg) {
        gt_mem_free(style->font_cfg);
        style->font_cfg = NULL;
    }
}

GT_ATTRIBUTE_RAM_TEXT
static void _formula_event_cb(struct gt_obj_s * obj, gt_event_st * e)
{
    gt_event_type_et code_val = gt_event_get_code(e);

    if (GT_EVENT_TYPE_DRAW_START == code_val) {
        gt_disp_invalid_area(obj);
    }
    else if (GT_EVENT_TYPE_UPDATE_VALUE == code_val) {
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
    }
}


/* global functions / API interface -------------------------------------*/
GT_ATTRIBUTE_RAM_TEXT
gt_obj_st * gt_formula_create(gt_obj_st * parent) {
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    if (NULL == obj) {
        return obj;
    }

    return obj;
}

void gt_formula_set_text(gt_obj_st * obj, const char * fmt, ...)
{
    if (NULL == fmt || false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    char buffer[8] = {0};
    va_list args;
    va_list args2;

    _gt_formula_st * style = (_gt_formula_st * )obj;

    va_start(args, fmt);
    va_copy(args2, args);
    uint16_t size = (NULL == fmt) ? 0 : (vsnprintf(buffer, sizeof(buffer), fmt, args) + 1);
    va_end(args);
    if (!size) {
        goto free_lb;
    }

    if (NULL == style->text) {
        style->text = gt_mem_malloc(size);
    } else if (size != strlen(style->text) + 1) {
        style->text = gt_mem_realloc(style->text, size);
    }
    if (NULL == style->text) {
        goto free_lb;
    }

    gt_memset(style->text, 0, size);
    va_start(args2, fmt);
    vsnprintf(style->text, size, fmt, args2);
    va_end(args2);
    style->text_len = size - 1;

    gt_event_send(obj, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
    return;

free_lb:
    va_end(args2);
}

GT_ATTRIBUTE_RAM_TEXT
void gt_formula_set_text_by_len(gt_obj_st * obj, const char * text, uint16_t len)
{
    if (NULL == text || 0 == len || false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_formula_st * style = (_gt_formula_st * )obj;
    uint16_t size = len + 1;

    if (NULL == style->text) {
        style->text = gt_mem_malloc(size);
    } else if (size != style->text_len + 1) {
        style->text = gt_mem_realloc(style->text, size);
    }
    if (NULL == style->text) {
        return;
    }

    gt_memcpy(style->text, text, len);
    style->text[len] = '\0';
    style->text_len = len;

    gt_event_send(obj, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

GT_ATTRIBUTE_RAM_TEXT
void gt_formula_set_font_color(gt_obj_st * obj, gt_color_t color)
{
    if(false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_formula_st * style = (_gt_formula_st * )obj;

    style->font_color = color;
}

GT_ATTRIBUTE_RAM_TEXT
void gt_formula_set_max_width(gt_obj_st* obj, uint16_t max_width)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_formula_st * style = (_gt_formula_st * )obj;
    style->max_width = max_width;
}

GT_ATTRIBUTE_RAM_TEXT
void gt_formula_set_font_config(gt_obj_st * obj, const gt_formula_font_st *cfg)
{
    if (NULL == cfg || false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_formula_st * style = (_gt_formula_st * )obj;

    if(NULL == style->font_cfg) {
        style->font_cfg = gt_mem_malloc(sizeof(gt_formula_font_st));
    }
    if(NULL == style->font_cfg) {
        return ;
    }
    gt_memset(style->font_cfg, 0, sizeof(gt_formula_font_st));

    gt_memcpy(style->font_cfg, cfg, sizeof(gt_formula_font_st));
}
GT_ATTRIBUTE_RAM_TEXT
void gt_formula_set_first_line_offset(gt_obj_st* obj, uint16_t off_x, uint16_t off_y)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_formula_st * style = (_gt_formula_st * )obj;
    style->latex_off.x = off_x;
    style->latex_off.y = off_y;
}
/* end of file ----------------------------------------------------------*/
#endif

