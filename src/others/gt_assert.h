/**
 * @file gt_assert.h
 * @author yongg
 * @brief code assertion
 * @version 0.1
 * @date 2022-07-01 10:06:11
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_ASSERT_H_
#define _GT_ASSERT_H_

#include "stdlib.h"
#include "./gt_log.h"

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/



/* define ---------------------------------------------------------------*/

#ifndef GT_USE_ASSERT_ENABLE
    #define GT_USE_ASSERT_ENABLE    01
#endif

#ifndef GT_USE_ASSERT_MALLOC
    #define GT_USE_ASSERT_MALLOC    01
#endif

#ifdef _GT_PORT_SIMULATOR_ENVS
    #define _ASSERT_ABORT()     abort()
#else
    #define _ASSERT_ABORT()     while(1){}
#endif

#define GT_ASSERT_MSG(expr, msg) do { \
        if (!(expr)) { \
            GT_LOG_A(GT_LOG_TAG_ASSERT, "Failed: %s is %s at %s:%d", #expr, msg,  __FILE__, __LINE__); \
        } \
    } while (0)


/* check null assert */
#if GT_USE_ASSERT_ENABLE
    #define GT_ASSERT(p)  do { \
        if (!p) { \
            GT_LOG_A(GT_LOG_TAG_ASSERT, "Failed: %s is NULL at %s:%d", #p, __FILE__, __LINE__); \
            _ASSERT_ABORT(); \
        } \
    } while(0)

    #define GT_ASSERT_EXPR(expr) do { \
        if (!(expr)) { \
            GT_LOG_A(GT_LOG_TAG_ASSERT, "Failed: %s at %s:%d", #expr, __FILE__, __LINE__); \
            _ASSERT_ABORT(); \
        } \
    } while(0)
#else
    #define GT_ASSERT(p)
    #define GT_ASSERT_EXPR(expr)
#endif

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
