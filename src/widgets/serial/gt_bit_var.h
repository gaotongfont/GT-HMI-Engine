/**
 * @file gt_bin_var.h
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2024-10-12 09:41:12
 * @copyright Copyright (c) 2014-2024, Company Genitop. Co., Ltd.
 */
#ifndef _GT_BIT_VAR_H_
#define _GT_BIT_VAR_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_BIT_VAR
#include "stdbool.h"
#include "gt_obj.h"
#include "../../core/gt_timer.h"


#if GT_USE_FILE_HEADER
#include "../../hal/gt_hal_file_header.h"
#endif
/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef enum{
    BIN_VAR_DISP_MODE_0 = 0,
    BIN_VAR_DISP_MODE_1,
    BIN_VAR_DISP_MODE_2,
    BIN_VAR_DISP_MODE_3,
    BIN_VAR_DISP_MODE_4,
    BIN_VAR_DISP_MODE_5,
    BIN_VAR_DISP_MODE_6,
    BIN_VAR_DISP_MODE_7,
}gt_bit_var_disp_mode_et;

typedef enum {
    MOVE_X_POSITION_NOT_RETAIN = 0, // Move on the X-axis, active bit is 0, not retain
    MOVE_Y_POSITION_NOT_RETAIN,     // Move on the Y-axis, active bit is 0, not retain
    MOVE_X_POSITION_RETAIN,         // Move on the X-axis, active bit is 0, retain
    MOVE_Y_POSITION_RETAIN,         // Move on the Y-axis, active bit is 0, retain
}gt_bit_var_move_mode_et;

/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

gt_obj_st * gt_bit_var_create(gt_obj_st * parent);


/**
 * @brief Remove all items of bit_var
 *
 * @param obj The bit_var object
 */
void gt_bit_var_remove_all_items(gt_obj_st * obj);

/**
 * @brief Add the bit_var project content
 *
 * @param obj
 * @param item The item to be added
 * @param item_byte_size The item memory size in bytes
 * @return gt_size_t -1: failed, The item count of bit_var
 */
gt_size_t gt_bit_var_add_item0(gt_obj_st * obj, void * item, uint16_t item_byte_size);
gt_size_t gt_bit_var_add_item1(gt_obj_st * obj, void * item, uint16_t item_byte_size);

#if GT_USE_FILE_HEADER
/**
 * @brief Add the bit_var project content by file header
 *
 * @param obj
 * @param fh idx -1[defalut]: Disabled file header, using img path to open file; >=0: index number, valid value
 *              < 0: invalid value.
 *           package_idx 0[defalut]: The index of element within item, as the first element
 * @return gt_size_t -1: failed, The item count of bit_var
 */
gt_size_t gt_bit_var_add_item0_by_file_header(gt_obj_st * obj, gt_file_header_param_st * fh);
gt_size_t gt_bit_var_add_item1_by_file_header(gt_obj_st * obj, gt_file_header_param_st * fh);
/**
 * @brief Add the bit_var project content by file header list
 *
 * @param obj
 * @param fh_array The array of gt_file_header_param_st
 * @param count The count of gt_file_header_param_st[]
 * @return gt_size_t -1: failed, The item count of bit_var
 */
gt_size_t gt_bit_var_add_item0_list_by_file_header(gt_obj_st * obj, gt_file_header_param_st const * const fh_array, uint16_t count);
gt_size_t gt_bit_var_add_item1_list_by_file_header(gt_obj_st * obj, gt_file_header_param_st const * const fh_array, uint16_t count);
#endif

#if GT_USE_DIRECT_ADDR
/**
 * @brief Add the bit_var project content by direct address
 *
 * @param obj
 * @param addr
 * @return gt_size_t
 */
gt_size_t gt_bit_var_add_item0_by_direct_addr(gt_obj_st * obj, gt_addr_t addr);
gt_size_t gt_bit_var_add_item1_by_direct_addr(gt_obj_st * obj, gt_addr_t addr);

/**
 * @brief Add the bit_var project content by direct address list
 *
 * @param obj
 * @param addr_array  The array of gt_addr_t
 * @param count The count of gt_addr_t[]
 * @return gt_size_t
 */
gt_size_t gt_bit_var_add_item0_list_by_direct_addr(gt_obj_st * obj, gt_addr_t const * const addr_array, uint16_t count);
gt_size_t gt_bit_var_add_item1_list_by_direct_addr(gt_obj_st * obj, gt_addr_t const * const addr_array, uint16_t count);
#endif


/**
 * @brief Set the bit_var display mode
 * |----  disp mode  -----|-------- value bit --------|
 * |----------------------|----- 0 -----|----- 1 -----|
 * | BIN_VAR_DISP_MODE_0: | item0 first | item1 first |
 * | BIN_VAR_DISP_MODE_1: | item0 first | none        |
 * | BIN_VAR_DISP_MODE_2: | item0 first | item1 anim  |
 * | BIN_VAR_DISP_MODE_3: | none        | item1 first |
 * | BIN_VAR_DISP_MODE_4: | none        | item1 anim  |
 * | BIN_VAR_DISP_MODE_5: | item0 anim  | item1 first |
 * | BIN_VAR_DISP_MODE_6: | item0 anim  | none        |
 * | BIN_VAR_DISP_MODE_7: | item0 anim  | item1 anim  |
 *
 * @param obj The bit_var object
 * @param mode The display mode
 */
void gt_bit_var_set_disp_mode(gt_obj_st * obj, gt_bit_var_disp_mode_et mode);

/**
 * @brief Set the bit_var move mode
 *
 * @param obj The bit_var object
 * @param mode The move mode
 */
void gt_bit_var_set_move_mode(gt_obj_st * obj, gt_bit_var_move_mode_et mode);

/**
 * @brief Set the bit_var move gap
 *
 * @param obj The bit_var object
 * @param gap The move gap
 */
void gt_bit_var_set_move_gap(gt_obj_st * obj, uint16_t gap);

/**
 * @brief Set the active bit_var
 *
 * @param obj The bit_var object bit [0]-active [1]-inactive
 * @param act The active bit_var index
 */
void gt_bit_var_set_active(gt_obj_st * obj, uint16_t act);
/**
 * @brief Get the active bit_var
 *
 * @param obj The bit_var object
 * @return uint16_t The active bit_var index
 */
uint16_t gt_bit_var_get_active(gt_obj_st * obj);

/**
 * @brief Set the bit_var value
 *
 * @param obj The bit_var object
 * @param value The bit_var value
 */
void gt_bit_var_set_value(gt_obj_st * obj, uint16_t value);

/**
 * @brief Get the bit_var value
 *
 * @param obj The bit_var object
 * @return uint16_t The bit_var value
 */
uint16_t gt_bit_var_get_value(gt_obj_st * obj);

/**
 * @brief Set the bit_var value bit
 *
 * @param obj The bit_var object
 * @param bit The bit index
 * @param value The bit value
 */
void gt_bit_var_set_value_bit(gt_obj_st* obj, uint8_t bit, bool value);
/**
 * @brief Get the bit_var value bit
 *
 * @param obj The bit_var object
 * @param bit The bit index
 * @return bool The bit value
 */
bool gt_bit_var_get_value_bit(gt_obj_st* obj, uint8_t bit);

/**
 * @brief Set the bit_var time (unit: ms)
 *
 * @param obj The bit_var object
 * @param time The bit_var time
 */
void gt_bit_var_set_time(gt_obj_st* obj, uint16_t time);

#endif // #if GT_CFG_ENABLE_BIT_VAR
//
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_BIT_VAR_H_

/* end of file ----------------------------------------------------------*/


