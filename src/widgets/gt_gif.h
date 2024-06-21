/**
 * @file gt_gif.h
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2023-10-24 16:32:21
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_GIF_H_
#define _GT_GIF_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_GIF
#include "gt_obj.h"
#include "gt_obj_class.h"

#if GT_USE_FILE_HEADER
#include "../hal/gt_hal_file_header.h"
#endif


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

gt_obj_st * gt_gif_create(gt_obj_st * parent);

void gt_gif_set_src(gt_obj_st * obj, char * src);

char * gt_gif_get_src(gt_obj_st *  obj);

#if GT_USE_FILE_HEADER
/**
 * @brief
 *
 * @param obj
 * @param fh idx -1[defalut]: Disabled file header, using img path to open file; >=0: index number, valid value
 *              < 0: invalid value.
 *           package_idx 0[defalut]: The index of element within item, as the first element
 */
void gt_gif_set_src_by_file_header(gt_obj_st * obj, gt_file_header_param_st * fh);
#endif

gt_size_t gt_gif_get_frame_width(gt_obj_st * obj);

gt_size_t gt_gif_get_frame_height(gt_obj_st * obj);

void gt_gif_play(gt_obj_st * obj);

void gt_gif_stop(gt_obj_st * obj);

void gt_gif_toggle(gt_obj_st * obj);

bool gt_gif_is_play(gt_obj_st * obj);

void gt_gif_reset(gt_obj_st * obj);



#endif  /** GT_CFG_ENABLE_GIF */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_GIF_H_
