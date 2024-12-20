/**
 * @file gt_table.c
 * @author Feyoung
 * @brief
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


/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_TABLE
#define MY_CLASS    &gt_table_class


/* private typedef ------------------------------------------------------*/
typedef struct _gt_table_col_s {
    gt_align_et align;
    uint16_t max_width;
}_gt_table_col_st;

typedef struct _gt_table_s {
    gt_obj_st obj;

    gt_color_t color_border;
    uint8_t border_width;

    _gt_table_col_st * col_list;   /** count by header items count */
    _gt_vector_st * header;
    _gt_vector_st * contents;
}_gt_table_st;


/* static variables -----------------------------------------------------*/
static void _table_init_cb(gt_obj_st * obj);
static void _table_deinit_cb(gt_obj_st * obj);
static void _table_event_cb(struct gt_obj_s * obj, gt_event_st * e);

static GT_ATTRIBUTE_RAM_DATA const gt_obj_class_st gt_table_class = {
    ._init_cb      = _table_init_cb,
    ._deinit_cb    = _table_deinit_cb,
    ._event_cb     = _table_event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_table_st)
};


/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static void _table_init_cb(gt_obj_st * obj) {
    _gt_table_st * style = (_gt_table_st * )obj;
    gt_attr_rect_st rect_attr = {0};
    if (obj->show_bg) {
        gt_graph_init_rect_attr(&rect_attr);
        rect_attr.reg.is_fill   = true;
        rect_attr.radius        = obj->radius;
        rect_attr.bg_opa        = obj->opa;
        rect_attr.fg_color      = obj->bgcolor;
        rect_attr.bg_color      = obj->bgcolor;
        rect_attr.border_width  = style->border_width;
        rect_attr.border_color  = style->color_border;

        gt_area_st area = gt_area_reduce(obj->area, gt_obj_get_reduce(obj));
        draw_bg(obj->draw_ctx, &rect_attr, &area);
    }

    // line

    // header

    // content


    // GT_LOG_A("", ">>> %d, %d", _gt_vector_get_count(style->header), _gt_vector_get_count(style->contents));

    draw_focus(obj, obj->radius);
}

static void _table_deinit_cb(gt_obj_st * obj) {
    _gt_table_st * style = (_gt_table_st * )obj;
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

static GT_ATTRIBUTE_RAM_TEXT gt_res_t
_calc_each_column_display_max_width(gt_obj_st * table) {

    return GT_RES_OK;
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
    obj->area.h = 200;

    _gt_table_st * style = (_gt_table_st * )obj;
    style->color_border = gt_color_hex(0x7f7f7f);
    style->border_width = 1;
    style->header = _gt_vector_create(_str_free_item_cb, _str_equal_cb);
    style->contents = _gt_vector_create(_str_free_item_cb, _str_equal_cb);
    return obj;
}

gt_res_t gt_table_add_header_text_by_len(gt_obj_st * table, char * text, uint16_t len, gt_align_et align)
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

gt_res_t gt_table_add_content_text_by_len(gt_obj_st * table, char * text, uint16_t len)
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
    return GT_RES_OK;

free_text_lb:
    gt_mem_free(content);
    content = NULL;
    return GT_RES_FAIL;
}

#endif  /** GT_CFG_ENABLE_TABLE */
/* end ------------------------------------------------------------------*/
