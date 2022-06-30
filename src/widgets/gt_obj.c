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


/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_OBJ
#define MY_CLASS    &gt_obj_class

/* private typedef ------------------------------------------------------*/


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class);
static void _deinit_cb(gt_obj_st * obj,const gt_obj_class_st * class);
static void _event_cb(gt_obj_st * obj,const gt_obj_class_st * class);

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
static void _init_cb(gt_obj_st * obj,const gt_obj_class_st * class)
{
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");
    /* code */

    GT_LOGV(GT_LOG_TAG_GUI, "end init_cb");
}

static void _deinit_cb(gt_obj_st * obj,const gt_obj_class_st * class)
{
    GT_LOGV(GT_LOG_TAG_GUI, "start deinit_cb");
    /* code */

    GT_LOGV(GT_LOG_TAG_GUI, "end deinit_cb");
}

static void _event_cb(gt_obj_st * obj,const gt_obj_class_st * class){
    
}


/* global functions / API interface -------------------------------------*/



gt_obj_st * gt_obj_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    GT_LOGV(GT_LOG_TAG_GUI, "create obj type:%d", *MY_CLASS.type);
    return obj;
}

void gt_obj_del(gt_obj_st * obj)
{
    GT_LOGV(GT_LOG_TAG_GUI, "create obj:%p", obj);
    gt_mem_free(obj);
}



/* end ------------------------------------------------------------------*/
