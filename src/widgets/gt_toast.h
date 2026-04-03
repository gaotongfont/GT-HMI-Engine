/**
 * @file gt_toast.h
 * @author Feyoung
 * @brief
 *      msg: width = (str_width + font_width) <= (scr_width - font_width)
 *      m: half of font width
 *      |---------------------------------|
 *      | m |          msg            | m |
 *      |---------------------------------|
 * @version 0.1
 * @date 2025-08-21
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_TOAST_H_
#define _GT_TOAST_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_TOAST
#include "../font/gt_font.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef void ( * gt_toast_cb)(void *);


typedef enum {
    GT_TOAST_POS_MIDDLE = 0,    /** default */
    GT_TOAST_POS_TOP,
    GT_TOAST_POS_BOTTOM,
}gt_toast_pos_st;


typedef struct {
    char * message;
    // char * icon;

    gt_toast_cb open_cb;
    gt_toast_cb close_cb;
    void * data_p;          /** called by open_cb / close_cb, just a remark pointer */

    gt_color_t bg_color;
    gt_color_t font_color;
    gt_opa_t opa;

    uint32_t duration;        /** display period, default: 2000ms, 0ms: alive forever */
    uint8_t  position : 2;    /** default: middle @ref gt_toast_pos_st */
    // uint8_t loading_type : 2;
    // uint8_t overlay                : 1;     /** background overlay */
    // uint8_t forbid_click           : 1;     /** forbid click background */
    // uint8_t close_on_click         : 1;     /** close when toast was clicked */
    // uint8_t close_on_click_overlay : 1;     /** close when the mask was clicked */
}gt_toast_param_st;


/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief Except font setting.
 *
 * @param toast_or_null NULL: reset toast global setting; or reset struct.
 */
void gt_toast_reset(gt_toast_param_st * toast);

void gt_toast(gt_toast_param_st * toast);
void gt_toast_loading(gt_toast_param_st * toast);
bool gt_toast_is_showing(void);

void gt_toast_close(void);


void gt_toast_global_set_font_size(uint8_t size);
void gt_toast_global_set_font_gray(uint8_t gray);

/**
 * @brief Set software scale target size for font rendering (global setting).
 *        The font library size (set by gt_toast_global_set_font_size) is used as source;
 *        the glyph is scaled to scale_size pixels before display.
 *        0 or equal to font size: no scaling.
 *        gray=1 uses nearest-neighbor; gray=2/4 uses bilinear interpolation.
 *
 * @param scale_size target display size in pixels, 0 = disabled
 */
void gt_toast_global_set_font_scale_size(uint8_t scale_size);

void gt_toast_set_indent(uint16_t indent);
void gt_toast_global_set_font_align(gt_align_et align);
#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
void gt_toast_global_set_font_family_cn(gt_family_t family);
void gt_toast_global_set_font_family_en(gt_family_t family);
void gt_toast_global_set_font_family_fl(gt_family_t family);
void gt_toast_global_set_font_family_numb(gt_family_t family);
#else
void gt_toast_global_set_font_family(gt_family_t family);
void gt_toast_global_set_font_cjk(gt_obj_st* toast, gt_font_cjk_et cjk);
#endif
void gt_toast_global_set_font_thick_en(uint8_t thick);
void gt_toast_global_set_font_thick_cn(uint8_t thick);
void gt_toast_global_set_font_encoding(gt_encoding_et encoding);

void gt_toast_global_set_space(uint8_t space_x, uint8_t space_y);

void gt_toast_global_set_font_style(gt_font_style_et font_style);

uint8_t gt_toast_global_get_font_size(void);



void gt_toast_global_set_font_info(gt_font_info_st * font_info);
gt_font_info_st * gt_toast_global_get_font_info(void);

#endif  /** GT_CFG_ENABLE_TOAST */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_TOAST_H_
