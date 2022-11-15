/**
 * @file gt.h
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-05-11 18:22:31
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_H_
#define _GT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
/* standard */
#include "stdint.h"
#include "string.h"
#include "math.h"
#include "stdlib.h"

/* core */
#include "src/core/gt_handler.h"
#include "src/core/gt_graph_base.h"
#include "src/core/gt_fs.h"
#include "src/core/gt_event.h"
#include "src/core/gt_disp.h"
#include "src/core/gt_mem.h"
#include "src/core/gt_style.h"
#include "src/core/gt_obj_pos.h"
#include "src/core/gt_timer.h"
#include "src/core/gt_draw.h"

/* font */
#include "src/font/gt_font.h"
#include "src/font/gt_symbol.h"

/* hal */
#include "src/hal/gt_hal_disp.h"
#include "src/hal/gt_hal_indev.h"
#include "src/hal/gt_hal_vf.h"
#include "src/hal/gt_hal_tick.h"

/* widgets */
#include "src/widgets/gt_conf_widgets.h"
#include "src/widgets/gt_obj.h"

/* draw */
#include "src/extra/draw/gt_draw_blend.h"


#if GT_CFG_ENABLE_BTN == 1
#include "src/widgets/gt_btn.h"
#endif

#if GT_CFG_ENABLE_CHECKBOX == 1
#include "src/widgets/gt_checkbox.h"
#endif

#if GT_CFG_ENABLE_GROUP == 1
#include "src/widgets/gt_group.h"
#endif

#if GT_CFG_ENABLE_IMG == 1
#include "src/widgets/gt_img.h"
#endif

#if GT_CFG_ENABLE_INPUT == 1
#include "src/widgets/gt_input.h"
#include "src/widgets/gt_keypad.h"
#endif



#if GT_CFG_ENABLE_LABEL == 1
#include "src/widgets/gt_label.h"
#endif

#if GT_CFG_ENABLE_LISTVIEW == 1
#include "src/widgets/gt_listview.h"
#endif

#if GT_CFG_ENABLE_PROGRESS_BAR == 1
#include "src/widgets/gt_progress_bar.h"
#endif

#if GT_CFG_ENABLE_RADIO == 1
#include "src/widgets/gt_radio.h"
#endif

#if GT_CFG_ENABLE_SLIDER == 1
#include "src/widgets/gt_slider.h"
#endif

#if GT_CFG_ENABLE_SWITCH == 1
#include "src/widgets/gt_switch.h"
#endif

#if GT_CFG_ENABLE_TEXTAREA == 1
#include "src/widgets/gt_textarea.h"
#endif

#if GT_CFG_ENABLE_IMGBTN == 1
#include "src/widgets/gt_imgbtn.h"
#endif

#if GT_CFG_ENABLE_RECT == 1
#include "src/widgets/gt_rect.h"
#endif

/* other */
#include "src/others/gt_color.h"
#include "src/others/gt_log.h"
#include "src/others/gt_txt.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_H_
