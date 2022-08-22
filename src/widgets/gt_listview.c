/**
 * @file gt_listviewview.c
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
static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class);
static void _deinit_cb(gt_obj_st * obj,const gt_obj_class_st * class);
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
    return style->idx_show_cnt;
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
    int h_item      = (obj->area.h/cnt_show);
    int _cnt_dis_start = -1 * (y_scroll/h_item);
    int idx_selected = idx_start + _cnt_dis_start;

    gt_listview_style_set_selected_item_by_idx(obj, idx_selected);
}


static void _gt_listview_init_widget(gt_obj_st * listview) {
    _gt_style_listview_st * style = listview->style;
    gt_size_t x, y, w, h;           //base attr
    
    // set default size
    if( listview->area.w == 0 || listview->area.h == 0 ){
        listview->area.w = 72;
        listview->area.h = 30 * style->idx_show_cnt;
    }

    // can not set area.h less than items numb * font size
    if( listview->area.h < (GT_FONT_SIZE_16x16 * style->idx_show_cnt) ){
        listview->area.h = (GT_FONT_SIZE_16x16+2) * style->idx_show_cnt;
    }

    x = listview->area.x;
    y = listview->area.y;
    w = listview->area.w;
    h = listview->area.h;

    gt_attr_rect_st rect_attr = {
        .is_fill = 1,
        .border_width = 2,
        .bg_opa = GT_OPA_COVER
    };
    rect_attr.border_color  = style->color_border;
    rect_attr.fg_color      = style->fg_color_unselected;
    rect_attr.bg_color      = style->bg_color;
    
    gt_area_st area_rect = {
        .w = w, .h = h
    };

    uint8_t fp[64];
    gt_font_st font = {
        .style_cn = 1, .style_en = 2, .res = fp, .size = GT_FONT_SIZE_16x16, 
    };

    gt_attr_font_st font_attr = {
        .font = &font,
        .space_x = 0,
        .space_y = 0,
        .align = GT_ALIGN_CENTER,
        .font_color = gt_color_black()
    };
    uint16_t __h = h/style->idx_show_cnt;
    uint8_t idx_start = style->idx_show_start;
    uint8_t idx_end = style->idx_show_start + (style->idx_show_cnt==1?1:style->idx_show_cnt);
    gt_color_t color_font;

    gt_area_st area_item = listview->area,  area_font;
    area_item.h = area_item.h/style->idx_show_cnt;
    uint16_t width_font = 0;
    while( idx_start < idx_end ){
        if( idx_start == style->idx_selected ){            
            rect_attr.border_color = style->color_border;
            rect_attr.bg_color = style->fg_color_selected;
            color_font = style->color_font_selected;
        }else{
            rect_attr.border_color = style->color_border;
            rect_attr.bg_color = style->fg_color_unselected;
            color_font = style->color_font_unselected;
        }
        
        draw_bg(listview->draw_ctx, &rect_attr, &area_item);
        

        area_font = area_item;
        font.utf8 = (uint8_t *)style->item[idx_start];
        font.len = strlen(style->item[idx_start]);
        width_font = gt_font_get_string_width(&font);
        draw_text(listview->draw_ctx, &font_attr, &area_font);

        area_item.y += area_item.h;
        idx_start++;
    }
  
}

/**
 * @brief obj init listview widget cb
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");
    /* code */
    _gt_listview_init_widget(obj);
    GT_LOGV(GT_LOG_TAG_GUI, "end init_cb");
}

/**
 * @brief obj deinit cb
 *
 * @param obj
 */
static void _deinit_cb(gt_obj_st * obj, const gt_obj_class_st * class) {
    GT_LOGV(GT_LOG_TAG_GUI, "start deinit_cb");
    /* code */
    GT_LOGV(GT_LOG_TAG_GUI, "end deinit_cb");
}


/**
 * @brief obj event handler cb
 *
 * @param obj
 * @param e event
 */
static void _event_cb(struct _gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code = gt_event_get_code(e);
    switch(code) {
        case GT_EVENT_TYPE_DRAW_START:
            GT_LOGD(GT_LOG_TAG_GUI, "start draw");
            if( gt_obj_check_scr(obj) ){        
                extern void gt_disp_ref_area(gt_area_st * area);
                gt_disp_ref_area(&obj->area);
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
            printf("list--clicking\n");
            break;

        case GT_EVENT_TYPE_INPUT_SCROLL:
            GT_LOGD(GT_LOG_TAG_GUI, "scroll");
            printf("list--scroll\n");
            printf("scroll y:%d\n", obj->process_attr.y_scroll);
            _gt_listview_style_update_by_scroll_y(obj, obj->process_attr.y_scroll);
            
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_PROCESS:   /* remove clicking style and process click event */
            GT_LOGD(GT_LOG_TAG_GUI, "process");
            printf("list--process\n");
            /* code here */

            _gt_listview_style_update_by_click_point(obj, obj->process_attr.point);

            gt_event_send(obj, GT_EVENT_TYPE_INPUT_PROCESSED, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_PROCESSED: /* click event finish */
            GT_LOGD(GT_LOG_TAG_GUI, "processed");
            
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        default:
            break;
    }
}


static void _gt_listview_init_style(gt_obj_st * listview)
{
    _gt_style_listview_st * style = listview->style;
    gt_memset(style, 0, sizeof(_gt_style_listview_st));
    GT_COLOR_SET(style->color_font_selected, 0xFFFF);
    GT_COLOR_SET(style->color_font_unselected, 0x0000);
    GT_COLOR_SET(style->bg_color, 0xFFFF);
    GT_COLOR_SET(style->color_border, 0xD6FC);
    GT_COLOR_SET(style->fg_color_selected, 0x0516);
    GT_COLOR_SET(style->fg_color_unselected, 0xFFFF);
    style->idx_show_cnt = 5;
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
        return;
    }
    if( idx < 0 ){
        return;
    }
    style->idx_selected = idx;
    
    if( style->idx_selected >= (style->idx_show_cnt/2) ){
        style->idx_show_start = style->idx_selected - (style->idx_show_cnt/2);
    }

    if( (style->idx_show_start + style->idx_show_cnt) > style->cnt ){
        if( style->cnt >= style->idx_show_cnt ){
            style->idx_show_start = style->cnt - style->idx_show_cnt;
        }else{
            style->idx_show_start = 0;
        }
    }

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

char * gt_listview_style_get_slected_item_text(gt_obj_st * listview){
    _gt_style_listview_st * style = listview->style;
    return style->item[style->idx_selected];
}
/* end ------------------------------------------------------------------*/
