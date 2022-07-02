/**
 * @file gt_assert.h
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-07-01 10:06:11
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_ASSERT_H_
#define _GT_ASSERT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/



/* define ---------------------------------------------------------------*/

/* check null assert */
#define GT_ASSERT(p)    { if(!p) while(1){} }


/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_ASSERT_H_
