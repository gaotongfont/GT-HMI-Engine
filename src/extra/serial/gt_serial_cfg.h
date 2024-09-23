/**
 * @file gt_serial_cfg.h
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2024-09-20 16:06:38
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_SERIAL_CFG_H_
#define _GT_SERIAL_CFG_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../../gt_conf.h"
#include "../../others/gt_types.h"

#if GT_USE_SERIAL


/* define ---------------------------------------------------------------*/
#ifndef GT_USE_SERIAL_CFG
    /**
     * @brief Serial configuration
     *      [default: 0]
     */
    #define GT_USE_SERIAL_CFG       01
#endif

#endif  /** GT_USE_SERIAL */


#if GT_USE_SERIAL_CFG && GT_USE_BIN_CONVERT

/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

void gt_serial_cfg_init(void);





#endif  /** GT_USE_SERIAL_CFG && GT_USE_BIN_CONVERT */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_SERIAL_CFG_H_
