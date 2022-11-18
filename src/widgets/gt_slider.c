/**
 * @file gt_slider.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-07-21 19:53:32
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_slider.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../font/gt_font.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
#include "../core/gt_fs.h"
/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_SCROLLBAR
#define MY_CLASS    &gt_slider_class

/* private typedef ------------------------------------------------------*/


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct _gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_slider_class = {
    ._init_cb       = _init_cb,
    ._deinit_cb     = _deinit_cb,
    ._event_cb      = _event_cb,
    .type           = OBJ_TYPE,
    .size_instance  = (sizeof(gt_obj_st)),
    .size_style     = (sizeof(_gt_style_slider_st))
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static void _gt_slider_init_widget(gt_obj_st * slider) {
    _gt_style_slider_st * style = slider->style;
    gt_size_t w, h;
    int all_pos = style->end - style->start;
    int dist = 0, dist_min;
    // set default size
    if( !slider->area.w || !slider->area.h ){
        slider->area.w = 100;
        slider->area.h = 16;
    };

    if( style->tag_visible && style->tag && style->area_base.w != 0 && style->area_base.h != 0){
        slider->area = style->area_base;
    }
    w = slider->area.w;
    h = slider->area.h;

    gt_attr_rect_st rect_attr = {
        .is_fill        = 1,
        .radius         = h>>2,
        .bg_color       = style->color_ina,
        .border_color   = style->color_ina,
        .bg_opa         = slider->opa,
    };

    /* 1 base shape */
    gt_area_st area_base = slider->area;
    if( style->dir == GT_BAR_DIR_HOR_L2R || style->dir == GT_BAR_DIR_HOR_R2L ){
        area_base.h -= h>>1;
        area_base.y += (h>>1)>>1;
        rect_attr.radius = area_base.h>>1;
    }else if( style->dir == GT_BAR_DIR_VER_U2D || style->dir == GT_BAR_DIR_VER_D2U ){
        area_base.w -= w>>1;
        area_base.x += (w>>1)>>1;
        rect_attr.radius = area_base.w>>1;
    }

    draw_bg(slider->draw_ctx, &rect_attr, &area_base);

    /* 2 val shape */
    // rect val

    gt_area_st area_val = area_base;
    if( style->dir == GT_BAR_DIR_HOR_L2R ){
        dist = ((float)style->pos/(float)all_pos) * (float)w;
        area_val.w = dist;
    }else if( style->dir == GT_BAR_DIR_HOR_R2L ){
        dist = ((float)style->pos/(float)all_pos) * (float)w;
        area_val.w = dist;
        area_val.x += (area_base.w - dist);
    }else if( style->dir == GT_BAR_DIR_VER_U2D ){
        dist = ((float)style->pos/(float)all_pos) * (float)h;
        area_val.h = dist;
    }else if( style->dir == GT_BAR_DIR_VER_D2U ){
        dist = ((float)style->pos/(float)all_pos) * (float)h;
        area_val.h = dist;
        area_val.y += (area_base.h - dist);
    }else{
        return;
    }
    rect_attr.bg_color = style->color_act;

    uint16_t _w,_h;
    bool isTag = false;
    if( style->tag != NULL ){
        if( !gt_fs_read_img_wh(style->tag, &_w, &_h) ){
            w = _w;
            h = _h;
            isTag = true;
        }
    }

    if( style->dir == GT_BAR_DIR_HOR_L2R || style->dir == GT_BAR_DIR_HOR_R2L ){
        dist_min = (h>>1);
    }else if( style->dir == GT_BAR_DIR_VER_D2U || style->dir == GT_BAR_DIR_VER_U2D ){
        dist_min = (w>>1);
    }



    if( dist > dist_min ){
        draw_bg(slider->draw_ctx, &rect_attr, &area_val);
    }
    /* big big big operate circle */
    if( !style->tag_visible ){
        return;
    }
    gt_area_st area_big_circle = slider->area;

    area_big_circle.y = slider->area.y;
    area_big_circle.x = slider->area.x;
    if( style->dir == GT_BAR_DIR_HOR_L2R ){
        area_big_circle.h = h;
        area_big_circle.x += (dist - (h>>1));
    }else if( style->dir == GT_BAR_DIR_HOR_R2L ){
        area_big_circle.h = h;
        area_big_circle.x = (area_val.x - (h>>1));
    }else if( style->dir == GT_BAR_DIR_VER_U2D ){
        area_big_circle.h = w;
        area_big_circle.y += (dist - (w>>1));
    }else if( style->dir == GT_BAR_DIR_VER_D2U ){
        area_big_circle.h = w;
        area_big_circle.y = (area_val.y - (w>>1));
    }
    area_big_circle.w = area_big_circle.h;
    rect_attr.radius = area_big_circle.w>>1;
    // limit big circle overflow
    if( style->dir == GT_BAR_DIR_HOR_L2R || style->dir == GT_BAR_DIR_HOR_R2L ){
        if( area_big_circle.x < slider->area.x ){
            area_big_circle.x = slider->area.x;
        }
        if( area_big_circle.x > (slider->area.x + slider->area.w - area_big_circle.w) ){
            area_big_circle.x = slider->area.x + slider->area.w - area_big_circle.w;
        }
    }else if( style->dir == GT_BAR_DIR_VER_U2D || style->dir == GT_BAR_DIR_VER_D2U ){
        if( area_big_circle.y < slider->area.y ){
            area_big_circle.y = slider->area.y;
        }
        if( area_big_circle.y > (slider->area.y + slider->area.h - area_big_circle.h) ){
            area_big_circle.y = slider->area.y + slider->area.h - area_big_circle.h;
        }
    }
    if( isTag == false ){
        draw_bg(slider->draw_ctx, &rect_attr, &area_big_circle);
    }else{
        rect_attr.bg_img_src = style->tag;
        area_big_circle.w = w;
        area_big_circle.h = h;
        if( style->area_base.w == 0 || style->area_base.h == 0 ){
            style->area_base = slider->area;
        }
        if( slider->area.w < w ){
            int wof = w - slider->area.w;
            slider->area.w = w;
            slider->area.x = slider->area.x - (wof>>1);
            area_big_circle.x = slider->area.x;
        }
        if( slider->area.h < h ){
            int hof = h - slider->area.h;
            slider->area.h = h;
            slider->area.y = slider->area.y - (hof>>1);
            area_big_circle.y = slider->area.y;
        }
        draw_bg_img(slider->draw_ctx, &rect_attr, &area_big_circle);
    }
}

/**
 * @brief obj init slider widget cb
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");

    _gt_slider_init_widget(obj);
}

/**
 * @brief obj deinit cb
 *
 * @param obj
 */
static void _deinit_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "start deinit_cb");
    if (NULL == obj) {
        return ;
    }

    _gt_style_slider_st ** style_p = (_gt_style_slider_st *)&obj->style;
    if (NULL == *style_p) {
        return ;
    }

    if (NULL != (*style_p)->tag) {
        gt_mem_free((*style_p)->tag);
        (*style_p)->tag = NULL;
    }

    gt_mem_free(*style_p);
    *style_p = NULL;
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
    gt_point_st point_click;
    int _pos;
    float pos;
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
            pos = gt_slider_style_get_pos(obj);
            if( gt_slider_style_get_dir(obj) == GT_BAR_DIR_HOR_L2R ){
                pos += obj->process_attr.x_scroll;
            }else if( gt_slider_style_get_dir(obj) == GT_BAR_DIR_HOR_R2L ){
                pos -= obj->process_attr.x_scroll;
            }else if( gt_slider_style_get_dir(obj) == GT_BAR_DIR_VER_U2D ){
                pos += obj->process_attr.y_scroll;
            }else if( gt_slider_style_get_dir(obj) == GT_BAR_DIR_VER_D2U ){
                pos -= obj->process_attr.y_scroll;
            }
            gt_slider_style_set_pos(obj, (int)pos);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_PROCESS:   /* remove clicking style and process click event */
            GT_LOGD(GT_LOG_TAG_GUI, "process");
            /* code here */
            point_click.x = obj->process_attr.point.x;
            point_click.y = obj->process_attr.point.y;
            if( gt_slider_style_get_dir(obj) == GT_BAR_DIR_HOR_L2R ){
                _pos = point_click.x;
                pos = ((float)_pos)/((float)obj->area.w);
            }else if( gt_slider_style_get_dir(obj) == GT_BAR_DIR_HOR_R2L ){
                _pos = obj->area.w - point_click.x;
                pos = ((float)_pos)/((float)obj->area.w);
            }else if( gt_slider_style_get_dir(obj) == GT_BAR_DIR_VER_U2D ){
                _pos = point_click.y;
                pos = ((float)_pos)/((float)obj->area.h);
            }else if( gt_slider_style_get_dir(obj) == GT_BAR_DIR_VER_D2U ){
                _pos = obj->area.h - point_click.y;
                pos = ((float)_pos)/((float)obj->area.h);
            }


            pos = pos * (float)gt_slider_style_get_total(obj);
            pos += gt_slider_style_get_start(obj);
            gt_slider_style_set_pos(obj, (int)pos);
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


static void _gt_slider_init_style(gt_obj_st * slider)
{
    _gt_style_slider_st * style = (_gt_style_slider_st *)slider->style;

    gt_memset(style,0,sizeof(_gt_style_slider_st));

    style->color_act = gt_color_hex(0x409eff);
    style->color_ina = gt_color_hex(0xebeef5);
    style->start = 0;
    style->end = 100;
    style->pos = 50;
    style->dir = 0;
    style->step = 1;
    style->tag_visible = true;
}




/* global functions / API interface -------------------------------------*/

/**
 * @brief create a slider obj
 *
 * @param parent slider's parent element
 * @return gt_obj_st* slider obj
 */
gt_obj_st * gt_slider_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    _gt_slider_init_style(obj);
    return obj;
}


/**
 * @brief set scorllbar pos
 *
 * @param slider scorllbar
 * @param pos pos
 */
void gt_slider_style_set_pos(gt_obj_st * slider, gt_size_t pos)
{
    _gt_style_slider_st * style = (_gt_style_slider_st *)slider->style;
    if( pos < style->start ){
        style->pos = style->start;
    }else if( pos > style->end ){
        style->pos = style->end;
    }else{
        style->pos = ( (int)(pos / style->step) * ((int)(style->step)) );
    }
    gt_event_send(slider, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_slider_style_set_start_end(gt_obj_st * slider, gt_size_t start, gt_size_t end)
{
    _gt_style_slider_st * style = (_gt_style_slider_st *)slider->style;
    style->start = start;
    style->end = end;
}

gt_size_t gt_slider_style_get_pos(gt_obj_st * slider)
{
    _gt_style_slider_st * style = (_gt_style_slider_st *)slider->style;
    return style->pos;
}

gt_size_t gt_slider_style_get_start(gt_obj_st * slider)
{
    _gt_style_slider_st * style = (_gt_style_slider_st *)slider->style;
    return style->start;
}

gt_size_t gt_slider_style_get_end(gt_obj_st * slider)
{
    _gt_style_slider_st * style = (_gt_style_slider_st *)slider->style;
    return style->end;
}

gt_size_t gt_slider_style_get_total(gt_obj_st * slider)
{
    _gt_style_slider_st * style = (_gt_style_slider_st *)slider->style;
    return ( style->end - style->start);
}

void gt_slider_style_set_color_act(gt_obj_st * slider, gt_color_t color)
{
    _gt_style_slider_st * style = slider->style;
    style->color_act = color;
}
void gt_slider_style_set_color_ina(gt_obj_st * slider, gt_color_t color)
{
    _gt_style_slider_st * style = slider->style;
    style->color_ina = color;
}

void gt_slider_style_set_dir(gt_obj_st * slider, gt_bar_dir_et dir)
{
    _gt_style_slider_st * style = (_gt_style_slider_st *)slider->style;
    style->dir = dir;
}
gt_bar_dir_et gt_slider_style_get_dir(gt_obj_st * slider)
{
    _gt_style_slider_st * style = (_gt_style_slider_st *)slider->style;
    return ( style->dir);
}

void gt_slider_style_set_step(gt_obj_st * slider, uint8_t step)
{
    _gt_style_slider_st * style = (_gt_style_slider_st *)slider->style;
    style->step = step;
}

uint8_t gt_slider_style_get_step(gt_obj_st * slider)
{
    _gt_style_slider_st * style = (_gt_style_slider_st *)slider->style;
    return ( style->step);
}

void gt_slider_style_set_tag(gt_obj_st * slider, char * src)
{
    _gt_style_slider_st * style = (_gt_style_slider_st *)slider->style;
    if( style->tag != NULL ){
        gt_mem_free(style->tag);
    }
    style->tag = gt_mem_malloc(strlen(src) + 1);

    gt_memcpy(style->tag, src, strlen(src));
    style->tag[strlen(src)] = 0;
}

void gt_slider_style_set_tag_visible(gt_obj_st * slider, bool visible)
{
    _gt_style_slider_st * style = (_gt_style_slider_st *)slider->style;
    style->tag_visible = visible;
}
/* end ------------------------------------------------------------------*/
