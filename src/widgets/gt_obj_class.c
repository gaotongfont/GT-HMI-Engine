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
#include "../core/gt_recycle.h"
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
    /** No realloc on delete: keep the reserved capacity so a concurrent screen
     *  scan never observes a moved array. Free + reset cap only when empty. */
    if (0 == disp->cnt_scr) {
        gt_mem_free(disp->screens);
        disp->screens = NULL;
        disp->cap_scr = 0;
    }
#if GT_OBJ_CHILD_ARRAY_SHRINK
    else if (disp->cap_scr >= 8 && disp->cnt_scr <= disp->cap_scr / 4) {
        /** High-watermark shrink: release the reserved headroom when few screens
         *  remain. realloc moves the array base, but this runs in the destroy
         *  path where the two-phase iterators already snapshot the count, so no
         *  in-flight scan observes the moved base. */
        uint16_t new_cap = disp->cap_scr / 2;
        gt_obj_st ** tmp = (gt_obj_st **)gt_mem_realloc(disp->screens, sizeof(gt_obj_st *) * new_cap);
        if (NULL != tmp) {
            disp->screens = tmp;
            disp->cap_scr = new_cap;
        }
    }
#endif

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
    /** No realloc on delete: keep the reserved capacity so a concurrent scan of
     *  parent->child[] never observes a moved array. The slot is freed and the
     *  capacity reset only when the container becomes empty. */
    if (0 == parent->cnt_child) {
        gt_mem_free(parent->child);
        parent->child = NULL;
        parent->cap_child = 0;
    }
#if GT_OBJ_CHILD_ARRAY_SHRINK
    else if (parent->cap_child >= 8 && parent->cnt_child <= parent->cap_child / 4) {
        /** High-watermark shrink (see _gt_obj_class_destroy_screen). */
        uint16_t new_cap = parent->cap_child / 2;
        gt_obj_st ** tmp = (gt_obj_st **)gt_mem_realloc(parent->child, sizeof(gt_obj_st *) * new_cap);
        if (NULL != tmp) {
            parent->child = tmp;
            parent->cap_child = new_cap;
        }
    }
#endif
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

/**
 * @brief Detach and destroy every child of @p self (recursively, two-phase).
 *        The child array itself is deferred for reclamation. @p self is NOT
 *        destroyed by this helper.
 *
 * @param self container whose children are released
 */
static GT_ATTRIBUTE_RAM_TEXT void _destroy_children_array(gt_obj_st * self) {
    gt_obj_st ** child_arr = self->child;
    gt_size_t cnt = self->cnt_child;
    self->child = NULL;
    self->cnt_child = 0;
    self->cap_child = 0;
    if (NULL == child_arr) {
        return;
    }

    /** release child object from bottom to top */
    for (gt_size_t i = cnt; i > 0; i--) {
        gt_obj_st * child = child_arr[i - 1];
        if (NULL == child) {
            continue;
        }
        /** Two-phase destroy: unlink + ZOMBIE + deferred reclamation. The parent
         *  child array is already detached above, so _gt_obj_class_destroy_from_parent()
         *  no-ops inside _gt_obj_class_destroy(). */
        _gt_obj_class_destroy(child);
    }

    gt_recycle_defer(child_arr);
}

static inline bool _add_obj_to_parent(gt_obj_st * obj, gt_obj_st * parent) {
    if (parent->cnt_child >= parent->cap_child) {
        /** Grow capacity (4 -> 8 -> 16 ...). Realloc happens only here, on
         *  growth, and parent->child is updated atomically right after, so a
         *  concurrent iteration of parent->child[] never observes a moved
         *  array. The delete path (see _gt_obj_class_destroy_from_parent)
         *  never reallocs, so shrinking can not invalidate an in-flight scan. */
        uint16_t new_cap = parent->cap_child ? parent->cap_child * 2 : 4;
        gt_obj_st ** tmp = parent->child
            ? (gt_obj_st **)gt_mem_realloc(parent->child, sizeof(gt_obj_st *) * new_cap)
            : (gt_obj_st **)gt_mem_malloc(sizeof(gt_obj_st *) * new_cap);
        if (!tmp) {
            return false;
        }
        parent->child = tmp;
        parent->cap_child = new_cap;
    }
    parent->child[parent->cnt_child++] = obj;
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

    if (disp->cnt_scr >= disp->cap_scr) {
        /** Grow capacity (4 -> 8 -> 16 ...). Realloc only on growth and the
         *  pointer is updated atomically, mirroring _add_obj_to_parent; the
         *  delete path never reallocs so a concurrent screen scan is safe. */
        uint16_t new_cap = disp->cap_scr ? disp->cap_scr * 2 : 4;
        gt_obj_st ** tmp = disp->screens
            ? (gt_obj_st **)gt_mem_realloc(disp->screens, sizeof(gt_obj_st *) * new_cap)
            : (gt_obj_st **)gt_mem_malloc(sizeof(gt_obj_st *) * new_cap);
        if (!tmp) {
            return false;
        }
        disp->screens = tmp;
        disp->cap_scr = new_cap;
    }
    disp->screens[disp->cnt_scr++] = obj;
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
    if (NULL == c) {
        /** public API: a NULL class table would crash get_style_size() */
        return NULL;
    }
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
    if (NULL == obj || NULL == obj->classes || GT_OBJ_LIFE_ZOMBIE == obj->life_state) {
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
    if (NULL == obj || NULL == obj->classes || GT_OBJ_LIFE_ZOMBIE == obj->life_state) {
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
    if (NULL == obj) {
        /** public API gt_obj_change_parent(NULL, to): obj->parent would crash */
        return NULL;
    }
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
    gt_obj_type_et type = gt_obj_class_get_type(self);
    if (GT_TYPE_UNKNOWN == type) {
        /** includes NULL/invalid and ZOMBIE: idempotent no-op */
        GT_LOGW(GT_LOG_TAG_CHECK, "Want to destroy unknown obj: %p(%d)", self, type);
        return GT_RES_FAIL;
    }
    if (GT_OBJ_LIFE_ALIVE != self->life_state) {
        /** Re-entrant destroy while a destroy is already in progress (DESTROYING)
         *  or already completed (ZOMBIE): idempotent no-op. */
        return GT_RES_FAIL;
    }

    bool has_child = (NULL != self->child && 0 != self->cnt_child);

#if GT_USE_LAYER_TOP
    if (gt_disp_get_layer_top() == self) {
        /** Do not release the layer-top object itself. Preserve the original
         *  semantics: its children are destroyed and it is unlinked from the
         *  screen registry, but the shell stays alive. */
        if (has_child) {
            gt_event_send(self, GT_EVENT_TYPE_CHANGE_CHILD_DELETE, NULL);
        }
        _destroy_children_array(self);
        if (has_child) {
            gt_event_send(self, GT_EVENT_TYPE_CHANGE_CHILD_DELETED, NULL);
        }
        _gt_obj_class_destroy_screen(self);
        _gt_scr_stack_remove_pop_scr(self);
        return GT_RES_OK;
    }
#endif

    /** Phase 1 gate: block re-entrant destroy while still letting
     *  gt_obj_is_type() resolve the real type so the teardown notifications
     *  (CHANGE_CHILD_DELETE/DELETED, CHANGE_DELETED) reach this object. */
    self->life_state = GT_OBJ_LIFE_DESTROYING;

    /** Phase 1a: destroy the subtree first (original event order:
     *  CHANGE_CHILD_DELETE -> children -> CHANGE_CHILD_DELETED). */
    if (has_child) {
        gt_event_send(self, GT_EVENT_TYPE_CHANGE_CHILD_DELETE, NULL);
    }
    _destroy_children_array(self);
    if (has_child) {
        gt_event_send(self, GT_EVENT_TYPE_CHANGE_CHILD_DELETED, NULL);
    }

    /** Phase 1b: unlink self from the screen registry / parent child array. */
    if (GT_TYPE_SCREEN == type) {
        if (NULL == _gt_obj_class_destroy_screen(self)) {
            GT_LOGW(GT_LOG_TAG_CHECK, "Destroy screen(%p) failed", self);
        }
        _gt_scr_stack_remove_pop_scr(self);
#if _GT_OBJ_CLASS_SCREEN_LOG
        GT_LOGD(GT_LOG_TAG_CHECK, "Destroy scr: %p(%d)", self, type);
#endif
    } else {
        if (NULL == _gt_obj_class_destroy_from_parent(self)
            && self->parent && self->parent->child && self->parent->cnt_child) {
            GT_LOGW(GT_LOG_TAG_CHECK, "Destroy obj: %p(%d) from parent: %p(%d) failed", self,
                type, self->parent, gt_obj_class_get_type(self->parent));
        }
#if _GT_OBJ_CLASS_WIDGETS_LOG
        GT_LOGD(GT_LOG_TAG_CHECK, "Destroy obj: %p(%d)", self, type);
#endif
    }

    /** Phase 1c: teardown property (CHANGE_DELETED, anim/indev/event unlink,
     *  _deinit_cb, scroll_cold). */
    _gt_obj_class_destroy_property(self);

    /** Phase 2: mark ZOMBIE and defer reclamation to the next safe point. */
    self->life_state = GT_OBJ_LIFE_ZOMBIE;
    gt_recycle_defer(self);
    return GT_RES_OK;
}

gt_res_t _gt_obj_class_destroy_children(struct gt_obj_s * self)
{
    if (gt_obj_is_type(self, GT_TYPE_UNKNOWN)) {
        GT_LOGW(GT_LOG_TAG_CHECK, "Want to destroy unknown obj's childs: %p(%d)", self, gt_obj_class_get_type(self));
        return GT_RES_FAIL;
    }
    /** Unlink every child up front so the container is empty the moment this
     *  call returns (a handler that rebuilds the list right after clearing it
     *  starts from index 0). Memory reclamation is deferred via the GC queue,
     *  so a child still referenced by the in-flight event dispatch chain is
     *  safe: its memory is not touched until the dispatch drains and the queue
     *  is flushed at the next safe point. */
    _destroy_children_array(self);
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
