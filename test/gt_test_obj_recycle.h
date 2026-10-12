/**
 * @file gt_test_obj_recycle.h
 * @author Genitop
 * @brief Object destroy / deferred-recycle stress test interface
 * @version 0.1
 * @date 2026-09-23
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_TEST_OBJ_RECYCLE_H_
#define _GT_TEST_OBJ_RECYCLE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../src/gt_conf.h"
#if _GT_USE_TEST

#ifndef _GT_TEST_OBJ_DESTROY_STRESS
    /**
     * @brief Enable the object destroy / deferred-recycle stress test.
     *        Runs a bounded sequence of re-entrant destroy, idempotent
     *        double-destroy, screen create/destroy, children clear+rebuild
     *        and rapid per-frame churn. Combine with GT_MEM_USE_CANARY=1 so
     *        any UAF / double-free aborts the run.
     */
    #define _GT_TEST_OBJ_DESTROY_STRESS      0
#endif

#if _GT_TEST_OBJ_DESTROY_STRESS

/* global functions / API interface -------------------------------------*/
/**
 * @brief Advance the stress test by one frame. Drive it from gt_task_handler()
 *        at frame boundary, before gt_recycle_flush() drains the queue.
 */
void _gt_test_obj_destroy_stress(void);

#endif  /** _GT_TEST_OBJ_DESTROY_STRESS */
#endif  /** _GT_USE_TEST */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_TEST_OBJ_RECYCLE_H_
