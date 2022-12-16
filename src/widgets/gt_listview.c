/**
 * @file gt_listview.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-07-18 13:39:15
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_listview.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../font/gt_font.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_BTN
#define MY_CLASS    &gt_listview_class

/* private typedef ------------------------------------------------------*/


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct _gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_listview_class = {
    ._init_cb = _init_cb,
    ._deinit_cb = _deinit_cb,
    ._event_cb = _event_cb,
    .type = OBJ_TYPE,
    .size_instance = (sizeof(gt_obj_st)),
    .size_style = (sizeof(_gt_style_listview_st))
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/


static int _gt_listview_style_get_idx_start(gt_obj_st * listview)
{
    _gt_style_listview_st * style = listview->style;
    return style->idx_show_start;
}

static int _gt_listview_style_get_show_cnt(gt_obj_st * listview)
{
    _gt_style_listview_st * style = listview->style;
    return style->cnt_show;
}

static void _gt_listview_style_update_by_click_point(gt_obj_st * obj, gt_point_st point)
{
    int y_click     = point.y;
    int idx_start   = _gt_listview_style_get_idx_start(obj);
    int cnt_show    = _gt_listview_style_get_show_cnt(obj);
    int h_item      = (obj->area.h/cnt_show);
    int _cnt_dis_start = y_click/h_item;
    int idx_selected = idx_start + _cnt_dis_start;

    gt_listview_style_set_selected_item_by_idx(obj, idx_selected);
}

static void _gt_listview_style_update_by_scroll_y(gt_obj_st * obj, gt_size_t y_scroll)
{
    int idx_start   = _gt_listview_style_get_idx_start(obj);
    int cnt_show    = _gt_listview_style_get_show_cnt(obj);
    _gt_style_listview_st * style = (_gt_style_listview_st*)(obj->style);
    // int h_item      = (obj->area.h/cnt_show);
    // int _cnt_dis_start = -1 * (y_scroll/h_item);
    // int idx_selected = idx_start + _cnt_dis_start;

    style->content_offset_y += y_scroll;
    gt_listview_style_set_selected_item_by_idx(obj, idx_start);
}

static void _gt_listview_style_update(gt_obj_st * listview)
{
    _gt_style_listview_st * style = (_gt_style_listview_st*)(listview->style);

    // set default size
    if( listview->area.w == 0 || listview->area.h == 0 ){
        listview->area.w = style->font_size << 3;
        listview->area.h = (style->font_size + 16) * style->cnt_show;
    }

    // can not set area.h less than items numb * font size
    if( listview->area.h < (style->font_size * style->cnt_show) ){
        listview->area.h = (style->font_size + 2) * style->cnt_show;
    }

    printf("update style\n");

    style->content_offset_y = listview->area.y;
    style->idx_selected = 0;
}

static inline void _gt_listview_init_widget(gt_obj_st * listview) {
    _gt_style_listview_st * style = (_gt_style_listview_st * )listview->style;
    gt_size_t x, y, w, h;

    if (GT_STATE_UNSELECTED == style->idx_selected) {
        printf("init widget re-update\n");
        _gt_listview_style_update(listview);
    }

    // limit invalid values
    if (gt_abs(listview->process_attr.y_scroll) > GT_CFG_DEFAULT_POINT_SCROLL_PIXEL_INVALID) {
        listview->process_attr.y_scroll = listview->process_attr.y_scroll > 0 ? 16 : -16;
    }

    x = listview->area.x;
    y = listview->area.y;
    w = listview->area.w;
    h = listview->area.h;

    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.is_fill        = 1;
    rect_attr.border_width   = 2;
    rect_attr.bg_opa         = listview->opa;
    rect_attr.border_color  = style->border_color;
    rect_attr.fg_color      = style->color_ina;
    rect_attr.bg_color      = style->color_background;

    gt_area_st area_rect = {
        .w = w, .h = h
    };

    uint8_t fp[64];
    gt_font_st font = {
        .style_cn   = style->font_family_cn,
        .style_en   = style->font_family_en,
        .res        = fp,
        .size       = style->font_size,
    };

    gt_attr_font_st font_attr = {
        .font       = &font,
        .space_x    = 0,
        .space_y    = 0,
        .align      = GT_ALIGN_CENTER,
        .font_color = gt_color_black(),
        .opa        = listview->opa,
    };
    uint8_t idx_start = style->idx_show_start;
    uint8_t idx_end = style->idx_show_start + style->cnt_show;

    gt_area_st area_item = listview->area,  area_font;
    area_item.h = area_item.h/style->cnt_show;
    uint16_t width_font = 0;

    if( idx_end > (idx_start + style->cnt) ){
        idx_end = idx_start + style->cnt;
    }

    rect_attr.border_color  = style->border_color;
    rect_attr.bg_color      = style->color_ina;
    area_item.h *= style->cnt_show;
    draw_bg(listview->draw_ctx, &rect_attr, &area_item);

    rect_attr.bg_color      = style->color_act;
    area_item.h /= style->cnt_show;
    area_item.y += (style->cnt_show >> 1) * area_item.h;
    draw_bg(listview->draw_ctx, &rect_attr, &area_item);
    area_item.y -= (style->cnt_show >> 1) * area_item.h;

    area_font = area_item;
    area_font.y = style->content_offset_y;
    while( idx_start < idx_end ){
        // if( idx_start == style->idx_selected ){
        //     rect_attr.border_color  = style->border_color;
        //     rect_attr.bg_color      = style->color_act;
        //     color_font              = style->font_color_act;
        // }else{
        //     rect_attr.border_color  = style->border_color;
        //     rect_attr.bg_color      = style->color_ina;
        //     color_font              = style->font_color_ina;
        // }

        // draw_bg(listview->draw_ctx, &rect_attr, &area_item);

        font.utf8 = style->item[idx_start];
        font.len = strlen(style->item[idx_start]);
        width_font = gt_font_get_string_width(&font);
        draw_text(listview->draw_ctx, &font_attr, &area_font);

        // area_item.y += area_item.h;
        area_font.y += area_font.h;
        idx_start++;
    }

}

/**
 * @brief obj init listview widget cb
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");

    _gt_listview_init_widget(obj);
}

/**
 * @brief obj deinit cb
 *
 * @param obj
 */
static void _deinit_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "listview start deinit_cb");
    _gt_style_listview_st ** style_p = (_gt_style_listview_st ** )&obj->style;

    // release item memory



    GT_LOGV(GT_LOG_TAG_GUI, "end deinit_cb");
}


/**
 * @brief obj event handler cb
 *
 * @param obj
 * @param e event
 */
static void _event_cb(struct _gt_obj_s * obj, gt_event_st * e) {
    extern void _gt_disp_refr_append_area(gt_area_st * area);
    gt_event_type_et code = gt_event_get_code(e);

    switch(code) {
        case GT_EVENT_TYPE_DRAW_START:
            GT_LOGD(GT_LOG_TAG_GUI, "start draw");
            if( gt_obj_check_scr(obj) ){
                _gt_disp_refr_append_area(&obj->area);
            }
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;

        case GT_EVENT_TYPE_DRAW_END:
            GT_LOGD(GT_LOG_TAG_GUI, "end draw");
            break;

        case GT_EVENT_TYPE_CHANGE_CHILD_REMOVE: /* remove child from screen but not delete */
            GT_LOGD(GT_LOG_TAG_GUI, "child remove");
			break;

        case GT_EVENT_TYPE_CHANGE_CHILD_DELETE: /* delete child */
            GT_LOGD(GT_LOG_TAG_GUI, "child delete");
            break;

        case GT_EVENT_TYPE_INPUT_CLICKING:   /* add clicking style and process clicking event */
            GT_LOGD(GT_LOG_TAG_GUI, "clicking");
            break;

        case GT_EVENT_TYPE_INPUT_SCROLL:
            GT_LOGD(GT_LOG_TAG_GUI, "scroll");
            printf("scroll y:%d\n", obj->process_attr.y_scroll);
            _gt_listview_style_update_by_scroll_y(obj, obj->process_attr.y_scroll);

            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_PROCESS:   /* remove clicking style and process click event */
            GT_LOGD(GT_LOG_TAG_GUI, "process");
            /* code here */

            _gt_listview_style_update_by_click_point(obj, obj->process_attr.point);

            gt_event_send(obj, GT_EVENT_TYPE_INPUT_PROCESSED, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_PROCESSED: /* click event finish */
            GT_LOGD(GT_LOG_TAG_GUI, "processed");
            printf("processed\n");

            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_PROCESS_LOST: {
            GT_LOGD(GT_LOG_TAG_GUI, "process lost");
            printf("process_lost\n");

            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;
        }

        case GT_EVENT_TYPE_UPDATE_STYLE: {
            _gt_listview_style_update(obj);
            printf("this\n");
            break;
        }

        default:
            break;
    }
}


static void _gt_listview_init_style(gt_obj_st * listview)
{
    _gt_style_listview_st * style = listview->style;

    gt_memset(style, 0, sizeof(_gt_style_listview_st));

    style->font_color_act   = gt_color_hex(0xFFFFFF);
    style->font_color_ina   = gt_color_hex(0x000000);
    style->color_background = gt_color_hex(0xFFFFFF);
    style->border_color     = gt_color_hex(0x909399);
    style->color_act        = gt_color_hex(0x409EFF);
    style->color_ina        = gt_color_hex(0xFFFFFF);
    style->cnt_show = 5;
    style->idx_selected     = -1;
    style->content_offset_y =  listview->area.y;

    style->font_family_cn       = GT_CFG_DEFAULT_FONT_FAMILY_CN;
    style->font_family_en       = GT_CFG_DEFAULT_FONT_FAMILY_EN;
    style->font_size            = GT_CFG_DEFAULT_FONT_SIZE;
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


void gt_listview_style_add_item(gt_obj_st * listview, char * item)
{
    _gt_style_listview_st * style = listview->style;
    int len_item = strlen(item);
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

void gt_listview_style_set_selected_item_by_idx(gt_obj_st * listview, int idx){
    _gt_style_listview_st * style = listview->style;
    if( idx >= style->cnt ){
        // idx = style->cnt - 1;
        return;
    }
    if( idx < 0 ){
        // idx = 0;
        return;
    }
    // style->idx_selected = idx;

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


gt_size_t gt_listview_style_get_list_items_cnt(gt_obj_st * listview)
{
    _gt_style_listview_st * style = listview->style;
    return style->cnt;
}

gt_size_t gt_listview_style_get_idx_selected(gt_obj_st * listview)
{
    _gt_style_listview_st * style = listview->style;
    return style->idx_selected;
}

char * gt_listview_style_get_selected_item_text(gt_obj_st * listview)
{
    _gt_style_listview_st * style = listview->style;
    return style->item[style->idx_selected];
}
/* end ------------------------------------------------------------------*/
