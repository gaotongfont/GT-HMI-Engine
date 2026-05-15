/**
 * @file gt_example_img.c
 * @author Feyoung
 * @brief Image widget examples: basic, transform (rotate/zoom/pivot), masking + rotation animation
 * @version 0.2
 * @date 2022-10-10 10:26:29
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/

#include "../gt_examples.h"
#include "../../src/widgets/gt_img.h"
#include "../../src/core/gt_event.h"
#include "../../src/others/gt_anim.h"
#include "../../src/widgets/gt_label.h"
#include "../../src/widgets/gt_btn.h"
#include "../../src/core/gt_disp.h"
#include "../../src/extra/svg/gt_svg.h"


/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES


/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/

static gt_obj_st * _screen = NULL;
static gt_obj_st * image   = NULL;
static gt_obj_st * image2  = NULL;
static int         _bg_idx = 0;

/* ---- transform demo ---- */
#if GT_USE_IMG_TRANSFORM
static gt_obj_st * _img_transform = NULL;
static uint16_t    _angle_val     = 0;
static uint16_t    _zoom_val      = 256;
#endif

/* ---- masking + rotation anim demo ---- */
#if GT_USE_IMG_TRANSFORM
static gt_obj_st * _img_mask_rot  = NULL;
#endif

/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static void _image_one_cb(gt_event_st * e) {
    GT_LOGD(GT_LOG_TAG_EXTRA, "image one call back");
    static int _cnt_1 = 0;
    gt_img_set_src(image, (_cnt_1++ % 2) ? "./img_hmi_02_common.png" : "./img_hmi_01_ai.png");
}

static void _bg_color_cb(gt_event_st * e) {
    static const uint32_t _bg_colors[] = { 0xFFFFFF, 0x000000, 0x444444, 0x1E3A5F };
    _bg_idx = (_bg_idx + 1) % 4;
    gt_screen_set_bgcolor(_screen, gt_color_hex(_bg_colors[_bg_idx]));
    gt_disp_invalid_area(_screen);
}

static void _image_two_cb(gt_event_st * e) {
    GT_LOGD(GT_LOG_TAG_EXTRA, "image two call back");
    static int _cnt_2 = 0;
    gt_img_set_src(image2, (_cnt_2++ % 2) ? "./img_hmi_01_ai.png" : "./img_hmi_02_common.png");
}

/* ---- transform callbacks ---- */
#if GT_USE_IMG_TRANSFORM
static void _rotate_cw_cb(gt_event_st * e) {
    _angle_val = (_angle_val + 50) % 3600;   /* +5 degrees */
    gt_img_set_angle(_img_transform, _angle_val);
}

static void _rotate_ccw_cb(gt_event_st * e) {
    _angle_val = (_angle_val + 3550) % 3600;  /* -5 degrees */
    gt_img_set_angle(_img_transform, _angle_val);
}

static void _zoom_in_cb(gt_event_st * e) {
    if (_zoom_val < 512) { _zoom_val += 32; }
    gt_img_set_zoom(_img_transform, _zoom_val);
}

static void _zoom_out_cb(gt_event_st * e) {
    if (_zoom_val > 64) { _zoom_val -= 32; }
    gt_img_set_zoom(_img_transform, _zoom_val);
}

static void _pivot_corner_cb(gt_event_st * e) {
    /* pivot at top-left corner */
    gt_img_set_pivot(_img_transform, 0, 0);
    gt_disp_invalid_area(_img_transform);
}

static void _pivot_center_cb(gt_event_st * e) {
    /* restore pivot to image center (default) */
    uint16_t w = gt_img_get_width(_img_transform);
    uint16_t h = gt_img_get_height(_img_transform);
    gt_img_set_pivot(_img_transform, (int16_t)(w / 2), (int16_t)(h / 2));
    gt_disp_invalid_area(_img_transform);
}
#endif /* GT_USE_IMG_TRANSFORM */

/* ---- masking + rotation animation ---- */
#if GT_USE_IMG_TRANSFORM
static void _mask_rot_anim_exec(void * tar, int32_t value) {
    gt_img_set_angle((gt_obj_st *)tar, (uint16_t)value);
}
#endif

/* global functions / API interface -------------------------------------*/

gt_obj_st * gt_examples_img(void)
{
    _screen = gt_obj_create(NULL);
    gt_screen_set_bgcolor(_screen, gt_color_white());

    /* ------------------------------------------------------------------ */
    /* Section 1: basic image switch (original demo)                       */
    /* ------------------------------------------------------------------ */
    image = gt_img_create(_screen);
    gt_img_set_src(image, "./img_hmi_02_common.png");
    gt_obj_set_pos(image, 0, 0);
    gt_obj_add_event_cb(image, _image_one_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    image2 = gt_img_create(_screen);
    gt_img_set_src(image2, "./img_hmi_01_ai.png");
    gt_obj_set_pos(image2, 0, 100);
    gt_obj_add_event_cb(image2, _image_two_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    gt_obj_st * btn_bg = gt_btn_create(_screen);
    gt_btn_set_text(btn_bg, "BG Color");
    gt_obj_set_pos(btn_bg, 0, 200);
    gt_obj_set_size(btn_bg, 80, 28);
    gt_obj_add_event_cb(btn_bg, _bg_color_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    /* ------------------------------------------------------------------ */
    /* Section 2: GT_USE_IMG_TRANSFORM — rotate / zoom / pivot demo       */
    /* ------------------------------------------------------------------ */
#if GT_USE_IMG_TRANSFORM
    gt_obj_st * lbl_tf = gt_label_create(_screen);
    gt_label_set_text(lbl_tf, "Transform: rotate / zoom / pivot");
    gt_obj_set_pos(lbl_tf, 420, 10);
    gt_obj_set_size(lbl_tf, 360, 20);

    _img_transform = gt_img_create(_screen);
    // gt_img_set_src(_img_transform, "./img_hmi_01_ai.png");
    gt_img_set_src(_img_transform, "./50x50.png");
    gt_obj_set_pos(_img_transform, 500, 40);
    _angle_val = 0;
    _zoom_val  = 256;
    gt_img_set_angle(_img_transform, _angle_val);
    gt_img_set_zoom(_img_transform, _zoom_val);
    /* default pivot = image center, set explicitly */
    {
        uint16_t w = gt_img_get_width(_img_transform);
        uint16_t h = gt_img_get_height(_img_transform);
        gt_img_set_pivot(_img_transform, (int16_t)(w / 2), (int16_t)(h / 2));
    }

    /* masking: rounded vignette, radius=40, gap=20 */
    gt_color_masking_st mask = { .radius = 8, .gap = 0 };
    gt_img_set_masking(_img_transform, &mask);


    /* control buttons */
    gt_obj_st * btn_cw = gt_btn_create(_screen);
    gt_btn_set_text(btn_cw, "CW 5");
    gt_obj_set_pos(btn_cw, 420, 220);
    gt_obj_set_size(btn_cw, 70, 28);
    gt_obj_add_event_cb(btn_cw, _rotate_cw_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    gt_obj_st * btn_ccw = gt_btn_create(_screen);
    gt_btn_set_text(btn_ccw, "CCW 5");
    gt_obj_set_pos(btn_ccw, 500, 220);
    gt_obj_set_size(btn_ccw, 70, 28);
    gt_obj_add_event_cb(btn_ccw, _rotate_ccw_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    gt_obj_st * btn_zi = gt_btn_create(_screen);
    gt_btn_set_text(btn_zi, "Zoom+");
    gt_obj_set_pos(btn_zi, 420, 255);
    gt_obj_set_size(btn_zi, 70, 28);
    gt_obj_add_event_cb(btn_zi, _zoom_in_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    gt_obj_st * btn_zo = gt_btn_create(_screen);
    gt_btn_set_text(btn_zo, "Zoom-");
    gt_obj_set_pos(btn_zo, 500, 255);
    gt_obj_set_size(btn_zo, 70, 28);
    gt_obj_add_event_cb(btn_zo, _zoom_out_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    gt_obj_st * btn_pc = gt_btn_create(_screen);
    gt_btn_set_text(btn_pc, "Pivot:Ctr");
    gt_obj_set_pos(btn_pc, 420, 290);
    gt_obj_set_size(btn_pc, 70, 28);
    gt_obj_add_event_cb(btn_pc, _pivot_center_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    gt_obj_st * btn_pk = gt_btn_create(_screen);
    gt_btn_set_text(btn_pk, "Pivot:TL");
    gt_obj_set_pos(btn_pk, 500, 290);
    gt_obj_set_size(btn_pk, 70, 28);
    gt_obj_add_event_cb(btn_pk, _pivot_corner_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    /* ------------------------------------------------------------------ */
    /* Section 3: masking effect + continuous rotation animation           */
    /* ------------------------------------------------------------------ */
    gt_obj_st * lbl_mk = gt_label_create(_screen);
    gt_label_set_text(lbl_mk, "Masking + Rotation Anim");
    gt_obj_set_pos(lbl_mk, 420, 330);
    gt_obj_set_size(lbl_mk, 360, 20);

    _img_mask_rot = gt_img_create(_screen);
    gt_img_set_src(_img_mask_rot, "./img_hmi_02_common.png");
    gt_obj_set_pos(_img_mask_rot, 500, 355);

    gt_img_set_masking(_img_mask_rot, &mask);

    /* pivot at center for smooth spin */
    {
        uint16_t w = gt_img_get_width(_img_mask_rot);
        uint16_t h = gt_img_get_height(_img_mask_rot);
        gt_img_set_pivot(_img_mask_rot, (int16_t)(w / 2), (int16_t)(h / 2));
    }

    /* continuous 360° rotation animation, 3 s per revolution */
    gt_anim_st anim;
    gt_anim_init(&anim);
    gt_anim_set_target(&anim, _img_mask_rot);
    gt_anim_set_exec_cb(&anim, _mask_rot_anim_exec);
    gt_anim_set_value(&anim, 0, 3599);
    gt_anim_set_time(&anim, 3000);
    gt_anim_set_repeat_count(&anim, GT_ANIM_REPEAT_INFINITE);
    gt_anim_start(&anim);

#endif /* GT_USE_IMG_TRANSFORM */

    /* ------------------------------------------------------------------ */
    /* Section 4: border effect demo                                       */
    /* ------------------------------------------------------------------ */
    gt_obj_st * lbl_bd = gt_label_create(_screen);
    gt_label_set_text(lbl_bd, "Border Demo");
    gt_obj_set_pos(lbl_bd, 0, 240);
    gt_obj_set_size(lbl_bd, 200, 20);

    /* plain border, width=3, red */
    gt_obj_st * img_bd1 = gt_img_create(_screen);
    gt_img_set_src(img_bd1, "./img_hmi_01_ai.png");
    gt_obj_set_pos(img_bd1, 0, 262);
    gt_img_set_border_width(img_bd1, 3);
    gt_img_set_border_color(img_bd1, gt_color_hex(0xFF0000));

    /* border with radius (reuse obj->radius) */
    gt_obj_st * img_bd2 = gt_img_create(_screen);
    gt_img_set_src(img_bd2, "./img_hmi_02_common.png");
    gt_obj_set_pos(img_bd2, 100, 262);
    gt_img_set_border_width(img_bd2, 4);
    gt_img_set_border_color(img_bd2, gt_color_hex(0x00AAFF));
    gt_obj_set_radius(img_bd2, 8);

#if GT_USE_IMG_TRANSFORM
    /* border + rotation together */
    gt_obj_st * img_bd3 = gt_img_create(_screen);
    gt_img_set_src(img_bd3, "./50x50.png");
    gt_obj_set_pos(img_bd3, 220, 262);
    gt_img_set_border_width(img_bd3, 2);
    gt_img_set_border_color(img_bd3, gt_color_hex(0x00FF44));
    gt_img_set_angle(img_bd3, 450);  /* 45 degrees */
    gt_img_set_zoom(img_bd3, 256);
#endif /* GT_USE_IMG_TRANSFORM */

    GT_EXAMPLE_ADD_BACK_BTN(_screen);
    return _screen;
}


#endif

/* ======================================================================
 * SVG example
 * ====================================================================== */
#if GT_USE_EXAMPLES

#if GT_USE_SVG
static gt_obj_st * _svg_img1 = NULL;
static gt_obj_st * _svg_img2 = NULL;

static void _svg_switch_cb(gt_event_st * e) {
    static int _cnt = 1;
    gt_img_set_src(_svg_img1, (_cnt++ % 2) ? "./test2.svg" : "./test.svg");
}
#endif  /* GT_USE_SVG */

gt_obj_st * gt_examples_svg(void)
{
    gt_obj_st * scr = gt_obj_create(NULL);
    gt_screen_set_bgcolor(scr, gt_color_hex(0x1E1E2E));

#if GT_USE_SVG
    /* title */
    gt_obj_st * lbl = gt_label_create(scr);
    gt_label_set_text(lbl, "SVG Decoder Test");
    gt_obj_set_pos(lbl, 10, 8);
    gt_obj_set_size(lbl, 300, 22);
    gt_label_set_font_color(lbl, gt_color_white());

    /* SVG image 1 — loaded via user_src[], click to switch */
    _svg_img1 = gt_img_create(scr);
    gt_img_set_src(_svg_img1, "./test.svg");
    gt_obj_set_pos(_svg_img1, 10, 40);

    gt_obj_st * lbl1 = gt_label_create(scr);
    gt_label_set_text(lbl1, "test.svg (click to switch)");
    gt_obj_set_pos(lbl1, 10, 215);
    gt_obj_set_size(lbl1, 260, 20);
    gt_label_set_font_color(lbl1, gt_color_hex(0xAAAAAA));
    gt_obj_add_event_cb(_svg_img1, _svg_switch_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    /* SVG image 2 — loaded via user_src[], static */
    _svg_img2 = gt_img_create(scr);
    gt_img_set_src(_svg_img2, "./test2.svg");
    gt_obj_set_pos(_svg_img2, 200, 40);

    gt_obj_st * lbl2 = gt_label_create(scr);
    gt_label_set_text(lbl2, "test2.svg");
    gt_obj_set_pos(lbl2, 250, 215);
    gt_obj_set_size(lbl2, 200, 20);
    gt_label_set_font_color(lbl2, gt_color_hex(0xAAAAAA));

    /* info label */
    gt_obj_st * lbl_info = gt_label_create(scr);
    gt_label_set_text(lbl_info, "GT_USE_SVG=1  nanosvg SVG Tiny 1.2");
    gt_obj_set_pos(lbl_info, 10, 235);
    gt_obj_set_size(lbl_info, 500, 20);
    gt_label_set_font_color(lbl_info, gt_color_hex(0x888888));

#else
    gt_obj_st * lbl_off = gt_label_create(scr);
    gt_label_set_text(lbl_off, "GT_USE_SVG is disabled.\nEnable it in gt_conf.h to run this demo.");
    gt_obj_set_pos(lbl_off, 20, 60);
    gt_obj_set_size(lbl_off, 500, 60);
    gt_label_set_font_color(lbl_off, gt_color_hex(0xFF6666));
#endif  /* GT_USE_SVG */

    GT_EXAMPLE_ADD_BACK_BTN(scr);
    return scr;
}

#endif  /* GT_USE_EXAMPLES */
/* end ------------------------------------------------------------------*/
