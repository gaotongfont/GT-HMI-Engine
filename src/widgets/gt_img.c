/**
 * @file gt_img.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-05-11 15:03:35
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_img.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../core/gt_fs.h"
#include "string.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"
#include "../core/gt_event.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_IMG
#define MY_CLASS    &gt_img_class

/* private typedef ------------------------------------------------------*/
typedef struct _gt_img_s {
    char * src;         // img src, It can be a virt file name/path or an array of C
}_gt_img_st;

/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_img_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_img_st)
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
/**
 * @brief obj init img widget call back
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");

    _gt_img_st * style = (_gt_img_st * )obj->style;
    gt_attr_rect_st dsc = {
        .bg_img_src = style->src,
        .bg_opa = obj->opa,
    };
    /* start draw obj */
    draw_bg_img(obj->draw_ctx, &dsc, &obj->area);

    // focus
    draw_focus(obj , 0);
}

/**
 * @brief obj deinit call back
 *
 * @param obj
 */
static void _deinit_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "start deinit_cb");
    if (NULL == obj) {
        return ;
    }

    _gt_img_st ** style_p = (_gt_img_st ** )&obj->style;
    if (NULL == *style_p) {
        return ;
    }

    if (NULL != (*style_p)->src) {
        gt_mem_free((*style_p)->src);
        (*style_p)->src = NULL;
    }

    gt_mem_free(*style_p);
    *style_p = NULL;
}

/**
 * @brief obj event handler call back
 *
 * @param obj
 * @param e event
 */
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code = gt_event_get_code(e);
    gt_area_st area;

    switch(code) {
        case GT_EVENT_TYPE_DRAW_START:
            GT_LOGV(GT_LOG_TAG_GUI, "start draw");
            gt_disp_invalid_area(obj);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;

        case GT_EVENT_TYPE_DRAW_END:
            GT_LOGV(GT_LOG_TAG_GUI, "end draw");
            break;

        case GT_EVENT_TYPE_CHANGE_CHILD_REMOVE: /* remove child from screen but not delete */
            GT_LOGV(GT_LOG_TAG_GUI, "child remove");
			break;

        case GT_EVENT_TYPE_CHANGE_CHILD_DELETE: /* delete child */
            GT_LOGV(GT_LOG_TAG_GUI, "child delete");
            break;

        case GT_EVENT_TYPE_INPUT_PRESSING:   /* add clicking style and process clicking event */
            GT_LOGV(GT_LOG_TAG_GUI, "clicking");
            break;
        case GT_EVENT_TYPE_UPDATE_VALUE:
            area = obj->area;
            gt_fs_read_img_wh(gt_img_get_src(obj), &area.w, &area.h);
            gt_obj_size_change(obj, &area);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        case GT_EVENT_TYPE_DRAW_REDRAW: {
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;
        }

        case GT_EVENT_TYPE_INPUT_SCROLL:
            GT_LOGV(GT_LOG_TAG_GUI, "scroll");
            break;

        case GT_EVENT_TYPE_INPUT_RELEASED: /* click event finish */
            GT_LOGV(GT_LOG_TAG_GUI, "processed");
            // gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        default:
            break;
    }
}


static void _gt_img_init_style(gt_obj_st * img)
{
    _gt_img_st * style = (_gt_img_st * )img->style;

    gt_memset(style, 0, sizeof(_gt_img_st));
}




/* global functions / API interface -------------------------------------*/

/**
 * @brief create a img obj
 *
 * @param parent img's parent element
 * @return gt_obj_st* img obj
 */
gt_obj_st * gt_img_create(gt_obj_st * parent) {
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    _gt_img_init_style(obj);
    return obj;
}


void gt_img_set_src(gt_obj_st * img, char * src){
    if (GT_TYPE_IMG != gt_obj_class_get_type(img)) {
        return ;
    }
    _gt_img_st * style = (_gt_img_st *)img->style;
    if (NULL != style->src && !strcmp(src, style->src)) {
        return ;
    }

    if( NULL != style->src ){
        gt_mem_free(style->src);
        style->src = NULL;
    }
    uint16_t len = src == NULL ? 0 : strlen(src);
    style->src = gt_mem_malloc( len + 1 );
    strcpy(style->src, src);
    style->src[len] = 0;

    gt_event_send(img, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

char * gt_img_get_src(gt_obj_st * img){
    if (GT_TYPE_IMG != gt_obj_class_get_type(img)) {
        return NULL;
    }
    _gt_img_st * style = (_gt_img_st *)img->style;
    return style->src;
}
/* end ------------------------------------------------------------------*/
