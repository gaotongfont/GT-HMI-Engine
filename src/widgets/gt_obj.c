/**
 * @file gt_obj.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-05-11 15:03:47
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_obj.h"
#include "gt_obj_class.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "../core/gt_style.h"


/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_OBJ
#define MY_CLASS    &gt_obj_class

/* private typedef ------------------------------------------------------*/
typedef struct _gt_style_obj_s
{
    uint8_t selected;
    gt_color_t bgcolor;
}_gt_style_obj_st;

/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class);
static void _deinit_cb(gt_obj_st * obj, const gt_obj_class_st * class);
static void _event_cb(gt_obj_st * obj, struct _gt_event_s * e);

const gt_obj_class_st gt_obj_class = {
    ._init_cb = _init_cb,
    ._deinit_cb = _deinit_cb,
    ._event_cb = _event_cb,
    .type = OBJ_TYPE,
    .size_instance = (sizeof(gt_obj_st)),
    .size_style = (sizeof(_gt_style_obj_st))
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static void _gt_obj_init_style(gt_obj_st * obj)
{
    _gt_style_obj_st * style = obj->style;
    style->bgcolor = gt_color_hex(0xFFFFFF);
}

static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class)
{
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");
    /* code */

    GT_LOGV(GT_LOG_TAG_GUI, "end init_cb");
}

static void _deinit_cb(gt_obj_st * obj, const gt_obj_class_st * class)
{
    GT_LOGV(GT_LOG_TAG_GUI, "start deinit_cb");
    /* code */

    GT_LOGV(GT_LOG_TAG_GUI, "end deinit_cb");
}

static void _event_cb(gt_obj_st * obj, struct _gt_event_s * e)
{
    gt_event_type_et code = gt_event_get_code(e);
    switch(code) {
        case GT_EVENT_TYPE_DRAW_START:
            GT_LOGD(GT_LOG_TAG_GUI, "start draw");
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
            GT_LOGD(GT_LOG_TAG_GUI, "obj scroll");
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


/* global functions / API interface -------------------------------------*/



gt_obj_st * gt_obj_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    _gt_obj_init_style(obj);
    GT_LOGV(GT_LOG_TAG_GUI, "create obj type:%d", *MY_CLASS.type);
    return obj;
}

// need fix
void gt_obj_del(gt_obj_st * obj)
{
    GT_LOGV(GT_LOG_TAG_GUI, "del obj:%p", obj);
    gt_mem_free(obj);
}

void gt_screen_set_bgcolor(gt_obj_st * obj, gt_color_t color)
{
    if( obj->class->type != GT_TYPE_OBJ ){
        return;
    }
    _gt_style_obj_st * style = obj->style;
    style->bgcolor = color;
}

gt_color_t gt_screen_get_bgcolor(gt_obj_st * obj)
{
    if( obj->class->type != GT_TYPE_OBJ ){
        return *(gt_color_t*)0;
    }
    _gt_style_obj_st * style = obj->style;
    return style->bgcolor;
}

/* end ------------------------------------------------------------------*/
