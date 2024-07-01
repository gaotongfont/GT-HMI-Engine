/**
 * @file gt_player.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2023-05-04 14:22:18
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
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
#define _GT_PLAYER_BIT_DEFAULT_DIR_LENGTH 1
#define _GT_PLAYER_BIT_RESERVED_LENGTH  4


#define _GET_BIT_SHIFT_SIZE(_val)       (1 << (_val))

/* private typedef ------------------------------------------------------*/

typedef struct _gt_player_item_s {
    void * src;

#if GT_USE_FILE_HEADER
    gt_file_header_param_st fh_param;
#endif  /** GT_USE_FILE_HEADER */

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
    uint16_t default_dir: _GT_PLAYER_BIT_DEFAULT_DIR_LENGTH;  ///< 0: default direction is forward, 1: direction is backward
    uint16_t reserved: _GT_PLAYER_BIT_RESERVED_LENGTH;  ///< reserved bit length
}_gt_player_reg_st;

typedef struct _gt_player_s {
    gt_obj_st obj;
    uint32_t last_player_tick;
    _gt_player_reg_st reg;
    _gt_player_obj_st target;
    _gt_timer_st * timer;

#if GT_PLAYER_REPEAT_FINISH_CALLBACK
    gt_event_cb_t repeat_finish_cb;
    void * parms;
#endif

    int32_t repeat_cnt;         /** -1: infinite; > 0: repeat count; 0: stop player */
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


static void _turn_prev_item(gt_obj_st * obj);
static void _turn_next_item(gt_obj_st * obj);
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

#if GT_USE_FILE_HEADER
static inline gt_file_header_param_st * _get_file_header_param(_gt_player_st * style) {
    return &style->target.list[style->target.index]->fh_param;
}
#endif

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

static void _init_cb(gt_obj_st * obj) {
    _gt_player_st * style = (_gt_player_st * )obj;
    if (NULL == style->target.list) {
        return;
    }
    if (0 == style->target.count) {
        return;
    }
    void * src = _get_src(style);

    _unlock(style);
    if (GT_PLAYER_TYPE_IMG == style->reg.type) {
        gt_attr_rect_st dsc = {
            .bg_img_src = src,
            .bg_opa = obj->opa,
        };
#if GT_USE_FILE_HEADER
        dsc.file_header = gt_file_header_param_check_valid(_get_file_header_param(style));
#endif
        /* start draw obj */
        draw_bg_img(obj->draw_ctx, &dsc, &obj->area);
    }

    // focus
    draw_focus(obj , 0);
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
    _gt_player_st * style_p = (_gt_player_st * )obj;
    _destroy_timer(style_p);

    if (GT_PLAYER_TYPE_IMG == style_p->reg.type) {
        _destroy_type_default(style_p);
    }
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
    gt_fs_res_et ret = GT_FS_RES_FAIL;
#if GT_USE_FILE_HEADER
    gt_file_header_param_st * param = _get_file_header_param(style);
    if (GT_FS_RES_OK != ret) {
        ret = gt_fs_fh_read_img_wh(param, &w, &h);
    }
#endif
    if (GT_FS_RES_OK != ret) {
        if (GT_FS_RES_FAIL == gt_fs_read_img_wh((char * )src, &w, &h)) {
            return ;
        }
        if (0 == w || 0 == h) {
            return ;
        }
    }
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

static void _toggle_dir_handler(_gt_player_st * style) {
    if (_is_type_none(style)) {
        GT_LOGV(GT_LOG_TAG_GUI, "player type is none, please set it.");
        return ;
    }
    _set_dir(style, _is_dir_next(style) ? false : true);
}

#if GT_PLAYER_REPEAT_FINISH_CALLBACK
static inline void _repeat_finish_callback_handler(_gt_player_st * style) {
    if (NULL == style->repeat_finish_cb) {
        return ;
    }
    gt_event_st e = {
        .code = GT_EVENT_TYPE_NONE,
        .origin = (gt_obj_st * )style,
        .target = (gt_obj_st * )style,
        .param = style->parms,
    };
    style->repeat_finish_cb(&e);
}
#endif

static inline void _check_repeat_count(_gt_player_st * style) {
    if (style->repeat_cnt < 1) {
        return ;
    }
    if (_is_dir_next(style) != style->reg.default_dir) {
        --style->repeat_cnt;
    }
}

/**
 * @brief Turn to the previous item
 *
 * @param obj
 */
static void _turn_prev_item(gt_obj_st * obj) {
    _gt_player_st * style = (_gt_player_st * )obj;
    if (NULL == style->target.list) {
        return ;
    }
    if (style->target.index > 0) {
        if (0 == style->repeat_cnt) {
            _gt_timer_set_paused(style->timer, true);
#if GT_PLAYER_REPEAT_FINISH_CALLBACK
            _repeat_finish_callback_handler(style);
#endif
            return;
        }
        --style->target.index;
        _invalid_area(obj, style);
        return ;
    }
    if (_is_mode_equal(style, GT_PLAYER_MODE_LOOP)) {
        style->target.index = style->target.count - 1;
        if (style->repeat_cnt > 0) {
            --style->repeat_cnt;
        }
        _invalid_area(obj, style);
        return ;
    }
    if (_is_mode_equal(style, GT_PLAYER_MODE_ONCE)) {
        _destroy_timer(style);
        return ;
    }
    if (_is_mode_equal(style, GT_PLAYER_MODE_PLAYBACK)) {
        _toggle_dir_handler(style);
        _check_repeat_count(style);
        _turn_next_item(obj);
        return ;
    }
}

/**
 * @brief Turn to the next item
 *
 * @param obj
 */
static void _turn_next_item(gt_obj_st * obj) {
    _gt_player_st * style = (_gt_player_st * )obj;
    if (NULL == style->target.list) {
        return ;
    }
    if (style->target.index + 1 < style->target.count) {
        if (0 == style->repeat_cnt) {
            _gt_timer_set_paused(style->timer, true);
#if GT_PLAYER_REPEAT_FINISH_CALLBACK
            _repeat_finish_callback_handler(style);
#endif
            return;
        }
        ++style->target.index;
        _invalid_area(obj, style);
        return ;
    }
    if (_is_mode_equal(style, GT_PLAYER_MODE_LOOP)) {
        style->target.index = 0;
        if (style->repeat_cnt > 0) {
            --style->repeat_cnt;
        }
        _invalid_area(obj, style);
        return ;
    }
    if (_is_mode_equal(style, GT_PLAYER_MODE_ONCE)) {
        _destroy_timer(style);
        return ;
    }
    if (_is_mode_equal(style, GT_PLAYER_MODE_PLAYBACK)) {
        _toggle_dir_handler(style);
        _check_repeat_count(style);
        _turn_prev_item(obj);
        return ;
    }
}

static inline void _change_item_handler(gt_obj_st * obj, bool is_next) {
    return is_next ? _turn_next_item(obj) : _turn_prev_item(obj);
}

static void _event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code = gt_event_get_code(e);

    if (GT_EVENT_TYPE_DRAW_START == code) {
        gt_disp_invalid_area(obj);
    } else if (GT_EVENT_TYPE_UPDATE_STYLE == code) {
        gt_disp_invalid_area(obj);
    }
}

static inline _gt_player_item_st * _add_item_into_list(_gt_player_st * style) {
    if (NULL == style) {
        return NULL;
    }
    if (style->target.list) {
        style->target.list = gt_mem_realloc(style->target.list, sizeof(_gt_player_item_st * ) * (style->target.count + 1));
    } else {
        style->target.count = 0;
        style->target.list = gt_mem_malloc(sizeof(_gt_player_item_st * ));
    }
    if (0 == style->target.list) {
        style->target.count = 0;
        style->target.index = 0;
        GT_LOGE(GT_LOG_TAG_MEM, "Failed to allocate memory for the player item list.");
        return NULL;
    }
    _gt_player_item_st ** list = style->target.list;
    list[style->target.count] = gt_mem_malloc(sizeof(_gt_player_item_st));
    if (NULL == list[style->target.count]) {
        return NULL;
    }
    gt_memset(list[style->target.count], 0, sizeof(_gt_player_item_st));

    _gt_player_item_st * ret = (_gt_player_item_st * )list[style->target.count];

#if GT_USE_FILE_HEADER
    gt_file_header_param_init(&ret->fh_param);
#endif

    ++style->target.count;
    return ret;
}

/**
 * @brief Create player object, or add item to player object
 *
 * @param obj
 * @param item
 * @param item_byte_size
 * @return uint16_t 0: failed to add item; >0: the count of items
 */
static inline uint16_t _add_item(gt_obj_st * obj, void * item, uint16_t item_byte_size) {
    GT_CHECK_BACK_VAL(obj, 0);
    if (NULL == item || 0 == item_byte_size) {
        return 0;
    }
    _gt_player_st * style = (_gt_player_st * )obj;

    _gt_player_item_st * new_item = _add_item_into_list(style);
    if (NULL == new_item) {
        return 0;
    }

    new_item->src = gt_mem_malloc(item_byte_size);
    if (NULL == new_item->src) {
        --style->target.count;
        style->target.list = gt_mem_realloc(style->target.list, sizeof(_gt_player_item_st * ) * (style->target.count));
        return 0;
    }
    gt_memcpy(new_item->src, item, item_byte_size);
    return style->target.count;
}

static inline void _set_play_status(_gt_player_st * style, bool is_play) {
    if (NULL == style) {
        return ;
    }
    if (!style->timer) {
        return ;
    }
    if (NULL == style->target.list) {
        is_play = false;
    }
    if (style->target.count < 2) {
        is_play = false;
    }
    style->last_player_tick = gt_tick_get();
    _gt_timer_set_paused(style->timer, is_play ? false : true);

    if (false == is_play) {
        /** redraw */
        gt_event_send((gt_obj_st * )style, GT_EVENT_TYPE_DRAW_START, NULL);
    }
}

static inline void _remove_item(_gt_player_st * style, uint16_t idx) {
    if (!style->target.count) {
        return ;
    }
    gt_mem_free(style->target.list[idx]->src);
    style->target.list[idx]->src = NULL;

    gt_mem_free(style->target.list[idx]);
    style->target.list[idx] = NULL;

    if (idx < --style->target.count) {
        gt_memmove(&style->target.list[idx], &style->target.list[idx + 1], (style->target.count - idx) * sizeof(_gt_player_item_st * ));
    }
    style->target.list[style->target.count] = NULL;
    style->target.list = (_gt_player_item_st ** )gt_mem_realloc(style->target.list, style->target.count *  sizeof(_gt_player_item_st * ));

    _set_play_status(style, _gt_timer_get_paused(style->timer));
}

static void _auto_play_callback(struct _gt_timer_s * timer) {
    gt_obj_st * obj = (gt_obj_st * )_gt_timer_get_user_data(timer);
    if (false == gt_obj_is_type(obj, GT_TYPE_PLAYER)) {
        return ;
    }
    _gt_player_st * style = (_gt_player_st * )obj;
    if (!_lock(style)) {
        /** The callback has been executed, waiting for it to be displayed */
        gt_disp_invalid_area(obj);
        return ;
    }
    gt_size_t count = 1;
    if (style->reg.real_time_update) {
        count = gt_tick_elapse(style->last_player_tick) / _gt_timer_get_period(timer);
    }

    while (count-- > 0) {
        _change_item_handler(obj, _is_dir_next(style));
    }

    style->last_player_tick =  gt_tick_get();
}

/* global functions / API interface -------------------------------------*/

gt_obj_st * gt_player_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    if (NULL == obj) {
        return obj;
    }
    _gt_player_st * style = (_gt_player_st * )obj;
    style->reg.type = GT_PLAYER_TYPE_IMG;
    style->repeat_cnt = -1;

    style->timer = _gt_timer_create(_auto_play_callback, 500, (void * )obj);
    _gt_timer_set_paused(style->timer, true);

    return obj;
}

void gt_player_set_type(gt_obj_st * obj, gt_player_type_et type)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    if (type >= GT_PLAYER_TYPE_MAX_COUNT || (gt_size_t)type < 0) {
        return ;
    }
    if (type > _GET_BIT_SHIFT_SIZE(_GT_PLAYER_BIT_TYPE_LENGTH)) {
        type = GT_PLAYER_TYPE_NONE;
    }
    ((_gt_player_st * )obj)->reg.type = type;
}

void gt_player_set_mode(gt_obj_st * obj, gt_player_mode_et mode)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    if (mode >= GT_PLAYER_MODE_MAX_COUNT || (gt_size_t)mode < 0) {
        return ;
    }
    if (mode > _GET_BIT_SHIFT_SIZE(_GT_PLAYER_BIT_MODE_LENGTH)) {
        mode = GT_PLAYER_MODE_NONE;
    }
    ((_gt_player_st * )obj)->reg.mode = mode;
}

void gt_player_set_auto_play_period(gt_obj_st * obj, uint32_t period)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_player_st * style = (_gt_player_st * )obj;
    style->reg.auto_play = period ? 1 : 0;

    if (!style->reg.auto_play) {
        _destroy_timer(style);
        return ;
    }
    if (style->timer) {
        _gt_timer_set_period(style->timer, period);
        return ;
    }
    _gt_timer_set_period(style->timer, period);
    _set_play_status(style, false);
}

void gt_player_remove_item(gt_obj_st * obj, void * item)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    if (NULL == item) {
        return ;
    }

    _gt_player_st * style = (_gt_player_st * )obj;
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

void gt_player_remove_all_items(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_player_st * style = (_gt_player_st * )obj;
    if (NULL == style->target.list) {
        return ;
    }
    for (gt_size_t i = style->target.count - 1; i >= 0; i--) {
        if (style->target.list[i]->src) {
            gt_mem_free(style->target.list[i]->src);
            style->target.list[i]->src = NULL;
        }
        gt_mem_free(style->target.list[i]);
        style->target.list[i] = NULL;
    }
    gt_mem_free(style->target.list);
    style->target.list = NULL;

    _set_play_status(style, _gt_timer_get_paused(style->timer));
}

void gt_player_remove_item_by_index(gt_obj_st * obj, uint16_t idx) {
    if (NULL == obj) {
        return ;
    }

    _gt_player_st * style = (_gt_player_st * )obj;
    if (NULL == style->target.list) {
        return ;
    }

    if (idx > style->target.count) {
        return ;
    }

    _remove_item(style, idx);
}

uint16_t gt_player_add_item(gt_obj_st * obj, void * item, uint16_t item_byte_size)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return 0;
    }
    return _add_item(obj, item, item_byte_size);
}

#if GT_USE_FILE_HEADER
uint16_t gt_player_add_item_by_file_header(gt_obj_st * obj, gt_file_header_param_st * fh)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return 0;
    }
    if (NULL == fh) {
        return 0;
    }
    _gt_player_st * style = (_gt_player_st * )obj;
    if (fh->idx >= gt_file_header_get_count()) {
        return 0;
    }

    _gt_player_item_st * new_item = _add_item_into_list(style);
    if (NULL == new_item) {
        return 0;
    }

#if GT_USE_FILE_HEADER
    new_item->fh_param = *fh;
#endif
    return style->target.count;
}
#endif

void gt_player_set_index(gt_obj_st * obj, gt_size_t index)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_player_st * style = (_gt_player_st * )obj;
    if (NULL == style->target.list) {
        return ;
    }
    if (index + 1 > style->target.count) {
        return ;
    }
    style->target.index = (-1 == index) ? (style->target.count - 1) : index;
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

gt_size_t gt_player_get_index(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return -1;
    }
    return ((_gt_player_st * )obj)->target.index;
}

uint16_t gt_player_get_item_count(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return 0;
    }
    return ((_gt_player_st * )obj)->target.count;
}

float gt_player_get_percentage(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return 0.0;
    }
    _gt_player_st * style = (_gt_player_st * )obj;
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
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    if (_is_type_none((_gt_player_st * )obj)) {
        GT_LOGV(GT_LOG_TAG_GUI, "player type is none, please set it.");
        return ;
    }
    _change_item_handler(obj, false);
}

void gt_player_turn_next(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    if (_is_type_none((_gt_player_st * )obj)) {
        GT_LOGV(GT_LOG_TAG_GUI, "player type is none, please set it.");
        return ;
    }
    _change_item_handler(obj, true);
}

void gt_player_play(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_player_st * style = (_gt_player_st * )obj;
    if (_is_type_none(style)) {
        GT_LOGV(GT_LOG_TAG_GUI, "player type is none, please set it.");
        return ;
    }
    _set_play_status(style, true);
}

void gt_player_stop(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_player_st * style = (_gt_player_st * )obj;
    if (_is_type_none(style)) {
        GT_LOGV(GT_LOG_TAG_GUI, "player type is none, please set it.");
        return ;
    }
    _set_play_status(style, false);
}

void gt_player_toggle(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_player_st * style = (_gt_player_st * )obj;
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
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_player_st * style = (_gt_player_st * )obj;
    style->reg.default_dir = style->reg.default_dir ? 0 : 1;

    _toggle_dir_handler(style);
}

void gt_player_set_repeat_count(gt_obj_st * obj, int32_t count)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_player_st * style = (_gt_player_st * )obj;

    style->repeat_cnt = count;
}

#if GT_PLAYER_REPEAT_FINISH_CALLBACK
void gt_player_set_repeat_finish_callback(gt_obj_st * obj, gt_event_cb_t cb, void * parms)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_player_st * style = (_gt_player_st * )obj;

    style->repeat_finish_cb = cb;
    style->parms = parms;
}
#endif

bool gt_player_dir_is_forward(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return false;
    }
    _gt_player_st * style = (_gt_player_st * )obj;
    return _is_dir_next(style);
}

void gt_player_set_real_time_update(gt_obj_st * obj, bool is_real_time_update)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_player_st * style = (_gt_player_st * )obj;
    style->reg.real_time_update = is_real_time_update;
}

bool gt_player_is_real_time_update(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return false;
    }
    _gt_player_st * style = (_gt_player_st * )obj;
    return style->reg.real_time_update;
}

#endif /** GT_CFG_ENABLE_PLAYER */
/* end ------------------------------------------------------------------*/
