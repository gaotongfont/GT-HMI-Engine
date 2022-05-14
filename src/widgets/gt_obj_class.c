/**
 * @file gt_obj_class.c
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-05-12 10:34:19
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_obj_class.h"
#include "gt_obj.h"
#include "../core/gt_mem.h"
#include "../others/gt_types.h"


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static uint32_t get_instance_size(const gt_obj_class_st * class){
    return class->size_instance;
}

static uint32_t get_style_size(const gt_obj_class_st * class){
    return class->size_style;
}


/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_obj_class_create(const gt_obj_class_st * class, gt_obj_st * parent){
    uint32_t is = get_instance_size(class);
    gt_obj_st * obj = gt_mem_malloc(is);
    obj->class = class;
    obj->parent = parent;
    
    uint32_t ss = get_style_size(class);
    obj->style = gt_mem_malloc(ss);

    gt_memset_0(obj, is);

    if(parent == NULL){
        GT_LOGV(GT_LOG_TAG_GUI, "create class type is screen");
    }

    return obj;
}

gt_obj_type_et gt_obj_class_get_type(struct _gt_obj_s * obj){
    return obj->class->type;
}


/* end ------------------------------------------------------------------*/
