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
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_IMGBTN
#include "gt_obj.h"
#include "gt_obj_class.h"
#include "../utils/gt_vector.h"

#if GT_USE_FILE_HEADER
#include "../hal/gt_hal_file_header.h"
#endif


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

#if GT_USE_FILE_HEADER
/**
 * @brief set the path of the imgbtn
 *
 * @param imgbtn imgbtn obj
 * @param fh idx -1[defalut]: Disabled file header, using img path to open file; >=0: index number, valid value
 *              < 0: invalid value.
 *           package_idx 0[defalut]: The index of element within item, as the first element
 */
void gt_imgbtn_set_src_by_file_header(gt_obj_st * imgbtn, gt_file_header_param_st * fh);

/**
 * @brief set the path of imgbtn of press
 *
 * @param imgbtn
 * @param fh idx -1[defalut]: Disabled file header, using img path to open file; >=0: index number, valid value
 *              < 0: invalid value.
 *           package_idx 0[defalut]: The index of element within item, as the first element
 */
void gt_imgbtn_set_src_press_by_file_header(gt_obj_st * imgbtn, gt_file_header_param_st * fh);

/**
 * @brief set the path of imgbtn of release
 *
 * @param imgbtn
 * @param fh idx -1[defalut]: Disabled file header, using img path to open file; >=0: index number, valid value
 *              < 0: invalid value.
 *           package_idx 0[defalut]: The index of element within item, as the first element
 */
void gt_imgbtn_set_src_release_by_file_header(gt_obj_st * imgbtn, gt_file_header_param_st * fh);

/**
 * @brief Add a state item to the imgbtn
 *
 * @param obj
 * @param fh idx -1[defalut]: Disabled file header, using img path to open file; >=0: index number, valid value
 *              < 0: invalid value.
 *           package_idx 0[defalut]: The index of element within item, as the first element
 * @return true success
 * @return false failed
 */
bool gt_imgbtn_add_state_item_by_file_header(gt_obj_st * obj, gt_file_header_param_st * fh);

/**
 * @brief Remove a state item from the imgbtn
 *
 * @param obj
 * @param fh idx -1[defalut]: Disabled file header, using img path to open file; >=0: index number, valid value
 *              < 0: invalid value.
 *           package_idx 0[defalut]: The index of element within item, as the first element
 * @return true success
 * @return false failed
 */
bool gt_imgbtn_remove_state_item_by_file_header(gt_obj_st * obj, gt_file_header_param_st * fh);
#endif

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


#endif  /** GT_CFG_ENABLE_IMGBTN */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_IMG_H_
