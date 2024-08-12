/**
 * @file gt_hal_disp.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-05-13 09:52:03
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_hal_disp.h"
#include "../core/gt_mem.h"
#include "../core/gt_obj_pos.h"
#include "../others/gt_color.h"
#include "../gt_conf.h"
#include "../others/gt_log.h"
#include "../core/gt_refr.h"
#include "../core/gt_timer.h"
#include "../hal/gt_hal_tick.h"
#include "../others/gt_area.h"
#include "../others/gt_gc.h"
#include "../widgets/gt_obj_class.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_LAYER_TOP
#define MY_CLASS    &gt_layer_top_class


/* private typedef ------------------------------------------------------*/
typedef struct _gt_style_layer_top_s {
    gt_obj_st obj;
}_gt_style_layer_top_st;



/* static variables -----------------------------------------------------*/
#if (GT_REFRESH_STYLE_1 == GT_REFRESH_STYLE)
    static gt_color_t * graph_buf1;
#elif (GT_REFRESH_STYLE_2 == GT_REFRESH_STYLE)
    static gt_color_t * graph_buf1, * graph_buf2;
#elif (GT_REFRESH_STYLE_3 == GT_REFRESH_STYLE)
    static gt_color_t * graph_buf_all;
#endif

static const gt_obj_class_st gt_layer_top_class = {
    ._init_cb      = (_gt_init_cb)NULL,
    ._deinit_cb    = (_gt_deinit_cb)NULL,
    ._event_cb     = (_gt_event_cb)NULL,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_style_layer_top_st)
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static inline gt_size_t _gt_disp_area_get_idx(gt_size_t idx) {
    return idx == _GT_REFR_AREA_MAX ? 0 : idx;
}

static void _gt_disp_task_handler(struct _gt_timer_s * timer) {
    GT_UNUSED(timer);
    gt_refr_timer();
}

static gt_disp_st * _gt_disp_create(gt_disp_drv_st * drv) {
    gt_disp_st * ret = gt_mem_malloc(sizeof(gt_disp_st));
    if (NULL == ret) {
        GT_LOGE(GT_LOG_TAG_GUI, "disp create failed");
        return NULL;
    }
    gt_memset(ret, 0, sizeof(gt_disp_st));

    ret->vbd_color = gt_disp_graph_get_buf_default();
    ret->drv = drv;

    _gt_gc_set_disp(ret);

#if GT_USE_LAYER_TOP
    ret->layer_top = gt_obj_class_create(MY_CLASS, NULL);
    if (NULL == ret->layer_top) {
        GT_LOGE(GT_LOG_TAG_GUI, "Layer top create failed");
        return ret;
    }
    gt_screen_set_bgcolor(ret->layer_top, gt_color_hex(0x000000));
    gt_obj_set_virtual(ret->layer_top, true);
#endif

    return ret;
}

static void _calc_max_area_recursive(gt_obj_st * obj, bool is_root) {
    GT_CHECK_BACK(obj);
    gt_obj_st * ptr = NULL;
    gt_size_t i = 0, cnt = obj->cnt_child;

    for (; i < cnt; i++) {
        ptr = obj->child[i];
        if (NULL == ptr) {
            GT_CHECK_PRINT(ptr);
            continue;
        }

        if (false == gt_obj_get_visible(ptr)) {
            continue;
        }
        if (ptr->cnt_child || gt_obj_get_virtual(ptr)) {
            _calc_max_area_recursive(ptr, false);
        }

        _gt_disp_update_max_area(&ptr->area, _gt_obj_is_ignore_calc_max_area(ptr));
    }

    if (is_root || gt_obj_get_virtual(obj)) {
        return;
    }
    _gt_disp_update_max_area(&obj->area, _gt_obj_is_ignore_calc_max_area(obj));
}

/* global functions / API interface -------------------------------------*/

/*************************************/
/*                drv               */
/*************************************/
void gt_disp_drv_init(gt_disp_drv_st * drv)
{
    drv->res_hor = GT_SCREEN_WIDTH;
    drv->res_ver = GT_SCREEN_HEIGHT;
}

void gt_disp_drv_register(gt_disp_drv_st * drv)
{
    if(drv->flush_cb == NULL){
        GT_LOGW(GT_LOG_TAG_GUI,"flush_cb is null");
        return;
    }
    gt_disp_st * disp_p = _gt_gc_get_disp();

    if( disp_p == NULL ){
        disp_p = _gt_disp_create(drv);
    } else {
        disp_p->drv = drv;
    }

    _gt_timer_create(_gt_disp_task_handler, GT_TASK_PERIOD_TIME_REFR, NULL);
}



/*************************************/
/*                disp               */
/*************************************/

gt_disp_st * gt_disp_get_default(void)
{
    return _gt_gc_get_disp();
}

uint16_t gt_disp_get_res_hor(gt_disp_st * disp)
{
    if( NULL == disp ) disp = gt_disp_get_default();
    if( NULL == disp ) return 0;
    switch( disp->drv->rotated ){
        case GT_ROTATED_90:
        case GT_ROTATED_270:
            return disp->drv->res_ver;
        default:
            break;
    }
    return disp->drv->res_hor;
}
uint16_t gt_disp_get_res_ver(gt_disp_st * disp)
{
    if( NULL == disp ) disp = gt_disp_get_default();
    if( NULL == disp ) return 0;
    switch( disp->drv->rotated ){
        case GT_ROTATED_90:
        case GT_ROTATED_270:
            return disp->drv->res_hor;
        default:
            break;
    }
    return disp->drv->res_ver;
}

void gt_disp_graph_buf_init(gt_color_t *buf1, gt_color_t *buf2, gt_color_t *buf_all)
{

#if (GT_REFRESH_STYLE == GT_REFRESH_STYLE_0)
    GT_LOGV(GT_LOG_TAG_GUI, "use refresh style 0");
#elif (GT_REFRESH_STYLE == GT_REFRESH_STYLE_1)
    GT_LOGV(GT_LOG_TAG_GUI, "use refresh style 1");
    graph_buf1 = buf1;
#elif (GT_REFRESH_STYLE == GT_REFRESH_STYLE_2)
    GT_LOGV(GT_LOG_TAG_GUI, "use refresh style 2");
    graph_buf1 = buf1;
    graph_buf2 = buf2;
#elif (GT_REFRESH_STYLE == GT_REFRESH_STYLE_3)
    GT_LOGV(GT_LOG_TAG_GUI, "use refresh style 3");
    graph_buf_all = buf_all;
#else
    GT_LOGV(GT_LOG_TAG_GUI, "use refresh style 0");
#endif

}


gt_color_t * gt_disp_graph_get_buf_default(void)
{
#if (GT_REFRESH_STYLE_1 == GT_REFRESH_STYLE)
    return graph_buf1;
#elif (GT_REFRESH_STYLE_2 == GT_REFRESH_STYLE)
    return graph_buf1;
#elif (GT_REFRESH_STYLE_3 == GT_REFRESH_STYLE)
    return graph_buf_all;
#else
    return NULL;
#endif
}

gt_color_t * gt_disp_graph_get_buf_backup(void)
{
#if (GT_REFRESH_STYLE_2 == GT_REFRESH_STYLE)
    return graph_buf2;
#else
    return NULL;
#endif
}

void _gt_disp_refr_area_push(gt_disp_st * disp)
{
    disp->refr.idx_w = _gt_disp_area_get_idx(++disp->refr.idx_w);
}

void _gt_disp_refr_area_pop(gt_disp_st * disp)
{
    gt_size_t idx = disp->refr.idx_r;
    disp->refr.joined[idx] = 0; /* value will be clean up. */
    disp->refr.idx_r = _gt_disp_area_get_idx(++idx);
}

void _gt_disp_refr_reset_areas(void)
{
    gt_disp_st * disp = gt_disp_get_default();

    gt_memset(disp->refr.areas, 0, sizeof(gt_area_st) * _GT_REFR_AREA_MAX);
    gt_memset(disp->refr.joined, 0, sizeof(uint8_t) * _GT_REFR_AREA_MAX);
    disp->refr.idx_r = 0;
    disp->refr.idx_w = 0;
}

void _gt_disp_refr_append_area(gt_area_st * area)
{
    gt_disp_st * disp = gt_disp_get_default();
    gt_area_st * area_p = &disp->refr.areas[0];
    gt_area_st res_area;
    gt_size_t i = 0;
    gt_size_t idx, idx_newly;
    gt_size_t len = disp->refr.idx_w < disp->refr.idx_r ? _GT_REFR_AREA_MAX : 0;
    bool is_on = false;

    idx = disp->refr.idx_r;
    idx_newly = disp->refr.idx_w;
    disp->refr.areas[idx_newly] = *area;
    len += idx_newly - idx; /* except the newly one */

    // joined the newly area into before area
    for (i = 0; i < len; i++) {
        if (disp->refr.joined[idx]) {
            idx = _gt_disp_area_get_idx(++idx);
            continue;
        }
        is_on = gt_area_is_on(&area_p[idx], &area_p[idx_newly]);
        if (is_on) {
            gt_area_join(&res_area, &area_p[idx], &area_p[idx_newly]);
            gt_area_copy(&disp->refr.areas[idx], &res_area);
            disp->refr.joined[idx_newly] = 1;
        }
        idx = _gt_disp_area_get_idx(++idx);
    }

    if (disp->refr.joined[idx_newly]) {
        disp->refr.joined[idx_newly] = 0;
    } else {
        _gt_disp_refr_area_push(disp);
    }
}

uint8_t _gt_disp_refr_check(gt_disp_st * disp)
{
    return disp->refr.idx_w != disp->refr.idx_r;
}

gt_area_st * _gt_disp_refr_get_area(gt_disp_st * disp)
{
    gt_size_t i = 0;
    gt_size_t idx = disp->refr.idx_r;
    gt_size_t len = (disp->refr.idx_w < idx ? _GT_REFR_AREA_MAX : 0) + disp->refr.idx_w - idx;

    while(i++ < len) {
        if (!disp->refr.joined[idx]) {
            goto get_area_lb;
        }
        _gt_disp_refr_area_pop(disp);
        idx = _gt_disp_area_get_idx(++idx);
    }

    return NULL;
get_area_lb:
    return &disp->refr.areas[idx];
}

void _gt_disp_update_max_area(const gt_area_st * const area, bool is_ignore_calc)
{
    gt_disp_st * disp = gt_disp_get_default();
    GT_CHECK_BACK(disp);
    /** Note: update into disp->area_max */
    gt_area_abs_st * max_area = &disp->area_max;

    if (is_ignore_calc) {
        /** allow widget overflow screen display, can not scroll to over area */
        return;
    }

    if (area->x < max_area->left) {
        max_area->left = area->x;
    }
    if (area->y < max_area->top) {
        max_area->top = area->y;
    }
    if (area->x + area->w > max_area->right) {
        max_area->right = area->x + area->w;
    }
    if (area->y + area->h > max_area->bottom) {
        max_area->bottom = area->y + area->h;
    }
}

void _gt_disp_reload_max_area(gt_obj_st * scr)
{
    GT_CHECK_BACK(scr);
    gt_disp_st * disp = gt_disp_get_default();
    GT_CHECK_BACK(disp);
    gt_area_abs_st * max_area = &disp->area_max;

    max_area->left = 0;
    max_area->top = 0;
    max_area->right = gt_disp_get_res_hor(disp);
    max_area->bottom = gt_disp_get_res_ver(disp);

    _calc_max_area_recursive(scr, true);
}

void _gt_disp_hided_layer_top_widgets(gt_obj_st * top)
{
    gt_obj_st * obj = NULL;
    GT_CHECK_BACK(top);

    for (gt_size_t i = 0; i < top->cnt_child; i++) {
        obj = top->child[i];
        if (GT_INVISIBLE == gt_obj_get_visible(obj)) {
            continue;
        }
        gt_event_send(obj, GT_EVENT_TYPE_INPUT_HIDED, top);
    }
}


/* end ------------------------------------------------------------------*/
