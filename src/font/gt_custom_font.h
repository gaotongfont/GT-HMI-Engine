/**
 * @file gt_custom_font.h
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2025-10-14 19:50:25
 * @copyright Copyright (c) 2014-2025, Company Genitop. Co., Ltd.
 */
#ifndef _GT_CUSTOM_FONT_H_
#define _GT_CUSTOM_FONT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../widgets/gt_conf_widgets.h"

#if GT_CUSTOM_FONT_ENABLE
#include "stdint.h"
#include "stdbool.h"
#include  "gt_font.h"

/* define ---------------------------------------------------------------*/
#define GT_CUSTOM_FONT_OPTION(option)  (option | (1 << 14))
#define IS_GT_CUSTOM_FONT_OPTION(option)  ((option >> 14) & 0x01)
/* typedef --------------------------------------------------------------*/
typedef struct gt_custom_font_s{
    gt_font_func_st ffs;
    int (*get_dot_width)(struct gt_custom_font_s* , uint32_t );
    int (*get_width)(struct gt_custom_font_s* , uint32_t );
    int (*get_height)(struct gt_custom_font_s* , uint32_t );
}gt_custom_font_st;


typedef struct gt_custom_emoji_s{
    gt_custom_font_st *font;
    bool (* is_emoji)(struct gt_custom_emoji_s * , uint32_t );
}gt_custom_emoji_st;
/* macros ---------------------------------------------------------------*/

/* global functions / API interface -------------------------------------*/
void gt_custom_font_init(void);
void gt_custom_register_font(gt_custom_font_st* font);
void gt_custom_register_emoji(gt_custom_emoji_st* emj);
bool gt_custom_is_emoji(uint32_t option, uint32_t font_code);
gt_custom_font_st* gt_custom_get_font(uint32_t option);


#endif //!GT_CUSTOM_FONT_ENABLE
//
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_CUSTOM_FONT_H_

/* end of file ----------------------------------------------------------*/


