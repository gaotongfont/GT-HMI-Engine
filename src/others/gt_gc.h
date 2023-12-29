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


/* define ---------------------------------------------------------------*/

#define _GT_GC_FUNC_DEFINED(_type, _name)   _type _name;
#define _GT_GC_FUNC_EXTERN(_type, _name)   extern _type _name;

#define _GT_GC_ROOT_PREFIX(_f, _type, _name)   _f(_type, _name)

#define _GT_GC_ITERATE_LIST(_f)    \
    _GT_GC_ROOT_PREFIX(_f, _gt_timer_st *, _gt_timer_act)              \
    _GT_GC_ROOT_PREFIX(_f, struct _gt_list_head, _gt_anim_ll)           \
    _GT_GC_ROOT_PREFIX(_f, struct _gt_list_head, _gt_timer_ll)          \
    _GT_GC_ROOT_PREFIX(_f, struct _gt_list_head, _gt_img_decoder_ll)    \


_GT_GC_ITERATE_LIST(_GT_GC_FUNC_EXTERN)
#define _GT_ROOT _GT_GC_ITERATE_LIST(_GT_GC_FUNC_DEFINED)

#define _GT_GC_GET_ROOT(_name)  _name

/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/

static inline int gt_gc_is_ll_empty(const struct _gt_list_head * head) {
    return _gt_list_empty(head);
}


/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_GC_H_
