/**
 * @file gt_example_img_button.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2022-10-10 10:51:58
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_imgbtn.h"
#include "../../src/widgets/gt_label.h"
#include "../../src/core/gt_img_decoder.h"


/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

#define RAW_IMG_W   80
#define RAW_IMG_H   40
#define RAW_IMG_PX  (RAW_IMG_W * RAW_IMG_H)


/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/
static gt_color_t _raw_buf_default[RAW_IMG_PX];
static gt_color_t _raw_buf_press[RAW_IMG_PX];
static gt_color_t _raw_buf_state[RAW_IMG_PX];

static gt_color_img_raw_st _raw_default;
static gt_color_img_raw_st _raw_press;
static gt_color_img_raw_st _raw_state;


/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

/**
 * @brief Fill a gt_color_img_raw_st with a solid RGB565 color
 */
static void _fill_raw_img(gt_color_img_raw_st * raw, gt_color_t * buf,
                          uint16_t w, uint16_t h, gt_color_t color)
{
    for (uint32_t i = 0; i < (uint32_t)(w * h); i++) {
        buf[i] = color;
    }
    raw->buffer     = buf;
    raw->opa        = NULL;
    raw->file_raw_p = NULL;
    raw->raw_len    = 0;
    raw->color      = gt_color_black();
    raw->width      = w;
    raw->height     = h;
    raw->fmt        = GT_IMG_CF_RGB565;
}


/* global functions / API interface -------------------------------------*/

gt_obj_st * gt_examples_img_button(void)
{
    gt_obj_st * screen;
    screen = gt_obj_create(NULL);

    /* ===== 1. file-path mode (original) ===== */
    gt_obj_st * label1 = gt_label_create(screen);
    gt_obj_set_pos(label1, 20, 10);
    gt_obj_set_size(label1, 200, 25);
    gt_label_set_text(label1, "File-path mode:");

    gt_obj_st * file_btn = gt_imgbtn_create(screen);
    gt_imgbtn_set_src(file_btn, "./2.png");
    gt_imgbtn_set_src_press(file_btn, "./1.bmp");
    gt_imgbtn_set_src_release(file_btn, "./2.png");
    gt_obj_set_pos(file_btn, 20, 40);

    /* ===== 2. raw mode (MODE 1: pre-decoded pixel buffer) ===== */
    /* raw pixel data must remain valid for widget lifetime — use static buffers */
    _fill_raw_img(&_raw_default, _raw_buf_default,
                  RAW_IMG_W, RAW_IMG_H, gt_color_blue());    /* blue */
    _fill_raw_img(&_raw_press, _raw_buf_press,
                  RAW_IMG_W, RAW_IMG_H, gt_color_red());     /* red */
    _fill_raw_img(&_raw_state, _raw_buf_state,
                  RAW_IMG_W, RAW_IMG_H, gt_color_green());   /* green */

    gt_obj_st * label2 = gt_label_create(screen);
    gt_obj_set_pos(label2, 20, 100);
    gt_obj_set_size(label2, 250, 25);
    gt_label_set_text(label2, "Raw mode (press=red):");

    gt_obj_st * raw_btn = gt_imgbtn_create(screen);
    gt_imgbtn_set_src_raw(raw_btn, &_raw_default);
    gt_imgbtn_set_src_press_raw(raw_btn, &_raw_press);
    gt_obj_set_pos(raw_btn, 20, 130);

    /* ===== 3. raw mode with state items ===== */
    gt_obj_st * label3 = gt_label_create(screen);
    gt_obj_set_pos(label3, 20, 200);
    gt_obj_set_size(label3, 400, 25);
    gt_label_set_text(label3, "Raw mode (state items, click to cycle):");

    gt_obj_st * state_btn = gt_imgbtn_create(screen);
    gt_imgbtn_set_src_raw(state_btn, &_raw_default);
    gt_imgbtn_set_src_press_raw(state_btn, &_raw_press);
    gt_imgbtn_add_state_item_raw(state_btn, &_raw_state);    /* green */
    gt_obj_set_pos(state_btn, 20, 230);

    gt_obj_st * hint = gt_label_create(screen);
    gt_obj_set_pos(hint, 120, 235);
    gt_obj_set_size(hint, 350, 25);
    gt_label_set_text(hint, "click-release to cycle: blue -> green");

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}


#endif
/* end ------------------------------------------------------------------*/
