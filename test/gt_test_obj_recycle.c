/**
 * @file gt_test_obj_recycle.c
 * @author Genitop
 * @brief Object destroy / deferred-recycle stress test.
 *
 * Exercises the two-phase destroy + deferred reclamation framework with:
 *   - re-entrant destroy from CHANGE_DELETED / CHANGE_CHILD_DELETED handlers,
 *   - idempotent double destroy (ZOMBIE gate),
 *   - screen create + destroy,
 *   - children clear + rebuild (_gt_obj_class_destroy_children),
 *   - rapid per-frame create/destroy churn.
 *
 * A crash here (canary abort / ASan) means the destroy or recycle path has a
 * use-after-free / double-free. Enable with _GT_TEST_OBJ_DESTROY_STRESS=1 and
 * GT_MEM_USE_CANARY=1.
 *
 * @version 0.1
 * @date 2026-09-23
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "./gt_test_obj_recycle.h"
#if _GT_USE_TEST
#if _GT_TEST_OBJ_DESTROY_STRESS

#include "stdint.h"
#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "../src/others/gt_log.h"
#include "../src/core/gt_disp.h"
#include "../src/core/gt_mem.h"
#include "../src/core/gt_recycle.h"
#include "../src/core/gt_scr_stack.h"
#include "../src/widgets/gt_obj.h"
#include "../src/widgets/gt_obj_class.h"

/* private define -------------------------------------------------------*/
#ifndef _GT_TEST_PRINT
    #define _GT_TEST_PRINT      1
#endif

/** frames spent in the churn phase */
#define _STRESS_CHURN_FRAMES   120
/** churn trees created/destroyed per frame */
#define _STRESS_CHURN_TREES    4
/** children per churn tree */
#define _STRESS_CHURN_CHILD    6

/** screen ids used by the stack jump stress */
#define _STRESS_SCR_A          0x7001
#define _STRESS_SCR_B          0x7002
#define _STRESS_SCR_C          0x7003
/** push/back rounds in the stack stress (each round = 2 push + 2 back) */
#define _STRESS_STACK_ROUNDS   40
/** idle frames after stack stress to let deferred reclaim settle */
#define _STRESS_STACK_SETTLE   12
/** frames of current-page widget churn */
#define _STRESS_PAGE_CHURN_FRAMES     200
/** widget create+destroy pairs per churn frame */
#define _STRESS_PAGE_CHURN_PER_FRAME  8

/** bulk-destroy phase: children of one tree destroyed in a single frame. Each
 *  child carries one leaf, so the tree defers ~1802 slots (objects + child
 *  arrays), far above GT_RECYCLE_QUEUE_MAX(512): exercises the recycle queue's
 *  on-demand growth instead of an assert. */
#define _STRESS_BULK_CHILD            600

/** fuzz/soak 时长（分钟）：make GT_STRESS_MINUTES=N 覆盖，默认 2 分钟 */
#ifndef GT_STRESS_MINUTES
    #define GT_STRESS_MINUTES            2
#endif
/** fuzz 随机种子：默认固定值保证崩溃可复现，make GT_STRESS_RAND_SEED=... 覆盖 */
#ifndef GT_STRESS_RAND_SEED
    #define GT_STRESS_RAND_SEED          0xC0FFEE42u
#endif

/** fuzz 活对象池容量（随机创建的子树的根，供随机销毁） */
#define _STRESS_FUZZ_POOL_MAX           64
/** fuzz 趋势打印间隔（秒） */
#define _STRESS_FUZZ_REPORT_SEC         5
/** fuzz 每帧随机动作数（上限需保证单帧 defer 量远小于 GT_RECYCLE_QUEUE_MAX=512） */
#define _STRESS_FUZZ_OPS_PER_FRAME      4
/** 每次纯内存 fuzz 的 malloc/realloc/free 次数 */
#define _STRESS_MEM_FUZZ_OPS            8
/** 纯内存 fuzz 最大块大小（字节），覆盖 8B ~ 8KB */
#define _STRESS_MEM_FUZZ_MAX_SIZE       8192

#if _GT_TEST_PRINT
    /** printf is block-buffered when stdout is redirected, so flush each
     *  milestone to make the stress result visible to CI / headless runs. */
    #define _STRESS_LOG(_fmt, _arg...) do { \
            GT_LOG_A(GT_LOG_TAG_TEST, _fmt, ##_arg); \
            fflush(stdout); \
        }while(0)
#else
    #define _STRESS_LOG(_fmt, _arg...)    ((void)0)
#endif

/* private typedef ------------------------------------------------------*/
typedef enum {
    _STRESS_PH_REENTER = 0,
    _STRESS_PH_SELF_DESTROY,
    _STRESS_PH_SCREEN,
    _STRESS_PH_CHILDREN,
    _STRESS_PH_CHURN,
    _STRESS_PH_STACK,
    _STRESS_PH_PAGE_CHURN,
    _STRESS_PH_BULK,
    _STRESS_PH_FUZZ,
    _STRESS_PH_DONE,
}_stress_phase_et;

/* static variables -----------------------------------------------------*/
static _stress_phase_et _s_phase = _STRESS_PH_REENTER;
static uint16_t _s_frame = 0;
static uint32_t _s_reenter_hits = 0;
static uint32_t _s_self_destroy_hits = 0;
static bool _s_owned_anchor = false;
static gt_obj_st * _s_anchor = NULL;
static uint32_t _s_used_base = 0;

/** fuzz/soak 阶段状态 */
static gt_obj_st * _s_fuzz_pool[_STRESS_FUZZ_POOL_MAX];
static uint32_t _s_fuzz_pool_cnt = 0;
static time_t _s_fuzz_start = 0;
static time_t _s_fuzz_last_report = 0;
static uint32_t _s_fuzz_create_ops = 0;
static uint32_t _s_fuzz_destroy_ops = 0;
static uint32_t _s_fuzz_mem_ops = 0;

/** re-entrancy probes, referenced from the CHANGE_* handlers */
static gt_obj_st * _s_ree_a = NULL;
static gt_obj_st * _s_ree_b = NULL;
static gt_obj_st * _s_ree_c = NULL;

/* static functions -----------------------------------------------------*/
static gt_obj_st * _stress_scr_create(uint16_t branch_cnt, uint16_t leaf_cnt)
{
    gt_obj_st * scr = gt_obj_create(NULL);
    if (NULL == scr) {
        return NULL;
    }
    for (uint16_t i = 0; i < branch_cnt; ++i) {
        gt_obj_st * branch = gt_obj_create(scr);
        if (NULL == branch) {
            continue;
        }
        for (uint16_t j = 0; j < leaf_cnt; ++j) {
            gt_obj_create(branch);
        }
    }
    return scr;
}

static gt_obj_st * _stress_scr_a(void) { return _stress_scr_create(3, 3); }
static gt_obj_st * _stress_scr_b(void) { return _stress_scr_create(4, 4); }
static gt_obj_st * _stress_scr_c(void) { return _stress_scr_create(5, 5); }

static const gt_scr_list_st _s_stress_scr_list[] = {
    { _STRESS_SCR_A, _stress_scr_a },
    { _STRESS_SCR_B, _stress_scr_b },
    { _STRESS_SCR_C, _stress_scr_c },
};

static void _stress_ensure_anchor(void)
{
    if (NULL != _s_anchor) {
        return;
    }
    _s_anchor = gt_disp_get_scr();
    if (NULL == _s_anchor) {
        _s_anchor = gt_obj_create(NULL);
        _s_owned_anchor = true;
    }
}

static void _reenter_on_deleted(gt_event_st * e)
{
    (void)e;
    ++_s_reenter_hits;
    /** self + ancestor are DESTROYING right now; every call must no-op. */
    gt_obj_destroy_immediately(_s_ree_b);
    gt_obj_destroy_immediately(_s_ree_a);
    gt_obj_destroy(_s_ree_a);
}

static void _reenter_on_child_deleted(gt_event_st * e)
{
    (void)e;
    ++_s_reenter_hits;
    /** container is DESTROYING; no-op. */
    gt_obj_destroy(_s_ree_c);
}

static void _self_destroy_cb(gt_event_st * e)
{
    ++_s_self_destroy_hits;
    /** Destroy self from within its own callback dispatch. The dispatch loop
     *  must survive via cached-next + re-validate, and the event_attr chain
     *  free must be deferred (event_cb_depth > 0). */
    gt_obj_destroy_immediately(e->target);
}

static void _stress_reenter(void)
{
    _stress_ensure_anchor();

    _s_ree_c = gt_obj_create(_s_anchor);
    _s_ree_a = gt_obj_create(_s_ree_c);
    _s_ree_b = gt_obj_create(_s_ree_a);
    if (NULL == _s_ree_c || NULL == _s_ree_a || NULL == _s_ree_b) {
        _s_ree_c = _s_ree_a = _s_ree_b = NULL;
        return;
    }

    gt_obj_add_event_cb(_s_ree_b, _reenter_on_deleted, GT_EVENT_TYPE_CHANGE_DELETED, NULL);
    gt_obj_add_event_cb(_s_ree_c, _reenter_on_child_deleted, GT_EVENT_TYPE_CHANGE_CHILD_DELETED, NULL);

    gt_obj_destroy(_s_ree_c);

    /** idempotent double destroy: all already ZOMBIE (still valid until flush). */
    gt_obj_destroy(_s_ree_c);
    gt_obj_destroy_immediately(_s_ree_c);
    gt_obj_destroy(_s_ree_a);
    gt_obj_destroy(NULL);

    _STRESS_LOG("[recycle] reenter hits: %d (expect 2)", _s_reenter_hits);

    _s_ree_c = NULL;
    _s_ree_a = NULL;
    _s_ree_b = NULL;
}

static void _stress_self_destroy(void)
{
    _stress_ensure_anchor();

    gt_obj_st * obj = gt_obj_create(_s_anchor);
    if (NULL == obj) {
        return;
    }
    gt_obj_add_event_cb(obj, _self_destroy_cb, GT_EVENT_TYPE_INPUT_PRESSED, NULL);
    gt_event_send(obj, GT_EVENT_TYPE_INPUT_PRESSED, NULL);

    /** obj is now ZOMBIE; still valid until flush. Idempotent no-op. */
    gt_obj_destroy_immediately(obj);

    _STRESS_LOG("[recycle] self-destroy hits: %d (expect 1)", _s_self_destroy_hits);
}

static void _stress_screen(void)
{
    gt_obj_st * scr = gt_obj_create(NULL);
    if (NULL == scr) {
        return;
    }
    for (uint16_t i = 0; i < 3; ++i) {
        gt_obj_st * w = gt_obj_create(scr);
        gt_obj_create(w);
    }
    gt_obj_destroy_immediately(scr);
    /** ZOMBIE: idempotent no-op. */
    gt_obj_destroy_immediately(scr);
    _STRESS_LOG("[recycle] screen create/destroy ok");
}

static void _stress_children(void)
{
    _stress_ensure_anchor();

    gt_obj_st * c = gt_obj_create(_s_anchor);
    if (NULL == c) {
        return;
    }
    for (uint16_t i = 0; i < 4; ++i) {
        gt_obj_create(c);
    }
    _gt_obj_class_destroy_children(c);
    for (uint16_t i = 0; i < 4; ++i) {
        gt_obj_create(c);
    }
    gt_obj_destroy(c);
    _STRESS_LOG("[recycle] children clear+rebuild ok");
}

static void _stress_churn(void)
{
    _stress_ensure_anchor();

    for (uint16_t t = 0; t < _STRESS_CHURN_TREES; ++t) {
        gt_obj_st * c = gt_obj_create(_s_anchor);
        if (NULL == c) {
            return;
        }
        for (uint16_t i = 0; i < _STRESS_CHURN_CHILD; ++i) {
            gt_obj_st * w = gt_obj_create(c);
            gt_obj_create(w);
        }
        gt_obj_destroy(c);
    }
}

static void _stress_stack(void)
{
    if (0 == _s_frame) {
        gt_scr_stack_register_id_list(_s_stress_scr_list, 3, 8);
        gt_scr_stack_set_home_scr_id(_STRESS_SCR_A, false);
        gt_mem_info_st info;
        gt_mem_heap_get_space(&info);
        _s_used_base = (uint32_t)info.used;
        gt_disp_stack_load_scr(_STRESS_SCR_A);
        _STRESS_LOG("[recycle] stack start: used=%u", _s_used_base);
    }

    if (_s_frame < _STRESS_STACK_ROUNDS * 4) {
        switch (_s_frame & 3u) {
            case 0: gt_disp_stack_load_scr(_STRESS_SCR_B); break;
            case 1: gt_disp_stack_load_scr(_STRESS_SCR_C); break;
            case 2: gt_disp_stack_go_back(1); break;
            default: gt_disp_stack_go_back(1); break;
        }
        ++_s_frame;
        return;
    }

    ++_s_frame;
    if (_s_frame >= _STRESS_STACK_ROUNDS * 4 + _STRESS_STACK_SETTLE) {
        gt_mem_info_st info;
        gt_mem_heap_get_space(&info);
        uint32_t used_now = (uint32_t)info.used;
        _STRESS_LOG("[recycle] stack end: used=%u base=%u delta=%d",
            used_now, _s_used_base, (int32_t)(used_now - _s_used_base));
        _s_phase = _STRESS_PH_PAGE_CHURN;
        _s_frame = 0;
    }
}

static void _stress_page_churn(void)
{
    gt_obj_st * scr = gt_disp_get_scr();
    if (0 == _s_frame) {
        gt_mem_info_st info;
        gt_mem_heap_get_space(&info);
        _s_used_base = (uint32_t)info.used;
        _STRESS_LOG("[recycle] page-churn start: used=%u", _s_used_base);
    }
    if (NULL == scr) {
        _STRESS_LOG("[recycle] page-churn abort: no active screen");
        _s_phase = _STRESS_PH_DONE;
        return;
    }
    for (uint16_t i = 0; i < _STRESS_PAGE_CHURN_PER_FRAME; ++i) {
        gt_obj_st * w = gt_obj_create(scr);
        if (NULL == w) {
            continue;
        }
        gt_obj_st * leaf = gt_obj_create(w);
        (void)leaf;
        gt_obj_destroy(w);
    }
    ++_s_frame;
    if (_s_frame >= _STRESS_PAGE_CHURN_FRAMES) {
        gt_mem_info_st info;
        gt_mem_heap_get_space(&info);
        uint32_t used_now = (uint32_t)info.used;
        _STRESS_LOG("[recycle] page-churn end: used=%u base=%u delta=%d",
            used_now, _s_used_base, (int32_t)(used_now - _s_used_base));
        _s_phase = _STRESS_PH_BULK;
        _s_frame = 0;
    }
}

/**
 * @brief Destroy one large subtree in a single frame to exercise the recycle
 *        queue's on-demand growth: ~1802 deferred slots force 512 -> 1024 ->
 *        2048 (the flush then shrinks back to 512). Must not abort.
 */
static void _stress_bulk_destroy(void)
{
    gt_obj_st * root = gt_obj_create(NULL);
    if (NULL == root) {
        _STRESS_LOG("[recycle] bulk-destroy abort: no root");
        return;
    }
    for (uint16_t i = 0; i < _STRESS_BULK_CHILD; ++i) {
        gt_obj_st * w = gt_obj_create(root);
        if (NULL != w) {
            gt_obj_create(w);
        }
    }
    gt_obj_destroy(root);
    _STRESS_LOG("[recycle] bulk-destroy ok (%d children)", _STRESS_BULK_CHILD);
}

/* fuzz/soak helpers -----------------------------------------------------*/

/**
 * @brief 随机创建一棵深度/宽度随机的子树，挂到 parent 下
 * @param parent 父对象（必须有效，fuzz 阶段为固定 anchor）
 * @param depth 剩余递归深度（0 表示只创建单节点）
 * @return 子树根对象指针
 */
static gt_obj_st * _fuzz_rand_subtree(gt_obj_st * parent, uint32_t depth)
{
    gt_obj_st * node = gt_obj_create(parent);
    if (NULL == node || 0 == depth) {
        return node;
    }
    uint32_t width = rand() % 4;    /** 0~3 个子节点 */
    for (uint32_t w = 0; w < width; ++w) {
        _fuzz_rand_subtree(node, depth - 1);
    }
    return node;
}

/**
 * @brief 把一棵随机子树的根入活对象池；池满时先销毁被覆盖的旧对象，保证池总量有界、无泄漏
 * @param obj 子树根
 */
static void _fuzz_pool_push(gt_obj_st * obj)
{
    if (NULL == obj) {
        return;
    }
    if (_s_fuzz_pool_cnt < _STRESS_FUZZ_POOL_MAX) {
        _s_fuzz_pool[_s_fuzz_pool_cnt++] = obj;
        return;
    }
    uint32_t i = rand() % _STRESS_FUZZ_POOL_MAX;
    if (NULL != _s_fuzz_pool[i]) {
        gt_obj_destroy(_s_fuzz_pool[i]);
        ++_s_fuzz_destroy_ops;
    }
    _s_fuzz_pool[i] = obj;
}

/**
 * @brief 从活对象池随机挑一个对象销毁（幂等，池槽置 NULL）
 */
static void _fuzz_pool_destroy_one(void)
{
    if (0 == _s_fuzz_pool_cnt) {
        return;
    }
    for (uint32_t tries = 0; tries < _STRESS_FUZZ_POOL_MAX; ++tries) {
        uint32_t i = rand() % _s_fuzz_pool_cnt;
        if (NULL == _s_fuzz_pool[i]) {
            continue;
        }
        gt_obj_destroy(_s_fuzz_pool[i]);
        _s_fuzz_pool[i] = NULL;
        ++_s_fuzz_destroy_ops;
        return;
    }
}

/**
 * @brief 纯内存 fuzz：随机 size 的 malloc / realloc / free 交错，覆盖 8B~8KB 大小块；
 *        约一半走延迟回收路径（defer），帧末 flush 归还
 */
static void _fuzz_mem_ops(void)
{
    for (uint32_t i = 0; i < _STRESS_MEM_FUZZ_OPS; ++i) {
        uint32_t r = rand();
        size_t size = 8 + (r % _STRESS_MEM_FUZZ_MAX_SIZE);
        void * p = gt_mem_malloc(size);
        if (NULL == p) {
            continue;
        }
        if (r & 0x1u) {
            size_t nsize = 8 + (rand() % _STRESS_MEM_FUZZ_MAX_SIZE);
            void * np = gt_mem_realloc(p, nsize);
            if (np) {
                p = np;
            }
        }
        if (r & 0x2u) {
            gt_mem_free(p);
        } else {
            gt_recycle_defer(p);
        }
        ++_s_fuzz_mem_ops;
    }
}

/**
 * @brief 打印当前堆 used/blocks 趋势（抓慢泄漏）
 * @param tag 日志标签
 */
static void _fuzz_report(const char * tag)
{
    gt_mem_info_st info;
    gt_mem_heap_get_space(&info);
    _STRESS_LOG("[recycle] %s: used=%u blks=%u", tag, (uint32_t)info.used, (uint32_t)info.blocks);
}

/**
 * @brief fuzz/soak 主阶段：随机建树/销毁/纯内存交错，按 GT_STRESS_MINUTES 控制时长
 */
static void _stress_fuzz(void)
{
    if (0 == _s_fuzz_start) {
        /** 之前的 _s_anchor 可能在 CHURN/STACK/PAGE_CHURN 阶段被屏幕栈切换
         *  间接销毁（REENTER 阶段 anchor 若指向初始主屏，owned=false，CHURN
         *  结束不会清掉它，STACK 切换后才悬空）。此处强制重建独立屏幕作
         *  fuzz 父对象，不复用旧指针；fuzz 自身不压栈弹栈，anchor 全程稳定。 */
        _s_owned_anchor = false;
        _s_anchor = gt_obj_create(NULL);
        if (NULL != _s_anchor) {
            _s_owned_anchor = true;
        }
        _s_fuzz_start = time(NULL);
        _s_fuzz_last_report = _s_fuzz_start;
        srand(GT_STRESS_RAND_SEED);
        _STRESS_LOG("[recycle] fuzz start: minutes=%d seed=0x%x", GT_STRESS_MINUTES, GT_STRESS_RAND_SEED);
    }

    for (uint32_t op = 0; op < _STRESS_FUZZ_OPS_PER_FRAME; ++op) {
        switch (rand() % 3) {
            case 0:
                _fuzz_pool_push(_fuzz_rand_subtree(_s_anchor, rand() % 3));
                ++_s_fuzz_create_ops;
                break;
            case 1:
                _fuzz_pool_destroy_one();
                break;
            default:
                _fuzz_mem_ops();
                break;
        }
    }

    time_t now = time(NULL);
    if (now - _s_fuzz_last_report >= _STRESS_FUZZ_REPORT_SEC) {
        _s_fuzz_last_report = now;
        _fuzz_report("fuzz trend");
    }

    if (now - _s_fuzz_start >= (time_t)(GT_STRESS_MINUTES * 60)) {
        _fuzz_report("fuzz end");
        _STRESS_LOG("[recycle] fuzz done: create=%u destroy=%u mem=%u",
            _s_fuzz_create_ops, _s_fuzz_destroy_ops, _s_fuzz_mem_ops);
        _s_phase = _STRESS_PH_DONE;
    }
}

/* global functions / API interface -------------------------------------*/
void _gt_test_obj_destroy_stress(void)
{
    switch (_s_phase) {
        case _STRESS_PH_REENTER:
            _stress_reenter();
            _s_phase = _STRESS_PH_SELF_DESTROY;
            break;

        case _STRESS_PH_SELF_DESTROY:
            _stress_self_destroy();
            _s_phase = _STRESS_PH_SCREEN;
            break;

        case _STRESS_PH_SCREEN:
            _stress_screen();
            _s_phase = _STRESS_PH_CHILDREN;
            break;

        case _STRESS_PH_CHILDREN:
            _stress_children();
            _s_phase = _STRESS_PH_CHURN;
            _s_frame = 0;
            break;

        case _STRESS_PH_CHURN:
            _stress_churn();
            if (++_s_frame >= _STRESS_CHURN_FRAMES) {
                if (_s_owned_anchor) {
                    gt_obj_destroy_immediately(_s_anchor);
                    _s_anchor = NULL;
                    _s_owned_anchor = false;
                }
                _s_phase = _STRESS_PH_STACK;
                _s_frame = 0;
                _STRESS_LOG("[recycle] churn PASS (%d frames)", _STRESS_CHURN_FRAMES);
            }
            break;

        case _STRESS_PH_STACK:
            _stress_stack();
            break;

        case _STRESS_PH_PAGE_CHURN:
            _stress_page_churn();
            break;

        case _STRESS_PH_BULK:
            _stress_bulk_destroy();
            _s_phase = _STRESS_PH_FUZZ;
            break;

        case _STRESS_PH_FUZZ:
            _stress_fuzz();
            break;

        case _STRESS_PH_DONE:
            /** All stress phases passed. Terminate explicitly so `make run`
             *  and CI return instead of idling forever in the simulator's
             *  while(1) main loop (a GUI simulator is resident by design; the
             *  stress harness exits with code 0 on success, non-zero on abort). */
            _STRESS_LOG("[recycle] ALL PHASES PASS");
            fflush(stdout);
            fflush(stderr);
            exit(0);
            break;

        default:
            break;
    }
}

#endif  /** _GT_TEST_OBJ_DESTROY_STRESS */
#endif  /** _GT_USE_TEST */
/* end ------------------------------------------------------------------*/
