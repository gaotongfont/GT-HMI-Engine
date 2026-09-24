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

/* 文本缓冲区默认容量（含结尾 NUL），不足时按 2 倍增长 */
#ifndef GT_FORMULA_TEXT_DEFAULT_CAP
    #define GT_FORMULA_TEXT_DEFAULT_CAP     (64)
#endif


/* private typedef ------------------------------------------------------*/
typedef struct _gt_formula_s {
    gt_obj_st obj;
    char * text;
    gt_formula_font_st* font_cfg;
    gt_color_t font_color;

    gt_latex_off_t latex_off;

    uint16_t max_width;
    uint16_t text_len;
    uint32_t text_cap;      /* 文本缓冲区已分配容量（含结尾 NUL）；0 表示未分配 */

    gt_formula_mode_t mode;  /* 渲染模式：LaTeX 或 Markdown / Render mode: LaTeX or Markdown */

    gt_md_parser_t md_parser;
    gt_md_node_container_t * md_root;
    gt_md_layout_t md_layout;
    uint8_t md_cache_ready; // 1: have built cache, 0: need to rebuild cache

    gt_latex_lexer_t latex_lexer;
    gt_latex_parser_t latex_parser;
    gt_latex_layout_t latex_layout;
    uint8_t latex_cache_ready;  // 1: have built cache, 0: need to rebuild cache

    gt_formula_cache_mode_t cache_mode;
    uint8_t cache_dirty;    // 1: need to rebuild cache, 0: need @ md_cache_ready and latex_cache_ready to be checked
    uint8_t cache_failed;   // 1: last rebuild failed; retry after content/config changes

    uint8_t md_latex_mode;
}_gt_formula_st;


/* static prototypes ----------------------------------------------------*/
GT_ATTRIBUTE_RAM_TEXT
static void _formula_init_cb(gt_obj_st * obj);
GT_ATTRIBUTE_RAM_TEXT
static void _formula_deinit_cb(gt_obj_st * obj);
GT_ATTRIBUTE_RAM_TEXT
static void _formula_event_cb(struct gt_obj_s * obj, gt_event_st * e);
GT_ATTRIBUTE_RAM_TEXT
static void _formula_set_font_cfg(gt_obj_st * obj);
GT_ATTRIBUTE_RAM_TEXT
static void _formula_clear_font_cfg(gt_obj_st * obj);
GT_ATTRIBUTE_RAM_TEXT
static int _formula_get_metrics_cb(uint32_t code,
                                const gt_latex_font_level_t *level_cfg,
                                gt_latex_glyph_metrics_t    *metrics,
                                void* user_data);
static void _formula_invalidate_cache(gt_obj_st * obj);
static int _formula_rebuild_cache(gt_obj_st * obj);
static uint8_t _formula_is_cache_ready(const _gt_formula_st * style);
static int _formula_render_current(gt_obj_st * obj);


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
static void _formula_clear_md_cache(_gt_formula_st * style)
{
    if (NULL == style) {
        return ;
    }

    gt_md_layout_free(&style->md_layout);
    if (style->md_root) {
        gt_md_node_free((gt_md_node_base_t *)style->md_root);
        style->md_root = NULL;
    }
    gt_md_parser_deinit(&style->md_parser);
    gt_memset(&style->md_parser, 0, sizeof(style->md_parser));
    gt_memset(&style->md_layout, 0, sizeof(style->md_layout));
    style->md_cache_ready = 0;
}

static void _formula_clear_latex_cache(_gt_formula_st * style)
{
    if (NULL == style) {
        return ;
    }

    gt_latex_layout_deinit(&style->latex_layout);
    gt_latex_parse_free(&style->latex_parser, &style->latex_lexer);
    gt_memset(&style->latex_parser, 0, sizeof(style->latex_parser));
    gt_memset(&style->latex_lexer, 0, sizeof(style->latex_lexer));
    gt_memset(&style->latex_layout, 0, sizeof(style->latex_layout));
    style->latex_cache_ready = 0;
}

static void _formula_update_area_by_size(gt_obj_st * obj, uint16_t w, uint16_t h)
{
    _gt_formula_st * style = (_gt_formula_st * )obj;

    if(w < style->max_width) {
        w = style->max_width;
    }

    gt_obj_set_size(obj, w, h);
}

static void _formula_begin_layout_ctx(gt_obj_st * obj)
{
    gt_latex_font_set_callback(_formula_get_metrics_cb);
    _formula_set_font_cfg(obj);
}

static void _formula_end_layout_ctx(gt_obj_st * obj)
{
    _formula_clear_font_cfg(obj);
}

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

    if (level_cfg->font_flags & GT_LATEX_FONT_BOLD) {
        metrics->font_w += 1;
    }

    if (level_cfg->font_flags & GT_LATEX_FONT_ITALIC) {
         metrics->font_w += m.font_h >> 2;
    }

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

    if (level_cfg->font_flags & GT_LATEX_FONT_BOLD) {
        font.info.style.reg.bold = true;
    }

    if (level_cfg->font_flags & GT_LATEX_FONT_ITALIC) {
        font.info.style.reg.italic = true;
    }

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

static int _formula_build_md_cache(gt_obj_st * obj)
{
    _gt_formula_st * style = (_gt_formula_st * )obj;

    if(style->max_width <= 0) {
        style->max_width = obj->area.w;
    }

#if GT_MD_ENABLE
    gt_md_set_latex_mode(style->md_latex_mode);
    int ret = gt_md_parse_and_layout(style->text, style->text_len, style->max_width,
                            &style->md_parser, &style->md_root, &style->md_layout);
    if (ret != GT_MD_OK) {
        GT_LOGE(GT_LOG_TAG_GUI, "Failed to parse/layout MD, ret=%d", ret);
        _formula_clear_md_cache(style);
        return ret;
    }

    _formula_update_area_by_size(obj, style->md_layout.total_w, style->md_layout.total_h);
    style->md_cache_ready = 1;
    return GT_MD_OK;
#else
    GT_LOGE(GT_LOG_TAG_GUI, "Markdown mode not enabled (GT_MD_ENABLE=0)");
    return -1;
#endif
}

static int _formula_render_md_cache(gt_obj_st * obj)
{
    _gt_formula_st * style = (_gt_formula_st * )obj;

    if (0 == style->md_cache_ready) {
        return -1;
    }

    _formula_bg_show(obj);

    gt_latex_canvas_t cv = { 0 };
    cv.x = obj->area.x;
    cv.y = obj->area.y;
    cv.fg_color = gt_color_to32(style->font_color);
    cv.user_data = (void*)obj;

    gt_obj_st* parent = gt_obj_get_parent(obj);
    if(parent) {
        cv.ys = parent->area.y;
        cv.ye = parent->area.y + parent->area.h;
    }

    if (GT_MD_OK != gt_md_render(&style->md_layout, &cv)) {
        GT_LOGE(GT_LOG_TAG_GUI, "Failed to render MD cache");
        return -1;
    }
    return GT_MD_OK;
}

static int _formula_build_latex_cache(gt_obj_st * obj)
{
    _gt_formula_st * style = (_gt_formula_st * )obj;
    uint16_t w = 0, h = 0;

    if(style->max_width <= 0) {
        style->max_width = obj->area.w;
    }

    int ret = gt_latex_parse(style->text, style->text_len, &style->latex_parser, &style->latex_lexer);
    if(GT_LATEX_OK != ret) {
        _formula_clear_latex_cache(style);
        return ret;
    }

    ret = gt_latex_layout(&style->latex_parser, &style->latex_layout, style->max_width, style->latex_off);
    if(GT_LATEX_OK != ret) {
        _formula_clear_latex_cache(style);
        return ret;
    }
    gt_latex_layout_get_size(&style->latex_layout, &w, &h);
    if(0 == w && 0 == h) {
        GT_LOGE(GT_LOG_TAG_GUI, "Failed to layout latex formula, w=%d, h=%d", w, h);
        _formula_clear_latex_cache(style);
        return -1;
    }

    _formula_update_area_by_size(obj, w, h);
    style->latex_cache_ready = 1;
    return GT_LATEX_OK;
}

static int _formula_render_latex_cache(gt_obj_st * obj)
{
    _gt_formula_st * style = (_gt_formula_st * )obj;

    if (0 == style->latex_cache_ready) {
        return -1;
    }

    _formula_bg_show(obj);

    gt_latex_canvas_t cv = { 0 };
    cv.x = obj->area.x;
    cv.y = obj->area.y;
    cv.fg_color = gt_color_to32(style->font_color);
    cv.user_data = (void*)obj;

    if (GT_LATEX_OK != gt_latex_render_draw(&style->latex_layout, &cv)) {
        GT_LOGE(GT_LOG_TAG_GUI, "Failed to render LaTeX cache");
        return -1;
    }
    return GT_LATEX_OK;
}

static void _formula_clear_cache(_gt_formula_st * style)
{
    _formula_clear_md_cache(style);
    _formula_clear_latex_cache(style);
}

static void _formula_invalidate_cache(gt_obj_st * obj)
{
    _gt_formula_st * style = (_gt_formula_st * )obj;

    _formula_clear_cache(style);
    style->cache_dirty = 1;
    style->cache_failed = 0;
}

static uint8_t _formula_is_cache_ready(const _gt_formula_st * style)
{
    if (NULL == style) {
        return 0;
    }
    if (GT_FORMULA_MODE_MD == style->mode) {
        return style->md_cache_ready;
    }
    return style->latex_cache_ready;
}

static int _formula_rebuild_cache(gt_obj_st * obj)
{
    _gt_formula_st * style = (_gt_formula_st * )obj;
    int ret = -1;

    if(NULL == style->text || 0 == style->text_len) {
        style->cache_dirty = 0;
        style->cache_failed = 1;
        return -1;
    }

    _formula_invalidate_cache(obj);
    _formula_begin_layout_ctx(obj);

    if (GT_FORMULA_MODE_MD == style->mode) {
        ret = _formula_build_md_cache(obj);
    } else {
        ret = _formula_build_latex_cache(obj);
    }

    _formula_end_layout_ctx(obj);
    style->cache_dirty = 0;
    style->cache_failed = (0 == ret) ? 0 : 1;
    return ret;
}

static int _formula_render_current(gt_obj_st * obj)
{
    _gt_formula_st * style = (_gt_formula_st * )obj;

    _formula_begin_layout_ctx(obj);
    if (style->mode == GT_FORMULA_MODE_MD) {
        _formula_render_md_cache(obj);
    } else {
        _formula_render_latex_cache(obj);
    }
    _formula_end_layout_ctx(obj);
    return 0;
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

    if (GT_FORMULA_CACHE_MODE_PERSIST == style->cache_mode) {
        if (style->cache_dirty ||
            (0 == _formula_is_cache_ready(style) && 0 == style->cache_failed)) {
            _formula_rebuild_cache(obj);
        }
        if (_formula_is_cache_ready(style)) {
            _formula_render_current(obj);
        }
        return ;
    }

    if (style->cache_dirty ||
        (0 == _formula_is_cache_ready(style) && 0 == style->cache_failed)) {
        _formula_rebuild_cache(obj);
    }
    if (0 == _formula_is_cache_ready(style)) {
        return ;
    }
    _formula_render_current(obj);
   _formula_clear_cache(style);
    style->cache_dirty = 0;
    style->cache_failed = 0;
}

GT_ATTRIBUTE_RAM_TEXT
static void _formula_deinit_cb(gt_obj_st * obj)
{
    GT_LOGV(GT_LOG_TAG_GUI, "formula deinit");
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }

    _gt_formula_st * style = (_gt_formula_st * )obj;
    _formula_invalidate_cache(obj);

    if (style->text) {
        gt_mem_free(style->text);
        style->text = NULL;
        style->text_cap = 0;
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
    _gt_formula_st * style = (_gt_formula_st * )obj;
    if (NULL == obj) {
        return obj;
    }

    style->cache_mode = GT_FORMULA_CACHE_MODE_PERSIST;

    /* 预分配默认容量，避免首次写入文本时再分配 */
    style->text = (char * )gt_mem_malloc(GT_FORMULA_TEXT_DEFAULT_CAP);
    if (NULL == style->text) {
        GT_LOGE(GT_LOG_TAG_GUI, "formula text malloc failed, cap=%d", GT_FORMULA_TEXT_DEFAULT_CAP);
        return obj;
    }
    style->text[0] = '\0';
    style->text_cap = GT_FORMULA_TEXT_DEFAULT_CAP;

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
    int format_len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    if (format_len < 0) {
        return;
    }

    uint32_t size = (uint32_t)format_len + 1;
    if (size > 0x10000u) {   /* style->text_len 是 uint16_t，更长的文本无法表示 */
        GT_LOGW(GT_LOG_TAG_GUI, "formula text too long: %d", format_len);
        return;
    }

    if (NULL == style->text || size > style->text_cap) {
        uint32_t cap = style->text_cap ? style->text_cap : GT_FORMULA_TEXT_DEFAULT_CAP;
        while (cap < size) {
            cap <<= 1;
        }

        char *old_text = style->text;
        /* 容量不足：分配新缓冲区，写入完成后再释放旧内存，避免泄漏和别名问题 */
        char *new_text = (char *)gt_mem_malloc(cap);
        if (NULL == new_text) {
            GT_LOGE(GT_LOG_TAG_GUI, "style->text malloc failed, size=%u", (unsigned)cap);
            return;
        }
        va_start(args2, fmt);
        vsnprintf(new_text, size, fmt, args2);
        va_end(args2);

        /* Parser nodes and lexer tokens point into old_text. Clear them first. */
        _formula_invalidate_cache(obj);
        style->text = new_text;
        style->text_cap = cap;

        if (old_text != NULL) {
            gt_mem_free(old_text);
        }
    } else {
        /* 容量足够，复用原缓冲区；覆盖 text 前先清理引用它的缓存 */
        _formula_invalidate_cache(obj);
        va_start(args2, fmt);
        vsnprintf(style->text, size, fmt, args2);
        va_end(args2);
    }

    style->text_len = (uint16_t)(size - 1);

    if (GT_FORMULA_CACHE_MODE_PERSIST == style->cache_mode) {
        _formula_rebuild_cache(obj);
    } else {
        if (0 == _formula_rebuild_cache(obj)) {
            _formula_invalidate_cache(obj);
        }
    }
    gt_event_send(obj, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

GT_ATTRIBUTE_RAM_TEXT
void gt_formula_set_text_by_len(gt_obj_st * obj, const char * text, uint16_t len)
{
    if (NULL == text || 0 == len || false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }

    _gt_formula_st *style = (_gt_formula_st *)obj;
    uint32_t size = (uint32_t)len + 1;   /* len 最大 65535，+1 后超出 uint16_t 范围 */

    if (NULL == style->text || size > style->text_cap) {
        uint32_t cap = style->text_cap ? style->text_cap : GT_FORMULA_TEXT_DEFAULT_CAP;
        while (cap < size) {
            cap <<= 1;
        }

        char *old_text = style->text;
        /* 容量不足：分配新缓冲区，复制完成后再释放旧内存，避免泄漏和别名问题 */
        char *new_text = (char *)gt_mem_malloc(cap);
        if (NULL == new_text) {
            GT_LOGE(GT_LOG_TAG_GUI, "style->text malloc failed, size=%u", (unsigned)cap);
            return;
        }
        gt_memcpy(new_text, text, len);
        new_text[len] = '\0';

        /* Parser nodes and lexer tokens point into old_text. Clear them first. */
        _formula_invalidate_cache(obj);
        style->text = new_text;
        style->text_cap = cap;

        if (old_text != NULL) {
            gt_mem_free(old_text);
        }
    } else {
        /* 容量足够，直接复用原缓冲区；用 memmove 兼容 text 与 style->text 重叠的情况 */
        _formula_invalidate_cache(obj);
        if (text != style->text) {
            gt_memmove(style->text, text, len);
        }
        style->text[len] = '\0';
    }

    style->text_len = len;

    if (GT_FORMULA_CACHE_MODE_PERSIST == style->cache_mode) {
        _formula_rebuild_cache(obj);
    } else {
        if (0 == _formula_rebuild_cache(obj)) {
            _formula_invalidate_cache(obj);
        }
    }

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
    style->cache_dirty = 1;
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
    style->cache_dirty = 1;
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
    style->cache_dirty = 1;
}

GT_ATTRIBUTE_RAM_TEXT
void gt_formula_set_mode(gt_obj_st* obj, gt_formula_mode_t mode)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_formula_st * style = (_gt_formula_st * )obj;
    style->mode = mode;
    style->cache_dirty = 1;
}

GT_ATTRIBUTE_RAM_TEXT
gt_formula_mode_t gt_formula_get_mode(gt_obj_st* obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return GT_FORMULA_MODE_LATEX;
    }
    _gt_formula_st * style = (_gt_formula_st * )obj;
    return style->mode;
}

GT_ATTRIBUTE_RAM_TEXT
void gt_formula_set_cache_mode(gt_obj_st* obj, gt_formula_cache_mode_t cache_mode)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_formula_st * style = (_gt_formula_st * )obj;
    if (style->cache_mode == cache_mode) {
        return ;
    }

    style->cache_mode = cache_mode;
    if (GT_FORMULA_CACHE_MODE_NONE == cache_mode) {
        _formula_invalidate_cache(obj);
    } else {
        style->cache_dirty = 1;
    }
}

GT_ATTRIBUTE_RAM_TEXT
gt_formula_cache_mode_t gt_formula_get_cache_mode(gt_obj_st* obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return GT_FORMULA_CACHE_MODE_PERSIST;
    }
    return ((_gt_formula_st *)obj)->cache_mode;
}


GT_ATTRIBUTE_RAM_TEXT
void gt_formula_set_md_latex_mode(gt_obj_st* obj, uint8_t mode)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_formula_st * style = (_gt_formula_st * )obj;
    style->md_latex_mode = mode;
}


/* end of file ----------------------------------------------------------*/
#endif

