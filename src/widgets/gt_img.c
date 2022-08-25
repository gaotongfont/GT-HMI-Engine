/**
 * @file gt_img.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-05-11 15:03:35
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_img.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../core/gt_vf.h"
#include "string.h"
#include "../others/gt_assert.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_IMG
#define MY_CLASS    &gt_img_class

/* private typedef ------------------------------------------------------*/


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class);
static void _deinit_cb(gt_obj_st * obj,const gt_obj_class_st * class);
static void _event_cb(struct _gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_img_class = {
    ._init_cb = _init_cb,
    ._deinit_cb = _deinit_cb,
    ._event_cb = _event_cb,
    .type = OBJ_TYPE,
    .size_instance = (sizeof(gt_obj_st)),
    .size_style = (sizeof(_gt_style_img_st))
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static void _gt_img_init_widget(gt_obj_st * img)
{
    _gt_style_img_st * style = img->style;
    gt_size_t w, h;

    /* get img width and height, must ! */
    gt_vf_read_img_wh(style->src, &img->area.w, &img->area.h);

    gt_attr_rect_st dsc = {
        .bg_img_src = style->src,
        .bg_opa = GT_OPA_COVER
    };
    /* start draw img */
    draw_bg_img(img->draw_ctx, &dsc, &img->area);
}

/**
 * @brief obj init img widget cb
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");
    /* code */
    _gt_img_init_widget(obj);
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
            gt_img_style_set_selected(obj, 1);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_SCROLL:
            GT_LOGD(GT_LOG_TAG_GUI, "scroll");
            break;

        case GT_EVENT_TYPE_INPUT_PROCESS:   /* remove clicking style and process click event */
            GT_LOGD(GT_LOG_TAG_GUI, "process");
            /* code here */

            gt_event_send(obj, GT_EVENT_TYPE_INPUT_PROCESSED, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_PROCESSED: /* click event finish */
            GT_LOGD(GT_LOG_TAG_GUI, "processed");
            gt_img_style_set_selected(obj, 0);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        default:
            break;
    }
}


static void _gt_img_init_style(gt_obj_st * img)
{
    _gt_style_img_st * style = NULL;
    
    img->style = gt_mem_malloc(sizeof(_gt_style_img_st));
    style = img->style;

    gt_memset(style, 0, sizeof(_gt_style_img_st));
    
	style->selected = 0;
    style->src = NULL;
    GT_COLOR_SET(style->color_selected, 0xF800);    //default color_selected
    GT_COLOR_SET(style->color_unselected, 0x1537);  //default color_unselected
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

void gt_img_style_set_color_selected(gt_obj_st * img, gt_color_t color) {
    _gt_style_img_st * style = img->style;
    GT_COLOR_SET(style->color_selected, GT_COLOR_GET(color));
}

void gt_img_style_set_color_unselected(gt_obj_st * img, gt_color_t color) {
    _gt_style_img_st * style = img->style;
    GT_COLOR_SET(style->color_unselected, GT_COLOR_GET(color));
}

void gt_img_style_set_selected(gt_obj_st * img, uint8_t selected) {
    _gt_style_img_st * style = img->style;
    style->selected = selected!=0?1:0;
}

void gt_img_style_set_src(gt_obj_st * img, char * src){
    GT_ASSERT_NULL(img);
    _gt_style_img_st * style = (_gt_style_img_st *)img->style;
    if( NULL == style->src ){
        gt_mem_free(style->src);
    }
    style->src = gt_mem_malloc( strlen(src) );
    strcpy(style->src, src);
}
/* end ------------------------------------------------------------------*/
