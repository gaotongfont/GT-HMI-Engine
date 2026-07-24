/**
 * @file gt_example_vector.c
 * @author Feyoung
 * @brief 矢量字体独立测试：大字号 gray=1 平滑对比、gray 深度、粗细控制
 * @version 0.1
 * @date 2026-07-24
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_label.h"

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/* static functions -----------------------------------------------------*/

/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_examples_vector(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);

#if GT_VEC_GRAY1_SMOOTH
    uint16_t y = 0;

    /* ================================================================ */
    /* Section A: 多灰度逐行对比 (48~128pt, gray=1/2/4)                 */
    /*   gray=4 仅 ≤64pt 可用; >64pt 仅 gray=1/2                       */
    /* ================================================================ */

    /* A0. 48pt: gray=1 / gray=4 */
    {
        gt_obj_st * lbl = gt_label_create(screen);
        gt_obj_set_pos(lbl, 0, y);
        gt_obj_set_size(lbl, 780, 56);
        gt_label_set_font_family(lbl, family_vec);
        gt_label_set_font_size(lbl, 48);
        gt_label_set_font_gray(lbl, 1);
        gt_label_set_text(lbl, "vec 48pt g=1 中文ABC");
        y += 66;
    }
    {
        gt_obj_st * lbl = gt_label_create(screen);
        gt_obj_set_pos(lbl, 0, y);
        gt_obj_set_size(lbl, 780, 56);
        gt_label_set_font_family(lbl, family_vec);
        gt_label_set_font_size(lbl, 48);
        gt_label_set_font_gray(lbl, 4);
        gt_label_set_text(lbl, "vec 48pt g=4 中文ABC");
        y += 66;
    }

    /* A1. 64pt: gray=1 / gray=2 / gray=4 */
    {
        gt_obj_st * lbl = gt_label_create(screen);
        gt_obj_set_pos(lbl, 0, y);
        gt_obj_set_size(lbl, 780, 72);
        gt_label_set_font_family(lbl, family_vec);
        gt_label_set_font_size(lbl, 64);
        gt_label_set_font_gray(lbl, 1);
        gt_label_set_text(lbl, "vec 64pt g=1 中文ABC");
        y += 82;
    }
    {
        gt_obj_st * lbl = gt_label_create(screen);
        gt_obj_set_pos(lbl, 0, y);
        gt_obj_set_size(lbl, 780, 72);
        gt_label_set_font_family(lbl, family_vec);
        gt_label_set_font_size(lbl, 64);
        gt_label_set_font_gray(lbl, 2);
        gt_label_set_text(lbl, "vec 64pt g=2 中文ABC");
        y += 82;
    }
    {
        gt_obj_st * lbl = gt_label_create(screen);
        gt_obj_set_pos(lbl, 0, y);
        gt_obj_set_size(lbl, 780, 72);
        gt_label_set_font_family(lbl, family_vec);
        gt_label_set_font_size(lbl, 64);
        gt_label_set_font_gray(lbl, 4);
        gt_label_set_text(lbl, "vec 64pt g=4 中文ABC");
        y += 82;
    }

    /* A2. 80pt: gray=1 / gray=2 */
    {
        gt_obj_st * lbl = gt_label_create(screen);
        gt_obj_set_pos(lbl, 0, y);
        gt_obj_set_size(lbl, 780, 90);
        gt_label_set_font_family(lbl, family_vec);
        gt_label_set_font_size(lbl, 80);
        gt_label_set_font_gray(lbl, 1);
        gt_label_set_text(lbl, "vec 80pt g=1 中文ABC");
        y += 100;
    }
    {
        gt_obj_st * lbl = gt_label_create(screen);
        gt_obj_set_pos(lbl, 0, y);
        gt_obj_set_size(lbl, 780, 90);
        gt_label_set_font_family(lbl, family_vec);
        gt_label_set_font_size(lbl, 80);
        gt_label_set_font_gray(lbl, 2);
        gt_label_set_text(lbl, "vec 80pt g=2 中文ABC");
        y += 100;
    }

    /* A3. 96pt: gray=1 / gray=2 */
    {
        gt_obj_st * lbl = gt_label_create(screen);
        gt_obj_set_pos(lbl, 0, y);
        gt_obj_set_size(lbl, 780, 106);
        gt_label_set_font_family(lbl, family_vec);
        gt_label_set_font_size(lbl, 96);
        gt_label_set_font_gray(lbl, 1);
        gt_label_set_text(lbl, "vec 96pt g=1 中文ABC");
        y += 116;
    }
    {
        gt_obj_st * lbl = gt_label_create(screen);
        gt_obj_set_pos(lbl, 0, y);
        gt_obj_set_size(lbl, 780, 106);
        gt_label_set_font_family(lbl, family_vec);
        gt_label_set_font_size(lbl, 96);
        gt_label_set_font_gray(lbl, 2);
        gt_label_set_text(lbl, "vec 96pt g=2 中文ABC");
        y += 116;
    }

    /* A4. 128pt: gray=1 / gray=2 */
    {
        gt_obj_st * lbl = gt_label_create(screen);
        gt_obj_set_pos(lbl, 0, y);
        gt_obj_set_size(lbl, 780, 140);
        gt_label_set_font_family(lbl, family_vec);
        gt_label_set_font_size(lbl, 128);
        gt_label_set_font_gray(lbl, 1);
        gt_label_set_text(lbl, "vec 128pt g=1 中文ABC");
        y += 150;
    }
    {
        gt_obj_st * lbl = gt_label_create(screen);
        gt_obj_set_pos(lbl, 0, y);
        gt_obj_set_size(lbl, 780, 140);
        gt_label_set_font_family(lbl, family_vec);
        gt_label_set_font_size(lbl, 128);
        gt_label_set_font_gray(lbl, 2);
        gt_label_set_text(lbl, "vec 128pt g=2 中文ABC");
        y += 150;
    }

    /* ================================================================ */
    /* Section C: thick 粗细控制 (64pt gray=1)                          */
    /* ================================================================ */

    {
        const uint16_t thick_vals[] = {16, 24, 32, 48};
        const uint16_t lbl_w = 190;
        for (int i = 0; i < 4; i++) {
            gt_obj_st * lbl = gt_label_create(screen);
            gt_obj_set_pos(lbl, (gt_size_t)(i * lbl_w), y);
            gt_obj_set_size(lbl, lbl_w, 72);
            gt_label_set_font_family(lbl, family_vec);
            gt_label_set_font_size(lbl, 64);
            gt_label_set_font_gray(lbl, 1);
            gt_label_set_font_thick_en(lbl, thick_vals[i]);
            gt_label_set_font_thick_cn(lbl, thick_vals[i]);
            char buf[32];
            snprintf(buf, sizeof(buf), "vec 64pt th=%u 中文ABC", thick_vals[i]);
            gt_label_set_text(lbl, buf);
        }
        y += 82;
    }

    /* ================================================================ */
    /* Section D: scale_size 组合 (64pt gray=1 -> scale 48pt)           */
    /* ================================================================ */

    {
        gt_obj_st * lbl = gt_label_create(screen);
        gt_obj_set_pos(lbl, 0, y);
        gt_obj_set_size(lbl, 780, 56);
        gt_label_set_font_family(lbl, family_vec);
        gt_label_set_font_size(lbl, 64);
        gt_label_set_font_gray(lbl, 1);
        gt_label_set_font_scale_size(lbl, 48);
        gt_label_set_text(lbl, "vec 64->48 g=1 smooth+scale 中文ABC");
        y += 66;
    }

    /* ================================================================ */
    /* Section E: 超大字号 gray=1 平滑测试 (144pt ~ 256pt)              */
    /* ================================================================ */

    /* E1. 144pt gray=1 */
    {
        gt_obj_st * lbl = gt_label_create(screen);
        gt_obj_set_pos(lbl, 0, y);
        gt_obj_set_size(lbl, 780, 156);
        gt_label_set_font_family(lbl, family_vec);
        gt_label_set_font_size(lbl, 144);
        gt_label_set_font_gray(lbl, 1);
        gt_label_set_text(lbl, "vec 144pt g=1 中文ABC");
        y += 166;
    }

    /* E2. 192pt gray=1 */
    {
        gt_obj_st * lbl = gt_label_create(screen);
        gt_obj_set_pos(lbl, 0, y);
        gt_obj_set_size(lbl, 780, 206);
        gt_label_set_font_family(lbl, family_vec);
        gt_label_set_font_size(lbl, 192);
        gt_label_set_font_gray(lbl, 1);
        gt_label_set_text(lbl, "vec 192pt g=1 中文ABC");
        y += 216;
    }

    /* E3. 256pt gray=1 */
    {
        gt_obj_st * lbl = gt_label_create(screen);
        gt_obj_set_pos(lbl, 0, y);
        gt_obj_set_size(lbl, 780, 270);
        gt_label_set_font_family(lbl, family_vec);
        gt_label_set_font_size(lbl, 256);
        gt_label_set_font_gray(lbl, 1);
        gt_label_set_text(lbl, "vec 256pt g=1 中文ABC");
        y += 280;
    }

#else
    /* GT_VEC_GRAY1_SMOOTH disabled: show raw 1-bit rendering */
    {
        gt_obj_st * lbl = gt_label_create(screen);
        gt_obj_set_pos(lbl, 10, 10);
        gt_obj_set_size(lbl, 760, 40);
        gt_label_set_font_family(lbl, family_vec);
        gt_label_set_font_size(lbl, 24);
        gt_label_set_font_gray(lbl, 1);
        gt_label_set_text(lbl, "GT_VEC_GRAY1_SMOOTH=0: raw 1-bit (no smoothing)");
    }
#endif

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
