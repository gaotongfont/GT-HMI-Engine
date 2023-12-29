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
/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/
#if (GT_REFRESH_STYLE_1 == GT_REFRESH_STYLE)
    static gt_color_t * graph_buf1;
#elif (GT_REFRESH_STYLE_2 == GT_REFRESH_STYLE)
    static gt_color_t * graph_buf1, * graph_buf2;
#elif (GT_REFRESH_STYLE_3 == GT_REFRESH_STYLE)
    static gt_color_t * graph_buf_all;
#endif

static gt_disp_st * disp_p;


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static inline gt_size_t _gt_disp_area_get_idx(gt_size_t idx) {
    return idx == _GT_REFR_AREA_MAX ? 0 : idx;
}

static void _gt_disp_task_handler(struct _gt_timer_s * timer)
{
    GT_UNUSED(timer);
    gt_refr_timer();
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
    if( disp_p == NULL ){
        disp_p = gt_mem_malloc(sizeof(gt_disp_st));
        gt_memset(disp_p, 0, sizeof(gt_disp_st));
    }
    if(drv->flush_cb == NULL){
        GT_LOGW(GT_LOG_TAG_GUI,"flush_cb is null");
        return;
    }
    disp_p->vbd_color = gt_disp_graph_get_buf_default();
    disp_p->drv = drv;

    _gt_timer_create(_gt_disp_task_handler, GT_TASK_PERIOD_TIME_REFR, NULL);
}



/*************************************/
/*                disp               */
/*************************************/

gt_disp_st * gt_disp_get_default(void)
{
    return disp_p;
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
        _gt_disp_refr_area_push();
    }
}

uint8_t _gt_disp_refr_check(void)
{
    gt_disp_st * disp = gt_disp_get_default();
    return disp->refr.idx_w != disp->refr.idx_r;
}

void _gt_disp_refr_area_push(void)
{
    gt_disp_st * disp = gt_disp_get_default();
    disp->refr.idx_w = _gt_disp_area_get_idx(++disp->refr.idx_w);
}

void _gt_disp_refr_area_pop(void)
{
    gt_disp_st * disp = gt_disp_get_default();
    gt_size_t idx = disp->refr.idx_r;
    disp->refr.joined[idx] = 0; /* value will be clean up. */
    disp->refr.idx_r = _gt_disp_area_get_idx(++idx);
}

gt_area_st * _gt_disp_refr_get_area(void)
{
    gt_disp_st * disp = gt_disp_get_default();
    gt_size_t i = 0;
    gt_size_t idx = disp->refr.idx_r;
    gt_size_t len = (disp->refr.idx_w < idx ? _GT_REFR_AREA_MAX : 0) + disp->refr.idx_w - idx;

    while(i++ < len) {
        if (!disp->refr.joined[idx]) {
            goto get_area_lb;
        }
        _gt_disp_refr_area_pop();
        idx = _gt_disp_area_get_idx(++idx);
    }

    return NULL;
get_area_lb:
    return &disp->refr.areas[idx];
}

void _gt_disp_update_max_area(const gt_area_st * const area, bool is_ignore_calc)
{
    gt_disp_st * disp = gt_disp_get_default();
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
    gt_disp_st * disp = gt_disp_get_default();
    gt_area_abs_st * max_area = &disp->area_max;
    gt_obj_st * obj = NULL;
    gt_size_t i = 0;

    max_area->left = 0;
    max_area->top = 0;
    max_area->right = gt_disp_get_res_hor(disp);
    max_area->bottom = gt_disp_get_res_ver(disp);

    for (i = 0; i < scr->cnt_child; i++) {
        obj = scr->child[i];
        _gt_disp_update_max_area(&obj->area, _gt_obj_is_ignore_calc_max_area(obj));
    }
}




/* end ------------------------------------------------------------------*/
