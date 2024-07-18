/**
 * @file gt_layout.h
 * @author Feyoung
 * @brief Logical handling of layers
 * @version 0.1
 * @date 2024-03-29 19:53:33
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_LAYOUT_H_
#define _GT_LAYOUT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../widgets/gt_obj.h"



/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief Arrange controls in a row arrangement,
 *  the width of the control is the width of the control itself,
 *  when object prop setup row_layout = true, Reverse order needs
 *  to be set grow_invert = true.
 *
 * @param obj The target object
 */
void gt_layout_row_grow(gt_obj_st * obj);

void gt_layout_init(gt_obj_st * obj, gt_obj_container_st const * const container);

gt_res_t gt_layout_update_core(gt_obj_st * obj);

enum gt_layout_type_e gt_layout_get_type(gt_obj_st * obj);

bool gt_layout_is_type(gt_obj_st * obj, enum gt_layout_type_e type);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_LAYOUT_H_
