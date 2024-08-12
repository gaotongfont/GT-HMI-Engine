/**
 * @file gt_font_config.h
 * @author Yang (your@email.com)
 * @brief
 * @version 0.1
 * @date 2024-07-19 10:09:29
 * @copyright Copyright (c) 2014-2024, Company Genitop. Co., Ltd.
 */
#ifndef _GT_FONT_CONFIG_H_
#define _GT_FONT_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"


/* define ---------------------------------------------------------------*/


/* typedef --------------------------------------------------------------*/
typedef enum {
    family_none = -1,
}gt_font_family_et;



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
void gt_font_config_init(void);



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_FONT_CONFIG_H_

/* end of file ----------------------------------------------------------*/


