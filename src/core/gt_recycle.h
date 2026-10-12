/**
 * @file gt_recycle.h
 * @author Genitop
 * @brief Deferred reclamation queue (two-phase destroy support)
 *
 * The object destroy path is split into two phases: a synchronous "unlink"
 * phase (remove the object from every reference chain and mark it ZOMBIE)
 * and a deferred "reclaim" phase that returns the memory to the pool at a
 * safe point (end of frame, after the timer handler returns).
 *
 * This queue is backend-agnostic: it only holds payload pointers and releases
 * them through gt_mem_reclaim() (which routes to the active allocator). It is
 * intentionally decoupled from the TLSF/external-allocator choice.
 *
 * Note: the name must NOT collide with src/others/gt_gc.{h,c} (the "global
 * call" / screen-GC module), hence "recycle" rather than "gc".
 *
 * Threading: by default the queue is single-threaded (owner = UI/timer thread).
 * All defer/flush calls MUST run on that same thread. To share across threads
 * later, wrap defer/flush with GT_RECYCLE_USE_MUTEX hooks or replace the array
 * with a lock-free SPSC queue.
 *
 * @version 0.1
 * @date 2026-09-23
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_RECYCLE_H_
#define _GT_RECYCLE_H_

#include "stdint.h"
#include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif

/* define ---------------------------------------------------------------*/
#ifndef GT_RECYCLE_QUEUE_MAX
    /**
     * @brief Initial queue capacity, in deferred blocks, before a flush. When a
     *        single frame defers more than this, the queue grows on demand
     *        (doubles via malloc/realloc) instead of aborting; only genuine
     *        out-of-memory aborts via _ASSERT_ABORT(). The buffer shrinks back
     *        to this size after each flush, so the enlarged buffer is transient
     *        (one frame) rather than permanently pinned. Every object and every
     *        child-array counts as one entry (a non-leaf node costs 2). The
     *        queue is flushed at end of frame (after the timer handler returns),
     *        so it only needs to hold one frame's worth of destroys.
     *        [default: 512]
     */
    #define GT_RECYCLE_QUEUE_MAX     512
#endif

/* global functions / API interface -------------------------------------*/
/**
 * @brief Defer the release of a block until the next safe point.
 *
 * @param ptr payload pointer previously returned by gt_mem_malloc/realloc/calloc
 */
void gt_recycle_defer(void * ptr);

/**
 * @brief Reclaim all deferred blocks now. Call at a safe point only:
 *        end of frame, after the timer handler returns (every timer/event of
 *        the frame has been drained, so no in-flight dispatch still references
 *        the deferred blocks).
 */
void gt_recycle_flush(void);

/**
 * @brief Check whether any block is still pending reclamation.
 *
 * @return true queue is empty
 * @return false at least one block is pending
 */
bool gt_recycle_is_empty(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_RECYCLE_H_
