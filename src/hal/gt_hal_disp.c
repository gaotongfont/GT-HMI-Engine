/**
 * @file gt_hal_disp.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-05-13 09:52:03
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_hal_disp.h"
#include "../core/gt_mem.h"
#include "../others/gt_color.h"
#include "../gt_conf.h"
#include "../others/gt_log.h"

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
    }
    if(drv->flush_cb == NULL){
        GT_LOGW(GT_LOG_TAG_GUI,"flush_cb is null");
        return;
    }
    disp_p->vbd_color = gt_disp_graph_get_buf_default();
    disp_p->drv = drv;
}



/*************************************/
/*                disp               */
/*************************************/
void gt_disp_set_default(gt_disp_st * disp)
{
    disp_p = disp;
}


gt_disp_st * gt_disp_get_default()
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
            return disp->drv->res_hor;

    }
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
            return disp->drv->res_ver;
    }
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

/* end ------------------------------------------------------------------*/
