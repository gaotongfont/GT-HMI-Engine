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
struct gt_obj_s;
struct _gt_event_s;

/* typedef --------------------------------------------------------------*/
/**
 * @brief interface for other widgets
 *
 */
typedef struct _gt_obj_class_s
{
    void (*_init_cb)(struct gt_obj_s *);
    void (*_deinit_cb)(struct gt_obj_s *);
    void (*_event_cb)(struct gt_obj_s *, struct _gt_event_s *);
    gt_obj_type_et type;
    uint16_t size_style;        //used: malloc free
}gt_obj_class_st;



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief
 *
 * @param parent
 * @return struct gt_obj_s*
 */
struct gt_obj_s * gt_obj_class_create(const gt_obj_class_st * class, struct gt_obj_s * parent);

/**
 * @brief Change the parent of the control widget.
 *
 * @param obj
 * @param to
 * @return struct gt_obj_s* parent pointer
 */
struct gt_obj_s * _gt_obj_class_change_parent(struct gt_obj_s * obj, struct gt_obj_s * to);

/**
 * @brief delete control widget; Reclaim the memory of child controls.
 *
 * @param self
 */
void _gt_obj_class_destroy(struct gt_obj_s * self);

/**
 * @brief
 *
 * @param obj
 * @param parent
 */
void _gt_obj_class_inherent_attr_from_parent(struct gt_obj_s * obj, struct gt_obj_s * parent);

gt_obj_type_et gt_obj_class_get_type(struct gt_obj_s * obj);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_OBJ_CLASS_H_
