/**
 * @file gt_ll.h
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2022-06-14 20:53:51
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_LL_H_
#define _GT_LL_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_list.h"
#include "stdint.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef uint8_t _gt_node_t;

typedef struct _gt_ll_s {
    uint32_t count;
    _gt_node_t * head;
    struct gt_list_head list;
}_gt_ll_st;


/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_LL_H_
