/**
 * @file gt_serial_show.h
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2024-09-20 17:22:31
 * @copyright Copyright (c) 2014-2024, Company Genitop. Co., Ltd.
 */
#ifndef _GT_SERIAL_SHOW_H_
#define _GT_SERIAL_SHOW_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../../gt_conf.h"
#include "../../others/gt_types.h"
#include "../../widgets/gt_obj.h"

#if GT_USE_SERIAL


/* define ---------------------------------------------------------------*/
#ifndef GT_USE_SERIAL_SHOW
    /**
     * @brief Serial show
     *      [default: 0]
     */
    #define GT_USE_SERIAL_SHOW       01
#endif

#endif  /** GT_USE_SERIAL */
//

#if GT_USE_SERIAL_SHOW && GT_USE_BIN_CONVERT

// show file
#ifndef GT_SERIAL_SHOW_FILE_HEAD_SIZE
    /* show file head info size */
    #define GT_SERIAL_SHOW_FILE_HEAD_SIZE   (16)
#endif

#ifndef GT_SERIAL_SHOW_FILE_PAGE_INFO_SIZ
    /* show file page info size */
    #define GT_SERIAL_SHOW_FILE_PAGE_INFO_SIZ    (4)
#endif

#ifndef GT_SERIAL_UI_PAGE_STACK_DEPTH
    /* show file page stack depth */
    #define GT_SERIAL_UI_PAGE_STACK_DEPTH   (20)
#endif

#ifndef GT_SERIAL_UI_HOME_PAGE_INDEX
    /* show file home page index */
    #define GT_SERIAL_UI_HOME_PAGE_INDEX    (0)
#endif

#ifndef GT_SERIAL_UI_PAGE_CONTROL_SIZE
    #define GT_SERIAL_UI_PAGE_CONTROL_SIZE      (32)
#endif

#ifndef GT_SERIAL_UI_PAGE_CONTROL_MAX_NUMB
    #define GT_SERIAL_UI_PAGE_CONTROL_MAX_NUMB  (256)
#endif

// font and img
#ifndef GT_SERIAL_RESOURCE_FILE_HEAD_SIZE
    #define GT_SERIAL_RESOURCE_FILE_HEAD_SIZE   (10)
#endif

#ifndef GT_SERIAL_ZK_FILE_INFO_SIZE
    #define GT_SERIAL_ZK_FILE_INFO_SIZE     (6)
#endif

#ifndef GT_SERIAL_IMG_FILE_INFO_SIZE
    #define GT_SERIAL_IMG_FILE_INFO_SIZE    (9)
#endif

#ifndef GT_SERIAL_FONT_CONFIG_HEAD_SIZE
    #define GT_SERIAL_FONT_CONFIG_HEAD_SIZE  (6)
#endif

#define GT_SERIAL_FONT_CONFIG_INFO_SIZE      ((FONT_LAN_MAX_COUNT * 2) + 1)
/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief show init
 *
 */
void gt_serial_show_init(void);

/**
 * @brief get page count
 *
 * @return uint16_t page count
 */
uint16_t gt_serial_page_count_get(void);

/**
 * @brief get page index
 *
 * @return uint16_t page index
 */
uint16_t gt_serial_page_index_get(void);

/**
 * @brief load page
 *
 * @param index
 */
void gt_serial_load_page(gt_scr_id_t index);

/**
 * @brief
 *
 * @param scr_id
 * @param type
 * @param time
 * @param delay
 * @param del_prev_scr
 */
void gt_serial_load_page_anim(gt_scr_id_t scr_id, gt_scr_anim_type_et type, uint32_t time, uint32_t delay, bool del_prev_scr);

/**
 * @brief Go back to previous page
 *
 * @param step
 * @return gt_scr_id_t
 */
gt_scr_id_t gt_serial_go_back(int16_t step);

const uint8_t* gt_serial_get_info_by_obj(gt_obj_st* obj);

/**
 * @brief
 *
 * @param value 0x0000(vp / sp) + (0x00 ...)data
 * @param len byte length
 */
void gt_serial_set_value(uint8_t *value, uint16_t len);

uint16_t gt_serial_get_value(uint8_t * res_buffer, uint8_t * value, uint16_t len);

/**
 * @brief get value by addr
 *
 * @param res_buffer read data in to res_buffer
 * @param addr 0x0000
 * @param short_len the short or word length
 * @return uint16_t
 */
uint16_t gt_serial_get_value_by_addr(uint8_t * res_buffer, uint16_t addr, uint8_t short_len);

#endif /* GT_USE_SERIAL_CFG && GT_USE_BIN_CONVERT */
//
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_SERIAL_SHOW_H_

/* end of file ----------------------------------------------------------*/


