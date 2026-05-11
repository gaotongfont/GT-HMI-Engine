/**
 * @file gt_theme.c
 * @author Feyoung
 * @brief Global theme / dark-mode implementation.
 * @version 0.1
 * @date 2026-05-11
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_theme.h"
#include "../widgets/gt_obj.h"
#include "../widgets/gt_obj_class.h"
#include "gt_disp.h"
#include "gt_event.h"

/* static variables -----------------------------------------------------*/

/* ---- built-in light theme ---- */
static const gt_theme_t _theme_light = {
    .bg_primary     = GT_COLOR_MAKE(0xFF, 0xFF, 0xFF),
    .bg_secondary   = GT_COLOR_MAKE(0xF5, 0xF5, 0xF5),
    .bg_card        = GT_COLOR_MAKE(0xFF, 0xFF, 0xFF),
    .text_primary   = GT_COLOR_MAKE(0x1A, 0x1A, 0x1A),
    .text_secondary = GT_COLOR_MAKE(0x66, 0x66, 0x66),
    .text_disabled  = GT_COLOR_MAKE(0xBB, 0xBB, 0xBB),
    .accent         = GT_COLOR_MAKE(0x00, 0x80, 0xFF),
    .accent_pressed = GT_COLOR_MAKE(0x00, 0x60, 0xCC),
    .accent_disabled= GT_COLOR_MAKE(0x99, 0xC9, 0xFF),
    .divider        = GT_COLOR_MAKE(0xE0, 0xE0, 0xE0),
    .border         = GT_COLOR_MAKE(0xCC, 0xCC, 0xCC),
    .success        = GT_COLOR_MAKE(0x4C, 0xAF, 0x50),
    .warning        = GT_COLOR_MAKE(0xFF, 0x98, 0x00),
    .error          = GT_COLOR_MAKE(0xF4, 0x43, 0x36),
};

/* ---- built-in dark theme ---- */
static const gt_theme_t _theme_dark = {
    .bg_primary     = GT_COLOR_MAKE(0x1A, 0x1A, 0x1A),
    .bg_secondary   = GT_COLOR_MAKE(0x2C, 0x2C, 0x2C),
    .bg_card        = GT_COLOR_MAKE(0x3A, 0x3A, 0x3A),
    .text_primary   = GT_COLOR_MAKE(0xF0, 0xF0, 0xF0),
    .text_secondary = GT_COLOR_MAKE(0xAA, 0xAA, 0xAA),
    .text_disabled  = GT_COLOR_MAKE(0x55, 0x55, 0x55),
    .accent         = GT_COLOR_MAKE(0x4D, 0xA6, 0xFF),
    .accent_pressed = GT_COLOR_MAKE(0x22, 0x88, 0xEE),
    .accent_disabled= GT_COLOR_MAKE(0x2A, 0x4A, 0x6A),
    .divider        = GT_COLOR_MAKE(0x44, 0x44, 0x44),
    .border         = GT_COLOR_MAKE(0x55, 0x55, 0x55),
    .success        = GT_COLOR_MAKE(0x4C, 0xAF, 0x50),
    .warning        = GT_COLOR_MAKE(0xFF, 0x98, 0x00),
    .error          = GT_COLOR_MAKE(0xF4, 0x43, 0x36),
};

static gt_theme_t     _theme_custom;
static gt_theme_t   * _current_theme = NULL;
static gt_theme_id_et _current_id    = GT_THEME_LIGHT;

/* static functions -----------------------------------------------------*/

static void _apply_obj(gt_obj_st * obj) {
    if (!obj || !obj->use_theme) { return; }
    const gt_obj_class_st * cls = obj->classes;
    if (!obj->parent) {
        /* root screen object: apply bg_primary */
        obj->bgcolor = _current_theme->bg_primary;
    } else if (cls && cls->_apply_theme_cb) {
        cls->_apply_theme_cb(obj);
    }
    for (gt_size_t i = 0; i < obj->cnt_child; ++i) {
        _apply_obj(obj->child[i]);
    }
}

/* global functions / API interface -------------------------------------*/

void gt_theme_init(void) {
    _current_theme = (gt_theme_t *)&_theme_light;
    _current_id    = GT_THEME_LIGHT;
}

gt_theme_t * gt_theme_get(void) {
    return _current_theme;
}

gt_theme_id_et gt_theme_get_id(void) {
    return _current_id;
}

void gt_theme_set_custom(const gt_theme_t * theme) {
    if (!theme) { return; }
    _theme_custom  = *theme;
    _current_theme = &_theme_custom;
    _current_id    = GT_THEME_CUSTOM;
}

void gt_theme_apply(gt_obj_st * scr) {
    _apply_obj(scr);
    gt_event_send(scr, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_theme_apply_no_redraw(gt_obj_st * scr) {
    _apply_obj(scr);
}

void gt_theme_switch(gt_theme_id_et id) {
    switch (id) {
        case GT_THEME_LIGHT: _current_theme = (gt_theme_t *)&_theme_light; break;
        case GT_THEME_DARK:  _current_theme = (gt_theme_t *)&_theme_dark;  break;
        default: return;
    }
    _current_id = id;
    gt_obj_st * scr = gt_disp_get_scr();
    if (scr) { gt_theme_apply(scr); }
}

/* end ------------------------------------------------------------------*/
