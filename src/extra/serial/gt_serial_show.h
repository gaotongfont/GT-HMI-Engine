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
#include "../../others/gt_types.h"

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

#ifndef GT_SERIAL_SHOW_FILE_ADDR
    /* show file addrese */
    #define GT_SERIAL_SHOW_FILE_ADDR        (0x0007C7AC)//(0x380000)
#endif

#ifndef GT_SERIAL_SHOW_FILE_HEAD_SIZE
    /* show file head info size */
    #define GT_SERIAL_SHOW_FILE_HEAD_SIZE   (16)
#endif

#ifndef GT_SERIAL_SHOW_FILE_PAGE_INFO_ADDR
    /* show file page info addr */
    #define GT_SERIAL_SHOW_FILE_PAGE_INFO_ADDR  (GT_SERIAL_SHOW_FILE_ADDR + GT_SERIAL_SHOW_FILE_HEAD_SIZE)
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
    #define GT_SERIAL_UI_HOME_PAGE_INDEX   (1)
#endif

#ifndef GT_SERIAL_UI_PAGE_CONTROL_SIZE
    #define GT_SERIAL_UI_PAGE_CONTROL_SIZE      (32)
#endif

#ifndef GT_SERIAL_UI_PAGE_CONTROL_MAX_NUMB
    #define GT_SERIAL_UI_PAGE_CONTROL_MAX_NUMB  (255)
#endif
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


void gt_serial_set_value(uint8_t *value, uint16_t len);



#endif /* GT_USE_SERIAL_CFG && GT_USE_BIN_CONVERT */
//
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_SERIAL_SHOW_H_

/* end of file ----------------------------------------------------------*/


