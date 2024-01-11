/**
 * @file gt_obj_class.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-05-12 10:34:19
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

 /* include --------------------------------------------------------------*/
#include "gt_obj_class.h"
#include "gt_obj.h"
#include "../core/gt_mem.h"
#include "../core/gt_style.h"
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
static uint32_t get_instance_size(void) {
    return sizeof(gt_obj_st);
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
        return ret_p;
    }

    if (NULL == disp->screens) {
        return ret_p;
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
        return ret_p;
    }
    // release screen count, later data is moved to the front
    if (idx != disp->cnt_scr--) {
        gt_memmove(&disp->screens[idx], &disp->screens[idx + 1], (disp->cnt_scr - idx) * sizeof(gt_obj_st * ));
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
    if (-1 == idx) {
        return ret_p;
    }
    if (idx != parent->cnt_child--) {
        gt_memmove(&parent->child[idx], &parent->child[idx + 1], (parent->cnt_child - idx) * sizeof(gt_obj_st * ));
    }
    parent->child = gt_mem_realloc(parent->child, parent->cnt_child * sizeof(gt_obj_st * ));
    return ret_p;
}

static void _gt_obj_class_destroy_property(gt_obj_st * self) {
    if (NULL == self) {
        return;
    }
    // free event attribute
    if (NULL != self->event_attr) {
        gt_mem_free(self->event_attr);
        self->event_attr = NULL;
    }

    // free obj custom style
    if (self->class->_deinit_cb) {
        self->class->_deinit_cb(self);
    }

    // free the malloc memory, normally it will be release by widget's _deinit_cb()
    if (self->style) {
        gt_mem_free(self->style);
        self->style = NULL;
    }

    self->draw_ctx = NULL;
    self->class = NULL;
}

static inline void _gt_obj_class_destroy_self(gt_obj_st * self) {
    if (NULL == self) {
        return;
    }
    _gt_obj_class_destroy_property(self);
    gt_mem_free(self);
    self = NULL;
}

/**
 * @brief Destroy the object and its child objects
 *
 * @param self The Current object
 * @param is_root true: root object, false: child object
 * @return true The deepest child object is released, return recursively
 * @return false Go to the next level of child object
 */
static bool _gt_obj_class_destroy_children(gt_obj_st * self, bool is_root)
{
    gt_size_t i = 0;

    if (NULL == self->child || 0 == self->cnt_child) {
        _gt_obj_class_destroy_property(self);
        /**
         * @brief free obj memory, root need to release disp memory,
         *       child only need to release obj memory in here
         */
        if (!is_root) {
            gt_mem_free(self);
            self = NULL;
        }
        return true;
    }

    // release child object from bottom to top
    for (i = self->cnt_child - 1; i >= 0; i--) {
        if (_gt_obj_class_destroy_children(self->child[i], false)) {
            self->child[i] = NULL;  /** can not repeat free memory here */
        }
    }

    // release child array memory
    if (NULL != self->child) {
        gt_mem_free(self->child);
        self->child = NULL;
    }
    self->cnt_child = 0;
    if (!is_root) {
        _gt_obj_class_destroy_self(self);
        self = NULL;
    }
    return false;
}

static inline bool _add_obj_to_parent(gt_obj_st * obj, gt_obj_st * parent) {
    if (parent->child == NULL || parent->cnt_child == 0 ) {
        parent->child = gt_mem_malloc(sizeof(gt_obj_st *));
        if (!parent->child) {
            return false;
        }
        parent->child[0] = obj;
        parent->cnt_child = 1;
    } else {
        parent->child = gt_mem_realloc(parent->child, sizeof(gt_obj_st *) * (parent->cnt_child + 1));
        if (!parent->child) {
            return false;
        }
        parent->child[parent->cnt_child++] = obj;
    }
    // GT_LOGV(GT_LOG_TAG_GUI, "create a normal obj: %p, parent: %p, parent child: %p, count: %d", parent->child[parent->cnt_child - 1], parent, parent->child, parent->cnt_child);
    gt_event_send(parent, GT_EVENT_TYPE_CHANGE_CHILD_ADD, obj);
    return true;
}

/* global functions / API interface -------------------------------------*/
struct gt_obj_s * gt_obj_class_create(const gt_obj_class_st * class, struct gt_obj_s * parent) {

    /* check type */
    if ( parent && parent->class->type == class->type ){
        if (GT_TYPE_OBJ != class->type) {
            GT_LOGV(GT_LOG_TAG_GUI, "cannot create an object of the same type as the parent object onto the parent object");
            goto null_lb;
        }
    }

    uint32_t is = get_instance_size();
    gt_obj_st * obj = gt_mem_malloc(is);
    if (!obj) {
        goto null_lb;
    }
    gt_memset_0(obj, is);

    obj->class      = class;
    obj->parent     = parent;
    obj->opa        = GT_OPA_100;
    obj->style      = gt_mem_malloc(get_style_size(class));
    obj->visible    = GT_VISIBLE;
    obj->scroll_dir = GT_SCROLL_ALL;

    /** Inherit from the parent class */
    _gt_obj_class_inherent_attr_from_parent(obj, parent);

    if (!obj->style) {
        goto obj_lb;
    }

    if (parent == NULL) {
        GT_LOGV(GT_LOG_TAG_GUI, "create a screen");
        gt_disp_st * disp = gt_disp_get_default();

        if (disp == NULL) {
            GT_LOGW(GT_LOG_TAG_GUI, "disp is null, please init disp");
            goto style_lb;
        }

        if (disp->screens == NULL || disp->cnt_scr == 0 ) {
            disp->screens = gt_mem_malloc(sizeof(gt_obj_st *));
            if (!disp->screens) {
                goto style_lb;
            }
            disp->screens[disp->cnt_scr++] = obj;
        } else {
            // gt_mem_realloc can't operate NULL ptr
            disp->screens = gt_mem_realloc(disp->screens, sizeof(gt_obj_st *) * (disp->cnt_scr + 1));
            if (!disp->screens) {
                goto style_lb;
            }
            disp->screens[disp->cnt_scr++] = obj;
        }
        obj->area.x = 0;
        obj->area.y = 0;
        obj->area.w = gt_disp_get_res_hor(NULL);
        obj->area.h = gt_disp_get_res_ver(NULL);
    } else {
        if (false == _add_obj_to_parent(obj, parent)) {
            goto style_lb;
        }
    }

    return obj;

style_lb:
    gt_mem_free(obj->style);
obj_lb:
    gt_mem_free(obj);
null_lb:
    return NULL;
}

struct gt_obj_s * _gt_obj_class_change_parent(struct gt_obj_s * obj, struct gt_obj_s * to)
{
    gt_obj_st * parent = obj->parent;
    bool is_success = false;

    is_success = _add_obj_to_parent(obj, to);
    if (false == is_success) {
        return parent;
    }

    _gt_obj_class_destroy_from_parent(obj);
    obj->parent = to;
    return obj->parent;
}

/**
 * @brief Release the target object from memory
 *
 * @param self The target object which want to be delete from memory
 */
void _gt_obj_class_destroy(struct gt_obj_s * self)
{
    gt_obj_st * ret_p = NULL;

    _gt_obj_class_destroy_children(self, true);

    if (NULL == self->parent) {
        ret_p = _gt_obj_class_destroy_screen(self);
        if (NULL == ret_p) {
            GT_LOGW(GT_LOG_TAG_GUI, "Destroy screen failed");
        }
    } else {
        ret_p = _gt_obj_class_destroy_from_parent(self);
        if (NULL == ret_p) {
            GT_LOGW(GT_LOG_TAG_GUI, "Destroy obj from parent failed");
        }
    }

    _gt_obj_class_destroy_self(self);
    self = NULL;
}

void _gt_obj_class_inherent_attr_from_parent(struct gt_obj_s * obj, struct gt_obj_s * parent)
{
    if (NULL == obj || NULL == parent) {
        return;
    }
    gt_obj_set_overflow(obj, parent->overflow);
    gt_obj_child_set_prop(obj, GT_OBJ_PROP_TYPE_OVERFLOW, parent->overflow);
}


gt_obj_type_et gt_obj_class_get_type(struct gt_obj_s * obj) {
    if (NULL == obj || NULL == obj->class) {
        return GT_TYPE_UNKNOWN;
    }
    return obj->class->type;
}


/* end ------------------------------------------------------------------*/
