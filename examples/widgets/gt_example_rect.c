/**
 * @file gt_example_rect.c
 * @author Feyoung
 * @brief Rectangle widget example
 * @version 0.1
 * @date 2026-03-27
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_rect.h"
#include "../../src/core/gt_gradient.h"

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/

/* global functions / API interface -------------------------------------*/

/**
 * @brief Create rectangles with different styles.
 */
gt_obj_st * gt_examples_rect(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);
    // gt_screen_set_bgcolor(screen, gt_color_black());
    // gt_screen_set_bgcolor(screen, gt_color_hex(0x00fffff));

    gt_obj_st * rect1 = gt_rect_create(screen);
    gt_obj_set_pos(rect1, 10, 10);
    gt_obj_set_size(rect1, 100, 80);
    gt_rect_set_bg_color(rect1, gt_color_hex(0x2196F3));
    gt_rect_set_fill(rect1, 1);
    gt_rect_set_radius(rect1, 8);

    gt_obj_st * rect2 = gt_rect_create(screen);
    gt_obj_set_pos(rect2, 130, 10);
    gt_obj_set_size(rect2, 100, 80);
    gt_rect_set_bg_color(rect2, gt_color_white());
    gt_rect_set_color_border(rect2, gt_color_hex(0xFF5722));
    gt_rect_set_border(rect2, 2);
    gt_rect_set_fill(rect2, 0);
    gt_rect_set_radius(rect2, 0);

    gt_obj_st * rect3 = gt_rect_create(screen);
    gt_obj_set_pos(rect3, 250, 10);
    gt_obj_set_size(rect3, 100, 80);
    gt_rect_set_bg_color(rect3, gt_color_hex(0x4CAF50));
    gt_rect_set_fill(rect3, 1);
    gt_rect_set_radius(rect3, 50);

#if GT_USE_GRADIENT
    /* Horizontal gradient: Blue → Red */
    gt_obj_st * rg1 = gt_rect_create(screen);
    gt_obj_set_pos(rg1, 10, 110);
    gt_obj_set_size(rg1, 200, 60);
    gt_rect_set_fill(rg1, 1);
    {
        gt_gradient_dsc_st * g = gt_gradient_dsc_create(GT_GRAD_DIR_HOR);
        gt_gradient_dsc_add_stop(g, gt_color_hex(0x0000FF), gt_color_hex(0xFF0000), 60);
        gt_rect_set_gradient(rg1, g);
    }

    /* Vertical gradient: Red → Yellow → Green */
    gt_obj_st * rg2 = gt_rect_create(screen);
    gt_obj_set_pos(rg2, 230, 110);
    gt_obj_set_size(rg2, 200, 60);
    gt_rect_set_fill(rg2, 1);
    {
        gt_gradient_dsc_st * g = gt_gradient_dsc_create(GT_GRAD_DIR_VER);
        gt_gradient_dsc_add_stop(g, gt_color_hex(0xFF0000), gt_color_hex(0xFFFF00), 30);
        gt_gradient_dsc_add_stop(g, gt_color_hex(0xFFFF00), gt_color_hex(0x00FF00), 30);
        gt_rect_set_gradient(rg2, g);
    }

    /* SYM_VER: White → Black (center → top & bottom) */
    gt_obj_st * rg3 = gt_rect_create(screen);
    gt_obj_set_pos(rg3, 450, 110);
    gt_obj_set_size(rg3, 200, 60);
    gt_rect_set_fill(rg3, 1);
    {
        gt_gradient_dsc_st * g = gt_gradient_dsc_create(GT_GRAD_DIR_SYM_VER);
        gt_gradient_dsc_add_stop(g, gt_color_hex(0xFFFFFF), gt_color_hex(0x000000), 60);
        gt_rect_set_gradient(rg3, g);
    }

    /* Rainbow horizontal */
    gt_obj_st * rg4 = gt_rect_create(screen);
    gt_obj_set_pos(rg4, 10, 190);
    gt_obj_set_size(rg4, 430, 60);
    gt_rect_set_fill(rg4, 1);
    gt_rect_set_gradient(rg4, gt_gradient_dsc_rainbow(GT_GRAD_DIR_HOR, 30));

    /* SYM_HOR: White → Black (center → left & right) */
    gt_obj_st * rg5 = gt_rect_create(screen);
    gt_obj_set_pos(rg5, 450, 190);
    gt_obj_set_size(rg5, 200, 60);
    gt_rect_set_fill(rg5, 1);
    {
        gt_gradient_dsc_st * g = gt_gradient_dsc_create(GT_GRAD_DIR_SYM_HOR);
        gt_gradient_dsc_add_stop(g, gt_color_hex(0xFFFFFF), gt_color_hex(0x000000), 60);
        gt_rect_set_gradient(rg5, g);
    }
#endif /* GT_USE_GRADIENT */

#if GT_USE_SHADOW
    /* BOTTOM_RIGHT: 经典右下 3D 按钮阴影 */
    gt_obj_st * rs1 = gt_rect_create(screen);
    gt_obj_set_pos(rs1, 10, 270);
    gt_obj_set_size(rs1, 130, 50);
    gt_rect_set_fill(rs1, 1);
    gt_rect_set_radius(rs1, 6);
    gt_rect_set_bg_color(rs1, gt_color_hex(0x4488FF));
    gt_rect_set_shadow(rs1, gt_shadow_dsc_create(GT_SHADOW_STYLE_BOTTOM_RIGHT));

    /* BOTTOM: 卡片底部阴影 */
    gt_obj_st * rs2 = gt_rect_create(screen);
    gt_obj_set_pos(rs2, 160, 270);
    gt_obj_set_size(rs2, 130, 50);
    gt_rect_set_fill(rs2, 1);
    gt_rect_set_radius(rs2, 6);
    gt_rect_set_bg_color(rs2, gt_color_hex(0x44BB88));
    gt_rect_set_shadow(rs2, gt_shadow_dsc_create(GT_SHADOW_STYLE_BOTTOM));

    /* AROUND: 四周发光 / 浮层 */
    gt_obj_st * rs3 = gt_rect_create(screen);
    gt_obj_set_pos(rs3, 310, 270);
    gt_obj_set_size(rs3, 130, 50);
    gt_rect_set_fill(rs3, 1);
    gt_rect_set_radius(rs3, 6);
    gt_rect_set_bg_color(rs3, gt_color_hex(0xFFAA00));
    gt_rect_set_shadow(rs3, gt_shadow_dsc_create(GT_SHADOW_STYLE_AROUND));

    /* DEEP: 深度感强 */
    gt_obj_st * rs4 = gt_rect_create(screen);
    gt_obj_set_pos(rs4, 460, 270);
    gt_obj_set_size(rs4, 130, 50);
    gt_rect_set_fill(rs4, 1);
    gt_rect_set_radius(rs4, 6);
    gt_rect_set_bg_color(rs4, gt_color_hex(0xEE4444));
    gt_rect_set_shadow(rs4, gt_shadow_dsc_create(GT_SHADOW_STYLE_DEEP));

    /* SUBTLE: 微弱扁平阴影 */
    gt_obj_st * rs5 = gt_rect_create(screen);
    gt_obj_set_pos(rs5, 610, 270);
    gt_obj_set_size(rs5, 130, 50);
    gt_rect_set_fill(rs5, 1);
    gt_rect_set_radius(rs5, 6);
    gt_rect_set_bg_color(rs5, gt_color_hex(0xAA66CC));
    gt_rect_set_shadow(rs5, gt_shadow_dsc_create(GT_SHADOW_STYLE_SUBTLE));

    /* CUSTOM: 自定义参数示例（渐变 + 自定义阴影色） */
    gt_obj_st * rs6 = gt_rect_create(screen);
    gt_obj_set_pos(rs6, 10, 360);
    gt_obj_set_size(rs6, 200, 60);
    gt_rect_set_fill(rs6, 1);
    gt_rect_set_radius(rs6, 8);
    {
        gt_gradient_dsc_st * g = gt_gradient_dsc_create(GT_GRAD_DIR_HOR);
        gt_gradient_dsc_add_stop(g, gt_color_hex(0xFF6600), gt_color_hex(0xFFCC00), 60);
        gt_rect_set_gradient(rs6, g);
        gt_shadow_dsc_st * sh = gt_shadow_dsc_create(GT_SHADOW_STYLE_CUSTOM);
        sh->offset_x = 8;
        sh->offset_y = 8;
        sh->spread   = 4;
        sh->color    = gt_color_hex(0xFF6600);
        sh->opa      = GT_OPA_40;
        gt_rect_set_shadow(rs6, sh);
    }
#endif /* GT_USE_SHADOW */

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
