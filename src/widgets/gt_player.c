/**
 * @file gt_player.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2023-05-04 14:22:18
 * @copyright Copyright (c) 2014-2023, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_player.h"
#include "stdbool.h"
#include "../core/gt_mem.h"
#include "../others/gt_types.h"
#include "../core/gt_draw.h"
#include "../core/gt_fs.h"
#include "../others/gt_log.h"
#include "../core/gt_obj_pos.h"

#if GT_CFG_ENABLE_PLAYER
/* private define -------------------------------------------------------*/
#define OBJ_TYPE GT_TYPE_PLAYER
#define MY_CLASS &gt_player_class

/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct _gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_player_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_instance = sizeof(gt_obj_st),
    .size_style    = sizeof(_gt_style_player_st)
};


/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
/**
 * @brief Get the src object from the list item
 *
 * @param style
 * @return void*
 */
static inline void * _get_src(_gt_style_player_st * style) {
    return style->target.list[style->target.index]->src;
}

static inline void _default_init(gt_obj_st * obj, _gt_style_player_st * style) {
    void * src = _get_src(style);
    gt_fs_read_img_wh((char * )src, &obj->area.w, &obj->area.h);

    gt_attr_rect_st dsc = {
        .bg_img_src = src,
        .bg_opa = GT_OPA_COVER,
    };
    /* start draw obj */
    draw_bg_img(obj->draw_ctx, &dsc, &obj->area);
}

static void _init_cb(gt_obj_st * obj) {
    _gt_style_player_st * style = (_gt_style_player_st * )obj->style;
    switch (style->reg.type)
    {
        default:
            _default_init(obj, style);
            break;
    }

}

/**
 * @brief Frees the internal dynamic memory of the elements in the list
 *
 * @param style
 */
static void _destroy_type_default(_gt_style_player_st * style) {
    if (NULL == style) {
        return ;
    }
    if (NULL == style->target.list) {
        return ;
    }
    for (gt_size_t i = style->target.count - 1; i >= 0; i--) {
        if (NULL == style->target.list[i]) {
            continue;
        }
        if (style->target.list[i]->src) {
            gt_mem_free(style->target.list[i]->src);
            style->target.list[i]->src = NULL;
        }
        gt_mem_free(style->target.list[i]);
        style->target.list[i] = NULL;
    }
    gt_mem_free(style->target.list);
    gt_memset(&style->target, 0, sizeof(_gt_player_obj_st));
}

static void _deinit_cb(gt_obj_st * obj) {
    if (NULL == obj) {
        return ;
    }

    _gt_style_player_st ** style_p = (_gt_style_player_st ** )&obj->style;
    if (NULL == *style_p) {
        return ;
    }

    switch ((*style_p)->reg.type) {
        default:
            _destroy_type_default(*style_p);
            break;
    }

    gt_mem_free(*style_p);
    *style_p = NULL;
}

/**
 * @brief Refresh the display area
 *
 * @param obj
 * @param style
 */
static void _invalid_area(gt_obj_st * obj, _gt_style_player_st * style) {
    void * src = _get_src(style);
    uint16_t w = 0, h = 0;
    gt_fs_read_img_wh((char * )src, &w, &h);
    obj->area.w = GT_MAX(obj->area.w, w);
    obj->area.h =  GT_MAX(obj->area.h, h);
    if( gt_obj_check_scr(obj) ){
        _gt_disp_refr_append_area(&obj->area);
    }
}

/**
 * @brief Turn to the previous item
 *
 * @param obj
 */
static void _turn_prev_item(gt_obj_st * obj, bool is_loop) {
    _gt_style_player_st * style = (_gt_style_player_st * )obj->style;
    if (NULL == style) {
        return ;
    }
    if (NULL == style->target.list) {
        return ;
    }
    if (style->target.index > 0) {
        --style->target.index;
    } else {
        if (!is_loop) { return ; }
        style->target.index = style->target.count - 1;
    }
    _invalid_area(obj, style);
}

/**
 * @brief Turn to the next item
 *
 * @param obj
 */
static void _turn_next_item(gt_obj_st * obj, bool is_loop) {
    _gt_style_player_st * style = (_gt_style_player_st * )obj->style;
    if (NULL == style) {
        return ;
    }
    if (NULL == style->target.list) {
        return ;
    }
    if (style->target.index + 1 < style->target.count) {
        ++style->target.index;
    } else {
        if (!is_loop) { return ; }
        style->target.index = 0;
    }
    _invalid_area(obj, style);
}

static inline void _change_item_handler(gt_obj_st * obj, bool is_next) {
    _gt_style_player_st * style = (_gt_style_player_st * )obj->style;
    bool is_loop = true;

    switch (style->reg.mode)
    {
        case GT_PLAYER_MODE_ONCE: {
            is_loop = false;
            break;
        }
        default:
            break;
    }

    if (is_next) {
        _turn_next_item(obj, is_loop);
    } else {
        _turn_prev_item(obj, is_loop);
    }
}

static void _event_cb(struct _gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code = gt_event_get_code(e);
    switch (code)
    {
        case GT_EVENT_TYPE_UPDATE_STYLE:
        case GT_EVENT_TYPE_DRAW_START: {
            _invalid_area(obj, (_gt_style_player_st * )obj->style);
            break;
        }
        case GT_EVENT_TYPE_DRAW_END: {
            break;
        }
        case GT_EVENT_TYPE_DRAW_REDRAW: {
            break;
        }
        case GT_EVENT_TYPE_INPUT_PROCESSED: {
            break;
        }
        case GT_EVENT_TYPE_INPUT_CLICKING: {
            break;
        }
        default:
            break;
    }
}

/**
 * @brief Create player object, or add item to player object
 *
 * @param obj
 * @param item
 * @param item_byte_size
 * @return uint16_t
 */
static inline uint16_t _add_item(gt_obj_st * obj, void * item, uint16_t item_byte_size) {
    if (NULL == obj) {
        return 0;
    }
    if (NULL == item || 0 == item_byte_size) {
        return 0;
    }

    _gt_style_player_st * style = (_gt_style_player_st * )obj->style;
    if (NULL == style) {
        return 0;
    }
    if (style->target.list) {
        style->target.list = gt_mem_realloc(style->target.list, sizeof(_gt_player_item_st * ) * (style->target.count + 1));
    } else {
        style->target.count = 0;
        style->target.list = gt_mem_malloc(sizeof(_gt_player_item_st * ));
    }

    _gt_player_item_st ** list = style->target.list;
    list[style->target.count] = gt_mem_malloc(sizeof(_gt_player_item_st));
    gt_memset(list[style->target.count], 0, sizeof(_gt_player_item_st));

    list[style->target.count]->src = gt_mem_malloc(item_byte_size);
    gt_memcpy(list[style->target.count]->src, item, item_byte_size);
    return ++style->target.count;
}

static inline void _remove_item(_gt_style_player_st * style, uint16_t idx) {
    if (!style->target.count) {
        return ;
    }
    gt_mem_free(style->target.list[idx]->src);
    style->target.list[idx]->src = NULL;

    gt_mem_free(style->target.list[idx]);
    style->target.list[idx] = NULL;

    if (idx != style->target.count--) {
        gt_memmove(&style->target.list[idx], &style->target.list[idx + 1], (style->target.count - idx) * sizeof(_gt_player_item_st));
    }
    style->target.list = gt_mem_realloc(style->target.list, style->target.count *  sizeof(_gt_player_item_st));
}

/* global functions / API interface -------------------------------------*/

gt_obj_st * gt_player_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    _gt_style_player_st * style = (_gt_style_player_st * )obj->style;
    gt_memset(style, 0, sizeof(_gt_style_player_st));

    return obj;
}

void gt_player_set_type(gt_obj_st * obj, _gt_player_type_et type)
{
    if (type >= GT_PLAYER_TYPE_MAX_COUNT || type < 0) {
        return ;
    }
    ((_gt_style_player_st * )(obj->style))->reg.type = type;
}

void gt_player_set_mode(gt_obj_st * obj, _gt_player_mode_et mode)
{
    if (mode >= GT_PLAYER_MODE_MAX_COUNT || mode < 0) {
        return ;
    }
    ((_gt_style_player_st * )(obj->style))->reg.mode = mode;
}

/**
 * @brief Remove item from player object by item
 *
 * @param obj
 * @param item
 */
void gt_player_remove_item(gt_obj_st * obj, void * item)
{
    if (NULL == obj) {
        return ;
    }
    if (NULL == item) {
        return ;
    }

    _gt_style_player_st * style = (_gt_style_player_st * )obj->style;
    if (NULL == style) {
        return ;
    }

    if (NULL == style->target.list) {
        return ;
    }
    uint16_t idx = 0;
    for (gt_size_t i = style->target.count - 1; i >= 0; i--) {
        if (style->target.list[i]->src == item) {
            idx = i;
            break;
        }
    }
    _remove_item(style, idx);
}

/**
 * @brief Remove item from player object by index
 *
 * @param style
 * @param idx
 */
void gt_player_remove_item_by_index(gt_obj_st * obj, uint16_t idx) {
    if (NULL == obj) {
        return ;
    }

    _gt_style_player_st * style = (_gt_style_player_st * )obj->style;
    if (NULL == style) {
        return ;
    }

    if (NULL == style->target.list) {
        return ;
    }

    if (idx > style->target.count) {
        return ;
    }

    _remove_item(style, idx);
}

#if _GT_PLAYER_USE_CUSTOM_POS
uint16_t gt_player_add_item(gt_obj_st * obj, void * item, uint16_t item_byte_size, gt_area_st * area)
{
    uint16_t count = _add_item(obj, item, item_byte_size);
    if (!count) {
        return count;
    }
    uint16_t idx = count - 1;
    _gt_style_player_st * style = (_gt_style_player_st * )obj->style;
    style->target.list[idx]->area = *area;
    return count;
}
#else
uint16_t gt_player_add_item(gt_obj_st * obj, void * item, uint16_t item_byte_size)
{
    return _add_item(obj, item, item_byte_size);
}
#endif

void gt_player_turn_prev(gt_obj_st * obj)
{
    return _change_item_handler(obj, false);
}

void gt_player_turn_next(gt_obj_st * obj)
{
    return _change_item_handler(obj, true);
}


#endif /** GT_CFG_ENABLE_PLAYER */
/* end ------------------------------------------------------------------*/
