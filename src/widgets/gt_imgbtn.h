/**
 * @file gt_imgbtn.h
 * @author yongg
 * @brief image button
 * @version 0.1
 * @date 2022-09-27 11:03:46
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_IMGBTN_H_
#define _GT_IMGBTN_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_obj.h"
#include "gt_obj_class.h"
#include "../utils/gt_vector.h"


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief  create imgbtn obj
 *
 * @param parent imgbtn's parent obj
 * @return gt_obj_st* imgbtn obj ptr
 */
gt_obj_st * gt_imgbtn_create(gt_obj_st * parent);

/**
 * @brief set the path of the imgbtn
 *
 * @param imgbtn imgbtn obj
 * @param src path of imgbtn
 */
void gt_imgbtn_set_src(gt_obj_st * imgbtn, char * src);

/**
 * @brief set the path of imgbtn of press
 *
 * @param imgbtn
 * @param src
 */
void gt_imgbtn_set_src_press(gt_obj_st * imgbtn, char * src);

/**
 * @brief set the path of imgbtn of release
 *
 * @param imgbtn
 * @param src
 */
void gt_imgbtn_set_src_release(gt_obj_st * imgbtn, char * src);

/**
 * @brief Add a state item to the imgbtn
 *
 * @param obj
 * @param src path of image
 * @return true success
 * @return false failed
 */
bool gt_imgbtn_add_state_item(gt_obj_st * obj, char * src);

/**
 * @brief Remove a state item from the imgbtn
 *
 * @param obj
 * @param src The target image item
 * @return true success
 * @return false failed
 */
bool gt_imgbtn_remove_state_item(gt_obj_st * obj, char * src);

/**
 * @brief Clear all state items of the imgbtn, but do not free vector
 *
 * @param obj
 * @return true success
 * @return false failed
 */
bool gt_imgbtn_clear_all_state_item(gt_obj_st * obj);

/**
 * @brief Get the item index of list
 *
 * @param obj
 * @return int16_t -1: failed; >=0: success
 */
int16_t bt_imgbtn_get_state_item_index(gt_obj_st * obj);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_IMG_H_
