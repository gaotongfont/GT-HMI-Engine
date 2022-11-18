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

static gt_obj_st * _gt_obj_class_destroy_screen(gt_obj_st * self)
{
    gt_size_t i = 0;
    gt_size_t idx = -1;
    gt_disp_st * disp = gt_disp_get_default();
    gt_obj_st * ret_p = NULL;

    if (NULL == disp || 0 == disp->cnt_scr) {
        return ;
    }

    if (NULL == disp->screens) {
        return ;
    }

    // find out self index
    for (i = disp->cnt_scr - 1; i >= 0; i--) {
        if (disp->screens[i] == self) {
            idx = i;
            ret_p = disp->screens[idx];
            break;
        }
    }
    if (-1 == idx) {
        return ;
    }

    // release screen count, later data is moved to the front
    if (idx != disp->cnt_scr--) {
        gt_memcpy(disp->screens[idx], disp->screens[idx + 1], (disp->cnt_scr - idx) * sizeof(gt_obj_st *));
    }
    disp->screens = gt_mem_realloc(disp->screens, disp->cnt_scr * sizeof(gt_obj_st * ));
    return ret_p;
}

static gt_obj_st * _gt_obj_class_destroy_from_parent(gt_obj_st * self)
{
    gt_size_t i = 0;
    gt_size_t idx = -1;
    gt_obj_st * parent = self->parent;
    gt_obj_st * ret_p = NULL;

    if (NULL == parent) {
        return ret_p;
    }

    if (NULL == parent->child || 0 == parent->cnt_child) {
        return ret_p;
    }

    for (i = parent->cnt_child - 1; i >= 0; i--) {
        if (parent->child[i] == self) {
            idx = i;
            ret_p = parent->child[idx];
            break;
        }
    }
    if (idx != parent->cnt_child--) {
        gt_memcpy(&parent->child[idx], &parent->child[idx + 1], (parent->cnt_child - idx) * sizeof(gt_obj_st * ));
    }
    parent->child = gt_mem_realloc(parent->child, parent->cnt_child * sizeof(gt_obj_st * ));
    return ret_p;
}

static bool _gt_obj_class_destroy_children(gt_obj_st * self)
{
    gt_size_t i = 0;

    if (NULL == self->child || 0 == self->cnt_child) {
        self->class->_deinit_cb(self);

        // event attribute
        if (NULL != self->event_attr) {
            gt_mem_free(self->event_attr);
            self->event_attr = NULL;
        }

        self->draw_ctx = NULL;
        self->class = NULL;
        gt_mem_free(self);
        self = NULL;
        return true;
    }

    for (i = self->cnt_child - 1; i >= 0; i--) {
        if (_gt_obj_class_destroy_children(self->child[i])) {
            self->child[i] = NULL;
        }
    }

    // release child array memory
    if (NULL != self->child) {
        gt_mem_free(self->child);
        self->child = NULL;
    }
    self->cnt_child = 0;
    return false;
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
    obj->style = gt_mem_malloc(get_style_size(class));

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
            disp->screens[disp->cnt_scr++] = obj;
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

struct _gt_obj_s * gt_obj_class_destroy(struct _gt_obj_s * self)
{
    gt_obj_st * ret_p = NULL;

    _gt_obj_class_destroy_children(self);

    if (NULL == self->parent) {
        ret_p = _gt_obj_class_destroy_screen(self);
    } else {
        ret_p = _gt_obj_class_destroy_from_parent(self);
    }
    return ret_p;
}

gt_obj_type_et gt_obj_class_get_type(struct _gt_obj_s * obj) {
    return obj->class->type;
}


/* end ------------------------------------------------------------------*/
