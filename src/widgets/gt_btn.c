/**
 * @file gt_btn.c
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-05-11 15:03:35
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_btn.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"


/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_BTN
#define MY_CLASS    &gt_btn_class

/* private typedef ------------------------------------------------------*/


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj, gt_obj_class_st * class);
static void _deinit_cb(gt_obj_st * obj, gt_obj_class_st * class);

const gt_obj_class_st gt_btn_class = {
    ._init_cb = _init_cb,
    ._deinit_cb = _deinit_cb,
    .type = OBJ_TYPE,
    .size_instance = (sizeof(gt_obj_st)),
    .size_style = (sizeof(_gt_style_btn_st))
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj, gt_obj_class_st * class){}
static void _deinit_cb(gt_obj_st * obj, gt_obj_class_st * class){}



/* global functions / API interface -------------------------------------*/



/* end ------------------------------------------------------------------*/
