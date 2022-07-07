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

/* font */
#include "src/font/gt_font.h"

/* hal */
#include "src/hal/gt_hal_disp.h"
#include "src/hal/gt_hal_indev.h"
#include "src/hal/gt_hal_vf.h"
#include "src/hal/gt_hal_tick.h"

/* widgets */
#include "src/widgets/gt_obj.h"
#include "src/widgets/gt_btn.h"
#include "src/widgets/gt_img.h"

/* other */
#include "src/others/gt_color.h"
#include "src/others/gt_log.h"


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_H_
