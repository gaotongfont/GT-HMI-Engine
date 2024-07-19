/**
 * @file gt_obj_class.h
 * @author yongg
 * @brief object class information
 * @version 0.1
 * @date 2022-05-12 10:34:16
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_OBJ_CLASS_H_
#define _GT_OBJ_CLASS_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../others/gt_types.h"
#include "../core/gt_event.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
/** Control drawing logic */
typedef void (* _gt_init_cb)(struct gt_obj_s *);

/** Can only be used to free additional memory requested by the control when it is created */
typedef void (* _gt_deinit_cb)(struct gt_obj_s *);

/** Controls all behavior events */
typedef void (* _gt_event_cb)(struct gt_obj_s *, struct _gt_event_s *);

/**
 * @brief interface for other widgets
 */
typedef struct _gt_obj_class_s {
    _gt_init_cb     _init_cb;
    _gt_deinit_cb   _deinit_cb;
    _gt_event_cb    _event_cb;
    /** Control type @ref gt_obj_type_et */
    gt_obj_type_et type;
    /** Control styles contain the memory size of the base object,
     * which must exist at the very front of the struct */
    uint16_t size_style;
}gt_obj_class_st;



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief
 *
 * @param parent
 * @return struct gt_obj_s*
 */
struct gt_obj_s * gt_obj_class_create(const gt_obj_class_st * c, struct gt_obj_s * parent);

/**
 * @brief Get widget type
 *
 * @param obj
 * @return gt_obj_type_et @see gt_obj_type_et
 */
gt_obj_type_et gt_obj_class_get_type(struct gt_obj_s * obj);

/**
 * @brief Check if the object is target type, include NULL check
 *
 * @param obj
 * @param type GT_TYPE_TOTAL: select all types, except GT_TYPE_UNKNOWN
 * @return true
 * @return false
 */
bool gt_obj_is_type(struct gt_obj_s * obj, gt_obj_type_et type);

/**
 * @brief Change the parent of the control widget.
 *
 * @param obj
 * @param to
 * @return struct gt_obj_s* parent pointer
 */
struct gt_obj_s * _gt_obj_class_change_parent(struct gt_obj_s * obj, struct gt_obj_s * to);

/**
 * @brief delete control widget; Reclaim the memory of child controls,
 *      immediately and recursively.
 *
 * @param self
 */
void _gt_obj_class_destroy(struct gt_obj_s * self);

/**
 * @brief delete all child controls, immediately and recursively.
 *
 * @param self
 */
void _gt_obj_class_destroy_children(struct gt_obj_s * self);

/**
 * @brief Multiple properties that can be inherited from the superclass
 *
 * @param obj The object to be inherited
 * @param parent The parent object
 */
void _gt_obj_class_inherent_attr_from_parent(struct gt_obj_s * obj, struct gt_obj_s * parent);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_OBJ_CLASS_H_
