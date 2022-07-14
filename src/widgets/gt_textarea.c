/**
 * @file gt_textarea.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-07-11 15:03:35
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_textarea.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../font/gt_font.h"
#include "../others/gt_assert.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_TEXTAREA
#define MY_CLASS    &gt_textarea_class

/* private typedef ------------------------------------------------------*/


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class);
static void _deinit_cb(gt_obj_st * obj,const gt_obj_class_st * class);
static void _event_cb(struct _gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_textarea_class = {
    ._init_cb = _init_cb,
    ._deinit_cb = _deinit_cb,
    ._event_cb = _event_cb,
    .type = OBJ_TYPE,
    .size_instance = (sizeof(gt_obj_st)),
    .size_style = (sizeof(_gt_style_textarea_st))
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static void _gt_textarea_init_widget(gt_obj_st * textarea) {
    _gt_style_textarea_st * style = textarea->style;
    gt_size_t w, h;

    if( GT_CREATE_TYPE_PART == textarea->create_type ){
        // area_disp has been cpy to textarea->area_part
        gt_area_st area_valid;
        gt_area_st area_temp;
        gt_obj_get_valid_area(textarea, &textarea->area_part, &area_valid);

        area_temp.x = textarea->area.x;
        area_temp.w = textarea->area.w;
        area_temp.h = textarea->area.h;

        if( area_valid.y >= 0 ){
            area_temp.y = textarea->area.y + area_valid.y;
            area_temp.h = textarea->area.h - area_valid.y;
        }
        gt_area_copy(&textarea->area_back, &textarea->area);
        gt_area_copy(&textarea->area, &area_temp);

        w = area_temp.w;
        h = area_valid.h;
        textarea->cache.cache_color = gt_mem_malloc( w * h * sizeof(gt_color_t) );
        

    }else{
        w = textarea->area.w;
        h = textarea->area.h;

        textarea->cache.cache_color = gt_mem_malloc( w * h * sizeof(gt_color_t) );
        
    }
}

/**
 * @brief obj init textarea widget cb
 *
 * @param obj
 */





static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");
    /* code */
    _gt_textarea_init_widget(obj);
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
    gt_mem_free(obj->cache.cache_color);
    if( GT_CREATE_TYPE_PART == obj->create_type ){
        obj->create_type = GT_CREATE_TYPE_FULL;
        gt_area_copy(&obj->area, &obj->area_back);
    }
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
            _gt_textarea_init_widget(obj);
            extern void _flush_cb(struct _gt_disp_drv_s * drv, gt_area_st * area, gt_color_t * color);
            _flush_cb(NULL, &obj->area, (gt_color_t *)obj->cache.cache_color);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;

        case GT_EVENT_TYPE_DRAW_END:
            GT_LOGD(GT_LOG_TAG_GUI, "end draw");
            gt_mem_free(obj->cache.cache_color);
            break;

        case GT_EVENT_TYPE_CHANGE_CHILD_REMOVE: /* remove child from screen but not delete */
            GT_LOGD(GT_LOG_TAG_GUI, "child remove");
			break;

        case GT_EVENT_TYPE_CHANGE_CHILD_DELETE: /* delete child */
            GT_LOGD(GT_LOG_TAG_GUI, "child delete");
            break;

        case GT_EVENT_TYPE_INPUT_CLICKING:   /* add clicking style and process clicking event */
            GT_LOGD(GT_LOG_TAG_GUI, "clicking");
            
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
            
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        default:
            break;
    }
}


static void _gt_textarea_init_style(gt_obj_st * textarea)
{
    _gt_style_textarea_st * _style = textarea->style;
    
    _style->text = gt_mem_malloc(sizeof("textarea"));
    strcpy(_style->text, "textarea");
}




/* global functions / API interface -------------------------------------*/

/**
 * @brief create a textarea obj
 *
 * @param parent textarea's parent element
 * @return gt_obj_st* textarea obj
 */
gt_obj_st * gt_textarea_create(gt_obj_st * parent) {
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    _gt_textarea_init_style(obj);
    return obj;
}

void gt_textarea_style_set_text(gt_obj_st * textarea, char * text)
{
    _gt_style_textarea_st * style = textarea->style;
    if( NULL != style->text ){
        gt_mem_free(style->text);
    }
    uint16_t size = strlen(text);
    style->text = gt_mem_malloc(size+1);
    gt_memset(style->text, 0, size+1);
    gt_memcpy(style->text, text, size);
}

char * gt_textarea_style_get_text(gt_obj_st * textarea){
    GT_ASSERT_NULL(textarea);
    _gt_style_textarea_st * style = textarea->style;
    return style->text;
}

/* end ------------------------------------------------------------------*/
