/**
 * @file gt_table.c
 * @author Feyoung
 * @brief Table widget, support multi-row and multi-column table.
 *      Called gt_table_refresh() after add header and content text.
 * @version 0.1
 * @date 2024-12-19 17:35:53
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "./gt_table.h"

#if GT_CFG_ENABLE_TABLE
#include "../core/gt_mem.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"
#include "../utils/gt_vector.h"
#include "../others/gt_area.h"


/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_TABLE
#define MY_CLASS    &gt_table_class


/* private typedef ------------------------------------------------------*/
typedef struct _gt_table_col_s {
    gt_align_et align;
    uint16_t max_width;
}_gt_table_col_st;

typedef struct {
    uint16_t idx;           /** row index, begin from 1 */
    uint16_t max_height;
}_gt_table_row_item_st;

typedef struct _gt_table_s {
    gt_obj_st obj;

    gt_color_t color_border;
    uint8_t border_width;
    gt_font_info_st font_info;

    _gt_table_col_st * col_list;    /** count by header items count */

    _gt_vector_st * header;
    _gt_vector_st * contents;

    _gt_vector_st * row_el;         /** row element information */

    uint16_t height_header;         /** header total height */
    uint16_t height_contents;       /** all row element total height */

    uint8_t update_col : 1;
}_gt_table_st;


/* static variables -----------------------------------------------------*/
static void _table_init_cb(gt_obj_st * obj);
static void _table_deinit_cb(gt_obj_st * obj);
static void _table_event_cb(struct gt_obj_s * obj, gt_event_st * e);

static GT_ATTRIBUTE_RAM_TEXT gt_res_t
_calc_each_column_display_max_width(gt_obj_st * table);

/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/
static GT_ATTRIBUTE_RAM_DATA const gt_obj_class_st gt_table_class = {
    ._init_cb      = _table_init_cb,
    ._deinit_cb    = _table_deinit_cb,
    ._event_cb     = _table_event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_table_st)
};


/* static functions -----------------------------------------------------*/

static GT_ATTRIBUTE_RAM_TEXT void _draw_line(gt_obj_st * obj, gt_area_st * area, gt_color_t color, uint8_t width) {
    gt_area_st res = *area;
    if (gt_obj_get_inside(obj) &&
        false == gt_area_cover_screen(&obj->parent->area, area, &res)) {
        return ;
    }
    gt_attr_line_st line = {
        .start = {
            .x = res.x,
            .y = res.y,
        }, .end = {
            .x = res.x + res.w,
            .y = res.y + res.h,
        }, .line = {
            .width = width,
            .color = color,
            .opa = obj->opa,
            .type = GT_GRAPHS_TYPE_LINE,
            .brush = GT_BRUSH_TYPE_ROUND,
        },
    };
    gt_draw_line(obj->draw_ctx, &line, &obj->area);
}

static GT_ATTRIBUTE_RAM_TEXT inline uint16_t _get_label_offset(_gt_table_st * style) {
    return style->font_info.size >> 1;
}

static GT_ATTRIBUTE_RAM_TEXT void _draw_header(gt_obj_st * obj) {
    _gt_table_st * style = (_gt_table_st * )obj;
    uint16_t count = _gt_vector_get_count(style->header);
    uint16_t reduce = style->border_width;
    gt_font_st font = {
        .res = NULL,
        .info = style->font_info,
    };
    gt_attr_font_st font_attr = {
        .font = &font,
        .font_color = gt_color_black(),
        .opa = obj->opa,
    };
    gt_area_st line_area = {
        .x = obj->area.x + reduce + _get_label_offset(style),
        .y = obj->area.y + reduce,
        .w = style->border_width,
        .h = style->height_header + style->border_width,
    };
    gt_area_st text_area = {
        .x = line_area.x,
        .y = line_area.y,
        .w = 0,
        .h = style->height_header,
    };
    _gt_vector_iterator_st iter = _gt_vector_get_iterator(style->header);
    gt_size_t i = 0;
    while (iter.has_next(iter.dsc_t)) {
        i = iter.index(iter.dsc_t);
        text_area.w = style->col_list[i].max_width;
        font_attr.align = style->col_list[i].align;
        font_attr.logical_area = text_area;
        font.utf8 = iter.next(iter.dsc_t);
        font.len = strlen(font.utf8);

        draw_text(obj->draw_ctx, &font_attr, &text_area);

        text_area.x += text_area.w;
        if (i + 1 < count) {
            line_area.x += text_area.w;
            _draw_line(obj, &line_area, style->color_border, style->border_width);
        }
    }
    /** hor line */
    line_area.x = obj->area.x + reduce;
    line_area.y = obj->area.y + style->height_header + style->border_width;
    line_area.w = obj->area.w - (reduce << 1);
    line_area.h = style->border_width;
    _draw_line(obj, &line_area, style->color_border, style->border_width);
}

static GT_ATTRIBUTE_RAM_TEXT void _draw_contents(gt_obj_st * obj) {
    _gt_table_st * style = (_gt_table_st * )obj;
    uint16_t count = _gt_vector_get_count(style->header);
    uint16_t reduce = style->border_width;
    gt_font_st font = {
        .res = NULL,
        .info = style->font_info,
    };
    font.info.style.reg.bold = false;   /** Force non-bold style */
    gt_attr_font_st font_attr = {
        .font = &font,
        .font_color = gt_color_black(),
        .opa = obj->opa,
    };
    gt_area_st line_area = {
        .x = obj->area.x + reduce + _get_label_offset(style),
        .y = obj->area.y + reduce + style->height_header + style->border_width,
        .w = style->border_width,
        .h = 0,
    };
    gt_area_st text_area = {
        .x = line_area.x,
        .y = line_area.y,
        .w = 0,
        .h = 0,
    };
    gt_area_st hor_line_area = {
        .x = obj->area.x + reduce,
        .w = obj->area.w - (reduce << 1),
        .h = style->border_width,
    };
    _gt_vector_iterator_st iter = _gt_vector_get_iterator(style->contents);
    _gt_table_row_item_st * row_item = NULL;
    gt_size_t i = 0;

    while (iter.has_next(iter.dsc_t)) {
        row_item = (_gt_table_row_item_st * )_gt_vector_get_item(style->row_el, iter.index(iter.dsc_t) / count);
        for (i = 0; i < count; ++i) {
            /** col item */
            if (false == iter.has_next(iter.dsc_t)) {
                break;
            }
            text_area.w = style->col_list[i].max_width;
            text_area.h = row_item->max_height;
            font_attr.align = style->col_list[i].align;
            font_attr.logical_area = text_area;
            font.utf8 = iter.next(iter.dsc_t);
            font.len = strlen(font.utf8);

            draw_text(obj->draw_ctx, &font_attr, &text_area);
            text_area.x += text_area.w;
            if (i + 1 < count) {
                line_area.x += text_area.w;
                line_area.h = text_area.h + style->border_width;
                _draw_line(obj, &line_area, style->color_border, style->border_width);
            }
        }
        text_area.x = obj->area.x + reduce + _get_label_offset(style);
        text_area.y += row_item->max_height + style->border_width;
        line_area.x = text_area.x;
        line_area.y = text_area.y + reduce;
        if (iter.has_next(iter.dsc_t)) {
            hor_line_area.y = text_area.y;
            _draw_line(obj, &hor_line_area, style->color_border, style->border_width);
        }
    }
}

static void _table_init_cb(gt_obj_st * obj) {
    _gt_table_st * style = (_gt_table_st * )obj;
    gt_attr_rect_st rect_attr = {0};

    gt_font_info_update_font_thick(&style->font_info);

    if (style->update_col) {
        gt_table_refresh(obj);
        _gt_disp_reload_max_area(gt_disp_get_scr());
        return;
    }

    if (obj->show_bg) {
        gt_graph_init_rect_attr(&rect_attr);
        rect_attr.reg.is_fill   = true;
        rect_attr.radius        = obj->radius;
        rect_attr.bg_opa        = obj->opa;
        rect_attr.fg_color      = obj->bgcolor;
        rect_attr.bg_color      = obj->bgcolor;
        rect_attr.border_width  = style->border_width;
        rect_attr.border_color  = style->color_border;

        draw_bg(obj->draw_ctx, &rect_attr, &obj->area);
    }

    _draw_header(obj);
    _draw_contents(obj);

    draw_focus(obj);
}

static void _table_deinit_cb(gt_obj_st * obj) {
    _gt_table_st * style = (_gt_table_st * )obj;
    if (style->row_el) {
        _gt_vector_free(style->row_el);
        style->row_el = NULL;
    }
    if (style->contents) {
        _gt_vector_free(style->contents);
        style->contents = NULL;
    }
    if (style->col_list) {
        gt_mem_free(style->col_list);
        style->col_list = NULL;
    }
    if (style->header) {
        _gt_vector_free(style->header);
        style->header = NULL;
    }
}

static void _table_event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code_val = gt_event_get_code(e);

    if (GT_EVENT_TYPE_DRAW_START == code_val) {
        gt_disp_invalid_area(obj);
    }
}

static GT_ATTRIBUTE_RAM_TEXT bool _str_free_item_cb(void * item) {
    if (NULL == item) {
        return false;
    }
    gt_mem_free(item);
    return true;
}

static GT_ATTRIBUTE_RAM_TEXT bool _str_equal_cb(void * item, void * target) {
    return strcmp(item, target) ? false : true;
}

static GT_ATTRIBUTE_RAM_TEXT bool _row_free_item_cb(void * item) {
    return _str_free_item_cb(item);
}

static GT_ATTRIBUTE_RAM_TEXT bool _row_equal_cb(void * item, void * target) {
    _gt_table_row_item_st * row = (_gt_table_row_item_st * )item;
    _gt_table_row_item_st * tar = (_gt_table_row_item_st * )target;
    return row->idx == tar->idx ? true : false;
}

static GT_ATTRIBUTE_RAM_TEXT uint16_t
_get_row_max_width(_gt_table_st * style, uint16_t count) {
    uint16_t total_width = 0;
    for (gt_size_t i = 0; i < count; ++i) {
        total_width += style->col_list[i].max_width;
    }
    return total_width;
}

static GT_ATTRIBUTE_RAM_TEXT gt_res_t
_resize_max_width_by(_gt_table_st * style, uint16_t count, gt_size_t diff_width) {
    if (diff_width <= 0) {
        return GT_RES_OK;
    }

    for (gt_size_t i = 0; i < count; ++i) {
        if (style->col_list[i].max_width > diff_width) {
            style->col_list[i].max_width -= diff_width;
        }
    }

    if (_get_row_max_width(style, count) > ((gt_obj_st * )style)->area.w) {
        return _resize_max_width_by(style, count, --diff_width);
    }
    return GT_RES_OK;
}

static GT_ATTRIBUTE_RAM_TEXT gt_res_t
_calc_each_column_display_max_width(gt_obj_st * table) {
    _gt_table_st * style = (_gt_table_st * )table;
    _gt_vector_iterator_st iter = _gt_vector_get_iterator(style->header);
    _gt_table_col_st * col = NULL;
    uint16_t count = _gt_vector_get_count(style->header);
    uint16_t str_width = 0;
    char * content = NULL;
    if (0 == count) {
        return GT_RES_OK;
    }

    /** header */
    while (iter.has_next(iter.dsc_t)) {
        col = &style->col_list[iter.index(iter.dsc_t) % count];
        col->max_width = style->font_info.size;
        content = (char * )iter.next(iter.dsc_t);

        col->max_width += gt_font_get_longest_line_substring_width(&style->font_info, content, 0);
    }

    iter = _gt_vector_get_iterator(style->contents);
    while (iter.has_next(iter.dsc_t)) {
        col = &style->col_list[iter.index(iter.dsc_t) % count];
        content = (char * )iter.next(iter.dsc_t);

        str_width = gt_font_get_longest_line_substring_width(&style->font_info, content, 0);
        if (str_width > col->max_width) {
            col->max_width = str_width;
        }
    }

    uint16_t total_width = _get_row_max_width(style, count) + style->font_info.size;
    total_width += style->border_width << 1;
    if (table->area.w > total_width) {
        gt_obj_set_w(table, total_width);
    } else {
        _resize_max_width_by(style, count, (total_width - table->area.w) / count);
    }

    return GT_RES_OK;
}

static GT_ATTRIBUTE_RAM_TEXT uint16_t
_calc_vec_total_height_by(_gt_table_st * style, uint16_t col_count, _gt_vector_iterator_st iter, _gt_vector_st * row_vector) {
    uint32_t ret_w = 0;
    uint16_t total_height = 0, line_height = 0, row_idx = 0;
    gt_size_t i = 0, line_count;
    gt_font_split_line_st sp_line = {
        .text = NULL,
        .space = 0,
        .indent = 0,
    };
    gt_size_t row_count = row_vector ? _gt_vector_get_count(row_vector) : 0;
    _gt_table_row_item_st * row_item = NULL;

    while (iter.has_next(iter.dsc_t)) {
        for (i = 0, line_height = 0; i < col_count; ++i) {
            if (false == iter.has_next(iter.dsc_t)) {
                break;
            }
            sp_line.max_w = style->col_list[iter.index(iter.dsc_t) % col_count].max_width;
            sp_line.start_w = sp_line.max_w;
            sp_line.text = (char * )iter.next(iter.dsc_t);
            line_count = gt_font_split_line_numb(&style->font_info, &sp_line, &ret_w);
            if (style->font_info.size * line_count > line_height) {
                line_height = style->font_info.size * line_count;
            }
        }
        line_height += style->font_info.size;
        total_height += line_height;

        if (row_vector) {
            row_idx = iter.index(iter.dsc_t) / col_count;
            if (row_idx > _gt_vector_get_count(row_vector)) {
                row_item = gt_mem_malloc(sizeof(_gt_table_row_item_st));
                if (NULL == row_item) {
                    GT_LOGD(GT_LOG_TAG_ASSERT, "malloc row item info failed");
                    continue;
                }
                row_item->idx = row_idx;
                row_item->max_height = line_height;
                _gt_vector_add_item(row_vector, row_item);
            } else {
                row_item = (_gt_table_row_item_st * )_gt_vector_get_item(row_vector, row_idx - 1);
                row_item->max_height = line_height;
            }
        }
    }
    return total_height;
}

static GT_ATTRIBUTE_RAM_TEXT uint16_t
_calc_total_height(gt_obj_st * table) {
    _gt_table_st * style = (_gt_table_st * )table;
    uint16_t col_count = _gt_vector_get_count(style->header);

    style->height_header += _calc_vec_total_height_by(style, col_count,
        _gt_vector_get_iterator(style->header), NULL);
    style->height_contents += _calc_vec_total_height_by(style, col_count,
        _gt_vector_get_iterator(style->contents), style->row_el);

    gt_size_t row_count = _gt_vector_get_count(style->row_el) - 1;
    if (row_count > 0) {
        row_count *= style->border_width;
    } else {
        row_count = 0;
    }
    return style->height_header + style->height_contents + row_count + style->border_width;// + (table->reduce << 1);
}


/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_table_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    if (NULL == obj) {
        return obj;
    }
    obj->show_bg = true;
    obj->area.w = parent->area.w;
    obj->scroll_dir = GT_SCROLL_DISABLE;

    _gt_table_st * style = (_gt_table_st * )obj;
    style->color_border = gt_color_hex(0x7f7f7f);
    style->border_width = 1;

    gt_font_info_init(&style->font_info);

    style->header = _gt_vector_create(_str_free_item_cb, _str_equal_cb);
    style->contents = _gt_vector_create(_str_free_item_cb, _str_equal_cb);
    style->row_el = _gt_vector_create(_row_free_item_cb, _row_equal_cb);
    return obj;
}

gt_res_t gt_table_add_header_text_by_len(gt_obj_st * table, const char * text, uint16_t len, gt_align_et align)
{
    if (false == gt_obj_is_type(table, OBJ_TYPE)) {
        return GT_RES_FAIL;
    }
    _gt_table_st * style = (_gt_table_st * )table;
    char * title = gt_mem_malloc(len + 1);
    if (NULL == title) {
        return GT_RES_FAIL;
    }
    gt_memcpy(title, text, len);
    title[len] = '\0';
    if (false == _gt_vector_add_item(style->header, (void * )title)) {
        goto free_text_lb;
    }
    uint16_t count = _gt_vector_get_count(style->header);
    // align
    if (NULL == style->col_list) {
        style->col_list = gt_mem_malloc(sizeof(_gt_table_col_st));
        if (NULL == style->col_list) {
            goto free_item_lb;
        }
    } else {
        style->col_list = gt_mem_realloc(style->col_list, sizeof(_gt_table_col_st) * count);
        if (NULL == style->col_list) {
            goto resize_item_lb;
        }
    }
    style->col_list[count - 1].align = align;
    style->col_list[count - 1].max_width = 0;
    style->update_col = true;
    return GT_RES_OK;

resize_item_lb:
    if (NULL == style->col_list) {
        --count;
        style->col_list = gt_mem_realloc(style->col_list, sizeof(_gt_table_col_st) * count);
    }
free_item_lb:
    if (false == _gt_vector_remove_latest_item(style->header)) {
        GT_LOGD(GT_LOG_TAG_CHECK, "remove latest item failed");
    }
free_text_lb:
    gt_mem_free(title);
    title = NULL;
    return GT_RES_FAIL;
}

gt_res_t gt_table_add_content_text_by_len(gt_obj_st * table, const char * text, uint16_t len)
{
    if (false == gt_obj_is_type(table, OBJ_TYPE)) {
        return GT_RES_FAIL;
    }
    _gt_table_st * style = (_gt_table_st * )table;
    char * content = gt_mem_malloc(len + 1);
    if (NULL == content) {
        return GT_RES_FAIL;
    }
    gt_memcpy(content, text, len);
    content[len] = '\0';
    if (false == _gt_vector_add_item(style->contents, (void * )content)) {
        goto free_text_lb;
    }
    style->update_col = true;
    return GT_RES_OK;

free_text_lb:
    gt_mem_free(content);
    content = NULL;
    return GT_RES_FAIL;
}

gt_res_t gt_table_refresh(gt_obj_st * table)
{
    if (false == gt_obj_is_type(table, OBJ_TYPE)) {
        return GT_RES_FAIL;
    }
    _gt_table_st * style = (_gt_table_st * )table;
    if (false == style->update_col) {
        return GT_RES_OK;
    }
    style->update_col = false;
    gt_disp_invalid_area(table);
    _calc_each_column_display_max_width(table);
    gt_obj_set_h(table, _calc_total_height(table));
    gt_event_send(table, GT_EVENT_TYPE_DRAW_START, NULL);
    return GT_RES_OK;
}

void gt_table_set_font_color(gt_obj_st * table, gt_color_t color)
{
    if (false == gt_obj_is_type(table, OBJ_TYPE)) {
        return ;
    }
    _gt_table_st * style = (_gt_table_st * )table;
    style->font_info.palette = color;
    gt_event_send(table, GT_EVENT_TYPE_DRAW_START, NULL);
}

gt_color_t gt_table_get_font_color(gt_obj_st * table)
{
    if (false == gt_obj_is_type(table, OBJ_TYPE)) {
        return gt_color_black();
    }
    _gt_table_st * style = (_gt_table_st * )table;
    return style->font_info.palette;
}

void gt_table_set_font_size(gt_obj_st * table, uint8_t size)
{
    if (false == gt_obj_is_type(table, OBJ_TYPE)) {
        return ;
    }
    _gt_table_st * style = (_gt_table_st * )table;
    style->font_info.size = size;
}

void gt_table_set_font_gray(gt_obj_st * table, uint8_t gray)
{
    if (false == gt_obj_is_type(table, OBJ_TYPE)) {
        return ;
    }
    _gt_table_st * style = (_gt_table_st * )table;
    style->font_info.gray = gray;
}

#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
void gt_table_set_font_family_cn(gt_obj_st * table, gt_family_t family)
{
    if (false == gt_obj_is_type(table, OBJ_TYPE)) {
        return ;
    }
    _gt_table_st * style = (_gt_table_st * )table;
    style->font_info.style_cn = family;
}

void gt_table_set_font_family_en(gt_obj_st * table, gt_family_t family)
{
    if (false == gt_obj_is_type(table, OBJ_TYPE)) {
        return ;
    }
    _gt_table_st * style = (_gt_table_st * )table;
    style->font_info.style_en = family;
}

void gt_table_set_font_family_fl(gt_obj_st * table, gt_family_t family)
{
    if (false == gt_obj_is_type(table, OBJ_TYPE)) {
        return ;
    }
    _gt_table_st * style = (_gt_table_st * )table;
    style->font_info.style_fl = family;
}

void gt_table_set_font_family_numb(gt_obj_st * table, gt_family_t family)
{
    if (false == gt_obj_is_type(table, OBJ_TYPE)) {
        return ;
    }
    _gt_table_st * style = (_gt_table_st * )table;
    style->font_info.style_numb = family;
}
#else
void gt_table_set_font_family(gt_obj_st * table, gt_family_t family)
{
    if (false == gt_obj_is_type(table, OBJ_TYPE)) {
        return ;
    }
    _gt_table_st * style = (_gt_table_st * )table;
    gt_font_set_family(&style->font_info, family);
}

void gt_table_set_font_cjk(gt_obj_st* table, gt_font_cjk_et cjk)
{
    if (false == gt_obj_is_type(table, OBJ_TYPE)) {
        return ;
    }
    _gt_table_st * style = (_gt_table_st * )table;
    style->font_info.cjk = cjk;
}
#endif
void gt_table_set_font_thick_en(gt_obj_st * table, uint8_t thick)
{
    if (false == gt_obj_is_type(table, OBJ_TYPE)) {
        return ;
    }
    _gt_table_st * style = (_gt_table_st * )table;
    style->font_info.thick_en = thick;
}

void gt_table_set_font_thick_cn(gt_obj_st * table, uint8_t thick)
{
    if (false == gt_obj_is_type(table, OBJ_TYPE)) {
        return ;
    }
    _gt_table_st * style = (_gt_table_st * )table;
    style->font_info.thick_cn = thick;
}

void gt_table_set_font_encoding(gt_obj_st * table, gt_encoding_et encoding)
{
    if (false == gt_obj_is_type(table, OBJ_TYPE)) {
        return ;
    }
    _gt_table_st * style = (_gt_table_st * )table;
    style->font_info.encoding = encoding;
}

void gt_table_set_font_style(gt_obj_st * table, gt_font_style_et font_style)
{
    if (false == gt_obj_is_type(table, OBJ_TYPE)) {
        return ;
    }
    _gt_table_st * style = (_gt_table_st * )table;
    style->font_info.style.all = font_style;
    gt_event_send(table, GT_EVENT_TYPE_DRAW_START, NULL);
}

uint8_t gt_table_get_font_size(gt_obj_st * table)
{
    if (false == gt_obj_is_type(table, OBJ_TYPE)) {
        return 0;
    }
    _gt_table_st * style = (_gt_table_st * )table;
    return style->font_info.size;
}

void gt_table_set_font_info(gt_obj_st * table, gt_font_info_st * font_info)
{
    if (false == gt_obj_is_type(table, OBJ_TYPE)) {
        return ;
    }
    if (NULL == font_info) {
        return ;
    }
    _gt_table_st * style = (_gt_table_st * )table;
    gt_memcpy(&style->font_info, font_info, sizeof(gt_font_info_st));
}

gt_font_info_st * gt_table_get_font_info(gt_obj_st * table)
{
    if (false == gt_obj_is_type(table, OBJ_TYPE)) {
        return 0;
    }
    _gt_table_st * style = (_gt_table_st * )table;
    return &style->font_info;
}

void gt_table_set_header_bold(gt_obj_st * table, bool is_bold)
{
    if (false == gt_obj_is_type(table, OBJ_TYPE)) {
        return ;
    }
    _gt_table_st * style = (_gt_table_st * )table;
    style->font_info.style.reg.bold = is_bold;
}

#endif  /** GT_CFG_ENABLE_TABLE */
/* end ------------------------------------------------------------------*/
