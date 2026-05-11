/**
 * @file gt_theme.h
 * @author Feyoung
 * @brief Global theme / dark-mode system for GT-HMI-Engine.
 *        Provides two built-in themes (light / dark) and a user-defined slot.
 *        Widgets that follow the theme set use_theme=1 (default); widgets whose
 *        color was set manually have use_theme=0 and are skipped on theme switch.
 * @version 0.1
 * @date 2026-05-11
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_THEME_H_
#define _GT_THEME_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../others/gt_color.h"
#include "../others/gt_types.h"
#include "../widgets/gt_obj.h"

/* typedef --------------------------------------------------------------*/

typedef struct gt_theme_s {
    /* background */
    gt_color_t bg_primary;       /* screen / page main background */
    gt_color_t bg_secondary;     /* secondary container background */
    gt_color_t bg_card;          /* widget / card background (btn, listview item …) */

    /* text */
    gt_color_t text_primary;     /* primary text color */
    gt_color_t text_secondary;   /* secondary / hint text */
    gt_color_t text_disabled;    /* disabled state text */

    /* accent (brand / emphasis color) */
    gt_color_t accent;           /* main accent, e.g. button primary color */
    gt_color_t accent_pressed;   /* accent pressed state */
    gt_color_t accent_disabled;  /* accent disabled state */

    /* divider / border */
    gt_color_t divider;
    gt_color_t border;

    /* status colors (usually unchanged between light/dark) */
    gt_color_t success;
    gt_color_t warning;
    gt_color_t error;
} gt_theme_t;

typedef enum {
    GT_THEME_LIGHT  = 0,
    GT_THEME_DARK   = 1,
    GT_THEME_CUSTOM = 2,
} gt_theme_id_et;

/* global functions / API interface -------------------------------------*/

/**
 * @brief Initialize theme system with the built-in light theme.
 *        Must be called inside gt_init().
 */
void gt_theme_init(void);

/**
 * @brief Switch to a built-in theme and immediately repaint the current screen.
 * @param id GT_THEME_LIGHT or GT_THEME_DARK
 */
void gt_theme_switch(gt_theme_id_et id);

/**
 * @brief Register and activate a fully custom theme.
 * @param theme Pointer to caller-owned gt_theme_t (copied internally).
 */
void gt_theme_set_custom(const gt_theme_t * theme);

/**
 * @brief Get the currently active theme.
 * @return Pointer to the active gt_theme_t (never NULL after gt_theme_init).
 */
gt_theme_t * gt_theme_get(void);

/**
 * @brief Get the currently active theme ID.
 */
gt_theme_id_et gt_theme_get_id(void);

/**
 * @brief Walk the object tree rooted at scr and call each widget's
 *        _apply_theme_cb (if use_theme==1), then trigger a redraw.
 *        Called automatically by gt_theme_switch; can also be called manually.
 * @param scr Root object (usually a screen).
 */
void gt_theme_apply(gt_obj_st * scr);

/**
 * @brief Same as gt_theme_apply but does NOT trigger a redraw event.
 *        Use this when the screen has not been displayed yet (e.g. inside
 *        a screen-create function, before gt_disp_stack_load_scr).
 */
void gt_theme_apply_no_redraw(gt_obj_st * scr);

#ifdef __cplusplus
}
#endif
#endif /* _GT_THEME_H_ */
