/**
 * @file gt_qrcode.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-08-24 16:37:09
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_qrcode.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../font/gt_font.h"
#include "../others/gt_assert.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_BTN
#define MY_CLASS    &gt_qr_code_class

/* private typedef ------------------------------------------------------*/


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct _gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_qr_code_class = {
    ._init_cb = _init_cb,
    ._deinit_cb = _deinit_cb,
    ._event_cb = _event_cb,
    .type = OBJ_TYPE,
    .size_instance = (sizeof(gt_obj_st)),
    .size_style = (sizeof(_gt_style_qrcode_st))
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static inline void _gt_qr_code_init_widget(gt_obj_st * qr_code) {
    // _gt_style_qrcode_st * style = qr_code->style;
    // char * val = style->value;

    /**/

}

/**
 * @brief obj init qr_code widget cb
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");

    _gt_qr_code_init_widget(obj);
}

/**
 * @brief obj deinit cb
 *
 * @param obj
 */
static void _deinit_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "start deinit_cb");
    /* code */
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
        case GT_EVENT_TYPE_DRAW_START: {
            GT_LOGD(GT_LOG_TAG_GUI, "start draw");
            if( gt_obj_check_scr(obj) ){
                extern void gt_disp_ref_area(gt_area_st * area);
                gt_disp_ref_area(&obj->area);
            }
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;
        }
        case GT_EVENT_TYPE_DRAW_END: {
            GT_LOGD(GT_LOG_TAG_GUI, "end draw");
            break;
        }
        case GT_EVENT_TYPE_CHANGE_CHILD_REMOVE: {/* remove child from screen but not delete */
            GT_LOGD(GT_LOG_TAG_GUI, "child remove");
			break;
        }
        case GT_EVENT_TYPE_CHANGE_CHILD_DELETE: {/* delete child */
            GT_LOGD(GT_LOG_TAG_GUI, "child delete");
            break;
        }
        case GT_EVENT_TYPE_INPUT_CLICKING: {  /* add clicking style and process clicking event */
            GT_LOGD(GT_LOG_TAG_GUI, "clicking");

            break;
        }
        case GT_EVENT_TYPE_INPUT_SCROLL: {
            GT_LOGD(GT_LOG_TAG_GUI, "scroll");
            break;
        }
        case GT_EVENT_TYPE_INPUT_PROCESS: {  /* remove clicking style and process click event */
            GT_LOGD(GT_LOG_TAG_GUI, "process");
            /* code here */

            gt_event_send(obj, GT_EVENT_TYPE_INPUT_PROCESSED, NULL);
            break;
        }
        case GT_EVENT_TYPE_INPUT_PROCESSED: {/* click event finish */
            GT_LOGD(GT_LOG_TAG_GUI, "processed");
            break;
        }
        default:
            break;
    }
}


static void _gt_qrcode_init_style(gt_obj_st * qr_code)
{
    // _gt_style_qrcode_st * style = (_gt_style_qrcode_st * )qr_code->style;

    /* code */
}




/* global functions / API interface -------------------------------------*/

/**
 * @brief create a qr_code obj
 *
 * @param parent qr_code's parent element
 * @return gt_obj_st* qr_code obj
 */
gt_obj_st * gt_qrcode_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    _gt_qrcode_init_style(obj);
    return obj;
}

/* end ------------------------------------------------------------------*/
