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
#include "../core/gt_mem.h"
#include "../others/gt_types.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"
#include "../core/gt_fs.h"
#include "../others/gt_log.h"
#include "../core/gt_obj_pos.h"
#include "../hal/gt_hal_tick.h"

#if GT_CFG_ENABLE_PLAYER
/* private define -------------------------------------------------------*/
#define OBJ_TYPE GT_TYPE_PLAYER
#define MY_CLASS &gt_player_class

/**
 * @brief The register bit length of the player
 * uint16_t reg;
 */
#define _GT_PLAYER_BIT_TYPE_LENGTH      3
#define _GT_PLAYER_BIT_MODE_LENGTH      3
#define _GT_PLAYER_BIT_STATUS_LENGTH    2
#define _GT_PLAYER_BIT_AUTO_LENGTH      1
#define _GT_PLAYER_BIT_LOCK_LENGTH      1
#define _GT_PLAYER_BIT_DIR_LENGTH       1
#define _GT_PLAYER_BIT_REAL_TIME_UPDATE_LENGTH 1
#define _GT_PLAYER_BIT_RESERVED_LENGTH  4


#define _GET_BIT_SHIFT_SIZE(_val)       (1 << (_val))

/* private typedef ------------------------------------------------------*/

typedef struct _gt_player_item_s {
    void * src;
    uint16_t test;
#if _GT_PLAYER_USE_CUSTOM_POS
    gt_area_st area;
#endif
}_gt_player_item_st;

/**
 * @brief process object
 */
typedef struct _gt_player_obj_s {
    gt_size_t index;                ///< the current index of item list
    uint16_t count;                 ///< the count of items
    _gt_player_item_st ** list;      ///< the list of items
}_gt_player_obj_st;

/**
 * @brief control's register
 */
typedef struct _gt_player_reg_s {
    uint16_t type: _GT_PLAYER_BIT_TYPE_LENGTH;          ///< @ref gt_player_type_et
    uint16_t mode: _GT_PLAYER_BIT_MODE_LENGTH;          ///< @ref gt_player_mode_et
    uint16_t status: _GT_PLAYER_BIT_STATUS_LENGTH;      ///< status of the player; play, pause, stop
    uint16_t auto_play: _GT_PLAYER_BIT_AUTO_LENGTH;     ///< 0: manual, 1: auto
    uint16_t lock: _GT_PLAYER_BIT_LOCK_LENGTH;          ///< lock the player callback handler, 0: unlock, 1: lock
    uint16_t dir: _GT_PLAYER_BIT_DIR_LENGTH;            ///< direction of the player, 0: forward, 1: backward
    uint16_t real_time_update: _GT_PLAYER_BIT_REAL_TIME_UPDATE_LENGTH;  ///< 1: real time update, 0: update when the player is displayed
    uint16_t reserved: _GT_PLAYER_BIT_RESERVED_LENGTH;  ///< reserved bit length
}_gt_player_reg_st;

typedef struct _gt_player_s {
    uint32_t last_player_tick;
    _gt_player_reg_st reg;
    _gt_player_obj_st target;
    _gt_timer_st * timer;
}_gt_player_st;



/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_player_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_player_st)
};


static void _turn_prev_item(gt_obj_st * obj, bool is_loop);
static void _turn_next_item(gt_obj_st * obj, bool is_loop);
/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
/**
 * @brief Get the src object from the list item
 *
 * @param style
 * @return void*
 */
static inline void * _get_src(_gt_player_st * style) {
    return style->target.list[style->target.index]->src;
}

/**
 * @brief Lock the player callback  handler
 *
 * @param style
 * @return true Set lock status success
 * @return false Unable to set lock status
 */
static inline bool _lock(_gt_player_st * style) {
    if (NULL == style) {
        return false;
    }
    if (style->reg.lock) {
        return false;
    }
    style->reg.lock = true;
    return true;
}

/**
 * @brief Unlock the player callback handler
 *
 * @param style
 * @return true Set unlock status success
 * @return false Unable to set unlock status
 */
static inline bool _unlock(_gt_player_st * style) {
    if (NULL == style) {
        return false;
    }
    if (!style->reg.lock) {
        return false;
    }
    style->reg.lock = false;
    return true;
}

/**
 * @brief Get The player display direction
 *
 * @param style
 * @return true : Go next item direction;
 * @return false : Go previous item direction;
 */
static inline bool _is_dir_next(_gt_player_st * style) {
    return style->reg.dir ? false : true;
}

/**
 * @brief Set the player display direction
 *
 * @param style
 * @param is_next true: Go next item direction; false: Go previous item direction
 */
static inline void _set_dir(_gt_player_st * style, bool is_next) {
    style->reg.dir = is_next ? 0 : 1;
}

static inline void _default_init(gt_obj_st * obj, _gt_player_st * style) {
    void * src = _get_src(style);
    gt_attr_rect_st dsc = {
        .bg_img_src = src,
        .bg_opa = GT_OPA_COVER,
    };
    /* start draw obj */
    draw_bg_img(obj->draw_ctx, &dsc, &obj->area);

    // focus
    draw_focus(obj , 0);
}

static void _init_cb(gt_obj_st * obj) {
    _gt_player_st * style = (_gt_player_st * )obj->style;
    _unlock(style);
    switch (style->reg.type)
    {
        default:
            _default_init(obj, style);
            break;
    }

}

/**
 * @brief Deinit the player timer object, and free the memory
 *
 * @param style
 */
static void _destroy_timer(_gt_player_st * style) {
    if ( !style->timer ) {
        return ;
    }
    _gt_timer_set_paused(style->timer, true);
    _gt_timer_del(style->timer);
    style->timer = NULL;
}

/**
 * @brief Frees the internal dynamic memory of the elements in the list
 *
 * @param style
 */
static void _destroy_type_default(_gt_player_st * style) {
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

    _gt_player_st ** style_p = (_gt_player_st ** )&obj->style;
    if (NULL == *style_p) {
        return ;
    }

    switch ((*style_p)->reg.type) {
        default:
            _destroy_type_default(*style_p);
            break;
    }

    _destroy_timer(*style_p);
    gt_mem_free(*style_p);
    *style_p = NULL;
}

/**
 * @brief Refresh the display area
 *
 * @param obj
 * @param style
 */
static void _invalid_area(gt_obj_st * obj, _gt_player_st * style) {
    void * src = _get_src(style);
    uint16_t w = 0, h = 0;
    gt_fs_read_img_wh((char * )src, &w, &h);
    obj->area.w = GT_MAX(obj->area.w, w);
    obj->area.h = GT_MAX(obj->area.h, h);
    gt_disp_invalid_area(obj);
}

static inline bool _is_mode_equal(_gt_player_st * style, gt_player_mode_et mode) {
    return style->reg.mode == mode;
}

static inline bool _is_type_none(_gt_player_st * style) {
    return style->reg.type == GT_PLAYER_TYPE_NONE;
}

/**
 * @brief Turn to the previous item
 *
 * @param obj
 */
static void _turn_prev_item(gt_obj_st * obj, bool is_loop) {
    _gt_player_st * style = (_gt_player_st * )obj->style;
    if (NULL == style) {
        return ;
    }
    if (NULL == style->target.list) {
        return ;
    }
    if (style->target.index > 0) {
        --style->target.index;
        _invalid_area(obj, style);
        return ;
    }
    if (_is_mode_equal(style, GT_PLAYER_MODE_ONCE)) {
        _destroy_timer(style);
        return ;
    }
    if (_is_mode_equal(style, GT_PLAYER_MODE_PLAYBACK)) {
        gt_player_dir_toggle(obj);
        _turn_next_item(obj, is_loop);
        return ;
    }
    if (!is_loop) {
        return ;
    }
    style->target.index = style->target.count - 1;
    _invalid_area(obj, style);
}

/**
 * @brief Turn to the next item
 *
 * @param obj
 */
static void _turn_next_item(gt_obj_st * obj, bool is_loop) {
    _gt_player_st * style = (_gt_player_st * )obj->style;
    if (NULL == style) {
        return ;
    }
    if (NULL == style->target.list) {
        return ;
    }
    if (style->target.index + 1 < style->target.count) {
        ++style->target.index;
        _invalid_area(obj, style);
        return ;
    }
    if (_is_mode_equal(style, GT_PLAYER_MODE_ONCE)) {
        _destroy_timer(style);
        return ;
    }
    if (_is_mode_equal(style, GT_PLAYER_MODE_PLAYBACK)) {
        gt_player_dir_toggle(obj);
        _turn_prev_item(obj, is_loop);
        return ;
    }

    if (!is_loop) {
        return ;
    }
    style->target.index = 0;
    _invalid_area(obj, style);
}

static inline void _change_item_handler(gt_obj_st * obj, bool is_next) {
    if (NULL == obj) {
        return ;
    }
    if (NULL == obj->style) {
        return ;
    }

    _gt_player_st * style = (_gt_player_st * )obj->style;
    bool is_loop = true;

    switch (style->reg.mode)
    {
        case GT_PLAYER_MODE_NONE:
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

static void _event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code = gt_event_get_code(e);
    switch (code)
    {
        case GT_EVENT_TYPE_UPDATE_STYLE:
        case GT_EVENT_TYPE_DRAW_START: {
            _invalid_area(obj, (_gt_player_st * )obj->style);
            break;
        }
        case GT_EVENT_TYPE_DRAW_END: {
            break;
        }
        case GT_EVENT_TYPE_DRAW_REDRAW: {
            break;
        }
        case GT_EVENT_TYPE_INPUT_RELEASED: {
            break;
        }
        case GT_EVENT_TYPE_INPUT_PRESSING: {
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

    _gt_player_st * style = (_gt_player_st * )obj->style;
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

static inline void _remove_item(_gt_player_st * style, uint16_t idx) {
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

static void _auto_play_callback(struct _gt_timer_s * timer) {
    gt_obj_st * obj = (gt_obj_st * )_gt_timer_get_user_data(timer);
    if (NULL == obj) {
        return;
    }
    _gt_player_st * style = (_gt_player_st * )obj->style;
    if (!_lock(style)) {
        /** The callback has been executed, waiting for it to be displayed */
        return ;
    }
    uint16_t count = 1;
    if (style->reg.real_time_update) {
        count = gt_tick_elapse(style->last_player_tick) / _gt_timer_get_period(timer);
    }
    for (uint16_t i = 0; i < count; i++) {
        _change_item_handler(obj, _is_dir_next(style));
    }
    style->last_player_tick =  gt_tick_get();
}

static inline void _set_play_status(_gt_player_st * style, bool is_play) {
    if (NULL == style) {
        return ;
    }
    if (!style->timer) {
        return ;
    }
    if (NULL == style->target.list) {
        return ;
    }
    if (style->target.count < 2) {
        return ;
    }
    style->last_player_tick = gt_tick_get();
    _gt_timer_set_paused(style->timer, is_play ? false : true);
}

/* global functions / API interface -------------------------------------*/

gt_obj_st * gt_player_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    _gt_player_st * style = (_gt_player_st * )obj->style;
    gt_memset(style, 0, sizeof(_gt_player_st));

    return obj;
}

void gt_player_set_type(gt_obj_st * obj, gt_player_type_et type)
{
    if (type >= GT_PLAYER_TYPE_MAX_COUNT || type < 0) {
        return ;
    }
    if (type > _GET_BIT_SHIFT_SIZE(_GT_PLAYER_BIT_TYPE_LENGTH)) {
        type = GT_PLAYER_TYPE_NONE;
    }
    ((_gt_player_st * )(obj->style))->reg.type = type;
}

void gt_player_set_mode(gt_obj_st * obj, gt_player_mode_et mode)
{
    if (mode >= GT_PLAYER_MODE_MAX_COUNT || mode < 0) {
        return ;
    }
    if (mode > _GET_BIT_SHIFT_SIZE(_GT_PLAYER_BIT_MODE_LENGTH)) {
        mode = GT_PLAYER_MODE_NONE;
    }
    ((_gt_player_st * )(obj->style))->reg.mode = mode;
}

void gt_player_set_auto_play_period(gt_obj_st * obj, uint32_t period)
{
    if (NULL == obj) {
        return ;
    }
    if (NULL == obj->style) {
        return ;
    }
    _gt_player_st * style = (_gt_player_st * )obj->style;
    style->reg.auto_play = period ? 1 : 0;

    if (!style->reg.auto_play) {
        _destroy_timer(style);
        return ;
    }
    if (style->timer) {
        _gt_timer_set_period(style->timer, period);
        return ;
    }
    style->timer = _gt_timer_create(_auto_play_callback, period, (void * )obj);
    _set_play_status(style, false);
}

void gt_player_remove_item(gt_obj_st * obj, void * item)
{
    if (NULL == obj) {
        return ;
    }
    if (NULL == item) {
        return ;
    }

    _gt_player_st * style = (_gt_player_st * )obj->style;
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

void gt_player_remove_item_by_index(gt_obj_st * obj, uint16_t idx) {
    if (NULL == obj) {
        return ;
    }

    _gt_player_st * style = (_gt_player_st * )obj->style;
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
    _gt_player_st * style = (_gt_player_st * )obj->style;
    style->target.list[idx]->area = *area;
    return count;
}
#else
uint16_t gt_player_add_item(gt_obj_st * obj, void * item, uint16_t item_byte_size)
{
    return _add_item(obj, item, item_byte_size);
}
#endif

void gt_player_set_index(gt_obj_st * obj, gt_size_t index)
{
    if (NULL == obj) {
        return ;
    }
    _gt_player_st * style = (_gt_player_st * )obj->style;
    if (NULL == style) {
        return ;
    }
    if (NULL == style->target.list) {
        return ;
    }
    if (index + 1 > style->target.count) {
        return ;
    }
    style->target.index = (-1 == index) ? (style->target.count - 1) : index;
}

gt_size_t gt_player_get_index(gt_obj_st * obj)
{
    return ((_gt_player_st * )obj->style)->target.index;
}

uint16_t gt_player_get_item_count(gt_obj_st * obj)
{
    return ((_gt_player_st * )obj->style)->target.count;
}

float gt_player_get_percentage(gt_obj_st * obj)
{
    _gt_player_st * style = (_gt_player_st * )obj->style;
    if (!style->target.count) {
        return 0.0;
    }
    if (style->target.index + 1 > style->target.count) {
        return 0.0;
    }
    return (style->target.index + 1) * 100.0 / style->target.count;
}

void gt_player_turn_prev(gt_obj_st * obj)
{
    if (_is_type_none((_gt_player_st * )obj->style)) {
        GT_LOGV(GT_LOG_TAG_GUI, "player type is none, please set it.");
        return ;
    }
    _change_item_handler(obj, false);
}

void gt_player_turn_next(gt_obj_st * obj)
{
    if (_is_type_none((_gt_player_st * )obj->style)) {
        GT_LOGV(GT_LOG_TAG_GUI, "player type is none, please set it.");
        return ;
    }
    _change_item_handler(obj, true);
}

void gt_player_play(gt_obj_st * obj)
{
    _gt_player_st * style = (_gt_player_st * )obj->style;
    if (_is_type_none(style)) {
        GT_LOGV(GT_LOG_TAG_GUI, "player type is none, please set it.");
        return ;
    }
    _set_play_status(style, true);
}

void gt_player_stop(gt_obj_st * obj)
{
    _gt_player_st * style = (_gt_player_st * )obj->style;
    if (_is_type_none(style)) {
        GT_LOGV(GT_LOG_TAG_GUI, "player type is none, please set it.");
        return ;
    }
    _set_play_status(style, false);
}

void gt_player_toggle(gt_obj_st * obj)
{
    _gt_player_st * style = (_gt_player_st * )obj->style;
    if (_is_type_none(style)) {
        GT_LOGV(GT_LOG_TAG_GUI, "player type is none, please set it.");
        return ;
    }
    if (!style->timer) {
        return ;
    }
    bool is_stop = _gt_timer_get_paused(style->timer);
    _set_play_status(style, is_stop ? true : false);
}

void gt_player_dir_toggle(gt_obj_st * obj)
{
    _gt_player_st * style = (_gt_player_st * )obj->style;
    if (_is_type_none(style)) {
        GT_LOGV(GT_LOG_TAG_GUI, "player type is none, please set it.");
        return ;
    }
    _set_dir(style, _is_dir_next(style) ? false : true);
}

bool gt_player_dir_is_forward(gt_obj_st * obj)
{
    if (NULL == obj) {
        return false;
    }
    _gt_player_st * style = (_gt_player_st * )obj->style;
    return _is_dir_next(style);
}

void gt_player_set_real_time_update(gt_obj_st * obj, bool is_real_time_update)
{
    if (NULL == obj) {
        return ;
    }
    _gt_player_st * style = (_gt_player_st * )obj->style;
    style->reg.real_time_update = is_real_time_update;
}

bool gt_player_is_real_time_update(gt_obj_st * obj)
{
    if (NULL == obj) {
        return false;
    }
    _gt_player_st * style = (_gt_player_st * )obj->style;
    return style->reg.real_time_update;
}

#endif /** GT_CFG_ENABLE_PLAYER */
/* end ------------------------------------------------------------------*/
