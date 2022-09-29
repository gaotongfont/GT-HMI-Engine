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
#include "../hal/gt_hal.h"
#include "stdint.h"
#include "../others/gt_log.h"
#include "../others/gt_assert.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static uint32_t get_instance_size(const gt_obj_class_st * class) {
    return class->size_instance;
}

static uint32_t get_style_size(const gt_obj_class_st * class) {
    return class->size_style;
}


/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_obj_class_create(const gt_obj_class_st * class, gt_obj_st * parent) {

    /* check type */
    if ( parent && parent->class->type == class->type ){
        GT_LOGV(GT_LOG_TAG_GUI, "cannot create an object of the same type as the parent object onto the parent object");
        return NULL;
    }

    uint32_t is = get_instance_size(class);
    gt_obj_st * obj = gt_mem_malloc(is);

	gt_memset_0(obj, is);

    obj->class = class;
    obj->parent = parent;

    obj->opa = GT_OPA_100;

    uint32_t ss = get_style_size(class);
    obj->style = gt_mem_malloc(ss);

    if (parent == NULL) {
        GT_LOGV(GT_LOG_TAG_GUI, "create a screen");
        gt_disp_st * disp = gt_disp_get_default();

        if (disp == NULL) {
            GT_LOGW(GT_LOG_TAG_GUI, "disp is null, please init disp");
            return NULL;
        }

        if (disp->screens == NULL || disp->cnt_scr == 0 ) {
            disp->screens = gt_mem_malloc(sizeof(gt_obj_st *));
            disp->screens[disp->cnt_scr++] = obj;
        } else {
            // gt_mem_realloc can't operate NULL ptr
            disp->screens = gt_mem_realloc(disp->screens, sizeof(gt_obj_st *) * (disp->cnt_scr + 1));
            disp->screens[disp->cnt_scr++] = 0;
        }
        obj->area.x = 0;
        obj->area.y = 0;
        obj->area.w = gt_disp_get_res_hor(NULL);
        obj->area.h = gt_disp_get_res_ver(NULL);
    } else {
        GT_LOGV(GT_LOG_TAG_GUI, "create a normal obj");
        if (parent->child == NULL || parent->cnt_child == 0 ) {
            parent->child = gt_mem_malloc(sizeof(gt_obj_st *));
            parent->child[0] = obj;
            parent->cnt_child = 1;
        } else {
            parent->child = gt_mem_realloc(parent->child, sizeof(gt_obj_st *) * (parent->cnt_child + 1));
            parent->child[parent->cnt_child++] = obj;
        }
    }
    return obj;
}

gt_obj_type_et gt_obj_class_get_type(struct _gt_obj_s * obj) {
    return obj->class->type;
}


/* end ------------------------------------------------------------------*/
