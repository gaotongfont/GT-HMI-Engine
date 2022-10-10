/**
 * @file gt_processbar.h
 * @author yongg
 * @brief The progress bar implementation
 * @version 0.1
 * @date 2022-07-22 14:13:28
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_PROCESSBAR_H_
#define _GT_PROCESSBAR_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_obj.h"
#include "gt_obj_class.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef struct _gt_style_processbar_s
{
    gt_color_t color_act;
    gt_color_t color_ina;
    gt_color_t bg_color;
    gt_size_t start;
    gt_size_t end;
    gt_size_t pos;
}_gt_style_processbar_st;



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief create processbar obj
 *
 * @param parent processbar's parent obj
 * @return gt_obj_st* processbar obj ptr
 */
gt_obj_st * gt_processbar_create(gt_obj_st * parent);

/**
 * @brief set processbar pos
 *
 * @param processbar processbar obj
 * @param pos pos
 */
void gt_processbar_style_set_pos(gt_obj_st * processbar, gt_size_t pos);

/**
 * @brief set processbar start and end pos
 *
 * @param processbar processbar obj
 * @param start satrt pos
 * @param end end pos
 */
void gt_processbar_style_set_start_end(gt_obj_st * processbar, gt_size_t start, gt_size_t end);

/**
 * @brief get processbar pos
 *
 * @param processbar processbar obj
 * @return gt_size_t processbar pos
 */
gt_size_t gt_processbar_style_get_pos(gt_obj_st * processbar);

/**
 * @brief get processbar start pos
 *
 * @param processbar processbar obj
 * @return gt_size_t processbar start pos
 */
gt_size_t gt_processbar_style_get_start(gt_obj_st * processbar);

/**
 * @brief get processbar end pos
 *
 * @param processbar processbar obj
 * @return gt_size_t processbar end pos
 */
gt_size_t gt_processbar_style_get_end(gt_obj_st * processbar);

/**
 * @brief get processbar total pos
 *
 * @param processbar processbar obj
 * @return gt_size_t processbar total pos
 */
gt_size_t gt_processbar_style_get_total(gt_obj_st * processbar);



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_PROCESSBAR_H_
