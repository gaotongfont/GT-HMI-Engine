/**
 * @file gt_scr_stack.h
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2024-03-20 10:28:20
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_SCR_STACK_H_
#define _GT_SCR_STACK_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../others/gt_types.h"
#include "../hal/gt_hal_disp.h"
#include "../utils/gt_stack.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef struct gt_scr_list_s {
    gt_scr_id_t scr_id;
    gt_scr_init_func_cb_t init_cb;
}gt_scr_list_st;

typedef struct gt_scr_mapping_table_s {
    gt_scr_list_st const * scr_list;
    gt_scr_id_t count;
}gt_scr_mapping_table_st;



/**
 * @brief
 */
typedef struct gt_scr_stack_item_s {
    gt_scr_id_t prev_scr_id;    /** remark last screen id from current_scr_id */
    gt_scr_id_t current_scr_id; /** current screen id */

    /** NULL[default]: need to reload screen by prev_scr_id -> init_cb,
     * otherwise load current scj object directly.
     * [Warn] home screen always NULL!
     */
    gt_obj_st * prev_scr_alive;
    gt_obj_st * current_scr;

    uint32_t time;
    uint32_t delay;
    gt_scr_anim_type_et anim_type;
}gt_scr_stack_item_st;

typedef struct _gt_scr_home_s {
    gt_scr_id_t id;             /** -1: unset home page, >= 0: Have home page */
    gt_obj_st * screen;
    uint32_t time;
    uint8_t alive       : 1;    /** 0: not alive, free home page screen each time; 1: home page screen object alive */
    uint8_t reserved    : 7;
}_gt_scr_home_st;

typedef struct _gt_gc_scr_s {
    gt_stack_st * stack;
    _gt_scr_home_st home_scr;

    gt_scr_mapping_table_st mapping_table;
}_gt_gc_scr_st;



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief Get screen init function by screen id
 *
 * @param scr_id screen id
 * @return gt_scr_init_func_cb_t
 */
gt_scr_init_func_cb_t gt_scr_stack_get_init_func(gt_scr_id_t scr_id);

/**
 * @brief Register screen id list, and stack depth for the screen stack
 *
 * @param scr_list screen list
 * @param count count of screen
 * @param stack_depth screen stack depth
 * @return true success
 * @return false fail
 */
bool gt_scr_stack_register_id_list(gt_scr_list_st const * const scr_list, gt_scr_id_t count, gt_stack_size_t stack_depth);

/**
 * @brief Set home page screen id, and life cycle
 *
 * @param scr_id -1[default]: disable home page, >= 0: set home page id
 * @param alive_forever 0[default]: free home page screen each time, 1: home page screen object alive
 * @return true
 * @return false
 */
bool gt_scr_stack_set_home_scr_id(gt_scr_id_t scr_id, bool alive_forever);

void gt_scr_stack_set_home_scr(gt_obj_st * scr);
gt_obj_st * gt_scr_stack_get_home_scr(void);

gt_scr_id_t gt_scr_stack_get_home_scr_id(void);

/**
 * @brief home screen is alive
 *
 * @return true
 * @return false
 */
bool gt_scr_stack_is_home_scr_alive(void);

bool gt_scr_stack_set_depth(gt_stack_size_t stack_depth);

bool gt_scr_stack_push(gt_scr_stack_item_st * scr);

gt_scr_stack_item_st * gt_scr_stack_pop(gt_stack_size_t count);
gt_scr_stack_item_st * gt_scr_stack_peek(void);

gt_obj_st * gt_scr_stack_get_prev_scr(void);

/**
 * @brief judge the previous screen is the screen which we want
 *
 * @param prev_scr_id
 * @return true
 * @return false
 */
bool gt_scr_stack_is_prev(gt_scr_id_t prev_scr_id);

/**
 * @brief
 *
 * @param scr_id
 * @return true
 * @return false
 */
bool gt_scr_stack_is_current(gt_scr_id_t scr_id);

/**
 * @brief
 *
 * @return gt_scr_id_t -1: if no previous screen
 */
gt_scr_id_t gt_scr_stack_get_prev_id(void);

/**
 * @brief Get the previous screen id by step
 *
 * @param step >= 0: valid step, < 0: invalid step
 * @return gt_scr_id_t screen id
 */
gt_scr_id_t gt_scr_stack_get_prev_id_by(gt_stack_size_t step);

/**
 * @brief Get the previous screen object by step
 *
 * @param step >= 0: valid step, < 0: invalid step
 * @return gt_obj_st* screen object
 */
gt_obj_st * gt_scr_stack_get_prev_scr_by(gt_stack_size_t step);

/**
 * @brief Search the item in the stack, base on scr->current_scr_id value to
 *     find the item.
 *
 * @param scr which stack frame want to search, base on current_scr_id value
 * @return gt_stack_item_st ret.data is NULL or ret.index_from_top is -1, if not found
 */
gt_stack_item_st gt_scr_stack_has_before(gt_scr_stack_item_st * scr);

bool gt_scr_stack_is_empty(void);
bool gt_scr_stack_is_full(void);

gt_stack_size_t gt_scr_stack_get_count(void);

/**
 * @brief Clear all screen stack, expect alive home screen
 */
void gt_scr_stack_clear(void);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_SCR_STACK_H_
