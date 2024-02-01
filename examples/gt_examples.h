/**
 * @file gt_examples.h
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2022-09-28 11:40:52
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_EXAMPLES_H_
#define _GT_EXAMPLES_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/

#include "../src/gt_conf.h"
#include "../others/gt_log.h"
#include "../src/core/gt_style.h"
#include "../src/core/gt_disp.h"
#include "../src/widgets/gt_obj.h"


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

#if GT_USE_EXAMPLES

void gt_examples_anim1(void);

void gt_examples_button(void);

void gt_examples_checkbox(void);

void gt_examples_img_button(void);

void gt_examples_img(void);

void gt_example_input(void);

void gt_examples_keyboard(void);

void gt_examples_label(void);

void gt_examples_line(void);

void gt_examples_listview(void);

void gt_examples_progress_bar(void);

void gt_examples_roller(void);

#endif  /* !GT_USE_EXAMPLES */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_EXAMPLES_H_
