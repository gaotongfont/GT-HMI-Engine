/**
 * @file gt_test_rand_widget.h
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2024-06-29 13:33:54
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_TEST_RAND_WIDGET_H_
#define _GT_TEST_RAND_WIDGET_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../src/gt_conf.h"
#if _GT_USE_TEST

#ifndef _GT_TEST_RAND_WIDGET_EVENT
    /**
     * @brief Control random click event test
     */
    #define _GT_TEST_RAND_WIDGET_EVENT      0
#endif

#if _GT_TEST_RAND_WIDGET_EVENT

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
void _gt_test_rand_widget(void);


#endif  /** _GT_TEST_RAND_WIDGET_EVENT */
#endif  /** _GT_USE_TEST */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_TEST_RAND_WIDGET_H_
