/**
 * @file gt_gc.h
 * @author Feyoung
 * @brief The global call
 * @version 0.1
 * @date 2022-06-14 16:56:57
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_GC_H_
#define _GT_GC_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_ll.h"
#include "../core/gt_timer.h"
#include "../others/gt_list.h"
#include "../hal/gt_hal_disp.h"


/* define ---------------------------------------------------------------*/

typedef struct gt_gc_s {
    struct _gt_list_head _gt_anim_ll;
    struct _gt_list_head _gt_timer_ll;
    struct _gt_list_head _gt_img_decoder_ll;
    struct _gt_event_s * _gt_event_node_header_ll;

    gt_disp_st * disp;
}gt_gc_st;

extern gt_gc_st _gt_global_call;

#define _GT_GC_GET_ROOT(_name)  ((&_gt_global_call)->_name)

/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/

static inline int _gt_gc_is_ll_empty(const struct _gt_list_head * head) {
    return _gt_list_empty(head);
}


/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

gt_disp_st * _gt_gc_get_disp(void);
void _gt_gc_set_disp(gt_disp_st * disp);

uint16_t _gt_gc_get_size(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_GC_H_
