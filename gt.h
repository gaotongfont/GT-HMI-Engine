/**
 * @file gt.h
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-05-11 18:22:31
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
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
#include "src/core/gt_scr_stack.h"

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

#include "src/others/gt_anim.h"

#if GT_CFG_ENABLE_BTN
#include "src/widgets/gt_btn.h"
#endif

#if GT_CFG_ENABLE_CHECKBOX
#include "src/widgets/gt_checkbox.h"
#endif

#if GT_CFG_ENABLE_GROUP
#include "src/widgets/gt_group.h"
#endif

#if GT_CFG_ENABLE_IMG
#include "src/widgets/gt_img.h"
#endif

#if GT_CFG_ENABLE_INPUT
#include "src/widgets/gt_input.h"
#include "src/widgets/gt_keypad.h"
#endif



#if GT_CFG_ENABLE_LABEL
#include "src/widgets/gt_label.h"
#endif

#if GT_CFG_ENABLE_LISTVIEW
#include "src/widgets/gt_listview.h"
#endif

#if GT_CFG_ENABLE_PROGRESS_BAR
#include "src/widgets/gt_progress_bar.h"
#endif

#if GT_CFG_ENABLE_RADIO
#include "src/widgets/gt_radio.h"
#endif

#if GT_CFG_ENABLE_SLIDER
#include "src/widgets/gt_slider.h"
#endif

#if GT_CFG_ENABLE_SWITCH
#include "src/widgets/gt_switch.h"
#endif

#if GT_CFG_ENABLE_TEXTAREA
#include "src/widgets/gt_textarea.h"
#endif

#if GT_CFG_ENABLE_IMGBTN
#include "src/widgets/gt_imgbtn.h"
#endif

#if GT_CFG_ENABLE_RECT
#include "src/widgets/gt_rect.h"
#endif

#if GT_CFG_ENABLE_BARCODE
/* barcode */
#include "src/widgets/gt_barcode.h"
#endif

#if GT_CFG_ENABLE_QRCODE
#include "src/widgets/gt_qrcode.h"
#endif

#if GT_CFG_ENABLE_LINE
#include "src/widgets/gt_line.h"
#endif

#if GT_CFG_ENABLE_PLAYER
#include "src/widgets/gt_player.h"
#endif

#if GT_CFG_ENABLE_INPUT_NUMBER
#include "src/widgets/gt_input_number.h"
#endif

#if GT_CFG_ENABLE_CLOCK
#include "src/widgets/gt_clock.h"
#endif

#if GT_CFG_ENABLE_WORDART
#include "src/widgets/gt_wordart.h"
#endif

#if GT_CFG_ENABLE_VIEW_PAGER
#include "src/widgets/gt_view_pager.h"
#endif

#if GT_CFG_ENABLE_GIF
#include "src/widgets/gt_gif.h"
#endif

#if GT_CFG_ENABLE_ROLLER
#include "src/widgets/gt_roller.h"
#endif

#if GT_CFG_ENABLE_BTNMAP
#include "src/widgets/gt_btnmap.h"
#endif

#if GT_CFG_ENABLE_DIALOG
#include "src/widgets/gt_dialog.h"
#endif

#if GT_CFG_ENABLE_CHAT
#include "src/widgets/gt_chat.h"
#endif

#if GT_CFG_ENABLE_STATUS_BAR
#include "src/widgets/gt_status_bar.h"
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
