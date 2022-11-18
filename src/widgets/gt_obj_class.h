/**
 * @file gt_obj_class.h
 * @author yongg
 * @brief object class information
 * @version 0.1
 * @date 2022-05-12 10:34:16
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
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
struct _gt_obj_s;
struct _gt_event_s;

/* typedef --------------------------------------------------------------*/
/**
 * @brief interface for other widgets
 *
 */
typedef struct _gt_obj_class_s
{
    void (*_init_cb)(struct _gt_obj_s *, const struct _gt_obj_class_s *);
    void (*_deinit_cb)(struct _gt_obj_s *);
    void (*_event_cb)(struct _gt_obj_s *, struct _gt_event_s *);
    gt_obj_type_et type;
    uint16_t size_instance;     //used: malloc free
    uint16_t size_style;        //used: malloc free
}gt_obj_class_st;



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
struct _gt_obj_s * gt_obj_class_create(const gt_obj_class_st * class, struct _gt_obj_s * parent);
struct _gt_obj_s * gt_obj_class_destroy(struct _gt_obj_s * self);

gt_obj_type_et gt_obj_class_get_type(struct _gt_obj_s * obj);
void gt_obj_class_set_type(struct _gt_obj_s * obj, gt_obj_type_et type);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_OBJ_CLASS_H_
