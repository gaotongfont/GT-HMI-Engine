/**
 * @file gt_recycle.c
 * @author Genitop
 * @brief Deferred reclamation queue (two-phase destroy support)
 * @version 0.1
 * @date 2026-09-23
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_recycle.h"
#include "gt_mem.h"
#include "../others/gt_log.h"
#include "../others/gt_assert.h"

/* static variables -----------------------------------------------------*/
static void ** _recycle_queue = NULL;
static uint32_t _recycle_cnt = 0;
static uint32_t _recycle_cap = 0;
static uint32_t _recycle_frame_total = 0;

/* global functions / API interface -------------------------------------*/
void gt_recycle_defer(void * ptr)
{
    if (NULL == ptr) {
        return;
    }

    /** Mark QUARANTINED first (no-op when GT_MEM_USE_CANARY is 0) so a second
     *  defer or a direct free of the same block is caught as double-free. */
    gt_mem_defer(ptr);

    if (_recycle_cnt >= _recycle_cap) {
        /** Grow on demand (first allocation via malloc, then double) so one
         *  frame that destroys a huge subtree can not overflow a fixed queue
         *  and abort the engine. Immediate reclaim is never safe here: the
         *  deferred block may still be referenced by the in-flight dispatch
         *  chain (UAF). Abort only on genuine out-of-memory. */
        uint32_t new_cap = _recycle_cap ? _recycle_cap * 2 : GT_RECYCLE_QUEUE_MAX;
        void ** tmp = _recycle_queue
            ? (void **)gt_mem_realloc(_recycle_queue, sizeof(void *) * new_cap)
            : (void **)gt_mem_malloc(sizeof(void *) * new_cap);
        if (NULL == tmp) {
            GT_LOGE(GT_LOG_TAG_MEM,
                    "recycle queue grow failed (%u -> %u): frame_total=%u",
                    _recycle_cap, new_cap, _recycle_frame_total);
            fflush(stdout);
            _ASSERT_ABORT();
            /** unreachable under a working assert; kept as last-resort so a
             *  misconfigured build degrades to reclaim instead of leaking. */
            gt_mem_reclaim(ptr);
            return;
        }
        _recycle_queue = tmp;
        _recycle_cap = new_cap;
    }

    _recycle_queue[_recycle_cnt++] = ptr;
    ++_recycle_frame_total;
}

void gt_recycle_flush(void)
{
    while (_recycle_cnt > 0) {
        void * p = _recycle_queue[--_recycle_cnt];
        if (p) {
            gt_mem_reclaim(p);
        }
    }
    _recycle_frame_total = 0;

    /** Release the transient peak capacity back to the initial size so a one-off
     *  giant destroy does not pin the enlarged buffer forever. A failed shrink
     *  is harmless: keep the larger buffer for the next frame. */
    if (_recycle_cap > GT_RECYCLE_QUEUE_MAX) {
        void ** tmp = (void **)gt_mem_realloc(_recycle_queue, sizeof(void *) * GT_RECYCLE_QUEUE_MAX);
        if (NULL != tmp) {
            _recycle_queue = tmp;
            _recycle_cap = GT_RECYCLE_QUEUE_MAX;
        }
    }
}

bool gt_recycle_is_empty(void)
{
    return 0 == _recycle_cnt;
}

/* end ------------------------------------------------------------------*/
