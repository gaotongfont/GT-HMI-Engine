/**
 * @file gt_listview.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-07-18 13:39:15
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_listview.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../core/gt_obj_scroll.h"
#include "../font/gt_font.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"
/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_LISTVIEW
#define MY_CLASS    &gt_listview_class

/* private typedef ------------------------------------------------------*/
typedef struct _gt_listview_s
{
    char ** item;

    gt_color_t color_act;
    gt_color_t color_ina;
    gt_color_t color_background;

    gt_color_t border_color;
    uint16_t   border_width;

    gt_size_t idx_selected; // -1: init status; 0 or > 0: normal select status;
    gt_size_t idx_show_start;  //When displaying the contents of the list, start from the first few options
    gt_size_t cnt;
    gt_size_t cnt_show;    //When displaying the contents of the list, end of options
    gt_size_t content_offset_y;

    gt_color_t  font_color_act;
    gt_color_t  font_color_ina;

    gt_font_info_st font_info;

    uint8_t     font_align;
    uint8_t     space_x;
    uint8_t     space_y;

    uint8_t     scrolling;  /** scrolling can not selected by mouse up */
}_gt_listview_st;


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_listview_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_listview_st)
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/


static int _gt_listview_get_idx_start(gt_obj_st * listview)
{
    _gt_listview_st * style = listview->style;
    return style->idx_show_start;
}

static int _gt_listview_get_show_cnt(gt_obj_st * listview)
{
    _gt_listview_st * style = listview->style;
    return style->cnt_show;
}

static void _gt_listview_update_by_click_point(gt_obj_st * obj, gt_point_st point)
{
    int y_click     = point.y;
    int idx_start   = _gt_listview_get_idx_start(obj);
    int cnt_show    = _gt_listview_get_show_cnt(obj);
    int h_item      = (obj->area.h/cnt_show);
    int _cnt_dis_start = y_click/h_item;
    int idx_selected = idx_start + _cnt_dis_start;

    gt_listview_set_selected_item_by_idx(obj, idx_selected);
}

static void _gt_listview_update_by_scroll_y(gt_obj_st * obj, gt_size_t y_scroll)
{
    _gt_listview_st * style = (_gt_listview_st * )obj->style;

    if(style->cnt < style->cnt_show)
    {
        return;
    }

    int idx_start   = style->idx_show_start;
    int h_item      = (obj->area.h / style->cnt_show);

    style->content_offset_y = y_scroll;
    idx_start = (-style->content_offset_y) / h_item;

    if(idx_start < 0)
    {
        idx_start = 0;
    }
    else if(idx_start > style->cnt - style->cnt_show)
    {
        idx_start = style->cnt - style->cnt_show;
    }

    style->idx_show_start = idx_start;
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);

    // gt_listview_set_selected_item_by_idx(obj, idx_selected);
}

static void _gt_listview_update(gt_obj_st * listview)
{
    _gt_listview_st * style = (_gt_listview_st*)(listview->style);

    // set default size
    if( listview->area.w == 0 || listview->area.h == 0 ){
        listview->area.w = style->font_info.size << 3;
        listview->area.h = (style->font_info.size + 16) * style->cnt_show;
    }

    // can not set area.h less than items numb * font size
    if( listview->area.h < (style->font_info.size * style->cnt_show) ){
        listview->area.h = (style->font_info.size + 2) * style->cnt_show;
    }
}

static inline void _gt_listview_init_widget(gt_obj_st * listview) {
    _gt_listview_st * style = (_gt_listview_st * )listview->style;

    _gt_listview_update(listview);

    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.reg.is_fill    = 1;
    rect_attr.border_width   = 2;
    rect_attr.bg_opa         = listview->opa;
    rect_attr.border_color  = style->border_color;
    rect_attr.fg_color      = style->color_ina;
    rect_attr.bg_color      = style->color_background;

    gt_font_st font = {
        .info = style->font_info,
        .res  = NULL,

    };
    font.info.thick_en = style->font_info.thick_en == 0 ? style->font_info.size + 6: style->font_info.thick_en;
    font.info.thick_cn = style->font_info.thick_cn == 0 ? style->font_info.size + 6: style->font_info.thick_cn;
    font.info.encoding = gt_project_encoding_get();

    gt_attr_font_st font_attr = {
        .font       = &font,
        .space_x    = style->space_x,
        .space_y    = style->space_y,
        .align      = style->font_align,
        .font_color = gt_color_black(),
        .opa        = listview->opa,
    };
    uint8_t idx_start = style->idx_show_start;
    uint8_t idx_end = style->idx_show_start + (style->cnt_show==1?1:style->cnt_show);
    gt_color_t color_font;


    if( idx_end > (idx_start + style->cnt) ){
        idx_end = idx_start + style->cnt;
    }

    gt_area_st area_item = gt_area_reduce(listview->area , REDUCE_DEFAULT),  area_font;
    // 外框
    draw_bg(listview->draw_ctx, &rect_attr, &area_item);

    // item
    area_item.h = area_item.h/style->cnt_show;
    while( idx_start < idx_end ){
        if( idx_start == style->idx_selected ){
            rect_attr.border_color  = style->border_color;
            rect_attr.bg_color      = style->color_act;
            color_font              = style->font_color_act;
        }else{
            rect_attr.border_color  = style->border_color;
            rect_attr.bg_color      = style->color_ina;
            color_font              = style->font_color_ina;
        }

        draw_bg(listview->draw_ctx, &rect_attr, &area_item);

        area_font = gt_area_reduce(area_item , REDUCE_DEFAULT);
        font.utf8 = style->item[idx_start];
        font.len = strlen(style->item[idx_start]);
        font_attr.font_color = color_font;
        draw_text(listview->draw_ctx, &font_attr, &area_font);
        area_item.y += area_item.h;
        idx_start++;
    }

    // focus
    draw_focus(listview , 0);
}

/**
 * @brief obj init listview widget call back
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");

    _gt_listview_init_widget(obj);
}

/**
 * @brief obj deinit call back
 *
 * @param obj
 */
static void _deinit_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "listview start deinit_cb");
    _gt_listview_st * style_p = (_gt_listview_st * )obj->style;
    int16_t i = 0;

    // release item memory
    if (style_p->item) {
        for (i = style_p->cnt - 1; i >= 0; i--) {
            if (NULL == style_p->item[i]) {
                continue;
            }
            gt_mem_free(style_p->item[i]);
            style_p->item[i] = NULL;
        }
        gt_mem_free(style_p->item);
        style_p->item = NULL;
        style_p->cnt = 0;
    }



    GT_LOGV(GT_LOG_TAG_GUI, "end deinit_cb");
}


/**
 * @brief obj event handler call back
 *
 * @param obj
 * @param e event
 */
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code = gt_event_get_code(e);
    _gt_listview_st * style = (_gt_listview_st * )obj->style;

    switch(code) {
        case GT_EVENT_TYPE_DRAW_START: {
            gt_disp_invalid_area(obj);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;
        }
        case GT_EVENT_TYPE_INPUT_SCROLL_START: {
            style->scrolling = true;
            break;
        }
        case GT_EVENT_TYPE_INPUT_SCROLL: {
            if (gt_obj_scroll_get_y(obj) > 0) {
                obj->process_attr.scroll.y = 0;
            }
            else if (-gt_obj_scroll_get_y(obj) > (obj->area.h / style->cnt_show) * (style->cnt - style->cnt_show)) {
                obj->process_attr.scroll.y = -(obj->area.h / style->cnt_show) * (style->cnt - style->cnt_show);
            }
            _gt_listview_update_by_scroll_y(obj, gt_obj_scroll_get_y(obj));

            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;
        }
        case GT_EVENT_TYPE_INPUT_PRESSED: {
            style->scrolling = false;
            break;
        }
        case GT_EVENT_TYPE_INPUT_RELEASED: {
            /* click event finish */
            if (style->scrolling) {
                style->scrolling = false;
                break;
            }
            style->scrolling = false;
            _gt_listview_update_by_click_point(obj, obj->process_attr.point);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;
        }
        case GT_EVENT_TYPE_INPUT_PROCESS_LOST: {
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;
        }
        case GT_EVENT_TYPE_UPDATE_STYLE: {
            _gt_listview_update(obj);
            break;
        }
        case GT_EVENT_TYPE_INPUT_SCROLL_LEFT:
        case GT_EVENT_TYPE_INPUT_SCROLL_UP: {
            break;
        }
        case GT_EVENT_TYPE_INPUT_SCROLL_RIGHT:
        case GT_EVENT_TYPE_INPUT_SCROLL_DOWN: {
            break;
        }

        default:
            break;
    }
}


static void _gt_listview_init_style(gt_obj_st * listview)
{
    _gt_listview_st * style = (_gt_listview_st * )listview->style;

    gt_memset(style, 0, sizeof(_gt_listview_st));

    style->font_color_act   = gt_color_hex(0xFFFFFF);
    style->font_color_ina   = gt_color_hex(0x000000);
    style->color_background = gt_color_hex(0xFFFFFF);
    style->border_color     = gt_color_hex(0x909399);
    style->color_act        = gt_color_hex(0x409EFF);
    style->color_ina        = gt_color_hex(0xFFFFFF);
    style->cnt_show = 5;
    style->idx_selected     = -1;
    style->content_offset_y = 0;

    style->font_info.style_cn       = GT_CFG_DEFAULT_FONT_FAMILY_CN;
    style->font_info.style_en       = GT_CFG_DEFAULT_FONT_FAMILY_EN;
    style->font_info.style_fl       = GT_CFG_DEFAULT_FONT_FAMILY_FL;
    style->font_info.style_numb     = GT_CFG_DEFAULT_FONT_FAMILY_NUMB;
    style->font_info.size            = GT_CFG_DEFAULT_FONT_SIZE;
    style->font_info.gray            = 1;
    style->font_align           = GT_ALIGN_CENTER_MID;
    style->font_info.thick_en             = 0;
    style->font_info.thick_cn             = 0;
    style->space_x              = 0;
    style->space_y              = 0;
}




/* global functions / API interface -------------------------------------*/

/**
 * @brief create a listview obj
 *
 * @param parent listview's parent element
 * @return gt_obj_st* listview obj
 */
gt_obj_st * gt_listview_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    _gt_listview_init_style(obj);
    return obj;
}


void gt_listview_add_item(gt_obj_st * listview, char * item)
{
    _gt_listview_st * style = listview->style;
    int len_item = item == NULL ? 0 : strlen(item);
    if( style->cnt == 0 ){
        style->item = gt_mem_malloc(sizeof(char *));
        style->item[0] = gt_mem_malloc( len_item + 1 );
        gt_memcpy(style->item[0], item, len_item);
        style->item[0][len_item] = 0;

        style->cnt = 1;
    }else{
        style->item = gt_mem_realloc(style->item, (style->cnt+1) * sizeof(char *));
        style->item[style->cnt] = gt_mem_malloc( len_item + 1 );
        gt_memcpy(style->item[style->cnt], item, len_item);
        style->item[style->cnt][len_item] = 0;

        style->cnt++;
    }
}

void gt_listview_set_selected_item_by_idx(gt_obj_st * listview, int idx){
    _gt_listview_st * style = listview->style;
    if( idx >= style->cnt ){
        // idx = style->cnt - 1;
        return;
    }
    if( idx < 0 ){
        // idx = 0;
        return;
    }
    style->idx_selected = idx;

    // if( style->idx_selected >= (style->cnt_show/2) ){
    //     style->idx_show_start = style->idx_selected - (style->cnt_show/2);
    // }

    // if( (style->idx_show_start + style->cnt_show) > style->cnt ){
    //     if( style->cnt >= style->cnt_show ){
    //         style->idx_show_start = style->cnt - style->cnt_show;
    //     }else{
    //         style->idx_show_start = 0;
    //     }
    // }

    gt_event_send(listview, GT_EVENT_TYPE_DRAW_START, NULL);
}


gt_size_t gt_listview_get_list_items_cnt(gt_obj_st * listview)
{
    _gt_listview_st * style = listview->style;
    return style->cnt;
}

gt_size_t gt_listview_get_idx_selected(gt_obj_st * listview)
{
    _gt_listview_st * style = listview->style;
    return style->idx_selected;
}

char * gt_listview_get_selected_item_text(gt_obj_st * listview)
{
    _gt_listview_st * style = listview->style;
    return style->item[style->idx_selected];
}

void gt_listview_set_cnt_show(gt_obj_st * listview, uint8_t cnt_show)
{
    _gt_listview_st * style = listview->style;
    style->cnt_show = cnt_show;
}

void gt_listview_set_font_size(gt_obj_st * listview, uint8_t size)
{
    _gt_listview_st * style = listview->style;
    style->font_info.size = size;
}
void gt_listview_set_font_gray(gt_obj_st * listview, uint8_t gray)
{
    _gt_listview_st * style = listview->style;
    style->font_info.gray = gray;
}
void gt_listview_set_font_align(gt_obj_st * listview, uint8_t align)
{
    _gt_listview_st * style = listview->style;
    style->font_align = align;
}
void gt_listview_set_font_color(gt_obj_st * listview, gt_color_t color)
{
    _gt_listview_st * style = listview->style;
    style->font_color_ina = color;
}

void gt_listview_set_font_family_en(gt_obj_st * listview, gt_family_t family)
{
    _gt_listview_st * style = listview->style;
    style->font_info.style_en = family;
}
void gt_listview_set_font_family_fl(gt_obj_st * listview, gt_family_t family)
{
    _gt_listview_st * style = listview->style;
    style->font_info.style_fl = family;
}
void gt_listview_set_font_family_cn(gt_obj_st * listview, gt_family_t family)
{
    _gt_listview_st * style = listview->style;
    style->font_info.style_cn = family;
}

void gt_listview_set_font_family_numb(gt_obj_st * listview, gt_family_t family)
{
    _gt_listview_st * style = listview->style;
    style->font_info.style_numb = family;
}

void gt_listview_set_font_thick_en(gt_obj_st * listview, uint8_t thick)
{
    _gt_listview_st * style = (_gt_listview_st * )listview->style;
    style->font_info.thick_en = thick;
}
void gt_listview_set_font_thick_cn(gt_obj_st * listview, uint8_t thick)
{
    _gt_listview_st * style = (_gt_listview_st * )listview->style;
    style->font_info.thick_cn = thick;
}
void gt_listview_set_space(gt_obj_st * listview, uint8_t space_x, uint8_t space_y)
{
    _gt_listview_st * style = (_gt_listview_st * )listview->style;
    style->space_x = space_x;
    style->space_y = space_y;
}
/* end ------------------------------------------------------------------*/
