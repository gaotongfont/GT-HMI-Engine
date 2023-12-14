/**
 * @file gt_gif.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2023-10-24 16:32:31
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_gif.h"

#if GT_CFG_ENABLE_GIF
#include "../core/gt_draw.h"
#include "../core/gt_mem.h"
#include "../others/gt_types.h"
#include "../others/gt_log.h"
#include "../core/gt_timer.h"
#include "../others/gt_anim.h"
#include "../hal/gt_hal_tick.h"
#include "../core/gt_img_decoder.h"
#include "../core/gt_disp.h"

#include "../extra/gif/gifdec.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_GIF
#define MY_CLASS    &gt_gif_class


/* private typedef ------------------------------------------------------*/
/**
 * @brief The gif object
 */
typedef struct _gt_gif_s {
    char * src;
    gd_GIF * gif;
    _gt_timer_st * timer;
    uint32_t last_run;
    _gt_img_dsc_st dsc;
}_gt_gif_st;


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_gif_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_gif_st)
};

/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static void _init_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");

    _gt_gif_st * style = (_gt_gif_st * )obj->style;
    gt_attr_rect_st dsc = {
        .bg_opa = obj->opa,
        .bg_img_src = NULL,
        .ram_img = &style->dsc,
    };
    /* start draw obj */
    draw_bg_img(obj->draw_ctx, &dsc, &obj->area);

    // focus
    draw_focus(obj , 0);

}

static void _deinit_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "start deinit_cb");
    if (NULL == obj) {
        return ;
    }

    _gt_gif_st ** style_p = (_gt_gif_st ** )&obj->style;
    if (NULL == *style_p) {
        return ;
    }

    if (NULL != (*style_p)->gif) {
        gd_close_gif((*style_p)->gif);
        (*style_p)->gif = NULL;
    }

    if (NULL != (*style_p)->src) {
        gt_mem_free((*style_p)->src);
        (*style_p)->src = NULL;
    }

    if (NULL != (*style_p)->timer) {
        _gt_timer_set_paused((*style_p)->timer, true);
        _gt_timer_del((*style_p)->timer);
        (*style_p)->timer = NULL;
    }

    gt_mem_free(*style_p);
    *style_p = NULL;
}

static void _event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code = gt_event_get_code(e);

    switch (code)
    {
        case GT_EVENT_TYPE_DRAW_START: {
            gt_disp_invalid_area(obj);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;
        }
        case GT_EVENT_TYPE_DRAW_END: {
            break;
        }
        case GT_EVENT_TYPE_INPUT_SCROLL_START: {
            break;
        }
        case GT_EVENT_TYPE_INPUT_SCROLL_END: {
            break;
        }
        default:
            break;
    }
}

static void _gif_next_frame_handler_cb(struct _gt_timer_s * timer) {
    gt_obj_st * obj = (gt_obj_st * )timer->user_data;
    if (GT_TYPE_GIF != gt_obj_class_get_type(obj)) {
        return;
    }
    _gt_gif_st * style = (_gt_gif_st * )obj->style;
    uint32_t time_gone = gt_tick_elapse(style->last_run);
    if (time_gone < style->gif->gce.delay * 10) {
        return;
    }
    gt_size_t has_next = gd_get_frame(style->gif);
    if (0 == has_next) {
        if (1 != style->gif->loop_count) {
            if (style->gif->loop_count > 1) {
                --style->gif->loop_count;
            }
            gd_rewind(style->gif);
        } else {
            /** last repeat */
        }
    }
    gd_render_frame(style->gif, style->dsc.img, style->dsc.alpha);
    style->last_run = gt_tick_get();
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

/* global functions / API interface -------------------------------------*/

gt_obj_st * gt_gif_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    _gt_gif_st * style = (_gt_gif_st * )obj->style;
    gt_memset(style, 0, sizeof(_gt_gif_st));

    gt_obj_set_fixed(obj, true);
    gt_obj_set_focus_disabled(obj, true);
    gt_obj_set_scroll_dir(obj, GT_SCROLL_DISABLE);

    style->timer = _gt_timer_create(_gif_next_frame_handler_cb, 10, obj);
    _gt_timer_set_paused(style->timer, true);

    return obj;
}

void gt_gif_set_src(gt_obj_st * obj, char * src)
{
    if (GT_TYPE_GIF != gt_obj_class_get_type(obj)) {
        return ;
    }
    _gt_gif_st * style = (_gt_gif_st *)obj->style;
    bool is_running = false;
    if (NULL != style->src && !strcmp(src, style->src)) {
        return ;
    }

    if( NULL != style->src ){
        gt_mem_free(style->src);
        style->src = NULL;
    }
    if (NULL != style->timer) {
        is_running = !_gt_timer_get_paused(style->timer);
        _gt_timer_set_paused(style->timer, true);
    }
    if (NULL != style->gif) {
        gd_close_gif(style->gif);
        style->gif = NULL;
    }
    uint16_t len = src == NULL ? 0 : strlen(src);
    style->src = gt_mem_malloc( len + 1 );
    strcpy(style->src, src);
    style->src[len] = 0;

    style->gif = gd_open_gif(style->src);
    if (NULL == style->gif) {
        return ;
    }
    style->dsc.src = NULL;
    style->dsc.img = style->gif->canvas;
    style->dsc.alpha = style->gif->mask;
    style->dsc.type = GT_FS_TYPE_UNKNOWN;
    style->dsc.header.w = style->gif->width;
    style->dsc.header.h = style->gif->height;
    style->dsc.header.type =  GT_IMG_DECODER_TYPE_RAM;
    style->dsc.header.color_format = GT_IMG_CF_TRUE_COLOR_ALPHA;
    style->last_run = gt_tick_get();

    if (is_running) {
        _gif_next_frame_handler_cb(style->timer);
        _gt_timer_set_paused(style->timer, false);
    }

    gt_event_send(obj, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

char * gt_gif_get_src(gt_obj_st *  obj)
{
    if (GT_TYPE_GIF != gt_obj_class_get_type(obj)) {
        return NULL;
    }
    _gt_gif_st * style = (_gt_gif_st *)obj->style;
    return style->src;
}

gt_size_t gt_gif_get_frame_width(gt_obj_st * obj)
{
    if (GT_TYPE_GIF != gt_obj_class_get_type(obj)) {
        return -1;
    }
    _gt_gif_st * style = (_gt_gif_st *)obj->style;
    return style->gif ? style->gif->width : 0;
}

gt_size_t gt_gif_get_frame_height(gt_obj_st * obj)
{
    if (GT_TYPE_GIF != gt_obj_class_get_type(obj)) {
        return -1;
    }
    _gt_gif_st * style = (_gt_gif_st *)obj->style;
    return style->gif ? style->gif->height : 0;
}

void gt_gif_play(gt_obj_st * obj)
{
    if (GT_TYPE_GIF != gt_obj_class_get_type(obj)) {
        return ;
    }
    _gt_gif_st * style = (_gt_gif_st *)obj->style;
    _gt_timer_set_paused(style->timer, false);
}

void gt_gif_stop(gt_obj_st * obj)
{
    if (GT_TYPE_GIF != gt_obj_class_get_type(obj)) {
        return ;
    }
    _gt_gif_st * style = (_gt_gif_st *)obj->style;
    _gt_timer_set_paused(style->timer, true);
}

void gt_gif_toggle(gt_obj_st * obj)
{
    if (GT_TYPE_GIF != gt_obj_class_get_type(obj)) {
        return ;
    }
    _gt_gif_st * style = (_gt_gif_st *)obj->style;
    _gt_timer_set_paused(style->timer, !_gt_timer_get_paused(style->timer));
}

bool gt_gif_is_play(gt_obj_st * obj)
{
    if (GT_TYPE_GIF != gt_obj_class_get_type(obj)) {
        return false;
    }
    _gt_gif_st * style = (_gt_gif_st *)obj->style;
    return !_gt_timer_get_paused(style->timer);
}

void gt_gif_reset(gt_obj_st * obj)
{
    if (GT_TYPE_GIF != gt_obj_class_get_type(obj)) {
        return ;
    }
    _gt_gif_st * style = (_gt_gif_st *)obj->style;
    gd_rewind(style->gif);
}

#endif  /** GT_CFG_ENABLE_GIF */
/* end ------------------------------------------------------------------*/
