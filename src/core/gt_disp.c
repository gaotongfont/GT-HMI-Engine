/**
 * @file gt_disp.c
 * @author yongg
 * @brief Display implementation for the screen.
 * @version 0.1
 * @date 2022-05-11 14:59:50
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

 /* include --------------------------------------------------------------*/
#include "../gt_conf.h"
#include "../widgets/gt_obj.h"
#include "../others/gt_log.h"
#include "gt_obj_pos.h"
#include "../hal/gt_hal_disp.h"
#include "gt_disp.h"
#include "../extra/draw/gt_draw_blend.h"
#include "./gt_draw.h"
#include "./gt_refr.h"
#include "./gt_obj_pos.h"
#include "../others/gt_area.h"
#include "../others/gt_anim.h"
#include "./gt_mem.h"
#include "./gt_handler.h"
#include "./gt_indev.h"
#include "./gt_layout.h"
#include "../others/gt_gc.h"

#include "./effects/gt_scroll_tip.h"
#include "./gt_obj_scroll.h"
#include "./gt_style.h"

/* private define -------------------------------------------------------*/
#ifndef _GT_DISP_ATTRIBUTE_SRAM_TEXT
    #define _GT_DISP_ATTRIBUTE_SRAM_TEXT        GT_ATTRIBUTE_SRAM_TEXT
#endif

#ifndef _GT_STACK_USE_LOADING_LOG
    /**
     * @brief 0[default]: Close the page stack log
     */
    #define _GT_STACK_USE_LOADING_LOG           0
#endif

#ifndef _GT_STACK_USE_LOADING_LOG_DETAIL
    /**
     * @brief 0[default]
     */
    #define _GT_STACK_USE_LOADING_LOG_DETAIL    0
#endif

#ifndef _GT_STACK_PUSH_BEFORE_INIT_SCR
    /**
     * @brief 0[default]: The screen object is initialized before the stack is pushed
     */
    #define _GT_STACK_PUSH_BEFORE_INIT_SCR      0
#endif

#ifndef _GT_DISP_USE_MALLOC_DRAW_CTX
    /**
     * @brief 0[default]: The drawing context is not allocated on the heap, by stack
     */
    #define _GT_DISP_USE_MALLOC_DRAW_CTX        0
#endif

#ifndef _GT_DISP_THREAD_SAFE_TIMEOUT
    /**
     * @brief [ms]
     */
    #define _GT_DISP_THREAD_SAFE_TIMEOUT        1000
#endif

/* private typedef ------------------------------------------------------*/

typedef enum{
    GT_NOT_BUSY,
    GT_BUSY,
}gt_busy_et;

typedef struct {
    /**
     * @brief Top layer full cover screen  area.
     *      0[default]: not full cover screen
     */
    uint8_t full_cover : 1;
    /**
     * @brief Top layer transparent.
     *      0: No transparent(opa cover), 1[default]: Transparent
     */
    uint8_t is_opa : 1;
}_gt_top_ret_st;

/**
 * @brief flush screen by animation or direct param
 */
typedef struct _flush_scr_param_s {
    gt_disp_st * disp;
    gt_area_st area_flush;         // The buffer or area of display flush area.
#if GT_USE_SCREEN_ANIM
    gt_area_st view_scr_abs;       // The new interface displays the absolute extent of the area on the physical screen
    gt_area_st view_scr_prev_abs;  // The previous interface displays the absolute extent within the area on the physical screen
#endif
    _gt_draw_valid_st valid;       // Interface intersection results and screen display start offset
    uint16_t line;                 // Number of rows per refresh
}_flush_scr_param_st;

typedef struct _bg_abs_area_st {
    gt_area_st area;
    gt_color_t color;
}_bg_abs_area_st;

/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

#if GT_DISP_SW_ROTATE_SUPPORT
/**
 * @brief Transpose pixels from logical area into physical full-screen buffer.
 *        Supports GT_ROTATED_0/90/180/270.
 */
static GT_ATTRIBUTE_RAM_TEXT void
_gt_disp_sw_rotate(gt_disp_st * disp, const gt_area_st * area,
                   const gt_color_t * src, gt_color_t * dst)
{
    const uint16_t phy_w = disp->drv->res_hor;
    const uint16_t phy_h = disp->drv->res_ver;
    const int32_t xe = area->x + area->w;
    const int32_t ye = area->y + area->h;
    uint32_t idx = 0;
    int32_t x, y;

    switch ((gt_rotated_et)disp->drv->rotated) {
        case GT_ROTATED_90:
            /* logical (lx,ly) -> physical (phy_w-1-ly, lx) */
            for (y = area->y; y < ye; y++) {
                int32_t px_base = (phy_w - 1) - y;
                for (x = area->x; x < xe; x++) {
                    dst[(uint32_t)x * phy_w + px_base] = src[idx++];
                }
            }
            break;
        case GT_ROTATED_180:
            for (y = area->y; y < ye; y++) {
                int32_t py = (phy_h - 1) - y;
                for (x = area->x; x < xe; x++) {
                    dst[(uint32_t)py * phy_w + (phy_w - 1 - x)] = src[idx++];
                }
            }
            break;
        case GT_ROTATED_270:
            /* logical (lx,ly) -> physical (ly, phy_h-1-lx) */
            for (y = area->y; y < ye; y++) {
                for (x = area->x; x < xe; x++) {
                    dst[(uint32_t)(phy_h - 1 - x) * phy_w + y] = src[idx++];
                }
            }
            break;
        default: /* GT_ROTATED_0 */
            for (y = area->y; y < ye; y++) {
                for (x = area->x; x < xe; x++) {
                    dst[(uint32_t)y * phy_w + x] = src[idx++];
                }
            }
            break;
    }
}
#endif /* GT_DISP_SW_ROTATE_SUPPORT */

/**
 * @brief Unified flush: optionally transpose pixels then call flush_cb.
 *        When sw_rotate is enabled, only accumulates into sw_rotate_buf;
 *        caller must invoke _gt_disp_flush_commit() after the tile loop.
 */
static GT_ATTRIBUTE_RAM_TEXT inline void
_gt_disp_flush_one(gt_disp_st * disp, gt_area_st * area, gt_color_t * src)
{
#if GT_DISP_SW_ROTATE_SUPPORT
    if (disp->drv->sw_rotate && disp->sw_rotate_buf) {
        /* Accumulate tile into the physical-layout buffer; flush deferred. */
        _gt_disp_sw_rotate(disp, area, src, disp->sw_rotate_buf);
        return;
    }
#endif
    gt_disp_drv_set_flushing(disp->drv, true);
    disp->drv->flush_cb(disp->drv, area, src);
}

#if GT_DISP_SW_ROTATE_SUPPORT
/**
 * @brief Flush the fully-composed sw_rotate_buf to the display driver.
 *        Call once after the tile loop completes a full logical frame.
 */
static GT_ATTRIBUTE_RAM_TEXT inline void
_gt_disp_flush_commit(gt_disp_st * disp)
{
    if (!disp->drv->sw_rotate || !disp->sw_rotate_buf) {
        return;
    }
    gt_area_st phy_area = {
        .x = 0, .y = 0,
        .w = disp->drv->res_hor,
        .h = disp->drv->res_ver
    };
    gt_disp_drv_set_flushing(disp->drv, true);
    disp->drv->flush_cb(disp->drv, &phy_area, disp->sw_rotate_buf);
}
#endif /* GT_DISP_SW_ROTATE_SUPPORT */


/**
 * @brief Is display busy
 *
 * @param disp
 * @return true busy now
 * @return false
 */
static GT_ATTRIBUTE_RAM_TEXT bool _gt_disp_get_state(gt_disp_st * disp) {
    GT_CHECK_BACK_VAL(disp, GT_BUSY);
    GT_CHECK_BACK_VAL(disp->drv, GT_BUSY);
    return disp->drv->busy;
}

static GT_ATTRIBUTE_RAM_TEXT void _gt_disp_set_state(gt_disp_st * disp, uint8_t state) {
    GT_CHECK_BACK(disp);
    GT_CHECK_BACK(disp->drv);
    disp->drv->busy = state;
}

static inline void _gt_scroll_tip_draw_inline(gt_obj_st * obj) {
    gt_scroll_tip_draw(obj);
}

/**
 * @brief Checked the obj is display area visible
 *
 * @param obj The object to be checked and copied
 * @param param
 * @param area_parent All parent displayable areas
 * @return true: visible or area need to refresh, false: invisible or area not need to refresh
 */
static inline void GT_ATTRIBUTE_RAM_TEXT
_build_and_call_draw_ctx(gt_obj_st * obj, _flush_scr_param_st * param, gt_area_st * area_parent,
                         void (*draw_fn)(gt_obj_st *)) {
#if _GT_DISP_USE_MALLOC_DRAW_CTX
    obj->draw_ctx = gt_mem_malloc(sizeof(struct _gt_draw_ctx_s));
    GT_CHECK_BACK(obj->draw_ctx);
    obj->draw_ctx->valid       = param->disp->scr_prev ? &param->valid : NULL;
    obj->draw_ctx->parent_area = obj->inside ? area_parent : NULL;
    obj->draw_ctx->buf         = param->disp->vbd_color;
    obj->draw_ctx->buf_area    = param->disp->area_disp;
    draw_fn(obj);
    gt_mem_free(obj->draw_ctx);
#else
    struct _gt_draw_ctx_s tmp_draw_ctx = {
        .valid       = param->disp->scr_prev ? &param->valid : NULL,
        .parent_area = obj->inside ? area_parent : NULL,
        .buf         = param->disp->vbd_color,
        .buf_area    = param->disp->area_disp,
    };
    obj->draw_ctx = &tmp_draw_ctx;
    draw_fn(obj);
#endif
    obj->draw_ctx = NULL;
}

static inline bool GT_ATTRIBUTE_RAM_TEXT
gt_check_obj_visible_and_copy(gt_obj_st * obj, _flush_scr_param_st * param, gt_area_st area_parent) {
    if (GT_INVISIBLE == gt_obj_get_visible(obj)) {
        return false;
    }
    if (NULL == obj->classes->_init_cb) {
        return true;
    }
    if (false == gt_area_is_intersect_screen(&param->disp->area_disp, &obj->area)) {
        return false;
    }
    _build_and_call_draw_ctx(obj, param, &area_parent, obj->classes->_init_cb);
    return true;
}

static inline void _GT_DISP_ATTRIBUTE_SRAM_TEXT
gt_check_obj_visible_and_copy_after_scroll_tip(gt_obj_st * obj, _flush_scr_param_st * param, gt_area_st area_parent) {
    if (GT_SCROLL_TIP_NONE == obj->scroll_tip) {
        return;
    }
    if (GT_INVISIBLE == gt_obj_get_visible(obj)) {
        return;
    }
    if (false == gt_area_is_intersect_screen(&param->disp->area_disp, &obj->area)) {
        return;
    }
    _build_and_call_draw_ctx(obj, param, &area_parent, _gt_scroll_tip_draw_inline);
}

/**
 * @brief
 *
 * @param obj The object to be checked and copied
 * @param param
 * @param area_parent All parent displayable areas
 */
/* forward declaration for mutual recursion between _draw_children and _check_and_copy_foreach */
static void GT_ATTRIBUTE_RAM_TEXT
_check_and_copy_foreach(gt_obj_st * obj, _flush_scr_param_st * param, gt_area_st area_parent);

static inline void GT_ATTRIBUTE_RAM_TEXT
_draw_children(gt_obj_st * obj, _flush_scr_param_st * param, gt_area_st area_clip) {
    uint16_t idx = 0;
    gt_obj_st * child_p = NULL;
    while (idx < obj->cnt_child) {
        child_p = obj->child[idx++];
        /* Floating direct children of a screen are drawn as its local top layer. */
        if (GT_TYPE_SCREEN == obj->classes->type && child_p->viewport_fixed) {
            continue;
        }
        if (false == gt_check_obj_visible_and_copy(child_p, param, area_clip)) {
            continue;
        }
        if (child_p->cnt_child) {
            _check_and_copy_foreach(child_p, param, area_clip);
        }
    }
    gt_check_obj_visible_and_copy_after_scroll_tip(obj, param, area_clip);

    if (GT_TYPE_SCREEN != obj->classes->type) {
        return;
    }
    for (idx = 0; idx < obj->cnt_child; ++idx) {
        child_p = obj->child[idx];
        if (!child_p->viewport_fixed) {
            continue;
        }
        if (false == gt_check_obj_visible_and_copy(child_p, param, area_clip)) {
            continue;
        }
        if (child_p->cnt_child) {
            _check_and_copy_foreach(child_p, param, area_clip);
        }
    }
}

static void GT_ATTRIBUTE_RAM_TEXT
_check_and_copy_foreach(gt_obj_st * obj, _flush_scr_param_st * param, gt_area_st area_parent) {
    gt_area_st area_cross = area_parent;

    if (false == gt_area_cover_screen(&area_parent, &obj->area, &area_cross)) {
        if (GT_TYPE_GROUP != obj->classes->type) {
            /** The Group ignores the area effects */
            return ;
        }
    }
    _draw_children(obj, param, area_cross);
}

static void _GT_DISP_ATTRIBUTE_SRAM_TEXT
_gt_disp_check_and_copy_foreach(gt_obj_st * scr_or_layer, _flush_scr_param_st * param) {
    if (GT_INVISIBLE == gt_obj_get_visible(scr_or_layer)) {
        return ;
    }
    _draw_children(scr_or_layer, param, param->disp->area_disp);
}

#if GT_USE_LAYER_TOP
/**
 * @brief if top widget full cover display, layer base will ignore
 *      draw by this function.
 *
 * @param layer_top
 * @returns _gt_top_ret_st
 *      full_cover true: ignore draw base layer; false: both need to draw.
 *      is_opa true: need to screenshot base screen area; false: do not draw base layer.
 */
static GT_ATTRIBUTE_RAM_TEXT _gt_top_ret_st
_gt_disp_is_top_full_cover_display(gt_obj_st * layer_top) {
    uint16_t idx = 0;
    gt_obj_st * child_p = NULL;
    _gt_top_ret_st ret = {
        .full_cover = false,
        .is_opa = true,
    };
    if (GT_INVISIBLE == gt_obj_get_visible(layer_top)) {
        return ret;
    }
    while (idx < layer_top->cnt_child) {
        child_p = layer_top->child[idx++];
        if (GT_INVISIBLE == gt_obj_get_visible(child_p)) {
            continue;
        }
        if (NULL == child_p->classes->_init_cb) {
            continue;
        }
        if (gt_area_is_equal(&layer_top->area, &child_p->area)) {
            ret.full_cover = true;
            if (child_p->opa < GT_OPA_COVER) {
                continue;
            }
            ret.is_opa = false;
            return ret;
        }
    }
    return ret;
}
#endif

static GT_ATTRIBUTE_RAM_TEXT void _scr_anim_exec_x_cb(void * obj, int32_t x) {
    gt_obj_st * scr = (gt_obj_st * )obj;
    gt_area_st area = scr->area;
    area.x = x;
    gt_obj_set_area(scr, area);
}

static GT_ATTRIBUTE_RAM_TEXT void _scr_anim_exec_y_cb(void * obj, int32_t y) {
    gt_obj_st * scr = (gt_obj_st * )obj;
    gt_area_st area = scr->area;
    area.y = y;
    gt_obj_set_area(scr, area);
}

/** UNCOVER: screen + children move together as a whole */
static GT_ATTRIBUTE_RAM_TEXT void _scr_anim_uncover_exec_x_cb(void * obj, int32_t x) {
    gt_obj_st * scr = (gt_obj_st * )obj;
    _gt_obj_move_child_by(scr, (gt_size_t)(x - scr->area.x), 0);
    gt_area_st area = scr->area;
    area.x = (gt_size_t)x;
    gt_obj_set_area(scr, area);
}

static GT_ATTRIBUTE_RAM_TEXT void _scr_anim_uncover_exec_y_cb(void * obj, int32_t y) {
    gt_obj_st * scr = (gt_obj_st * )obj;
    _gt_obj_move_child_by(scr, 0, (gt_size_t)(y - scr->area.y));
    gt_area_st area = scr->area;
    area.y = (gt_size_t)y;
    gt_obj_set_area(scr, area);
}

static GT_ATTRIBUTE_RAM_TEXT void _scr_anim_del_ready_cb(struct gt_anim_s * anim) {
    gt_obj_st * old_scr = (gt_obj_st * )anim->tar;
    old_scr->using_sta = 0;
    _gt_obj_class_destroy(old_scr);
}

#if GT_USE_SCREEN_ANIM
static GT_ATTRIBUTE_RAM_TEXT void _scr_anim_start_cb(struct gt_anim_s * anim) {
    gt_disp_st * disp = gt_disp_get_default();
    disp->scr_prev         = disp->scr_act;
    disp->scr_act          = (gt_obj_st * )anim->tar;

    /** Remark the begin status prev and scr position */
    disp->anim_scr_remark.x = disp->scr_act->area.x;
    disp->anim_scr_remark.y = disp->scr_act->area.y;

    gt_event_send(disp->scr_act, GT_EVENT_TYPE_SCREEN_LOAD_BEFORE, NULL);

    /** Can not calling other event by all of input device */
    gt_indev_set_enabled(false);
    gt_event_set_enabled(false);
}

/**
 * @brief Reset old screen display position
 *
 * @param anim
 */
static GT_ATTRIBUTE_RAM_TEXT void _old_scr_anim_ready_cb(struct gt_anim_s * anim) {
    gt_obj_st * old_scr = (gt_obj_st * )anim->tar;
    if (old_scr->delate || false == old_scr->using_sta) {
        return;
    }
    if (GT_TYPE_UNKNOWN == gt_obj_class_get_type(old_scr)) {
        return;
    }
    gt_area_st * saved = (gt_area_st * )anim->data;
    /* UNCOVER exec_cb moved children with _gt_obj_move_child_by,
     * restore them back to original positions */
    gt_disp_st * disp = gt_disp_get_default();
    if (disp && disp->scr_anim_type >= GT_SCR_ANIM_TYPE_UNCOVER_LEFT) {
        gt_size_t dx = saved->x - old_scr->area.x;
        gt_size_t dy = saved->y - old_scr->area.y;
        if (dx || dy) {
            _gt_obj_move_child_by(old_scr, dx, dy);
        }
    }
    gt_area_copy(&old_scr->area, saved);
}

static GT_ATTRIBUTE_RAM_TEXT void _scr_anim_ready_cb(struct gt_anim_s * anim) {
    gt_disp_st * disp = gt_disp_get_default();
    GT_CHECK_BACK(disp);
    if (disp->scr_prev && disp->scr_prev->delate) {
        disp->scr_prev->using_sta = 0;
        _gt_obj_class_destroy(disp->scr_prev);
    }

    gt_obj_st * scr_act = disp->scr_act;
    disp->scr_prev      = NULL;
    disp->area_act.x    = scr_act->area.x;
    disp->area_act.y    = scr_act->area.y;

    scr_act->area.w = gt_disp_get_res_hor(NULL);
    scr_act->area.h = gt_disp_get_res_ver(NULL);

    _gt_disp_reload_max_area((gt_obj_st * )anim->tar);
    gt_disp_invalid_area(scr_act);

    /** Enabled all of input device event */
    gt_indev_set_enabled(true);
    gt_event_set_enabled(true);
#if GT_USE_UD_LR_TO_CONTROL_FOCUS_EN
    gt_indev_release_focus_lock();
#endif
}

/* Bitmask for horizontal anim types: MOVE_LEFT=1,MOVE_RIGHT=2,COVER_LEFT=5,COVER_RIGHT=6,UNCOVER_LEFT=9,UNCOVER_RIGHT=10 */
#define _ANIM_TYPE_HOR_MASK  ((1u<<GT_SCR_ANIM_TYPE_MOVE_LEFT)|(1u<<GT_SCR_ANIM_TYPE_MOVE_RIGHT)|\
                              (1u<<GT_SCR_ANIM_TYPE_COVER_LEFT)|(1u<<GT_SCR_ANIM_TYPE_COVER_RIGHT)|\
                              (1u<<GT_SCR_ANIM_TYPE_UNCOVER_LEFT)|(1u<<GT_SCR_ANIM_TYPE_UNCOVER_RIGHT))
/* Bitmask for vertical anim types: MOVE_UP=3,MOVE_DOWN=4,COVER_UP=7,COVER_DOWN=8,UNCOVER_UP=11,UNCOVER_DOWN=12 */
#define _ANIM_TYPE_VER_MASK  ((1u<<GT_SCR_ANIM_TYPE_MOVE_UP)|(1u<<GT_SCR_ANIM_TYPE_MOVE_DOWN)|\
                              (1u<<GT_SCR_ANIM_TYPE_COVER_UP)|(1u<<GT_SCR_ANIM_TYPE_COVER_DOWN)|\
                              (1u<<GT_SCR_ANIM_TYPE_UNCOVER_UP)|(1u<<GT_SCR_ANIM_TYPE_UNCOVER_DOWN))

static GT_ATTRIBUTE_RAM_TEXT bool _is_anim_type_hor(gt_scr_anim_type_et anim_type) {
    return (_ANIM_TYPE_HOR_MASK >> anim_type) & 1u;
}

static GT_ATTRIBUTE_RAM_TEXT bool _is_anim_type_ver(gt_scr_anim_type_et anim_type) {
    return (_ANIM_TYPE_VER_MASK >> anim_type) & 1u;
}

static GT_ATTRIBUTE_RAM_TEXT void _fill_color_hor(_flush_scr_param_st * param, _bg_abs_area_st * left, _bg_abs_area_st * right, bool is_left) {
    gt_disp_st * disp = param->disp;
    uint32_t cnt = 0;
    uint16_t row = 0;
    uint16_t line = GT_REFRESH_FLUSH_LINE_PRE_TIME;
    uint16_t len_pre_line = disp->area_act.w * sizeof(gt_color_t);

    if (is_left) {
        gt_color_fill(&disp->vbd_color[cnt], left->area.w, left->color);
        len_pre_line = left->area.w * sizeof(gt_color_t);
        for (row = 1; row < line; row++) {
            gt_memmove(&disp->vbd_color[cnt + disp->area_act.w], &disp->vbd_color[cnt], len_pre_line);
            cnt += disp->area_act.w;
        }
        return;
    }

    /** right */
    uint16_t right_length = 0;
    if (left->area.w == disp->area_act.w) {
        /** new screen cover into old screen from right side */
        cnt = gt_abs(right->area.x);
    } else {
        cnt = left->area.w;
    }
    right_length = disp->area_act.w - cnt;
    gt_color_fill(&disp->vbd_color[cnt], right_length, right->color);
    len_pre_line = right_length * sizeof(gt_color_t);

    for (row = 1; row < line; row++) {
        gt_memmove(&disp->vbd_color[cnt + disp->area_act.w], &disp->vbd_color[cnt], len_pre_line);
        cnt += disp->area_act.w;
    }
}

static GT_ATTRIBUTE_RAM_TEXT void _fill_color_ver(_flush_scr_param_st * param, _bg_abs_area_st * top, _bg_abs_area_st * bottom, gt_size_t cur_row, bool is_top) {
    gt_disp_st * disp = param->disp;
    uint16_t line = GT_REFRESH_FLUSH_LINE_PRE_TIME;
    uint32_t len = line * disp->area_act.w;

    if (cur_row < top->area.h) {
        if (cur_row + line > top->area.h) {
            uint32_t top_len = (top->area.h - cur_row) * disp->area_act.w;
            if (is_top) {
                gt_color_fill(disp->vbd_color, top_len, top->color);
            } else {
                gt_color_fill(&disp->vbd_color[top_len], len - top_len, bottom->color);
            }
        } else {
            if (is_top) {
                gt_color_fill(disp->vbd_color, len, top->color);
            }
        }
    } else {
        if (false == is_top) {
            gt_color_fill(disp->vbd_color, len, bottom->color);
        }
    }
}

/**
 * @brief Horizontal animation direction, adapt to the display range of the interface
 *
 * @param param
 */
static GT_ATTRIBUTE_RAM_TEXT inline void _adapt_area_flush_hor(_flush_scr_param_st * param) {
    gt_obj_st * prev = param->disp->scr_prev;
    gt_obj_st * scr = param->disp->scr_act;
    gt_scr_anim_type_et anim_type = param->disp->scr_anim_type;

    param->valid.is_hor = true;

    if (GT_SCR_ANIM_TYPE_UNCOVER_LEFT == anim_type || GT_SCR_ANIM_TYPE_UNCOVER_RIGHT == anim_type) {
        /* UNCOVER: old screen (prev) moves, new screen (scr) stays still.
         * Visible width of prev = res - |prev.x| (UNCOVER_LEFT: prev.x < 0)
         *                       = res - prev.x   (UNCOVER_RIGHT: prev.x > 0) */
        uint16_t res = gt_disp_get_res_hor(param->disp);
        prev->area.w = res - (uint16_t)gt_abs(prev->area.x);
        scr->area.w  = res - prev->area.w;
        param->view_scr_abs.w      = scr->area.w;
        param->view_scr_prev_abs.w = prev->area.w;
        /* UNCOVER_LEFT: prev slides left, new screen appears on the right */
        bool is_uncover_left = (GT_SCR_ANIM_TYPE_UNCOVER_LEFT == anim_type);
        param->view_scr_abs.x      = is_uncover_left ? prev->area.w : 0;
        param->view_scr_prev_abs.x = is_uncover_left ? 0 : scr->area.w;
        return;
    }

    scr->area.w = gt_abs(scr->area.x - param->disp->anim_scr_remark.x);
    prev->area.w = gt_disp_get_res_hor(param->disp) - scr->area.w;
    param->view_scr_abs.w = scr->area.w;
    param->view_scr_prev_abs.w = prev->area.w;

    /* LEFT: new screen enters from left → abs.x = prev.w; RIGHT: new enters from right → abs.x = 0 */
    bool is_left = (GT_SCR_ANIM_TYPE_COVER_LEFT == anim_type ||
                    GT_SCR_ANIM_TYPE_MOVE_LEFT  == anim_type);
    param->view_scr_abs.x      = is_left ? prev->area.w : 0;
    param->view_scr_prev_abs.x = is_left ? 0 : scr->area.w;
}

/**
 * @brief In the direction of vertical animation, adapt to the display range of the interface
 *
 * @param param
 */
static GT_ATTRIBUTE_RAM_TEXT inline void _adapt_area_flush_ver(_flush_scr_param_st * param) {
    gt_obj_st * prev = param->disp->scr_prev;
    gt_obj_st * scr = param->disp->scr_act;
    gt_scr_anim_type_et anim_type = param->disp->scr_anim_type;

    param->valid.is_hor = false;

    if (GT_SCR_ANIM_TYPE_UNCOVER_UP == anim_type || GT_SCR_ANIM_TYPE_UNCOVER_DOWN == anim_type) {
        /* UNCOVER: old screen (prev) moves, new screen (scr) stays still. */
        uint16_t res = gt_disp_get_res_ver(param->disp);
        prev->area.h = res - (uint16_t)gt_abs(prev->area.y);
        scr->area.h  = res - prev->area.h;
        param->view_scr_abs.h      = scr->area.h;
        param->view_scr_prev_abs.h = prev->area.h;
        /* UNCOVER_UP: prev slides up, new screen appears at the bottom */
        bool is_uncover_up = (GT_SCR_ANIM_TYPE_UNCOVER_UP == anim_type);
        param->view_scr_abs.y      = is_uncover_up ? prev->area.h : 0;
        param->view_scr_prev_abs.y = is_uncover_up ? 0 : scr->area.h;
        return;
    }

    scr->area.h = gt_abs(scr->area.y - param->disp->anim_scr_remark.y);
    prev->area.h = gt_disp_get_res_ver(param->disp) - scr->area.h;
    param->view_scr_abs.h = scr->area.h;
    param->view_scr_prev_abs.h = prev->area.h;

    /* UP: new screen enters from top → abs.y = prev.h; DOWN: new enters from bottom → abs.y = 0 */
    bool is_up = (GT_SCR_ANIM_TYPE_COVER_UP == anim_type ||
                  GT_SCR_ANIM_TYPE_MOVE_UP  == anim_type);
    param->view_scr_abs.y      = is_up ? prev->area.h : 0;
    param->view_scr_prev_abs.y = is_up ? 0 : scr->area.h;
}

static GT_ATTRIBUTE_RAM_TEXT inline void _clear_buffer(_flush_scr_param_st * param, gt_color_t prev_c, gt_color_t cur_c, gt_size_t cur_row, bool is_prev) {
    gt_disp_st * disp = param->disp;
    _bg_abs_area_st prev = {
        .area = param->view_scr_prev_abs,
        .color = prev_c,
    };
    _bg_abs_area_st next = {
        .area = param->view_scr_abs,
        .color = cur_c,
    };

    switch (disp->scr_anim_type) {
        case GT_SCR_ANIM_TYPE_MOVE_LEFT:
        case GT_SCR_ANIM_TYPE_COVER_LEFT:
        case GT_SCR_ANIM_TYPE_UNCOVER_LEFT: {
            _fill_color_hor(param, &prev, &next, is_prev);
            break;
        }
        case GT_SCR_ANIM_TYPE_MOVE_RIGHT:
        case GT_SCR_ANIM_TYPE_COVER_RIGHT:
        case GT_SCR_ANIM_TYPE_UNCOVER_RIGHT: {
            _fill_color_hor(param, &next, &prev, !is_prev);
            break;
        }
        case GT_SCR_ANIM_TYPE_MOVE_UP:
        case GT_SCR_ANIM_TYPE_COVER_UP:
        case GT_SCR_ANIM_TYPE_UNCOVER_UP: {
            _fill_color_ver(param, &prev, &next, cur_row, is_prev);
            break;
        }
        case GT_SCR_ANIM_TYPE_MOVE_DOWN:
        case GT_SCR_ANIM_TYPE_COVER_DOWN:
        case GT_SCR_ANIM_TYPE_UNCOVER_DOWN: {
            _fill_color_ver(param, &next, &prev, cur_row, !is_prev);
            break;
        }
        default:
            break;
    }
}

static GT_ATTRIBUTE_RAM_TEXT inline void _adjust_clip_area_and_flush(_flush_scr_param_st * param, gt_obj_st * target_scr) {
    gt_size_t scr_offset_x = target_scr->area.x;
    gt_size_t scr_offset_y = target_scr->area.y;
    /* UNCOVER: children already moved with screen, area_disp must not
     * double-count the screen offset for the prev (old) screen */
    if (param->disp->scr_anim_type >= GT_SCR_ANIM_TYPE_UNCOVER_LEFT &&
        target_scr == param->disp->scr_prev) {
        scr_offset_x = 0;
        scr_offset_y = 0;
    }
    param->disp->area_disp.x = scr_offset_x + param->valid.area_clip.x;
    param->disp->area_disp.y = scr_offset_y + param->valid.area_clip.y;
    param->disp->area_disp.h = param->valid.area_clip.h;

    if (param->area_flush.y == param->valid.area_clip.y) {
        param->valid.area_clip.y = 0;
    } else {
        param->valid.area_clip.y = param->valid.area_clip.y - param->area_flush.y;
    }
    _gt_disp_check_and_copy_foreach(target_scr, param);
}

/**
 * @brief flush screen by animation
 *
 * @param param The package of all of param, using such as: disp, area_flush, scr, scr_prev, view_scr_prev_abs, valid, line.
 * @param disp
 * @param area_flush The buffer or area of display flush area.
 * @param scr       The current screen absolute display area
 * @param scr_prev  The previous screen absolute display area
 * @param view_scr_abs The next screen in the display area of the physical window
 * @param view_scr_prev_abs The previous screen in the display area of the physical window
 * @param valid Interface intersection results and screen display start offset
 * @param line Number of rows per refresh
 */
static GT_ATTRIBUTE_RAM_TEXT inline void _flush_scr_by_anim(_flush_scr_param_st * param) {
    gt_obj_st * prev = param->disp->scr_prev;
    gt_obj_st * scr = param->disp->scr_act;
    gt_obj_type_et type = gt_obj_class_get_type(prev);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    type = gt_obj_class_get_type(scr);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    bool is_cover = false;
    bool is_double_buf = gt_disp_graph_is_double_buf();
    gt_scr_anim_type_et anim_type = (gt_scr_anim_type_et)param->disp->scr_anim_type;
    if (_is_anim_type_hor(anim_type)) {
        _adapt_area_flush_hor(param);
    } else if (_is_anim_type_ver(anim_type)) {
        _adapt_area_flush_ver(param);
    }

    while(param->area_flush.y < param->disp->area_act.h) {
        /** prev screen */
        _clear_buffer(param, prev->bgcolor, scr->bgcolor, param->area_flush.y, true);

        is_cover = gt_area_cover_screen(&param->area_flush, &param->view_scr_prev_abs, &param->valid.area_clip);
        if (is_cover) {
            _adjust_clip_area_and_flush(param, prev);
        }

        /** new screen */
        _clear_buffer(param, prev->bgcolor, scr->bgcolor, param->area_flush.y, false);

        is_cover = gt_area_cover_screen(&param->area_flush, &param->view_scr_abs, &param->valid.area_clip);
        if (is_cover) {
            _adjust_clip_area_and_flush(param, scr);
        }

#if GT_USE_LAYER_TOP
        // flush top layer
        is_cover = gt_area_cover_screen(&param->area_flush, &param->disp->layer_top->area, &param->valid.area_clip);
        if (is_cover) {
            param->valid.layer_top = true;
            param->valid.area_clip.x = 0;
            param->valid.area_clip.y = 0;
            param->disp->area_disp.x = param->area_flush.x;
            param->disp->area_disp.y = param->area_flush.y;
            param->disp->area_disp.h = param->line;
            _gt_disp_check_and_copy_foreach(param->disp->layer_top, param);
            param->valid.layer_top = false;
        }
#endif
        if (is_double_buf) {
            while(gt_disp_drv_check_flushing(param->disp->drv)){
                if(param->disp->drv->wait_cb){ param->disp->drv->wait_cb(param->disp->drv); }
            }
        }

        /** flush display by buffer area */
        _gt_disp_flush_one(param->disp, &param->area_flush, param->disp->vbd_color);
        param->area_flush.y += param->line;

        if (is_double_buf) {
            param->disp->vbd_color = gt_disp_graph_get_buf_backup(param->disp->vbd_color);
        }

    }
#if GT_DISP_SW_ROTATE_SUPPORT
    _gt_disp_flush_commit(param->disp);
#endif
}
#endif  /** GT_USE_SCREEN_ANIM */

/**
 * @brief Map area_flush from logical screen space to physical display space,
 *        then clamp to screen bounds.
 *
 * @param param
 * @param scr
 * @return gt_area_st The adjusted flush area.
 */
static GT_ATTRIBUTE_RAM_TEXT gt_area_st
_update_area_flush_by_calc_partly_redraw_area(_flush_scr_param_st * param, gt_obj_st * scr) {
    const uint16_t width  = gt_disp_get_res_hor(param->disp);
    const uint16_t height = gt_disp_get_res_ver(param->disp);

    /** Full-screen fast path: only clamp origin, size is already correct */
    if (width == param->area_flush.w && height == param->area_flush.h) {
        if (param->area_flush.x < 0 || param->area_flush.x + param->area_flush.w > width) {
            param->area_flush.x = 0;
        }
        if (param->area_flush.y < 0 || param->area_flush.y + param->area_flush.h > height) {
            param->area_flush.y = 0;
        }
        return param->area_flush;
    }

    /** X axis */
    int32_t x = param->area_flush.x;
    int32_t w = param->area_flush.w;
    const int32_t sx = scr->area.x;

    if (sx == 0) {
        if (x < 0) { w += x; x = 0; }
    } else if (sx > 0) {
        if (x < 0) { w += x; x = 0; }
        if (x > sx) { x -= sx; }
        else        { w = (w + x) - sx; x = 0; }
    } else { /* sx < 0 */
        if (x < 0) { w += x - sx; x = 0; }
        if (x > sx || x > 0) { x -= sx; }
    }
    if (x + w > width) { w = (int32_t)width - x; }
    param->area_flush.x = (gt_size_t)x;
    param->area_flush.w = (uint16_t)(w > 0 ? w : 0);

    /** Y axis */
    int32_t y = param->area_flush.y;
    int32_t h = param->area_flush.h;
    const int32_t sy = scr->area.y;

    if (sy == 0) {
        if (y < 0) { h += y; y = 0; }
    } else if (sy > 0) {
        if (y < 0) { h += y; y = 0; }
        if (y > sy) { y -= sy; }
        else        { h = (h + y) - sy; y = 0; }
    } else { /* sy < 0 */
        if (y < 0) { h += y - sy; y = 0; }
        if (y > sy || y > 0) { y -= sy; }
    }
    if (y + h > height) { h = (int32_t)height - y; }
    param->area_flush.y = (gt_size_t)y;
    param->area_flush.h = (uint16_t)(h > 0 ? h : 0);

    /* The invalid area is expressed in screen-content coordinates. After
     * applying a negative screen offset it may lie completely outside the
     * physical viewport. Do not pass such coordinates to the display driver:
     * some LCD drivers wrap an out-of-range Y coordinate back to the top of
     * the panel, which makes a scrolled animation appear at Y=0. */
    if (param->area_flush.y >= height) {
        param->area_flush.h = 0;
    } else if (param->area_flush.y < 0) {
        gt_size_t clipped = -param->area_flush.y;
        if (clipped >= param->area_flush.h) {
            param->area_flush.h = 0;
            param->area_flush.y = 0;
        } else {
            param->area_flush.h -= clipped;
            param->area_flush.y = 0;
        }
    } else if (param->area_flush.y + param->area_flush.h > height) {
        param->area_flush.h = height - param->area_flush.y;
    }

#if GT_REFR_AREA_ALIGN_VER
    {
        uint16_t align_val = (uint16_t)gt_abs(param->area_flush.y % GT_REFR_AREA_ALIGN_VER_PIXEL);
        if (align_val) {
            param->area_flush.y -= align_val;
            param->area_flush.h += align_val;
        }
        align_val = param->area_flush.h % GT_REFR_AREA_ALIGN_VER_PIXEL;
        if (align_val) {
            param->area_flush.h += GT_REFR_AREA_ALIGN_VER_PIXEL - align_val;
        }
    }
#endif

    return param->area_flush;
}

#if GT_LAYER_TOP_SCREENSHOT_CACHE
/**
 * @brief
 *
 * @param param param->area_flush.y from 0
 * @return
 */
static void GT_ATTRIBUTE_RAM_TEXT
_gt_disp_paste_screenshot(_flush_scr_param_st * param) {
    gt_disp_st * disp = param->disp;
    gt_buffer_st * scr_shot = _gt_gc_get_screenshot_cache();
    uint32_t dst_offset = (param->area_flush.y * disp->drv->res_hor + param->area_flush.x) * sizeof(gt_color_t);
    uint32_t line_byte = disp->area_disp.w * sizeof(gt_color_t);
    uint32_t screen_line_byte = disp->drv->res_hor * sizeof(gt_color_t);
    uint32_t copy_size = disp->area_disp.h * line_byte;
    uint32_t offset = 0, line = 0;
    if (dst_offset + copy_size > scr_shot->len) {
        GT_LOGW("", "cache overflow: %d + %d > %d", dst_offset, copy_size, scr_shot->len);
        return;
    }
    if (disp->area_disp.w != disp->drv->res_hor || disp->area_disp.h != disp->drv->res_ver) {
        for (line = 0; line < disp->area_disp.h; line++) {
            gt_memcpy((char * )&disp->vbd_color[offset], &scr_shot->ptr[dst_offset], line_byte);
            dst_offset += screen_line_byte;
            offset += disp->area_disp.w;
        }
    } else {
        gt_memcpy((char * )disp->vbd_color, &scr_shot->ptr[dst_offset], copy_size);
    }
}

static void GT_ATTRIBUTE_RAM_TEXT
_gt_disp_copy_screenshot(_flush_scr_param_st * param) {
    gt_disp_st * disp = param->disp;
    gt_buffer_st * scr_shot = _gt_gc_get_screenshot_cache();
    uint32_t dst_offset = (param->area_flush.y * disp->area_disp.w + param->area_flush.x) * sizeof(gt_color_t);
    uint32_t copy_size = disp->area_disp.w * disp->area_disp.h * sizeof(gt_color_t);
    if (dst_offset + copy_size > scr_shot->len) {
        GT_LOGW("", "cache overflow: %d + %d > %d", dst_offset, copy_size, scr_shot->len);
        return;
    }
    gt_memcpy((char * )&scr_shot->ptr[dst_offset], (char * )disp->vbd_color, copy_size);
}
#endif

/**
 * @brief flush screen by direct or full screen refresh
 *
 * @param param The package of all of param, using such as: disp, area_flush, scr, line.
 * @param disp
 * @param area_flush The buffer or area of display flush area.
 * @param scr       The current screen absolute display area
 * @param line Number of rows per refresh
 */
static GT_ATTRIBUTE_RAM_TEXT inline void _flush_scr_by_direct(_flush_scr_param_st * param) {
    gt_disp_st * disp = param->disp;
    gt_obj_st * scr = disp->scr_act;

    /** Type check before any member access */
    if (GT_TYPE_UNKNOWN == gt_obj_class_get_type(scr)) {
        return;
    }

    gt_color_t color_fill = gt_screen_get_bgcolor(scr);
    scr->area.x = disp->area_act.x;
    scr->area.y = disp->area_act.y;

    if (false == gt_area_is_intersect_screen(&scr->area, &param->area_flush)) {
        return;
    }
    _update_area_flush_by_calc_partly_redraw_area(param, scr);

#if GT_USE_LAYER_TOP
    /** Check if top layer fully covers screen with GT_OPA_COVER to skip base layer draw */
    _gt_top_ret_st top_show = _gt_disp_is_top_full_cover_display(disp->layer_top);
    bool is_full_cover = top_show.full_cover && false == top_show.is_opa;
#if GT_LAYER_TOP_SCREENSHOT_CACHE
    bool is_half_opa = top_show.full_cover && top_show.is_opa;
    bool need_screenshot_backup = is_half_opa && false == disp->screenshot;
    bool is_screenshot_valid = _gt_gc_screenshot_cache_is_valid();
#endif
#endif  /** GT_USE_LAYER_TOP */

    int32_t end_y = (int32_t)param->area_flush.y + param->area_flush.h;
    disp->area_disp.w = param->area_flush.w;
    if (param->area_flush.h > param->line) {
        param->area_flush.h = param->line;
    }
    disp->area_disp.h = param->area_flush.h;

    bool is_double_buf = gt_disp_graph_is_double_buf();

    while (param->area_flush.y < end_y) {
        uint32_t len = (uint32_t)disp->area_disp.w * disp->area_disp.h;

#if GT_LAYER_TOP_SCREENSHOT_CACHE
        if (!is_screenshot_valid || !disp->screenshot) {
            gt_color_fill(disp->vbd_color, len, color_fill);
        }
#else
        gt_color_fill(disp->vbd_color, len, color_fill);
#endif

        disp->area_disp.x = param->area_flush.x + scr->area.x;
        disp->area_disp.y = param->area_flush.y + scr->area.y;

#if GT_USE_LAYER_TOP
#if GT_LAYER_TOP_SCREENSHOT_CACHE
        if (is_screenshot_valid && disp->screenshot) {
            _gt_disp_paste_screenshot(param);
        } else {
            if (false == is_full_cover) {
                _gt_disp_check_and_copy_foreach(scr, param);
            }
            if (is_screenshot_valid && need_screenshot_backup) {
                _gt_disp_copy_screenshot(param);
            }
        }
#else
        if (false == is_full_cover) {
            _gt_disp_check_and_copy_foreach(scr, param);
        }
#endif  /** GT_LAYER_TOP_SCREENSHOT_CACHE */

        /** flush top layer */
        disp->area_disp.x = param->area_flush.x;
        disp->area_disp.y = param->area_flush.y;
        _gt_disp_check_and_copy_foreach(disp->layer_top, param);
#else   /** !GT_USE_LAYER_TOP */
        _gt_disp_check_and_copy_foreach(scr, param);
#endif  /** GT_USE_LAYER_TOP */

        if (is_double_buf) {
            while (gt_disp_drv_check_flushing(disp->drv)) {
                if (disp->drv->wait_cb) { disp->drv->wait_cb(disp->drv); }
            }
        }

        _gt_disp_flush_one(disp, &param->area_flush, disp->vbd_color);
        param->area_flush.y += param->line;

        /** Clamp last tile height to avoid over-drawing */
        if (param->area_flush.y < end_y && end_y - param->area_flush.y < param->line) {
            param->area_flush.h = end_y - param->area_flush.y;
            disp->area_disp.h = param->area_flush.h;
        }

        if (is_double_buf) {
            disp->vbd_color = gt_disp_graph_get_buf_backup(disp->vbd_color);
        }
    }
#if GT_DISP_SW_ROTATE_SUPPORT
    _gt_disp_flush_commit(disp);
#endif
#if GT_LAYER_TOP_SCREENSHOT_CACHE
    disp->screenshot = is_half_opa;
#endif
}

static inline gt_scr_anim_type_et GT_ATTRIBUTE_RAM_TEXT
_get_anti_anim_type(gt_scr_anim_type_et type) {
#if GT_USE_SCREEN_ANIM
    if (GT_SCR_ANIM_TYPE_NONE == type) {
        return GT_SCR_ANIM_TYPE_NONE;
    }
    /* COVER_x → UNCOVER in reverse direction:
     * COVER_LEFT(5)→UNCOVER_RIGHT(10), COVER_RIGHT(6)→UNCOVER_LEFT(9)
     * COVER_UP(7)→UNCOVER_DOWN(12),    COVER_DOWN(8)→UNCOVER_UP(11)  */
    if (type >= GT_SCR_ANIM_TYPE_COVER_LEFT && type <= GT_SCR_ANIM_TYPE_COVER_DOWN) {
        static const gt_scr_anim_type_et cover_anti[] = {
            GT_SCR_ANIM_TYPE_UNCOVER_RIGHT, /* COVER_LEFT  → UNCOVER_RIGHT */
            GT_SCR_ANIM_TYPE_UNCOVER_LEFT,  /* COVER_RIGHT → UNCOVER_LEFT  */
            GT_SCR_ANIM_TYPE_UNCOVER_DOWN,  /* COVER_UP    → UNCOVER_DOWN  */
            GT_SCR_ANIM_TYPE_UNCOVER_UP,    /* COVER_DOWN  → UNCOVER_UP    */
        };
        return cover_anti[type - GT_SCR_ANIM_TYPE_COVER_LEFT];
    }
    /* UNCOVER_x → COVER in reverse direction:
     * UNCOVER_LEFT(9)→COVER_RIGHT(6), UNCOVER_RIGHT(10)→COVER_LEFT(5)
     * UNCOVER_UP(11)→COVER_DOWN(8),   UNCOVER_DOWN(12)→COVER_UP(7)  */
    if (type >= GT_SCR_ANIM_TYPE_UNCOVER_LEFT && type <= GT_SCR_ANIM_TYPE_UNCOVER_DOWN) {
        static const gt_scr_anim_type_et uncover_anti[] = {
            GT_SCR_ANIM_TYPE_COVER_RIGHT,   /* UNCOVER_LEFT  → COVER_RIGHT */
            GT_SCR_ANIM_TYPE_COVER_LEFT,     /* UNCOVER_RIGHT → COVER_LEFT  */
            GT_SCR_ANIM_TYPE_COVER_DOWN,     /* UNCOVER_UP    → COVER_DOWN  */
            GT_SCR_ANIM_TYPE_COVER_UP,       /* UNCOVER_DOWN  → COVER_UP    */
        };
        return uncover_anti[type - GT_SCR_ANIM_TYPE_UNCOVER_LEFT];
    }
    /* MOVE_x: pairs (1,2),(3,4) flip within pair */
    return (gt_scr_anim_type_et)(((type - 1u) ^ 1u) + 1u);
#else
    return GT_SCR_ANIM_TYPE_NONE;
#endif
}

static GT_ATTRIBUTE_RAM_TEXT gt_obj_st * _create_scr_by_id(gt_scr_id_t scr_id) {
    gt_scr_init_func_cb_t scr_init_cb = gt_scr_stack_get_init_func(scr_id);
    if (NULL == scr_init_cb) {
        GT_LOGE(GT_LOG_TAG_GUI, "Screen[0x%X = %d] init callback is NULL", scr_id, scr_id);
        return NULL;
    }
    return scr_init_cb();
}

static GT_ATTRIBUTE_RAM_TEXT void _layer_top_destroy_handler_cb(struct _gt_timer_s * timer) {
    gt_obj_st * layer_top = (gt_obj_st * )_gt_timer_get_user_data(timer);
    if (gt_obj_is_type(layer_top, GT_TYPE_UNKNOWN)) {
        return;
    }
    for (gt_size_t i = layer_top->cnt_child - 1; i >= 0; i--) {
        if (gt_obj_is_type(layer_top->child[i], GT_TYPE_STATUS_BAR)) {
            continue;
        }
        if (gt_obj_is_type(layer_top->child[i], GT_TYPE_DIALOG)) {
            continue;
        }
        _gt_obj_class_destroy(layer_top->child[i]);
    }
}

/**
 * @brief Create new screen by stack item
 *
 * @param item_p
 * @param home_id screen id
 * @return bool true: Create new screen successfully, false: Create new screen failed.
 */
static bool GT_ATTRIBUTE_RAM_TEXT
_create_scr_by_stack_item(gt_scr_stack_item_st * item_p, gt_scr_id_t home_id) {
    if (item_p->current_scr) {
        return true;
    }
#if GT_DISP_STACK_ABORT_LOADING_WHILE_NOT_ENOUGH_MEM
    gt_mem_reset_failed_count();
#endif
    item_p->current_scr = _create_scr_by_id(item_p->current_scr_id);
    if (NULL == item_p->current_scr) {
        GT_LOGE(GT_LOG_TAG_GUI, "item_p->current_scr is NULL! Create screen failed.");
        return false;
    }
#if GT_DISP_STACK_ABORT_LOADING_WHILE_NOT_ENOUGH_MEM
    if (gt_mem_has_failed()) {
        _gt_obj_class_destroy(item_p->current_scr);
        GT_LOGE(GT_LOG_TAG_GUI, "New screen has memory failed, abort loading.");
        return false;
    }
#endif
    if (home_id == item_p->current_scr_id) {
        gt_scr_stack_set_home_scr(item_p->current_scr);
    }
#if _GT_STACK_USE_LOADING_LOG_DETAIL
    GT_LOGI(GT_LOG_TAG_GUI, "create screen id %d(0x%x) obj: 0x%x", item_p->current_scr_id, item_p->current_scr_id, item_p->current_scr);
#endif
    return true;
}

static void inline GT_ATTRIBUTE_RAM_TEXT
_clear_stack_when_home_scr_id_current_valid(
    gt_scr_stack_item_st * reload_p, gt_scr_stack_item_st const * const target, gt_scr_id_t home_id, bool is_home_scr_alive) {
    if (-1 == reload_p->current_scr_id) {
        if (false == gt_scr_stack_is_empty()) {
            gt_scr_stack_clear();
        }
        /** set bottom stack screen */
        reload_p->current_scr_id = (-1 != home_id) ? home_id : (target ? target->current_scr_id : -1);
        if (-1 == home_id) {
            return;
        }
        if (is_home_scr_alive && NULL == reload_p->current_scr && target && target->current_scr) {
            reload_p->current_scr = target->current_scr;
        }
        return;
    }
    if (home_id != reload_p->current_scr_id) {
        return;
    }
    if (gt_scr_stack_is_empty()) {
        return;
    }
    gt_scr_stack_clear();
}

static gt_res_t GT_ATTRIBUTE_RAM_TEXT
_reset_stack_or_push_item_after_home(
    gt_disp_stack_res_st * ret_p, gt_scr_stack_item_st * new_item_p, gt_scr_id_t home_id, bool is_clear_stack_except_home) {
    if (-1 != home_id && new_item_p->current_scr_id == home_id) {
        /** Home page reset stack */
        if (false == gt_scr_stack_is_empty()) {
            gt_scr_stack_clear();
            /** scr_old is now a dangling pointer after clear, reset it to avoid
             *  false equality when new home screen is allocated at the same address */
            ret_p->scr_old = NULL;
        }
        if (new_item_p->current_scr && ret_p->scr_old == new_item_p->current_scr) {
            /** reload current screen */
            ret_p->del_prev_scr = false;
        } else {
            /** release current screen memory, try to reload home page screen */
            ret_p->del_prev_scr = true;
            new_item_p->current_scr = gt_scr_stack_get_home_scr();
        }
        return GT_RES_OK;
    }
    bool is_stack_empty = gt_scr_stack_is_empty();
    if (is_clear_stack_except_home && false == is_stack_empty) {
        gt_scr_stack_clear();
        is_stack_empty = true;
        ret_p->del_prev_scr = true;
    }
    if (is_stack_empty && new_item_p->current_scr_id != home_id) {
        /** Push home into stack root, then push current screen to second one. */
        gt_scr_stack_item_st home_item = {
            .current_scr = gt_scr_stack_get_home_scr(),
            .current_scr_id = home_id,
        };
        if (false == gt_scr_stack_push(&home_item)) {
            GT_LOGE(GT_LOG_TAG_GUI, "Push home root stack failed");
            return GT_RES_FAIL;
        }
    }
    return GT_RES_OK;
}

/**
 * @brief Handle duplicate screen found in stack: decide whether to reuse or rebuild screen object
 *
 * @param ret result (del_prev_scr may be modified)
 * @param new_item_p new stack item to push
 * @param same_scr popped stack item with the same screen id
 * @param param push parameters
 * @param home_id home screen id
 * @param is_home_alive whether home screen object is alive
 * @param is_at_stack_bottom whether the duplicate item was at stack bottom
 */
static void GT_ATTRIBUTE_RAM_TEXT
_handle_duplicate_scr(
    gt_disp_stack_res_st * ret,
    gt_scr_stack_item_st * new_item_p,
    gt_scr_stack_item_st * same_scr,
    gt_disp_stack_param_st const * const param,
    gt_scr_id_t home_id,
    bool is_home_alive,
    bool is_at_stack_bottom)
{
    if (NULL == same_scr) {
        /** Defensive fallback: no duplicate item available, rebuild. */
        new_item_p->current_scr = NULL;
        ret->del_prev_scr = true;
        return;
    }
    if (home_id == same_scr->current_scr_id && is_home_alive) {
        new_item_p->current_scr = same_scr->current_scr;
        ret->del_prev_scr = is_at_stack_bottom ? false : true;
        return;
    }
    if (param->force_create_reload
#if GT_DISP_STACK_IGNORE_REPEAT_SCREEN_ID
        || (false == param->ignore_repeat_screen_id)
#endif
        ) {
        if (same_scr->current_scr == ret->scr_old) {
            /** target is the current screen: the switch anim still references it
             *  (anim_old target), so only free its children now; the shell is
             *  destroyed by the anim ready callback via del_prev_scr. */
            if (false == _gt_obj_in_event_dispatch(same_scr->current_scr)) {
                _gt_obj_class_destroy_children(same_scr->current_scr);
            }
        } else {
            /** target is not the current screen: safe to destroy immediately. */
            gt_obj_destroy_immediately(same_scr->current_scr);
        }
        new_item_p->current_scr = NULL;
    } else {
        new_item_p->current_scr = same_scr->current_scr;
        if (new_item_p->current_scr) {
            /** Reused (refresh, not re-created). Restore the in-use flag that
             *  _gt_scr_stack_pop_keep_target cleared, and drop any stale delate
             *  mark left by an interrupted switch animation so the screen and
             *  its widgets can be destroyed normally again. */
            new_item_p->current_scr->using_sta = true;
            if (new_item_p->current_scr->delate) {
                new_item_p->current_scr->delate = false;
                gt_obj_child_set_prop(new_item_p->current_scr, GT_OBJ_PROP_TYPE_DELATE, false);
            }
        }
    }
    ret->del_prev_scr = true;    /** Must free old screen memory */
}

/**
 * @brief Handle new screen (no duplicate in stack): check stack top and decide whether to free old screen
 *
 * @param ret result (del_prev_scr may be modified)
 * @param param push parameters
 * @param home_id home screen id
 * @param is_home_alive whether home screen object is alive
 * @param is_at_stack_bottom whether the top item is at stack bottom
 * @return gt_scr_stack_item_st* stack top item pointer (may be NULL)
 */
static gt_scr_stack_item_st * GT_ATTRIBUTE_RAM_TEXT
_handle_new_scr(
    gt_disp_stack_res_st * ret,
    gt_disp_stack_param_st const * const param,
    gt_scr_id_t home_id,
    bool is_home_alive,
    bool is_at_stack_bottom)
{
    gt_scr_stack_item_st * prev = gt_scr_stack_peek();
    if (prev) {
        if (home_id == prev->current_scr_id && is_home_alive) {
            ret->del_prev_scr = false;
        } else if (is_at_stack_bottom && param->force_create_reload) {
            if (false == _gt_obj_in_event_dispatch(prev->current_scr)) {
                _gt_obj_class_destroy_children(prev->current_scr);
            }
            ret->del_prev_scr = true;
        }
        if (ret->del_prev_scr) { prev->current_scr = NULL; }
    }
    if (false == ret->del_prev_scr && _gt_scr_stack_has_pop_before(ret->scr_old)) {
        ret->del_prev_scr = true;
    }
    return prev;
}

/**
 * @brief [Warning] Only push screen object into stack, but not display or reload screen.
 *
 * @param param
 * @return gt_disp_stack_res_st
 */
static gt_disp_stack_res_st GT_ATTRIBUTE_RAM_TEXT
gt_disp_stack_push_scr_only_st(gt_disp_stack_param_st const * const param) {
    gt_disp_stack_res_st ret = {
        .ok = false,
    };
    gt_scr_stack_item_st * new_item_p = &ret.new_item;
    GT_CHECK_BACK_VAL(param, ret);
    if (false == gt_refr_is_enabled(NULL)) {
        return ret;
    }
    ret.scr_old = gt_disp_get_scr();
    ret.del_prev_scr = param->del_prev_scr;

    new_item_p->current_scr_id = param->scr_id;
    new_item_p->time = param->time;
    new_item_p->delay = param->delay;
    new_item_p->anim_type = param->type;

    gt_scr_id_t home_id = gt_scr_stack_get_home_scr_id();
    gt_obj_st * home_scr = gt_scr_stack_get_home_scr();
    bool is_home_alive = gt_scr_stack_is_home_scr_alive();

    /** Has the same page before and pop unused stack item */
    gt_stack_item_st result = gt_scr_stack_has_before(new_item_p);
    /** index_from_top is 1-based from the top, so the bottom-most item has
     *  index_from_top == count. */
    bool is_at_stack_bottom = result.index_from_top == gt_scr_stack_get_count();
    gt_scr_stack_item_st * prev = NULL;

#if GT_DISP_STACK_IGNORE_REPEAT_SCREEN_ID
    if (param->ignore_repeat_screen_id) {
        if (false == param->force_create_reload) {
            result.data = NULL; /** Need to create a new screen to load */
        }
    }
#endif

    if (result.data) {
        /** Pop down to the duplicate screen but keep its object for reuse,
         *  instead of destroying it (the destroy would make the reuse branch
         *  below always see current_scr == NULL and silently rebuild). */
        gt_scr_stack_item_st * same_scr = _gt_scr_stack_pop_keep_target(new_item_p->current_scr_id);
        _handle_duplicate_scr(&ret, new_item_p, same_scr, param,
                              home_id, is_home_alive, is_at_stack_bottom);
    } else {
        prev = _handle_new_scr(&ret, param, home_id, is_home_alive, is_at_stack_bottom);
    }

    if (GT_RES_FAIL == _reset_stack_or_push_item_after_home(
        &ret, new_item_p, home_id, param->clear_stack_except_home)) {
        return ret;
    }
    if ((home_scr == ret.scr_old || (prev && home_id == prev->current_scr_id)) && is_home_alive) {
        ret.del_prev_scr = false;
    }

#if _GT_STACK_PUSH_BEFORE_INIT_SCR
    if (gt_scr_stack_is_empty() && home_scr) {
        gt_scr_stack_item_st home = {
            .current_scr_id = home_id,
            .current_scr = home_scr,
        };
        if (false == gt_scr_stack_push(&home)) {
            GT_LOGE(GT_LOG_TAG_GUI, "Push home stack failed");
            return ret;
        }
    }
    if (false == gt_scr_stack_push(new_item_p)) {
        GT_LOGE(GT_LOG_TAG_GUI, "Push stack failed");
        return ret;
    }
    gt_scr_stack_item_st * item_p = gt_scr_stack_peek();
    GT_CHECK_BACK_VAL(item_p, ret);
    if (false == _create_scr_by_stack_item(item_p, home_id)) {
        GT_LOGE(GT_LOG_TAG_GUI, "Create screen %d failed", item_p->current_scr_id);
        gt_scr_stack_pop(1);
        return ret;
    }
    if (NULL == item_p->current_scr) {
        gt_scr_stack_pop(1);
        GT_LOGE(GT_LOG_TAG_GUI, "Create screen %d = NULL", item_p->current_scr_id);
        return ret;
    }
    new_item_p->current_scr = item_p->current_scr;
#else
    if (false == _create_scr_by_stack_item(new_item_p, home_id)) {
        return ret;
    }
    if (false == gt_scr_stack_push(new_item_p)) {
        GT_LOGE(GT_LOG_TAG_GUI, "Push stack failed");
        if (new_item_p->current_scr != ret.scr_old &&
            new_item_p->current_scr != home_scr) {
            gt_obj_destroy_immediately(new_item_p->current_scr);
            new_item_p->current_scr = NULL;
        }
        return ret;
    }
#endif
    ret.ok = true;
    return ret;
}

/* global functions / API interface -------------------------------------*/
gt_scr_id_t gt_disp_stack_go_back(gt_stack_size_t step)
{
    gt_disp_st * disp = gt_disp_get_default();
    GT_CHECK_BACK_VAL(disp, -1);
    if (false == gt_refr_is_enabled(disp)) {
        return -1;
    }
    if (step <= 0) {
        gt_scr_stack_item_st * top = gt_scr_stack_peek();
        return top ? top->current_scr_id : -1;
    }

    gt_obj_st * scr_old = gt_disp_get_scr();
    gt_scr_id_t home_id = gt_scr_stack_get_home_scr_id();
    bool is_home_alive = gt_scr_stack_is_home_scr_alive();

    /* Pop step layers, but keep at least the bottom when no home is set */
    gt_stack_size_t pop_cnt = step;
    if (-1 == home_id && pop_cnt >= gt_scr_stack_get_count()) {
        pop_cnt = gt_scr_stack_get_count() - 1;
    }
    gt_scr_stack_item_st * target = gt_scr_stack_pop(pop_cnt);
    gt_scr_stack_item_st * prev = gt_scr_stack_peek();

    /* Build reload screen info from previous stack item */
    gt_scr_stack_item_st reload_item = { 0 };
    if (prev) {
        reload_item.current_scr    = prev->current_scr;
        reload_item.current_scr_id = prev->current_scr_id;
    } else {
        reload_item.current_scr    = is_home_alive ? gt_scr_stack_get_home_scr() : NULL;
        reload_item.current_scr_id = home_id;
    }
    if (target) {
        reload_item.anim_type = _get_anti_anim_type(target->anim_type);
        reload_item.time      = target->time;
        reload_item.delay     = target->delay;
    }

    _clear_stack_when_home_scr_id_current_valid(&reload_item, target, home_id, is_home_alive);

    if (reload_item.current_scr && false == gt_obj_is_type(reload_item.current_scr, GT_TYPE_SCREEN)) {
        GT_LOGE(GT_LOG_TAG_UI, "Current screen is not screen type: %p(%d)",
            reload_item.current_scr, gt_obj_class_get_type(reload_item.current_scr));
    }
    if (false == _create_scr_by_stack_item(&reload_item, home_id)) {
        GT_LOGE(GT_LOG_TAG_UI, "Create screen failed");
    }
    if (prev && prev->current_scr_id == reload_item.current_scr_id) {
        prev->current_scr = reload_item.current_scr;
    }

    /* Ensure home screen is pushed when stack is empty or returning to home */
    bool is_empty = gt_scr_stack_is_empty();
    if (home_id == reload_item.current_scr_id || is_empty) {
        if (is_empty && is_home_alive) {
            reload_item.current_scr    = gt_scr_stack_get_home_scr();
            reload_item.current_scr_id = home_id;
            if (GT_TYPE_UNKNOWN == gt_obj_class_get_type(reload_item.current_scr) &&
                false == _create_scr_by_stack_item(&reload_item, home_id)) {
                GT_LOGE(GT_LOG_TAG_UI, "Create screen failed");
            }
            scr_old = NULL;
        }
        if (false == gt_scr_stack_push(&reload_item)) {
            return -1;
        }
    }

#if GT_LAYER_TOP_SCREENSHOT_CACHE
    disp->screenshot = false;
#endif
    if (scr_old != reload_item.current_scr) {
#if _GT_STACK_USE_LOADING_LOG
        GT_LOGD(GT_LOG_TAG_GUI, "go back %d:\n  stack depth: %d, scr id: 0x%X(%d), %p", step,
            gt_scr_stack_get_count(),
            reload_item.current_scr_id, reload_item.current_scr_id, reload_item.current_scr);
#endif
        gt_event_send(reload_item.current_scr, GT_EVENT_TYPE_SCREEN_BEFORE_GO_BACK, NULL);
        gt_disp_load_scr_anim(reload_item.current_scr, reload_item.anim_type, reload_item.time, reload_item.delay, true);
    }
    return reload_item.current_scr_id;
}

gt_scr_id_t gt_disp_stack_go_back_thread_safe(gt_stack_size_t step)
{
    if (false == gt_task_check_idle_timeout(_GT_DISP_THREAD_SAFE_TIMEOUT)) {
        return -1;
    }
    return gt_disp_stack_go_back(step);
}

void gt_disp_stack_load_scr(gt_scr_id_t scr_id)
{
    gt_disp_stack_param_st param = {
        .scr_id = scr_id,
        .type = GT_SCR_ANIM_TYPE_NONE,
        .time = 300,
        .delay = 0,
        .del_prev_scr = true,
#if GT_DISP_STACK_IGNORE_REPEAT_SCREEN_ID
        .ignore_repeat_screen_id = false,
#endif
        .force_create_reload = false,
    };
    gt_disp_stack_load_scr_anim_st(&param);
}

void gt_disp_stack_load_scr_thread_safe(gt_scr_id_t scr_id)
{
    if (false == gt_task_check_idle_timeout(_GT_DISP_THREAD_SAFE_TIMEOUT)) {
        return;
    }
    return gt_disp_stack_load_scr(scr_id);
}

void gt_disp_stack_load_scr_anim(gt_scr_id_t scr_id, gt_scr_anim_type_et type, uint32_t time, uint32_t delay, bool del_prev_scr)
{
    gt_disp_stack_param_st param = {
        .scr_id = scr_id,
        .type = type,
        .time = time,
        .delay = delay,
        .del_prev_scr = del_prev_scr,
#if GT_DISP_STACK_IGNORE_REPEAT_SCREEN_ID
        .ignore_repeat_screen_id = false,
#endif
        .force_create_reload = false,
        .clear_stack_except_home = true,
    };
    gt_disp_stack_load_scr_anim_st(&param);
}

void gt_disp_stack_load_scr_anim_thread_safe(gt_scr_id_t scr_id, gt_scr_anim_type_et type, uint32_t time, uint32_t delay, bool del_prev_scr)
{
    if (false == gt_task_check_idle_timeout(_GT_DISP_THREAD_SAFE_TIMEOUT)) {
        return;
    }
    return gt_disp_stack_load_scr_anim(scr_id, type, time, delay, del_prev_scr);
}

gt_res_t gt_disp_stack_load_scr_anim_st(gt_disp_stack_param_st const * const param)
{
    gt_disp_stack_res_st ret = gt_disp_stack_push_scr_only_st(param);
    if (false == ret.ok) {
        return GT_RES_FAIL;
    }
    gt_scr_stack_item_st * item_p = &ret.new_item;
#if _GT_STACK_USE_LOADING_LOG
    GT_LOGD(GT_LOG_TAG_GUI, "load:\n  stack depth: %d, scr id: 0x%X(%d) %p", gt_scr_stack_get_count(),
        item_p->current_scr_id, item_p->current_scr_id, item_p->current_scr);
#endif
    if (ret.scr_old == item_p->current_scr) {
        gt_disp_invalid_area(item_p->current_scr);
        return GT_RES_INV;
    }
    gt_disp_load_scr_anim(item_p->current_scr, item_p->anim_type, item_p->time, item_p->delay, ret.del_prev_scr);
    return GT_RES_OK;
}

gt_res_t gt_disp_stack_load_scr_anim_st_thread_safe(gt_disp_stack_param_st const * const param)
{
    if (false == gt_task_check_idle_timeout(_GT_DISP_THREAD_SAFE_TIMEOUT)) {
        return GT_RES_INV;
    }
    return gt_disp_stack_load_scr_anim_st(param);
}

void gt_disp_load_scr(gt_obj_st * scr)
{
    gt_disp_load_scr_anim(scr, GT_SCR_ANIM_TYPE_NONE, 300, 0, true);
}

void gt_disp_load_scr_thread_safe(gt_obj_st * scr)
{
    if (false == gt_task_check_idle_timeout(_GT_DISP_THREAD_SAFE_TIMEOUT)) {
        return;
    }
    return gt_disp_load_scr(scr);
}

void gt_disp_load_scr_anim(gt_obj_st * scr, gt_scr_anim_type_et type, uint32_t time, uint32_t delay, bool del_prev_scr)
{
    gt_disp_st * disp = gt_disp_get_default();
    gt_obj_st * scr_old = gt_disp_get_scr();
    if (!disp) {
        return;
    }
    if (false == gt_refr_is_enabled(disp)) {
        return;
    }
    if (false == gt_obj_is_type(scr, GT_TYPE_SCREEN)) {
        GT_LOGE(GT_LOG_TAG_UI, "Invalid screen: %p(%d)", scr, gt_obj_class_get_type(scr));
        return;
    }
#if !GT_USE_SCREEN_ANIM
    type = GT_SCR_ANIM_TYPE_NONE;
#endif
    _gt_indev_reset_status();
    _gt_disp_refr_reset_areas();
    _gt_scr_stack_clean_unused_pop_scrs(scr_old);
    /** A previous switch animation may still be running (rapid consecutive
     *  switches). disp->scr_prev holds its old screen, already marked delate;
     *  the upcoming anim start_cb overwrites scr_prev before the old anim's
     *  ready_cb can free it, which would leak that screen. Free it now, unless
     *  it is the very screen we are about to load (e.g. go_back to a screen
     *  that is still the old animation's scr_prev), in which case it is reused
     *  and must stay alive. */
    if (disp->scr_prev && disp->scr_prev->delate && disp->scr_prev != scr_old && disp->scr_prev != scr) {
        disp->scr_prev->using_sta = 0;
        _gt_obj_class_destroy(disp->scr_prev);
        disp->scr_prev = NULL;
    }
    if (NULL == scr_old) {
        if (GT_SCR_ANIM_TYPE_NONE != type) {
            type = GT_SCR_ANIM_TYPE_NONE;
        }
    } else if (scr && scr_old == scr) {
        /** reload current screen */
        gt_disp_invalid_area(scr);
        _gt_indev_check_reset_widgets_illegal_status(scr);
        return;
    }
#if GT_LAYER_TOP_SCREENSHOT_CACHE
    disp->screenshot = false;
#endif
    if (scr_old) { gt_event_send(scr_old, GT_EVENT_TYPE_SCREEN_HIDE, NULL); }
    scr->using_sta = true;
    disp->scr_anim_type = type;

    if (GT_SCR_ANIM_TYPE_NONE == type) {
        if( scr_old != scr ){
            disp->area_act.x = scr->area.x;
            disp->area_act.y = scr->area.y;

            /** reset screen range of activity */
            _gt_disp_reload_max_area(scr);
        }
        gt_disp_set_scr(scr);

        disp->area_act.h = gt_disp_get_res_ver(disp);
        disp->area_act.w = gt_disp_get_res_hor(disp);
#if GT_USE_UD_LR_TO_CONTROL_FOCUS_EN
        gt_indev_release_focus_lock();
#endif
        gt_event_send(scr, GT_EVENT_TYPE_SCREEN_LOAD_BEFORE, NULL);
        gt_disp_ref_area(&disp->area_act);

        if (del_prev_scr && scr_old && scr_old != scr) {
            gt_anim_st anim_del;
            gt_anim_init(&anim_del);
            gt_anim_set_time(&anim_del, 0);
            gt_anim_set_target(&anim_del, scr_old);
            gt_anim_set_ready_cb(&anim_del, _scr_anim_del_ready_cb);
            if (NULL == gt_anim_start(&anim_del)) {
                GT_CHECK_PRINT(gt_anim_start(&anim_del));
            }
        }
    }
#if GT_USE_SCREEN_ANIM
    else {
        gt_anim_st anim_old;
        gt_area_st area = scr_old->area;
        gt_anim_init(&anim_old);
        gt_anim_set_time(&anim_old, time);
        gt_anim_set_data(&anim_old, &area, sizeof(gt_area_st));
        gt_anim_set_time_delay_start(&anim_old, delay);
        if (false == del_prev_scr) {
            gt_anim_set_ready_cb(&anim_old, _old_scr_anim_ready_cb);
        }
        gt_anim_set_target(&anim_old, scr_old);

        gt_anim_st anim_new;
        gt_anim_init(&anim_new);
        gt_anim_set_time(&anim_new, time);
        gt_anim_set_time_delay_start(&anim_new, delay);
        gt_anim_set_target(&anim_new, scr);
        gt_anim_set_start_cb(&anim_new, _scr_anim_start_cb);
        gt_anim_set_ready_cb(&anim_new, _scr_anim_ready_cb);
        gt_anim_set_path_type(&anim_old, GT_ANIM_PATH_TYPE_EASE_IN_OUT);
        gt_anim_set_path_type(&anim_new, GT_ANIM_PATH_TYPE_EASE_IN_OUT);

        gt_scr_anim_type_et anim_type = disp->scr_anim_type;
        bool is_hor     = _is_anim_type_hor(anim_type);
        bool is_move    = (anim_type < GT_SCR_ANIM_TYPE_COVER_LEFT);
        bool is_uncover = (anim_type >= GT_SCR_ANIM_TYPE_UNCOVER_LEFT);
        /* For MOVE/COVER: odd = LEFT/UP (sign +1 → new enters from positive side);
         * For UNCOVER: odd = LEFT/UP (sign -1 → old exits to negative side) */
        int32_t sign;
        if (is_uncover) {
            sign = (anim_type & 1u) ? -1 : 1;
        } else {
            sign = (anim_type & 1u) ? 1 : -1;
        }
        gt_anim_exec_cb_t exec_cb = is_hor ? _scr_anim_exec_x_cb : _scr_anim_exec_y_cb;
        int32_t res    = is_hor ? (int32_t)disp->drv->res_hor : (int32_t)disp->drv->res_ver;
        int32_t old_v  = is_hor ? area.x : area.y;
        int32_t new_v  = is_hor ? scr->area.x : scr->area.y;
        int32_t offset = sign * res;

        if (is_uncover) {
            /* UNCOVER: old screen + children slide out together */
            gt_anim_exec_cb_t uncover_cb = is_hor ? _scr_anim_uncover_exec_x_cb : _scr_anim_uncover_exec_y_cb;
            gt_anim_set_exec_cb(&anim_old, uncover_cb);
            gt_anim_set_value(&anim_old, old_v, old_v + offset);
        } else {
            gt_anim_set_exec_cb(&anim_new, exec_cb);
            gt_anim_set_value(&anim_new, new_v - offset, new_v);
            exec_cb(scr, new_v - offset);
            if (is_move) {
                gt_anim_set_exec_cb(&anim_old, exec_cb);
                gt_anim_set_value(&anim_old, old_v, old_v + offset);
            }
        }

        gt_anim_start(&anim_new);
        gt_anim_start(&anim_old);

        if (del_prev_scr) {
            /** When the animation is finished, the memory is freed */
            scr_old->delate = true;
            /** Mark the whole subtree so a nested destroy during the anim
             *  (e.g. deferred listview/status-bar delete) is not scheduled twice. */
            gt_obj_child_set_prop(scr_old, GT_OBJ_PROP_TYPE_DELATE, true);
        }
    }
#endif
    _gt_indev_check_reset_widgets_illegal_status(scr);
}

void gt_disp_load_scr_anim_thread_safe(gt_obj_st * scr, gt_scr_anim_type_et type, uint32_t time, uint32_t delay, bool del_prev_scr)
{
    if (false == gt_task_check_idle_timeout(_GT_DISP_THREAD_SAFE_TIMEOUT)) {
        return;
    }
    return gt_disp_load_scr_anim(scr, type, time, delay, del_prev_scr);
}

void gt_disp_set_scr(gt_obj_st * scr)
{
    gt_disp_st * disp = gt_disp_get_default();
    if( NULL == disp ){
        return;
    }
    disp->scr_act = scr;
}

gt_obj_st * gt_disp_get_scr(void)
{
    gt_disp_st * disp = gt_disp_get_default();
    if( NULL == disp ){
        return NULL;
    }
    return disp->scr_act;
}

#if GT_USE_LAYER_TOP
gt_obj_st * gt_disp_get_layer_top(void)
{
    gt_disp_st * disp = gt_disp_get_default();
    if( NULL == disp ){
        return NULL;
    }
    return disp->layer_top;
}

gt_res_t gt_disp_destroy_layer_top_widgets(void)
{
    gt_obj_st * layer_top = gt_disp_get_layer_top();
    if (NULL == layer_top) {
        return GT_RES_FAIL;
    }
    for (gt_size_t i = layer_top->cnt_child - 1; i >= 0; i--) {
        if (gt_obj_is_type(layer_top->child[i], GT_TYPE_STATUS_BAR)) {
            continue;
        }
        if (gt_obj_is_type(layer_top->child[i], GT_TYPE_DIALOG)) {
            continue;
        }
        gt_obj_child_set_prop(layer_top->child[i], GT_OBJ_PROP_TYPE_VISIBLE, GT_INVISIBLE);
        layer_top->child[i]->visible = GT_INVISIBLE;
    }
    _gt_timer_st * tmp_timer = _gt_timer_create(_layer_top_destroy_handler_cb, GT_TASK_PERIOD_TIME_DESTROY, layer_top);
    if (NULL == tmp_timer) {
        return GT_RES_FAIL;
    }
    _gt_timer_set_repeat_count(tmp_timer, 1);
    return GT_RES_OK;
}

bool gt_disp_layer_top_has_active_obj(void)
{
    gt_obj_st * layer_top = gt_disp_get_layer_top();
    if (NULL == layer_top) {
        return false;
    }
    for (gt_size_t i = layer_top->cnt_child - 1; i >= 0; i--) {
        if (gt_obj_is_type(layer_top->child[i], GT_TYPE_STATUS_BAR)) {
            continue;
        }
        if (gt_obj_get_visible(layer_top->child[i])) {
            return true;
        }
    }
    return false;
}
#endif

void gt_disp_ref_area(const gt_area_st * coords)
{
    uint16_t scr_width = gt_disp_get_res_hor(NULL);
    uint16_t scr_height = gt_disp_get_res_ver(NULL);
    _flush_scr_param_st param = {
        .disp = gt_disp_get_default(),
        .area_flush = *coords,
#if GT_USE_SCREEN_ANIM
        .view_scr_abs = {0, 0, scr_width, scr_height},
        .view_scr_prev_abs = {0, 0, scr_width, scr_height},
#endif
        .valid = {
#if GT_USE_SCREEN_ANIM
            .area_clip = { 0, 0, 0, 0 },
#endif
            .is_hor    = false,
            .layer_top = false,
        },
        .line = GT_REFRESH_FLUSH_LINE_PRE_TIME,
    };
    if (false == gt_refr_is_enabled(param.disp)) {
        return;
    }

    gt_obj_type_et type = gt_obj_class_get_type(param.disp->scr_act);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    if ( _gt_disp_get_state(param.disp) == GT_BUSY ) {
        GT_LOGD(GT_LOG_TAG_GUI, "disp is busy");
        return;
    }
    _gt_disp_set_state(param.disp, GT_BUSY);

    param.disp->area_disp.w = scr_width;
    param.disp->area_disp.h = param.line;

#if GT_USE_SCREEN_ANIM
    if (param.disp->scr_prev) {
        param.area_flush.x = 0;
        param.area_flush.y = 0;
        param.area_flush.w = scr_width;
        param.area_flush.h = GT_REFRESH_FLUSH_LINE_PRE_TIME;
        _flush_scr_by_anim(&param);
    } else {
        _flush_scr_by_direct(&param);
    }
#else
    _flush_scr_by_direct(&param);
#endif

    // last line
    gt_disp_drv_set_flushing(param.disp->drv, false);
    _gt_disp_set_state(param.disp, GT_NOT_BUSY);
}

gt_area_st * gt_disp_get_area_act(void)
{
    gt_disp_st * disp_dev = gt_disp_get_default();
    GT_CHECK_BACK_VAL(disp_dev, NULL);
    return &disp_dev->area_act;
}

gt_area_abs_st * gt_disp_get_area_max(void)
{
    gt_disp_st * disp_dev = gt_disp_get_default();
    GT_CHECK_BACK_VAL(disp_dev, NULL);
    return &disp_dev->area_max;
}

void gt_disp_scroll_area_act(gt_size_t dist_x, gt_size_t dist_y)
{
    gt_area_st * area = gt_disp_get_area_act();
    gt_area_abs_st * max_area = gt_disp_get_area_max();

    area->x = dist_x;
    area->y = dist_y;

    /** Limit the range where the display can move */
    if (area->x < max_area->left) {
        area->x = max_area->left;
    } else if (area->x + area->w > max_area->right) {
        if (area->w < max_area->right) {
            area->x = max_area->right - area->w;
        } else if (area->x > 0) {
            area->x = 0;
        }
    }

    /** Limit the range where the display can move */
    if (area->y < max_area->top) {
        area->y = max_area->top;
    } else if (area->y + area->h > max_area->bottom) {
        if (area->h < max_area->bottom) {
            area->y = max_area->bottom - area->h;
        } else if (area->y > 0) {
            area->y = 0;
        }
    }

    _gt_disp_refr_reset_areas();
    _gt_disp_refr_append_area(area);
}

void gt_disp_invalid_area(gt_obj_st * obj)
{
    gt_disp_st * disp = gt_disp_get_default();
    if (NULL == disp || !disp->enabled) {
        return;
    }
    gt_obj_st * scr = disp->scr_act;
    if (NULL == scr) {
        return;
    }
    if (NULL == obj) {
        obj = scr;
        switch (disp->drv->rotated) {
            case GT_ROTATED_90:
            case GT_ROTATED_270:
                obj->area.w = disp->drv->res_ver;
                obj->area.h = disp->drv->res_hor;
                break;
            default:
                obj->area.w = disp->drv->res_hor;
                obj->area.h = disp->drv->res_ver;
                break;
        }
    }
    gt_area_st invalid = gt_area_reduce(obj->area, -(gt_obj_get_focus_offset_value(obj)));
#if GT_USE_LAYER_TOP
    gt_obj_st * top = disp->layer_top;

    if (top && gt_obj_is_child(obj, top)) {
        invalid.x += scr->area.x;
        invalid.y += scr->area.y;
    }
    else if (!gt_obj_check_scr(obj)) {
        return;
    }
#else
    if (!gt_obj_check_scr(obj)) {
        return;
    }
#endif
#if GT_LAYER_TOP_SCREENSHOT_CACHE
    else {
        _gt_top_ret_st top_show = _gt_disp_is_top_full_cover_display(top);
        bool is_half_opa = top_show.full_cover && top_show.is_opa;
        bool is_screenshot_vaild = _gt_gc_screenshot_cache_is_valid();
        if (is_half_opa && is_screenshot_vaild) {
            gt_memcpy(&invalid, &scr->area, sizeof(gt_area_st));
            disp->screenshot = false;
        }
    }
#endif
    _gt_disp_refr_append_area(&invalid);
}

void gt_disp_force_release_busy(void)
{
    gt_disp_st * disp = gt_disp_get_default();
    if (NULL == disp) {
        return;
    }
    _gt_disp_set_state(disp, GT_NOT_BUSY);
}

/* end ------------------------------------------------------------------*/
