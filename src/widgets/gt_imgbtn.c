/**
 * @file gt_imgbtn.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-05-11 15:03:35
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_imgbtn.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../core/gt_fs.h"
#include "string.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_IMGBTN
#define MY_CLASS    &gt_imgbtn_class

/* private typedef ------------------------------------------------------*/


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class);
static void _deinit_cb(gt_obj_st * obj,const gt_obj_class_st * class);
static void _event_cb(struct _gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_imgbtn_class = {
    ._init_cb = _init_cb,
    ._deinit_cb = _deinit_cb,
    ._event_cb = _event_cb,
    .type = OBJ_TYPE,
    .size_instance = (sizeof(gt_obj_st)),
    .size_style = (sizeof(_gt_style_imgbtn_st))
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static void _gt_imgbtn_set_src(gt_obj_st * imgbtn, char * src){
    _gt_style_imgbtn_st * style = imgbtn->style;
    
    if( src == NULL ){
        style->src = style->src_base;
    }else{
        style->src = src;
    }
}

static void _gt_imgbtn_init_widget(gt_obj_st * imgbtn)
{
    _gt_style_imgbtn_st * style = imgbtn->style;
    gt_size_t w, h;

    /* get imgbtn width and height, must ! */
    gt_fs_read_img_wh(style->src, &imgbtn->area.w, &imgbtn->area.h);

    gt_attr_rect_st dsc = {
        .bg_img_src = style->src,
        .bg_opa = imgbtn->opa
    };

    /* start draw imgbtn */
    draw_bg_img(imgbtn->draw_ctx, &dsc, &imgbtn->area);
}

/**
 * @brief obj init imgbtn widget cb
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");
    /* code */
    _gt_imgbtn_init_widget(obj);
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
    extern void _gt_disp_refr_append_area(gt_area_st * area);
    _gt_style_imgbtn_st * style = obj->style;
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
            _gt_imgbtn_set_src(obj, style->src_press);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;
        case GT_EVENT_TYPE_UPDATE_VALUE:
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_SCROLL:
            GT_LOGD(GT_LOG_TAG_GUI, "scroll");
            break;

        case GT_EVENT_TYPE_INPUT_PROCESS:   /* remove clicking style and process click event */
            GT_LOGD(GT_LOG_TAG_GUI, "process");
            /* code here */
            _gt_imgbtn_set_src(obj, style->src_release);
            gt_event_send(obj, GT_EVENT_TYPE_INPUT_PROCESSED, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_PROCESSED: /* click event finish */
            GT_LOGD(GT_LOG_TAG_GUI, "processed");
            _gt_imgbtn_set_src(obj, style->src_base);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        default:
            break;
    }
}


static void _gt_imgbtn_init_style(gt_obj_st * imgbtn)
{
    _gt_style_imgbtn_st * style = NULL;

    imgbtn->style = gt_mem_malloc(sizeof(_gt_style_imgbtn_st));
    style = imgbtn->style;

    gt_memset(style, 0, sizeof(_gt_style_imgbtn_st));

	style->selected = 0;
    style->src = NULL;
}




/* global functions / API interface -------------------------------------*/

/**
 * @brief create a imgbtn obj
 *
 * @param parent imgbtn's parent element
 * @return gt_obj_st* imgbtn obj
 */
gt_obj_st * gt_imgbtn_create(gt_obj_st * parent) {
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    _gt_imgbtn_init_style(obj);
    return obj;
}


void gt_imgbtn_style_set_selected(gt_obj_st * imgbtn, uint8_t selected) {
    _gt_style_imgbtn_st * style = imgbtn->style;
    style->selected = selected!=0?1:0;
}

void gt_imgbtn_style_set_src(gt_obj_st * imgbtn, char * src){
    GT_ASSERT_NULL(imgbtn);
    _gt_style_imgbtn_st * style = (_gt_style_imgbtn_st *)imgbtn->style;
    if( NULL != style->src_base ){
        gt_mem_free(style->src_base);
    }
    style->src_base = gt_mem_malloc( strlen(src) + 1 );
    strcpy(style->src_base, src);
    style->src_base[strlen(src)] = 0;
    style->src = style->src_base;
    gt_event_send(imgbtn, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

void gt_imgbtn_style_set_src_press(gt_obj_st * imgbtn, char * src){
    GT_ASSERT_NULL(imgbtn);
    _gt_style_imgbtn_st * style = (_gt_style_imgbtn_st *)imgbtn->style;
    if( NULL != style->src_press ){
        gt_mem_free(style->src_press);
    }
    style->src_press = gt_mem_malloc( strlen(src) + 1 );
    strcpy(style->src_press, src);
    style->src_press[strlen(src)] = 0;

    gt_event_send(imgbtn, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

void gt_imgbtn_style_set_src_release(gt_obj_st * imgbtn, char * src){
    GT_ASSERT_NULL(imgbtn);
    _gt_style_imgbtn_st * style = (_gt_style_imgbtn_st *)imgbtn->style;
    if( NULL != style->src_release ){
        gt_mem_free(style->src_release);
    }
    style->src_release = gt_mem_malloc( strlen(src) + 1 );
    strcpy(style->src_release, src);
    style->src_release[strlen(src)] = 0;

    gt_event_send(imgbtn, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}
/* end ------------------------------------------------------------------*/
