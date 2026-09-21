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
#include "./gt_obj.h"
#include "../core/gt_obj_pos.h"
#include "../core/gt_mem.h"
#include "../core/gt_style.h"
#include "../core/gt_disp.h"
#include "../core/gt_indev.h"
#include "../others/gt_types.h"
#include "../hal/gt_hal_disp.h"
#include "../hal/gt_hal_indev.h"
#include "stdint.h"
#include "../others/gt_log.h"
#include "../others/gt_assert.h"
#include "../core/gt_layout.h"

/* private define -------------------------------------------------------*/
#ifndef _GT_OBJ_CLASS_SCREEN_LOG
    #define _GT_OBJ_CLASS_SCREEN_LOG        0
#endif

#ifndef _GT_OBJ_CLASS_WIDGETS_LOG
    #define _GT_OBJ_CLASS_WIDGETS_LOG       0
#endif



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static inline uint32_t get_style_size(const gt_obj_class_st * c) {
    return c->size_style;
}

static GT_ATTRIBUTE_RAM_TEXT gt_obj_st * _gt_obj_class_destroy_screen(gt_obj_st * self) {
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
    if (idx < --disp->cnt_scr) {
        gt_memmove(&disp->screens[idx], &disp->screens[idx + 1], (disp->cnt_scr - idx) * sizeof(gt_obj_st * ));
    }
    disp->screens[disp->cnt_scr] = NULL;
    if (0 == disp->cnt_scr) {
        gt_mem_free(disp->screens);
        disp->screens = NULL;
    } else {
        gt_obj_st ** tmp = (gt_obj_st ** )gt_mem_realloc(disp->screens, disp->cnt_scr * sizeof(gt_obj_st * ));
        if (tmp) { disp->screens = tmp; }
    }

    /** The screen is being freed. Drop the switch-animation reference so an
     *  in-flight anim ready callback (or a pending load) never reads a dangling
     *  disp->scr_prev after this screen was popped and destroyed mid-anim. */
    if (disp->scr_prev == self) {
        disp->scr_prev = NULL;
    }

    return ret_p;
}

/**
 * @brief
 *
 * @param self
 * @return gt_obj_st * Want to free the object
 */
static GT_ATTRIBUTE_RAM_TEXT gt_obj_st * _gt_obj_class_destroy_from_parent(gt_obj_st * self) {
    gt_size_t i = 0;
    gt_size_t idx = -1;
    gt_obj_st * parent = self->parent;
    gt_obj_st * ret_p = NULL;

    if (gt_obj_is_type(parent, GT_TYPE_UNKNOWN)) {
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
    if (idx < --parent->cnt_child) {
        gt_memmove(&parent->child[idx], &parent->child[idx + 1], (parent->cnt_child - idx) * sizeof(gt_obj_st * ));
    }
    parent->child[parent->cnt_child] = NULL;
    if (0 == parent->cnt_child) {
        gt_mem_free(parent->child);
        parent->child = NULL;
    } else {
        gt_obj_st ** tmp = (gt_obj_st ** )gt_mem_realloc(parent->child, parent->cnt_child * sizeof(gt_obj_st * ));
        if (tmp) { parent->child = tmp; }
    }
    return ret_p;
}

static GT_ATTRIBUTE_RAM_TEXT void _gt_obj_class_destroy_property(gt_obj_st * self) {
    GT_CHECK_BACK(self);

    // free scroll cold data first to prevent re-entry via CHANGE_DELETED callbacks
    if (self->scroll_cold) {
        gt_cold_scroll_free(self->scroll_cold);
        self->scroll_cold = NULL;
    }

    gt_event_send(self, GT_EVENT_TYPE_CHANGE_DELETED, NULL);
    gt_anim_del_safe_by(self);
    _gt_indev_remove_want_delate_target(self);
    _gt_event_remove_want_delate_target(self);

    // free event attribute
    gt_obj_remove_all_event_cb(self);

    // free obj custom style
    if (self->classes->_deinit_cb) {
        self->classes->_deinit_cb(self);
    }
    self->draw_ctx = NULL;
    self->classes = NULL;
    self->parent = NULL;

    // free scroll cold data again in case callbacks re-allocated it
    if (self->scroll_cold) {
        gt_cold_scroll_free(self->scroll_cold);
        self->scroll_cold = NULL;
    }
}

static GT_ATTRIBUTE_RAM_TEXT inline void _gt_obj_class_destroy_self(gt_obj_st * self) {
    if (gt_obj_is_type(self, GT_TYPE_UNKNOWN)) {
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
static GT_ATTRIBUTE_RAM_TEXT bool _destroy_and_free_children(struct gt_obj_s * self, bool is_root) {
    gt_size_t i = 0;

    if (NULL == self->child || 0 == self->cnt_child) {
        self->cnt_child = 0;
        if (false == is_root) {
            /**
             * @brief free obj memory, root need to release disp memory,
             *       child only need to release obj memory in here
             */
            _gt_obj_class_destroy_self(self);
        }
        return true;
    }
    if (gt_obj_is_type(self, GT_TYPE_UNKNOWN)) {
        return false;
    }
    gt_event_send(self, GT_EVENT_TYPE_CHANGE_CHILD_DELETE, NULL);

    // release child object from bottom to top
    for (i = self->cnt_child - 1; i >= 0; i--) {
        gt_obj_st * child = self->child[i];
        if (_gt_obj_in_event_dispatch(child)) {
            child->using_sta = false;
            child->parent = NULL;
            gt_obj_destroy(child);
            self->child[i] = NULL;
        } else if (_destroy_and_free_children(child, false)) {
            self->child[i] = NULL;  /** can not repeat free memory here */
        }
    }

    gt_event_send(self, GT_EVENT_TYPE_CHANGE_CHILD_DELETED, NULL);

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
    if (parent->child == NULL) {
        parent->cnt_child = 0;
        gt_obj_st ** child = gt_mem_malloc(sizeof(gt_obj_st *));
        if (!child) {
            return false;
        }
        child[0] = obj;
        parent->child = child;
        parent->cnt_child = 1;
    } else {
        gt_obj_st ** tmp = gt_mem_realloc(parent->child, sizeof(gt_obj_st *) * (parent->cnt_child + 1));
        if (!tmp) {
            return false;
        }
        parent->child = tmp;
        parent->child[parent->cnt_child++] = obj;
    }
    // GT_LOGV(GT_LOG_TAG_GUI, "create a normal obj: %p, parent: %p, parent child: %p, count: %d", parent->child[parent->cnt_child - 1], parent, parent->child, parent->cnt_child);
    if (parent->row_layout) {
        // row calc width
        gt_layout_row_grow(parent);
    }

    gt_event_send(parent, GT_EVENT_TYPE_CHANGE_CHILD_ADD, obj);
    return true;
}

static GT_ATTRIBUTE_RAM_TEXT bool _create_new_screen_obj(gt_obj_st * obj) {
    gt_disp_st * disp = gt_disp_get_default();

    if (disp == NULL) {
        GT_LOGW(GT_LOG_TAG_GUI, "disp is null, please init disp");
        return false;
    }

    if (disp->screens == NULL) {
        disp->cnt_scr = 0;
        gt_obj_st ** screens = gt_mem_malloc(sizeof(gt_obj_st *));
        if (!screens) {
            return false;
        }
        screens[0] = obj;
        disp->screens = screens;
        disp->cnt_scr = 1;
    } else {
        gt_obj_st ** tmp = gt_mem_realloc(disp->screens, sizeof(gt_obj_st *) * (disp->cnt_scr + 1));
        if (!tmp) {
            return false;
        }
        disp->screens = tmp;
        disp->screens[disp->cnt_scr++] = obj;
    }
    obj->area.x = 0;
    obj->area.y = 0;
    obj->area.w = gt_disp_get_res_hor(NULL);
    obj->area.h = gt_disp_get_res_ver(NULL);
    obj->show_bg = true;

    return true;
}

/* global functions / API interface -------------------------------------*/
struct gt_obj_s * gt_obj_class_create(const gt_obj_class_st * c, struct gt_obj_s * parent)
{
    uint32_t is = get_style_size(c);
    gt_obj_st * obj = gt_mem_malloc(is);
    if (!obj) {
        return NULL;
    }
    gt_memset_0(obj, is);

    obj->classes    = c;
    obj->parent     = parent;
    obj->id         = -1;
    obj->opa        = GT_OPA_100;
    obj->visible    = GT_VISIBLE;
    obj->scroll_dir = GT_SCROLL_ALL;
    obj->bgcolor    = gt_color_white();
    obj->radius     = 4;
    obj->focus_w    = FOCUS_W_DEFAULT;
    obj->focus_gap  = FOCUS_W_DEFAULT;
    obj->focus_dis  = GT_DISABLED;
    obj->opa_tip_bar = GT_SCROLL_TIP_BAR_MAX_OPA;
    obj->use_theme   = 1;

    /** Inherit from the parent class */
    if (parent) {
        obj->area.x = parent->area.x;
        obj->area.y = parent->area.y;
    }
    _gt_obj_class_inherent_attr_from_parent(obj, parent);

    if (parent == NULL) {
        if (false == _create_new_screen_obj(obj)) {
            goto obj_lb;
        }
#if _GT_OBJ_CLASS_SCREEN_LOG
        GT_LOGD(GT_LOG_TAG_CHECK, "Create scr: %p(%d), size: 0x%x(%d)", obj, gt_obj_class_get_type(obj), is, is);
#endif
        return obj;
    }

    /** widget object */
    obj->fixed = true;
    obj->inside = true;
    if (false == _add_obj_to_parent(obj, parent)) {
        goto obj_lb;
    }
#if _GT_OBJ_CLASS_WIDGETS_LOG
        GT_LOGD(GT_LOG_TAG_CHECK, "Create obj: %p(%d), size: 0x%x(%d)", obj, gt_obj_class_get_type(obj), is, is);
#endif
    return obj;

obj_lb:
    gt_mem_free(obj);
    return NULL;
}

gt_obj_type_et gt_obj_class_get_type(struct gt_obj_s * obj)
{
    if (NULL == obj || NULL == obj->classes) {
        return GT_TYPE_UNKNOWN;
    }
    if (obj->classes->type >= GT_TYPE_TOTAL) {
        return GT_TYPE_UNKNOWN;
    }
    if (obj->classes->type < GT_TYPE_UNKNOWN) {
        return GT_TYPE_UNKNOWN;
    }
    return obj->classes->type;
}

bool gt_obj_is_type(struct gt_obj_s * obj, gt_obj_type_et type)
{
    if (NULL == obj || NULL == obj->classes) {
        return GT_TYPE_UNKNOWN == type ? true : false;
    }
    if (GT_TYPE_TOTAL == type) {
        if (GT_TYPE_UNKNOWN == obj->classes->type) {
            return false;
        }
        return true;
    }
    return obj->classes->type == type;
}

struct gt_obj_s * _gt_obj_class_change_parent(struct gt_obj_s * obj, struct gt_obj_s * to)
{
    gt_obj_st * parent = obj->parent;
    bool is_success = false;
    if (gt_obj_is_type(parent, GT_TYPE_UNKNOWN)) {
        return NULL;
    }
    if (gt_obj_is_type(to, GT_TYPE_UNKNOWN)) {
        return parent;
    }
    if (parent == to) {
        return parent;
    }

    gt_event_send(parent, GT_EVENT_TYPE_CHANGE_CHILD_REMOVE, obj);

    is_success = _add_obj_to_parent(obj, to);
    if (false == is_success) {
        return parent;
    }

    _gt_obj_class_destroy_from_parent(obj);
    obj->parent = to;

    if (obj->viewport_fixed) {
        obj->viewport_fixed = 0;
        if (gt_obj_is_type(parent, GT_TYPE_SCREEN) && parent == gt_disp_get_scr()) {
            _gt_disp_reload_max_area(parent);
        }
        GT_LOGW(GT_LOG_TAG_UI, "viewport_fixed cleared after reparent; configure it again on the new screen: %p(%d)",
                obj, gt_obj_class_get_type(obj));
    }

    gt_event_send(parent, GT_EVENT_TYPE_CHANGE_CHILD_REMOVED, obj);

    return obj->parent;
}

gt_res_t _gt_obj_class_destroy(struct gt_obj_s * self)
{
    gt_obj_st * ret_p = NULL;
    if (gt_obj_is_type(self, GT_TYPE_UNKNOWN)) {
        GT_LOGW(GT_LOG_TAG_CHECK, "Want to destroy unknown obj: %p(%d)", self, gt_obj_class_get_type(self));
        return GT_RES_FAIL;
    }
    _destroy_and_free_children(self, true);

    if (gt_obj_is_type(self, GT_TYPE_SCREEN)) {
        ret_p = _gt_obj_class_destroy_screen(self);
        if (NULL == ret_p) {
            GT_LOGW(GT_LOG_TAG_CHECK, "Destroy screen(%p) failed", self);
        }
        _gt_scr_stack_remove_pop_scr(self);
#if _GT_OBJ_CLASS_SCREEN_LOG
        GT_LOGD(GT_LOG_TAG_CHECK, "Destroy scr: %p(%d)", self, gt_obj_class_get_type(self));
#endif
    } else {
        ret_p = _gt_obj_class_destroy_from_parent(self);
        if (NULL == ret_p && self->parent && self->parent->child && self->parent->cnt_child) {
            GT_LOGW(GT_LOG_TAG_CHECK, "Destroy obj: %p(%d) from parent: %p(%d) failed", self,
                gt_obj_class_get_type(self), self->parent, gt_obj_class_get_type(self->parent));
        }
#if _GT_OBJ_CLASS_WIDGETS_LOG
        GT_LOGD(GT_LOG_TAG_CHECK, "Destroy obj: %p(%d)", self, gt_obj_class_get_type(self));
#endif
    }

#if GT_USE_LAYER_TOP
    if (gt_disp_get_layer_top() == self) {
        /** Do not release layer top object */
        return GT_RES_OK;
    }
#endif
    _gt_obj_class_destroy_self(self);
    self = NULL;
    return GT_RES_OK;
}

gt_res_t _gt_obj_class_destroy_children(struct gt_obj_s * self)
{
    if (gt_obj_is_type(self, GT_TYPE_UNKNOWN)) {
        GT_LOGW(GT_LOG_TAG_CHECK, "Want to destroy unknown obj's childs: %p(%d)", self, gt_obj_class_get_type(self));
        return GT_RES_FAIL;
    }
    /** Unlink every child from the parent up front so the container is empty the
     *  moment this call returns: a handler that rebuilds the list right after
     *  clearing it (list item click -> clear -> add) must start from index 0 and
     *  must not be wiped by a deferred clear that fires ~10ms later.
     *
     *  Only the memory release is deferred. A child still referenced by the
     *  in-flight event dispatch chain cannot be freed here or the dispatcher
     *  would keep walking freed memory, so it is handed to gt_obj_destroy()'s
     *  timer instead. Its ->parent is cleared so a later deferred destroy never
     *  dereferences a parent that may already have been freed. */
    gt_obj_st ** child_arr = self->child;
    gt_size_t cnt = self->cnt_child;
    self->child = NULL;
    self->cnt_child = 0;
    if (NULL == child_arr) {
        cnt = 0;
    }
    for (gt_size_t i = cnt - 1; i >= 0; i--) {
        gt_obj_st * child = child_arr[i];
        if (NULL == child) {
            continue;
        }
        if (_gt_obj_in_event_dispatch(child)) {
            child->using_sta = false;
            /** Detach from the parent: the child is destroyed later by its own
             *  deferred timer, but the parent (self) may already be freed by then.
             *  A dangling parent would be dereferenced by the event bubble
             *  (CHANGE_CHILD_DELETE) and by _gt_obj_class_destroy_from_parent(),
             *  both of which read parent->classes/child -> use-after-free. */
            child->parent = NULL;
            gt_obj_destroy(child);
        } else {
            /** Parent is already unlinked (self->child == NULL), so
             *  _gt_obj_class_destroy_from_parent() safely returns NULL inside
             *  _gt_obj_class_destroy(). Use the full recursive destroy so nested
             *  children (list item icons/labels) are released too. */
            _gt_obj_class_destroy(child);
        }
    }
    if (child_arr) {
        gt_mem_free(child_arr);
    }
    gt_event_send(self, GT_EVENT_TYPE_CHANGE_CHILD_DELETED, NULL);
    return GT_RES_OK;
}

gt_res_t _gt_obj_class_inherent_attr_from_parent(struct gt_obj_s * obj, struct gt_obj_s * parent)
{
    if (gt_obj_is_type(obj, GT_TYPE_UNKNOWN) || gt_obj_is_type(parent,  GT_TYPE_UNKNOWN)) {
        return GT_RES_FAIL;
    }
    gt_obj_set_overflow(obj, parent->overflow);
    gt_obj_child_set_prop(obj, GT_OBJ_PROP_TYPE_OVERFLOW, parent->overflow);
    return GT_RES_OK;
}

/* end ------------------------------------------------------------------*/
