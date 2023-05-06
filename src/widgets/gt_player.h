/**
 * @file gt_player.h
 * @author Feyoung
 * @brief Slide effect, play content according to the rule
 * @version 0.1
 * @date 2023-05-04 14:22:12
 * @copyright Copyright (c) 2014-2023, Company Genitop. Co., Ltd.
 */
#ifndef _GT_PLAYER_H_
#define _GT_PLAYER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "./gt_conf_widgets.h"
#include "gt_obj.h"


#if GT_CFG_ENABLE_PLAYER
/* define ---------------------------------------------------------------*/
/**
 * @brief The objects displayed use custom display position
 */
#define _GT_PLAYER_USE_CUSTOM_POS       0


#define _GT_PLAYER_BIT_TYPE_LENGTH      4
#define _GT_PLAYER_BIT_MODE_LENGTH      4
#define _GT_PLAYER_BIT_STATUS_LENGTH    2
#define _GT_PLAYER_BIT_AUTO_LENGTH      1


#define _GET_BIT_SHIFT_SIZE(_val)       (1 << (_val))

/**
 * @brief The max type count of the player
 */
#define _GT_PLAYER_TYPE_MAX_LIMIT_SIZE  _GET_BIT_SHIFT_SIZE(_GT_PLAYER_BIT_TYPE_LENGTH)
/**
 * @brief The max mode count of the player
 */
#define _GT_PLAYER_MODE_MAX_LIMIT_SIZE  _GET_BIT_SHIFT_SIZE(_GT_PLAYER_BIT_MODE_LENGTH)

/* typedef --------------------------------------------------------------*/
/**
 * @brief The object played by the player
 */
typedef enum _gt_player_type_e {
    GT_PLAYER_TYPE_IMG = 0,     ///< image object

    GT_PLAYER_TYPE_MAX_COUNT,
}_gt_player_type_et;

/**
 * @brief work mode of the player
 */
typedef enum _gt_player_mode_e {
    GT_PLAYER_MODE_NONE = 0,    ///< none mode
    GT_PLAYER_MODE_ONCE,        ///< play it once mode
    GT_PLAYER_MODE_LOOP,        ///< loop mode
    GT_PLAYER_MODE_PLAYBACK,    ///< playback mode

    GT_PLAYER_MODE_MAX_COUNT,
}_gt_player_mode_et;

typedef struct _gt_player_item_s {
    void * src;
    uint16_t test;
#if _GT_PLAYER_USE_CUSTOM_POS
    gt_area_st area;
#endif
}_gt_player_item_st;

/**
 * @brief process object
 */
typedef struct _gt_player_obj_s {
    gt_size_t index;                ///< the current index of item list
    uint16_t count;                 ///< the count of items
    _gt_player_item_st ** list;      ///< the list of items
}_gt_player_obj_st;

/**
 * @brief control's register
 */
typedef struct _gt_player_reg_s {
    uint16_t type: _GT_PLAYER_BIT_TYPE_LENGTH;          ///< @ref _gt_player_type_et
    uint16_t mode: _GT_PLAYER_BIT_MODE_LENGTH;          ///< @ref _gt_player_mode_et
    uint16_t status: _GT_PLAYER_BIT_STATUS_LENGTH;      ///< status of the player; play, pause, stop
    uint16_t auto_switch: _GT_PLAYER_BIT_AUTO_LENGTH;          ///< 0: manual, 1: auto
    uint16_t reserved: 5;
}_gt_player_reg_st;

typedef struct _gt_style_player_s {
    _gt_player_reg_st reg;
    _gt_player_obj_st target;
}_gt_style_player_st;


/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_player_create(gt_obj_st * parent);

void gt_player_set_type(gt_obj_st * obj, _gt_player_type_et type);
void gt_player_set_mode(gt_obj_st * obj, _gt_player_mode_et mode);

void gt_player_remove_item(gt_obj_st * obj, void * item);
void gt_player_remove_item_by_index(gt_obj_st * obj, uint16_t idx);

#if _GT_PLAYER_USE_CUSTOM_POS
uint16_t gt_player_add_item(gt_obj_st * obj, void * item, uint16_t item_byte_size, gt_area_st * area);
#else
uint16_t gt_player_add_item(gt_obj_st * obj, void * item, uint16_t item_byte_size);
#endif  /** !_GT_PLAYER_USE_CUSTOM_POS */


void gt_player_turn_prev(gt_obj_st * obj);
void gt_player_turn_next(gt_obj_st * obj);

#endif /** GT_CFG_ENABLE_PLAYER */
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_PLAYER_H_
