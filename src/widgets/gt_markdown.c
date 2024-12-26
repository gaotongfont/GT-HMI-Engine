/**
 * @file gt_markdown.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2024-12-10 10:24:48
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "./gt_markdown.h"

#if GT_CFG_ENABLE_MARKDOWN
#include "stdio.h"
#include "../core/gt_draw.h"
#include "../core/gt_mem.h"
#include "../core/gt_disp.h"
#include "../core/gt_obj_scroll.h"
#include "../others/gt_log.h"
#include "./gt_textarea.h"
#include "../extra/md4c/md4c.h"
#include "./gt_label.h"
#include "./gt_textarea.h"
#include "./gt_line.h"



/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_MARKDOWN
#define MY_CLASS    &gt_markdown_class

#define _MARK_DOWN_MAX_ACT_COUNT    (10)

#define _MARK_DOWN_LARGE_FONT_SIZE  (28)
#define _MARK_DOWN_MIDDLE_FONT_SIZE (20)
#define _MARK_DOWN_SMALL_FONT_SIZE  (16)

#define _MARK_DOWN_LOG              0

/* private typedef ------------------------------------------------------*/
typedef struct _gt_markdown_s {
    gt_obj_st obj;
    MD_PARSER md_parser;
    gt_font_info_st font_info_large;
    gt_font_info_st font_info_middle;
    gt_font_info_st font_info_small;

    gt_color_t color_border;
    uint8_t border_width;
}_gt_markdown_st;

typedef struct {
    MD_BLOCKTYPE block;
    MD_SPANTYPE span;
    MD_TEXTTYPE text;
    uint16_t item_count;
}_gt_markdown_type_st;

typedef struct {
    gt_obj_st * markdown_p;
    gt_obj_st * act_p[_MARK_DOWN_MAX_ACT_COUNT];
    uint16_t act_idx;
    gt_point_st offset;
    gt_font_style_et font_style;

    _gt_markdown_type_st prop;
}_gt_markdown_ptr_st;

typedef struct {
    void * detail;
    void * userdata;
    uint8_t type;
}_gt_markdown_cb_params_st;

/* static variables -----------------------------------------------------*/
static void _markdown_init_cb(gt_obj_st * obj);
static void _markdown_deinit_cb(gt_obj_st * obj);
static void _markdown_event_cb(struct gt_obj_s * obj, gt_event_st * e);

static GT_ATTRIBUTE_RAM_DATA const gt_obj_class_st gt_markdown_class = {
    ._init_cb      = _markdown_init_cb,
    ._deinit_cb    = _markdown_deinit_cb,
    ._event_cb     = _markdown_event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_markdown_st)
};


/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static void _markdown_init_cb(gt_obj_st * obj) {
    _gt_markdown_st * style = (_gt_markdown_st * )obj;
    gt_attr_rect_st rect_attr = {0};
    if (obj->show_bg) {
        gt_graph_init_rect_attr(&rect_attr);
        rect_attr.reg.is_fill   = true;
        rect_attr.radius        = obj->radius;
        rect_attr.bg_opa        = obj->opa;
        rect_attr.border_width  = style->border_width;
        rect_attr.fg_color      = obj->bgcolor;
        rect_attr.bg_color      = obj->bgcolor;
        rect_attr.border_color  = style->color_border;

        gt_area_st area = gt_area_reduce(obj->area, gt_obj_get_reduce(obj));
        draw_bg(obj->draw_ctx, &rect_attr, &area);
    }
    draw_focus(obj, obj->radius);
}

static void _markdown_deinit_cb(gt_obj_st * obj) {
}

static void _markdown_event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code_val = gt_event_get_code(e);

    switch(code_val) {
        case GT_EVENT_TYPE_INPUT_SCROLL: {
            _gt_obj_scroll_internal(obj);
            break;
        }
        case GT_EVENT_TYPE_DRAW_START: {
            gt_disp_invalid_area(obj);
            break;
        }
        default:
            break;
    }
}

static GT_ATTRIBUTE_RAM_TEXT gt_res_t _common_set_text(_gt_markdown_ptr_st * ptr, const char * text, uint32_t len) {
    gt_obj_st * obj = ptr->act_p[ptr->act_idx];
    gt_res_t ret = GT_RES_OK;
    if (NULL == obj || NULL == text) {
        return GT_RES_FAIL;
    }
    switch (gt_obj_class_get_type(obj)) {
        case GT_TYPE_LAB: {
            gt_label_set_text_by_len(obj, text, len);
            break;
        }
        case GT_TYPE_TEXTAREA: {
            gt_textarea_add_str_by_len(obj, (char * )text, len, ptr->font_style, gt_color_black());
            break;
        }
        default:
            ret = GT_RES_FAIL;
            break;
    }
    return ret;
}

static GT_ATTRIBUTE_RAM_TEXT int
_enter_md_block_default(_gt_markdown_cb_params_st * pm) {
    _gt_markdown_ptr_st * ptr = (_gt_markdown_ptr_st *)pm->userdata;
    _gt_markdown_st * style = (_gt_markdown_st * )ptr->markdown_p;

    ptr->prop.block = pm->type;
    ptr->prop.item_count = 0;

    ptr->act_p[ptr->act_idx] = gt_textarea_create(ptr->markdown_p);
    gt_obj_set_pos(ptr->act_p[ptr->act_idx], 0, ptr->offset.y + ptr->markdown_p->area.y);
    gt_obj_set_size(ptr->act_p[ptr->act_idx], ptr->markdown_p->area.w, 16);
    gt_obj_show_bg(ptr->act_p[ptr->act_idx], false);
    gt_obj_set_reduce(ptr->act_p[ptr->act_idx], 0);
    gt_obj_set_touch_parent(ptr->act_p[ptr->act_idx], true);
    gt_obj_set_fixed(ptr->act_p[ptr->act_idx], true);

    gt_textarea_set_font_info(ptr->act_p[ptr->act_idx], &style->font_info_small);
    gt_textarea_set_space(ptr->act_p[ptr->act_idx], 0, 8);

#if _MARK_DOWN_LOG
    GT_LOG_A("", "enter block p[default] %p act: %p size: %d %d<<<<", ptr->markdown_p, ptr->act_p[ptr->act_idx], ptr->markdown_p->area.w, ptr->offset.y);
#endif

    return 0;
}

static GT_ATTRIBUTE_RAM_TEXT int
_enter_md_block_h(_gt_markdown_cb_params_st * pm) {
    MD_BLOCK_H_DETAIL * det = (MD_BLOCK_H_DETAIL *)pm->detail;
    _gt_markdown_ptr_st * ptr = (_gt_markdown_ptr_st *)pm->userdata;
    _gt_markdown_st * style = (_gt_markdown_st * )ptr->markdown_p;

    _enter_md_block_default(pm);

    if (det->level < 3) {
        /** H1, H2 */
        gt_textarea_set_font_info(ptr->act_p[ptr->act_idx], &style->font_info_large);
    } else if (det->level < 4) {
        /** H3 */
        gt_textarea_set_font_info(ptr->act_p[ptr->act_idx], &style->font_info_middle);
    }

#if _MARK_DOWN_LOG
    GT_LOG_A("", "enter block h %d %p act: %p %d %d<<<<", det->level, ptr->markdown_p, ptr->act_p[ptr->act_idx], 0, ptr->offset.y);
#endif

    return 0;
}

static GT_ATTRIBUTE_RAM_TEXT int
_enter_md_block_li(_gt_markdown_cb_params_st * pm) {
    _gt_markdown_ptr_st * ptr = (_gt_markdown_ptr_st *)pm->userdata;
    _gt_markdown_st * style = (_gt_markdown_st * )ptr->markdown_p;
    MD_BLOCK_LI_DETAIL * detail = (MD_BLOCK_LI_DETAIL *)pm->detail;
    char buf[256] = {0};
    gt_size_t len = 0;

    ++ptr->prop.item_count;
    ptr->act_p[ptr->act_idx] = ptr->markdown_p->child[ptr->markdown_p->cnt_child - 1];

    if (gt_obj_is_type(ptr->act_p[ptr->act_idx], GT_TYPE_TEXTAREA) && gt_textarea_get_str_count(ptr->act_p[ptr->act_idx])) {
        gt_textarea_add_str_by_len(ptr->act_p[ptr->act_idx], "\n", 1, GT_FONT_STYLE_NONE, gt_color_black());
    }

#if _MARK_DOWN_LOG
    GT_LOG_A("", "enter block %d li %p act: %p <<<<", ptr->prop.block, ptr->markdown_p, ptr->act_p[ptr->act_idx]);
#endif

    switch (ptr->prop.block) {
        case MD_BLOCK_UL: {
            snprintf(buf, sizeof(buf), " %d. ", ptr->prop.item_count);
            break;
        }
        case MD_BLOCK_OL: {
            snprintf(buf, sizeof(buf), " · ");
            break;
        }
        default:
            return 0;
    }

    len = strlen(buf);
    if (len <= 0) { return 0; }

    gt_textarea_add_str_by_len(ptr->act_p[ptr->act_idx], buf, len, GT_FONT_STYLE_NONE, gt_color_black());

    return 0;
}

static GT_ATTRIBUTE_RAM_TEXT int
_enter_md_block_hr(_gt_markdown_cb_params_st * pm) {
    _gt_markdown_ptr_st * ptr = (_gt_markdown_ptr_st *)pm->userdata;
    _gt_markdown_st * style = (_gt_markdown_st * )ptr->markdown_p;
    uint16_t padding = 4;

    ptr->act_p[ptr->act_idx] = gt_line_create(ptr->markdown_p);
    gt_line_set_color(ptr->act_p[ptr->act_idx], gt_color_hex(0x7f7f7f));
    gt_obj_set_pos(ptr->act_p[ptr->act_idx], padding, 0);
    gt_line_set_line_width(ptr->act_p[ptr->act_idx], 2);
    gt_line_set_hor_line(ptr->act_p[ptr->act_idx], ptr->markdown_p->area.w - (padding << 1));
}

static GT_ATTRIBUTE_RAM_TEXT int
_enter_block_cb(MD_BLOCKTYPE type, void * detail, void * userdata) {
    int ret = 0;
    _gt_markdown_ptr_st * ptr = (_gt_markdown_ptr_st *)userdata;
    ++ptr->act_idx;
#if _MARK_DOWN_LOG
    GT_LOG_A("", "enter block %d [%d]", ptr->act_idx, type);
#endif

    _gt_markdown_cb_params_st params = {
        .type = (uint8_t)type,
        .detail = detail,
        .userdata = userdata
    };
    switch (type) {
        case MD_BLOCK_P: {  // 9
            ret = _enter_md_block_default(&params);
            break;
        }
        case MD_BLOCK_H: {  // 6
            ret = _enter_md_block_h(&params);
            break;
        }
        case MD_BLOCK_UL: { // 2
            ret = _enter_md_block_default(&params);
            break;
        }
        case MD_BLOCK_OL: { // 3
            ret = _enter_md_block_default(&params);
            break;
        }
        case MD_BLOCK_LI: { // 4
            ret = _enter_md_block_li(&params);
            break;
        }
        case MD_BLOCK_HR: {
            ret = _enter_md_block_hr(&params);
            break;
        }
        case MD_BLOCK_TABLE: {  // value is null
        MD_BLOCK_TABLE_DETAIL * det = (MD_BLOCK_TABLE_DETAIL *)detail;
            // ret = _enter_md_block_default(&params);
#if _MARK_DOWN_LOG
            GT_LOG_A("", ">>> table col: %d header row: %d, body row: %d", det->col_count, det->head_row_count, det->body_row_count);
#endif
            break;
        }
        case MD_BLOCK_THEAD: {  // is null, tell header row
            // ret = _enter_md_block_default(&params);
#if _MARK_DOWN_LOG
            GT_LOG_A("", ">>> thead");
#endif
            break;
        }
        case MD_BLOCK_TBODY: {  // is null, tell body row
            // ret = _enter_md_block_default(&params);
#if _MARK_DOWN_LOG
            GT_LOG_A("", ">>> tbody");
#endif
            break;
        }
        case MD_BLOCK_TR: { // turn new row
            // ret = _enter_md_block_default(&params);
#if _MARK_DOWN_LOG
            GT_LOG_A("", ">>> tr");
#endif
            break;
        }
        case MD_BLOCK_TH: { // | header title
        MD_BLOCK_TD_DETAIL * det = (MD_BLOCK_TD_DETAIL *)detail;
#if _MARK_DOWN_LOG
            GT_LOG_A("", ">>> th algin: %d", det->align);
#endif
            ret = _enter_md_block_default(&params);
            break;
        }
        case MD_BLOCK_TD: { // | body content
        MD_BLOCK_TD_DETAIL * det = (MD_BLOCK_TD_DETAIL *)detail;
#if _MARK_DOWN_LOG
            GT_LOG_A("", ">>> td algin: %d", det->align);
#endif
            ret = _enter_md_block_default(&params);
            break;
        }
        default:
            break;
    }
    return ret;
}

static GT_ATTRIBUTE_RAM_TEXT int
_leave_block_cb(MD_BLOCKTYPE type, void * detail, void * userdata) {
    int ret = 0;
    _gt_markdown_ptr_st * ptr = (_gt_markdown_ptr_st *)userdata;
    gt_obj_st * child = NULL;
    _gt_markdown_cb_params_st params = {
        .type = (uint8_t)type,
        .detail = detail,
        .userdata = userdata
    };
#if _MARK_DOWN_LOG
    GT_LOG_A("", "leave block %d [%d]", ptr->act_idx, type);
#endif
    if (gt_obj_is_type(ptr->act_p[ptr->act_idx], GT_TYPE_TEXTAREA)) {
        uint32_t height = gt_textarea_get_total_height(ptr->act_p[ptr->act_idx]);
#if _MARK_DOWN_LOG
        GT_LOG_A("", "   height: %d", height);
#endif
        gt_obj_set_h(ptr->act_p[ptr->act_idx], height);
    }

    if (MD_BLOCK_DOC == type) {
        /** resize all widget offset and markdown height */
        ptr->offset.y = ptr->markdown_p->area.y;
        for (gt_size_t i = 0; i < ptr->markdown_p->cnt_child; ++i) {
            child = ptr->markdown_p->child[i];
            gt_obj_set_pos(child, 0, ptr->offset.y);
            ptr->offset.y += gt_obj_get_h(child);
        }
    }
    ptr->act_p[ptr->act_idx] = NULL;
    --ptr->act_idx;
    return ret;
}

static GT_ATTRIBUTE_RAM_TEXT int
_enter_span_cb(MD_SPANTYPE type, void * detail, void * userdata) {
    int ret = 0;
    _gt_markdown_ptr_st * ptr = (_gt_markdown_ptr_st *)userdata;
#if _MARK_DOWN_LOG
    GT_LOG_A("", " enter span %d [%d]", ptr->act_idx, type);
#endif
    _gt_markdown_cb_params_st params = {
        .type = (uint8_t)type,
        .detail = detail,
        .userdata = userdata
    };
    switch (type) {
        case MD_SPAN_STRONG: {
#if _MARK_DOWN_LOG
            // GT_LOG_A("", " strong text");
#endif
            break;
        }
        case MD_SPAN_A: {
            // MD_SPAN_A_DETAIL * det = (MD_SPAN_A_DETAIL *)detail;
#if _MARK_DOWN_LOG
            // GT_LOG_A("", " linking text: %s, %d", det->href.text, det->title.substr_offsets);
#endif
            break;
        }
        default:
            break;
    }
    return ret;
}

static GT_ATTRIBUTE_RAM_TEXT int
_leave_span_cb(MD_SPANTYPE type, void * detail, void * userdata) {
    int ret = 0;
    _gt_markdown_ptr_st * ptr = (_gt_markdown_ptr_st *)userdata;
    _gt_markdown_cb_params_st params = {
        .type = (uint8_t)type,
        .detail = detail,
        .userdata = userdata
    };
#if _MARK_DOWN_LOG
    GT_LOG_A("", " leave span %d, [%d]", ptr->act_idx, type);
#endif
    return ret;
}

static GT_ATTRIBUTE_RAM_TEXT int
_text_cb(MD_TEXTTYPE type, const MD_CHAR * text, MD_SIZE size, void * userdata) {
    int ret = 0;
    _gt_markdown_ptr_st * ptr = (_gt_markdown_ptr_st *)userdata;
    if (NULL == ptr->act_p[ptr->act_idx]) {
        return ret;
    }

#if _MARK_DOWN_LOG
    MD_CHAR buf[512] = {0};
    snprintf(buf, size + 1, "%s", text);
    GT_LOG_A("", "  type: %d len: %d[%d] text: [%s]", type, size, sizeof(buf), buf);
#endif
    if (GT_RES_OK!= _common_set_text(ptr, (const char * )text, size)) {
        return ret;
    }
    return ret;
}

static GT_ATTRIBUTE_RAM_TEXT void
_markdown_init_flags(_gt_markdown_st * style) {
    style->md_parser.flags = 0;
    style->md_parser.flags |= MD_FLAG_TABLES;
    style->md_parser.flags |= MD_FLAG_NOHTML;
    // style->md_parser.flags |= MD_FLAG_UNDERLINE;
    style->md_parser.flags |= MD_FLAG_STRIKETHROUGH;
    // style->md_parser.flags |= MD_FLAG_PERMISSIVEAUTOLINKS;
}

static GT_ATTRIBUTE_RAM_TEXT void
_markdown_init_callback(_gt_markdown_st * style) {
    style->md_parser.enter_block = _enter_block_cb;
    style->md_parser.leave_block = _leave_block_cb;
    style->md_parser.enter_span = _enter_span_cb;
    style->md_parser.leave_span = _leave_span_cb;
    style->md_parser.text = _text_cb;
}

static GT_ATTRIBUTE_RAM_TEXT void
_set_font_info_by(gt_font_info_st * font_info, uint16_t font_size) {
#if GT_FONT_FAMILY_OLD_ENABLE

#else
    gt_size_t font_fam_id = gt_font_family_get_id_by(font_size);
    if (-1 == font_fam_id) {
        return ;
    }
    gt_font_set_family(font_info, font_fam_id);
    font_info->size = font_size;
#endif
}

/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_markdown_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    if (NULL == obj) {
        return obj;
    }

    _gt_markdown_st * style = (_gt_markdown_st *)obj;
    obj->area.w = 200;
    obj->area.h = 200;
    obj->fixed = false;
    obj->area.w = gt_disp_get_res_hor(NULL);
    obj->scroll_dir = GT_SCROLL_VERTICAL;
    obj->show_bg = true;

    gt_font_info_init(&style->font_info_large);
    _set_font_info_by(&style->font_info_large, _MARK_DOWN_LARGE_FONT_SIZE);

    gt_font_info_init(&style->font_info_middle);
    _set_font_info_by(&style->font_info_middle, _MARK_DOWN_MIDDLE_FONT_SIZE);

    gt_font_info_init(&style->font_info_small);
    _set_font_info_by(&style->font_info_small, _MARK_DOWN_SMALL_FONT_SIZE);

    _markdown_init_flags(style);
    _markdown_init_callback(style);

    return obj;
}

void gt_markdown_set_text(gt_obj_st * markdown, const char * text, uint32_t len)
{
    if (false == gt_obj_is_type(markdown, OBJ_TYPE)) {
        return;
    }
    _gt_markdown_st * style = (_gt_markdown_st *)markdown;
    _gt_markdown_ptr_st root = {
        .markdown_p = markdown,
    };
    if (markdown->cnt_child) {
        _gt_obj_class_destroy_children(markdown);
    }
    md_parse(text, len, &style->md_parser, &root);
    gt_event_send(markdown, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_markdown_set_font_info_large(gt_obj_st * markdown, gt_font_info_st * font_info)
{
    if (false == gt_obj_is_type(markdown, OBJ_TYPE)) {
        return;
    }
    if (NULL == font_info) {
        return ;
    }
    _gt_markdown_st * style = (_gt_markdown_st *)markdown;
    style->font_info_large = *font_info;
}

void gt_markdown_set_font_info_middle(gt_obj_st * markdown, gt_font_info_st * font_info)
{
    if (false == gt_obj_is_type(markdown, OBJ_TYPE)) {
        return;
    }
    if (NULL == font_info) {
        return ;
    }
    _gt_markdown_st * style = (_gt_markdown_st *)markdown;
    style->font_info_middle = *font_info;
}

void gt_markdown_set_font_info_small(gt_obj_st * markdown, gt_font_info_st * font_info)
{
    if (false == gt_obj_is_type(markdown, OBJ_TYPE)) {
        return;
    }
    if (NULL == font_info) {
        return ;
    }
    _gt_markdown_st * style = (_gt_markdown_st *)markdown;
    style->font_info_small = *font_info;
}

gt_font_info_st * gt_markdown_get_font_info_large(gt_obj_st * markdown)
{
    if (false == gt_obj_is_type(markdown, OBJ_TYPE)) {
        return NULL;
    }
    _gt_markdown_st * style = (_gt_markdown_st *)markdown;
    return &style->font_info_large;
}

gt_font_info_st * gt_markdown_get_font_info_middle(gt_obj_st * markdown)
{
    if (false == gt_obj_is_type(markdown, OBJ_TYPE)) {
        return NULL;
    }
    _gt_markdown_st * style = (_gt_markdown_st *)markdown;
    return &style->font_info_middle;
}

gt_font_info_st * gt_markdown_get_font_info_small(gt_obj_st * markdown)
{
    if (false == gt_obj_is_type(markdown, OBJ_TYPE)) {
        return NULL;
    }
    _gt_markdown_st * style = (_gt_markdown_st *)markdown;
    return &style->font_info_small;
}

#endif  /** GT_CFG_ENABLE_MARKDOWN */
/* end ------------------------------------------------------------------*/
