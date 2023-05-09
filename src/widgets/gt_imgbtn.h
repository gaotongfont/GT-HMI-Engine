/**
 * @file gt_imgbtn.h
 * @author yongg
 * @brief image button
 * @version 0.1
 * @date 2022-09-27 11:03:46
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_IMGBTN_H_
#define _GT_IMGBTN_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_obj.h"
#include "gt_obj_class.h"


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef struct _gt_style_imgbtn_s
{
    uint32_t selected;
    char * src;

    char * src_press;           ///< imgbtn src when press, It can be a virt file name/path or an array of C
    char * src_release;         ///< imgbtn src when release
    char * src_base;            ///< imgbtn src when nothing
}_gt_style_imgbtn_st;



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


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_IMG_H_
