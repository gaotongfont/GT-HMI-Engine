/**
 * @file gt_serial_show.h
 * @author Yang (your@email.com)
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
#include <stdint.h>
#include <stdbool.h>

#if GT_USE_SERIAL


/* define ---------------------------------------------------------------*/



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
void gt_serial_load_page(uint16_t index);

/**
 * @brief Go back to previous page
 *
 * @param step
 * @return int16_t
 */
int16_t gt_serial_go_back(int16_t step);

/**
 * @brief Set the value using vp
 *
 * @param vp
 * @param value
 * @param len
 */
void gt_serial_control_value_set(uint16_t vp, uint8_t* value, uint16_t len);


/**
 * @brief Set the param using sp
 *
 * @param sp
 * @param param
 * @param len
 */
void gt_serial_control_param_set(uint16_t sp, uint8_t* param, uint16_t len);


#endif
//
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_SERIAL_SHOW_H_

/* end of file ----------------------------------------------------------*/


