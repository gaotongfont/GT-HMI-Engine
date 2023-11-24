/**
 * @file gt_assert.h
 * @author yongg
 * @brief code assertion
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

#define GT_USE_ASSERT_ENABLE    1
#define GT_USE_ASSERT_MALLOC    1

#define GT_ASSERT_MSG(expr, msg)                                \
    do {                                                        \
        if (!(expr)) {                                          \
            GT_LOGE(GT_LOG_TAG_ASSERT, "%s, %s.", #expr, msg);  \
        }                                                       \
    } while (0)


/* check null assert */
#if GT_USE_ASSERT_ENABLE
    #define GT_ASSERT_NULL(p)       { if(!p) while(1){} }
#else
    #define GT_ASSERT_NULL(p)
#endif
    #define GT_ASSERT(p)        {}

#if GT_USE_ASSERT_MALLOC
    #define GT_ASSERT_MALLOC(expr)  GT_ASSERT_MSG(NULL != expr, "out of memory")
#else
    #define GT_ASSERT_MALLOC(expr)
#endif

/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_ASSERT_H_
