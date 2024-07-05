/**
 * @file gt_clock.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2023-06-05 15:22:14
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_clock.h"

#if GT_CFG_ENABLE_CLOCK
#include "../core/gt_mem.h"
#include "../others/gt_types.h"
#include "./gt_label.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"
#include "../core/gt_obj_pos.h"
#include "../others/gt_log.h"
#include "../core/gt_timer.h"
#include "./gt_label.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_CLOCK
#define MY_CLASS    &gt_clock_class


#define _HOUR_MAX_VALUE     24
#define _MINUTE_MAX_VALUE   60
#define _SECOND_MAX_VALUE   60

#define _SECOND_PERIOD_MS   1000

#define _MAX_NEXT_DAY_BIT_WIDTH     4
#define _MAX_ALERT_BIT_WIDTH        4

#define _get_count_by_bit_width(_bit_width)     (1 << _bit_width)

/* private typedef ------------------------------------------------------*/
typedef struct _reg_s {
    uint16_t mode             : 2;   /** @ref gt_clock_mode_et */
    uint16_t is_running       : 1;
    uint16_t is_12_mode       : 1;   /** 12hour mode, such as: 1: 00:25; 0: 13:25 */
    uint16_t is_full_day      : 1;   /** finish the day */
    uint16_t is_show_meridiem : 1;   /** display AM/PM format */
    uint16_t len_next_day_cb  : _MAX_NEXT_DAY_BIT_WIDTH;
    uint16_t len_alert_cb     : _MAX_ALERT_BIT_WIDTH;
    uint16_t reserved         : 2;
}_reg_st;

typedef struct _next_day_s {
    gt_clock_next_day_cb cb;
    void * user_data;
}_next_day_st;

typedef struct _alert_s {
    gt_clock_alert_cb cb;
    void * user_data;
}_alert_st;

typedef struct _gt_clock_s {
    gt_obj_st obj;
    gt_obj_st * label;    /** @ref gt_label.h */
    char * format;

    _gt_timer_st * timer;
    _next_day_st ** next_day;
    _alert_st ** alert;

    _reg_st reg;

    gt_clock_time_st time_current;
    gt_clock_time_st time_setup;
}_gt_clock_st;


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_clock_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_clock_st)
};

/* macros ---------------------------------------------------------------*/

#define _get_hour(_time)    (_time.hour)
#define _get_minute(_time)  (_time.minute)
#define _get_second(_time)  (_time.second)

#define _set_hour(_time, _value)    (_time.hour = _value)
#define _set_minute(_time, _value)  (_time.minute = _value)
#define _set_second(_time, _value)  (_time.second = _value)


/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static bool _is_double_target_ignore_case(const char * const str, char target) {
    char another_case = 0;

    if (*str < 'A' || *str > 'z') {
        return false;
    }
    if (*str > 'Z' && *str < 'a') {
        return false;
    }
    another_case = target < 'a' ? (target + 0x20) : (target - 0x20);
    if (*str == target && *(str + 1) == target) {
        return true;
    }
    if (*str == another_case && *(str + 1) == another_case) {
        return true;
    }
    return false;
}

static void _gt_clock_timer_cb(struct _gt_timer_s * timer) {
    gt_clock_turn_next_second((gt_obj_st * )_gt_timer_get_user_data(timer));
}

static char * _get_time_str(_gt_clock_st * clock) {
    uint8_t hours = _get_hour(clock->time_current);
    uint8_t meridiem_len = 3;
    char * meridiem = NULL;
    char * pointer = NULL;
    char * str = NULL;
    char * str_ptr = NULL;
    uint8_t len = 0;

    /** default */
    if (NULL == clock->format) {
        char * format_default = "hh:mm:ss";
        len = strlen(format_default);

        clock->format = gt_mem_malloc(len + 1);
        if (NULL == clock->format) {
            return NULL;
        }
        gt_memset(clock->format, 0, len + 1);
        gt_memcpy(clock->format, format_default, len);
    }

    len = strlen(clock->format) + meridiem_len + 1;
    str = gt_mem_malloc(len);
    if (NULL == str) {
        return str;
    }
    gt_memset(str, 0, len);

    if (clock->reg.is_12_mode) {
        if (clock->reg.is_show_meridiem) {
            meridiem = hours >= (_HOUR_MAX_VALUE >> 1) ? " PM" : " AM";
        }
        if (hours > (_HOUR_MAX_VALUE >> 1)) {
            hours -= (_HOUR_MAX_VALUE >> 1);
        }
    }

    pointer = clock->format;
    str_ptr = str;
    while (*pointer) {
        if (_is_double_target_ignore_case(pointer, 'h')) {
            sprintf(str_ptr, "%02d", hours);
            pointer += 2;
            str_ptr += 2;
        } else if (_is_double_target_ignore_case(pointer, 'm')) {
            sprintf(str_ptr, "%02d", _get_minute(clock->time_current));
            pointer += 2;
            str_ptr += 2;
        } else if (_is_double_target_ignore_case(pointer, 's')) {
            sprintf(str_ptr, "%02d", _get_second(clock->time_current));
            pointer += 2;
            str_ptr += 2;
        } else {
            *str_ptr++ = *pointer++;
        }
    }
    if (meridiem) {
        gt_memcpy(str_ptr, meridiem, meridiem_len);
    }

    return str;
}

static void _init_cb(gt_obj_st * obj) {
    _gt_clock_st * style = (_gt_clock_st * )obj;
    char * str = _get_time_str(style);
    if (NULL == style->label) {
        return;
    }
    if (NULL == str) {
        return;
    }
    gt_area_copy(&style->label->area, &obj->area);
    gt_label_set_text(style->label, str);

    gt_mem_free(str);
    str = NULL;

    // focus
    draw_focus(obj , 0);

}

static inline void _remove_all_next_day_cb(_gt_clock_st * style) {
    gt_size_t i = 0;

    if (NULL == style->next_day) {
        return ;
    }
    for (i = style->reg.len_next_day_cb - 1; i >= 0; i--) {
        gt_mem_free(style->next_day[i]);
        style->next_day[i] = NULL;
    }
    gt_mem_free(style->next_day);
    style->next_day = NULL;
}

static inline void _remove_all_alert_cb(_gt_clock_st * style) {
    gt_size_t i = 0;

    if (NULL == style->alert) {
        return ;
    }
    for (i = style->reg.len_alert_cb - 1; i >= 0; i--) {
        gt_mem_free(style->alert[i]);
        style->alert[i] = NULL;
    }
    gt_mem_free(style->alert);
    style->alert = NULL;
}

static void _deinit_cb(gt_obj_st * obj) {
    _gt_clock_st * style = (_gt_clock_st * )obj;

    if (style->timer) {
        _gt_timer_del(style->timer);
        style->timer = NULL;
    }

    _remove_all_alert_cb(style);
    _remove_all_next_day_cb(style);

    if (style->format) {
        gt_mem_free(style->format);
        style->format = NULL;
    }

}

static void _event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code = gt_event_get_code(e);
    switch (code) {
        case GT_EVENT_TYPE_UPDATE_VALUE:
        case GT_EVENT_TYPE_DRAW_START: {
            gt_disp_invalid_area(obj);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;
        }
        case GT_EVENT_TYPE_DRAW_REDRAW: {
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;
        }
        default:
            break;
    }
}

static inline bool _is_time_equal(gt_clock_time_st * dst, gt_clock_time_st * src) {
    if (dst->second != src->second) {
        return false;
    }
    if (dst->minute != src->minute) {
        return false;
    }
    if (dst->hour != src->hour) {
        return false;
    }
    return true;
}

static inline bool _is_time_zero(gt_clock_time_st * time) {
    if (time->second) {
        return false;
    }
    if (time->minute) {
        return false;
    }
    if (time->hour) {
        return false;
    }
    return true;
}

static void _go_next_sec(gt_obj_st * obj) {
    _gt_clock_st * clock = (_gt_clock_st * )obj;
    uint8_t i = 0, len = 0;

    ++clock->time_current.second;
    if (clock->alert &&
        GT_CLOCK_MODE_TIME == clock->reg.mode &&
        _is_time_equal(&clock->time_current, &clock->time_setup)) {
        for (i = 0, len = clock->reg.len_alert_cb; i < len; i++) {
            clock->alert[i]->cb(obj, clock->alert[i]->user_data);
        }
    }

    if (_get_second(clock->time_current) < _SECOND_MAX_VALUE) {
        return;
    }
    _set_second(clock->time_current, 0);
    ++clock->time_current.minute;
    if (_get_minute(clock->time_current) < _MINUTE_MAX_VALUE) {
        return;
    }
    _set_minute(clock->time_current, 0);
    ++clock->time_current.hour;
    if (_get_hour(clock->time_current) < _HOUR_MAX_VALUE) {
        return;
    }
    _set_hour(clock->time_current, 0);

    if (GT_CLOCK_MODE_TIME == clock->reg.mode && clock->next_day) {
        for (i = 0, len = clock->reg.len_next_day_cb; i < len; i++) {
            clock->next_day[i]->cb(obj, clock->next_day[i]->user_data);
        }
    }
}

static bool _go_prev_sec(gt_obj_st * obj) {
    _gt_clock_st * clock = (_gt_clock_st * )obj;
    uint8_t i = 0, len = 0;

    if (!_get_hour(clock->time_current) && !_get_minute(clock->time_current) && !_get_second(clock->time_current)) {
        /** return when value is 00:00:00 */
        if (GT_CLOCK_MODE_COUNTDOWN == clock->reg.mode) {
            return false;
        }
    }

    if (_get_second(clock->time_current)) {
        --clock->time_current.second;
        if (clock->alert &&
            GT_CLOCK_MODE_COUNTDOWN == clock->reg.mode &&
            _is_time_zero(&clock->time_current)) {
            for (i = 0, len = clock->reg.len_alert_cb; i < len; i++) {
                clock->alert[i]->cb(obj, clock->alert[i]->user_data);
            }
        }
        return true;
    }
    _set_second(clock->time_current, _SECOND_MAX_VALUE - 1);
    if (_get_minute(clock->time_current)) {
        --clock->time_current.minute;
        return true;
    }
    _set_minute(clock->time_current, _MINUTE_MAX_VALUE -1);
    if (_get_hour(clock->time_current)) {
        --clock->time_current.hour;
        return true;
    }
    _set_hour(clock->time_current, _HOUR_MAX_VALUE - 1);

    return true;
}

/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_clock_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    if (NULL == obj) {
        return obj;
    }
    _gt_clock_st * style = (_gt_clock_st * )obj;

    style->label = gt_label_create(obj);
    gt_obj_set_inside(style->label, true);
    style->label->focus_dis = GT_DISABLED;
    gt_label_set_text(style->label , "");
    return obj;
}

void gt_clock_set_time(gt_obj_st * obj, uint8_t hour, uint8_t minute, uint8_t second)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_clock_st * clock = (_gt_clock_st *)obj;
    _set_hour(clock->time_current, hour < _HOUR_MAX_VALUE ? hour : 0);
    _set_minute(clock->time_current, minute < _MINUTE_MAX_VALUE ? minute : 0);
    _set_second(clock->time_current, second < _SECOND_MAX_VALUE ? second : 0);
    gt_event_send(obj, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

void gt_clock_set_time_by_timestamp(gt_obj_st * obj, uint32_t timestamp)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_clock_st * clock = (_gt_clock_st *)obj;
    _set_hour(clock->time_current, (timestamp / 3600) % 24);
    _set_minute(clock->time_current, (timestamp / 60) % 60);
    _set_second(clock->time_current, timestamp % 60);
}

void gt_clock_set_alert_time(gt_obj_st * obj, uint8_t hour, uint8_t minute, uint8_t second)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_clock_st * clock = (_gt_clock_st *)obj;
    _set_hour(clock->time_setup, hour < _HOUR_MAX_VALUE ? hour : 0);
    _set_minute(clock->time_setup, minute < _MINUTE_MAX_VALUE ? minute : 0);
    _set_second(clock->time_setup, second < _SECOND_MAX_VALUE ? second : 0);
}

void gt_clock_set_alert_time_by_timestamp(gt_obj_st * obj, uint32_t timestamp)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_clock_st * clock = (_gt_clock_st *)obj;
    _set_hour(clock->time_setup, (timestamp / 3600) % 24);
    _set_minute(clock->time_setup, (timestamp / 60) % 60);
    _set_second(clock->time_setup, timestamp % 60);
}

uint32_t gt_clock_get_time(gt_obj_st * obj, gt_clock_time_st * time)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return 0;
    }
    _gt_clock_st * clock = (_gt_clock_st *)obj;
    if (time) {
        time->hour = _get_hour(clock->time_current);
        time->minute = _get_minute(clock->time_current);
        time->second = _get_second(clock->time_current);
    }
    return _get_hour(clock->time_current) * 3600 + _get_minute(clock->time_current) * 60 + _get_second(clock->time_current);
}

uint32_t gt_clock_get_alert_time(gt_obj_st * obj, gt_clock_time_st * time)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return 0;
    }
    _gt_clock_st * clock = (_gt_clock_st *)obj;
    if (time) {
        time->hour = _get_hour(clock->time_setup);
        time->minute = _get_minute(clock->time_setup);
        time->second = _get_second(clock->time_setup);
    }
    return _get_hour(clock->time_setup) * 3600 + _get_minute(clock->time_setup) * 60 + _get_second(clock->time_setup);
}

void gt_clock_turn_next_second(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_clock_st * clock = (_gt_clock_st *)obj;

    if (GT_CLOCK_MODE_COUNTDOWN == clock->reg.mode) {
        if (!_go_prev_sec(obj)) {
            return;
        }
    } else {
        _go_next_sec(obj);
    }
    gt_event_send(obj, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

void gt_clock_turn_prev_second(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_clock_st * clock = (_gt_clock_st *)obj;

    if (GT_CLOCK_MODE_COUNTDOWN == clock->reg.mode) {
        _go_next_sec(obj);
    } else {
        _go_prev_sec(obj);
    }
    gt_event_send(obj, GT_EVENT_TYPE_UPDATE_VALUE, NULL);

}

void gt_clock_set_mode(gt_obj_st * obj, gt_clock_mode_et mode)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_clock_st * clock = (_gt_clock_st *)obj;

    if (mode >= GT_CLOCK_MODE_MAX_COUNT) {
        mode = GT_CLOCK_MODE_TIME;  /** default mode */
    }
    clock->reg.mode = mode;
}

gt_clock_mode_et gt_clock_get_mode(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return GT_CLOCK_MODE_TIME;
    }
    _gt_clock_st * clock = (_gt_clock_st *)obj;
    return (gt_clock_mode_et)clock->reg.mode;
}

void gt_clock_set_12_hours_mode(gt_obj_st * obj, bool enabled)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_clock_st * clock = (_gt_clock_st *)obj;
    clock->reg.is_12_mode = enabled ? 1 : 0;
}

bool gt_clock_get_12_hours_mode(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return false;
    }
    _gt_clock_st * clock = (_gt_clock_st *)obj;
    return clock->reg.is_12_mode ? true : false;
}

void gt_clock_set_meridiem_mode(gt_obj_st * obj, bool enabled)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_clock_st * clock = (_gt_clock_st *)obj;
    clock->reg.is_show_meridiem = enabled ? 1 : 0;
}

bool gt_clock_get_meridiem_mode(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return false;
    }
    _gt_clock_st * clock = (_gt_clock_st *)obj;
    return clock->reg.is_show_meridiem ? true : false;
}

void gt_clock_set_format(gt_obj_st * obj, const char * const format)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_clock_st * clock = (_gt_clock_st *)obj;
    if (NULL == clock->format) {
        clock->format = (char * )gt_mem_malloc(strlen(format) + 1);
        GT_CHECK_BACK(clock->format);
        gt_memset(clock->format, 0, strlen(format) + 1);
        gt_memcpy(clock->format, format, strlen(format));
        return;
    }

    if (strlen(format) < strlen(clock->format)) {
        gt_memset(clock->format, 0, strlen(clock->format) + 1);
        gt_memcpy(clock->format, format, strlen(format));
        return;
    }

    clock->format = (char * )gt_mem_realloc(clock->format, strlen(format) + 1);
    GT_CHECK_BACK(clock->format);

    gt_memset(clock->format, 0, strlen(clock->format) + 1);
    gt_memcpy(clock->format, format, strlen(format));
}

void gt_clock_set_next_day_cb(gt_obj_st * obj, gt_clock_next_day_cb next_day_cb, void * user_data)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_clock_st * clock = (_gt_clock_st *)obj;
    if (NULL == next_day_cb) {
        return;
    }
    if (clock->reg.len_next_day_cb + 1 >= _get_count_by_bit_width(_MAX_NEXT_DAY_BIT_WIDTH)) {
        return;
    }

    if (NULL == clock->next_day) {
        clock->next_day = (_next_day_st ** )gt_mem_malloc(sizeof(_next_day_st ** ));
    } else {
        clock->next_day = (_next_day_st ** )gt_mem_realloc(clock->next_day, (clock->reg.len_next_day_cb + 1) + sizeof(_next_day_st ** ));
    }
    if (NULL == clock->next_day) {
        return;
    }
    clock->next_day[clock->reg.len_next_day_cb] = (_next_day_st * )gt_mem_malloc(sizeof(_next_day_st));
    if (NULL == clock->next_day[clock->reg.len_next_day_cb]) {
        clock->next_day = (_next_day_st ** )gt_mem_realloc(clock->next_day, (clock->reg.len_next_day_cb) + sizeof(_next_day_st ** ));
        return;
    }
    clock->next_day[clock->reg.len_next_day_cb]->cb = next_day_cb;
    clock->next_day[clock->reg.len_next_day_cb]->user_data = user_data;
    ++clock->reg.len_next_day_cb;
}

void gt_clock_set_alert_cb(gt_obj_st * obj, gt_clock_alert_cb alert_cb, void * user_data)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_clock_st * clock = (_gt_clock_st *)obj;
    if (NULL == alert_cb) {
        return;
    }
    if (clock->reg.len_alert_cb + 1 >= _get_count_by_bit_width(_MAX_ALERT_BIT_WIDTH)) {
        return;
    }

    if (NULL == clock->alert) {
        clock->alert = (_alert_st ** )gt_mem_malloc(sizeof(_alert_st ** ));
    } else {
        clock->alert = (_alert_st ** )gt_mem_realloc(clock->alert, (clock->reg.len_alert_cb + 1) + sizeof(_alert_st ** ));
    }
    if (NULL == clock->alert) {
        return;
    }
    clock->alert[clock->reg.len_alert_cb] = (_alert_st * )gt_mem_malloc(sizeof(_alert_st));
    if (NULL == clock->alert[clock->reg.len_alert_cb]) {
        clock->alert = (_alert_st ** )gt_mem_realloc(clock->alert, (clock->reg.len_alert_cb) + sizeof(_alert_st ** ));
        return;
    }
    clock->alert[clock->reg.len_alert_cb]->cb = alert_cb;
    clock->alert[clock->reg.len_alert_cb]->user_data = user_data;
    ++clock->reg.len_alert_cb;
}

void gt_clock_clr_all_next_day_cb(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _remove_all_next_day_cb((_gt_clock_st * )obj);
}

void gt_clock_clr_all_alert_cb(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _remove_all_alert_cb((_gt_clock_st * )obj);
}

bool gt_clock_remove_next_day_cb(gt_obj_st * obj, gt_clock_next_day_cb next_day_cb)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return false;
    }
    _gt_clock_st * style = (_gt_clock_st * )obj;
    gt_size_t i = 0;

    if (NULL == next_day_cb) {
        return false;
    }

    if (NULL == style->next_day || 0 == style->reg.len_next_day_cb) {
        return false;
    }

    for (i = style->reg.len_next_day_cb - 1; i >= 0; i--) {
        if (next_day_cb == style->next_day[i]->cb) {
            break;
        }
    }
    if (i < 0) {
        printf("not found next_day_cb\n");
        return false;
    }

    gt_mem_free(style->next_day[i]);
    style->next_day[i] = NULL;

    if (i < --style->reg.len_next_day_cb) {
        gt_memmove(&style->next_day[i], &style->next_day[i + 1], (style->reg.len_next_day_cb - i) * sizeof(_next_day_st * ));
    }
    style->next_day[style->reg.len_next_day_cb] = NULL;
    style->next_day = (_next_day_st ** )gt_mem_realloc(style->next_day, (style->reg.len_next_day_cb) * sizeof(_next_day_st * ));
    return true;
}

bool gt_clock_remove_alert_cb(gt_obj_st * obj, gt_clock_alert_cb alert_cb)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return false;
    }
    _gt_clock_st * style = (_gt_clock_st * )obj;
    gt_size_t i = 0;

    if (NULL == alert_cb) {
        return false;
    }

    if (NULL == style->alert || 0 == style->reg.len_alert_cb) {
        return false;
    }

    for (i = style->reg.len_alert_cb - 1; i >= 0; i--) {
        if (alert_cb == style->alert[i]->cb) {
            break;
        }
    }
    if (i < 0) {
        return false;
    }

    gt_mem_free(style->alert[i]);
    style->alert[i] = NULL;

    if (i < --style->reg.len_alert_cb) {
        gt_memmove(&style->alert[i], &style->alert[i + 1], (style->reg.len_alert_cb - i) * sizeof(_alert_st * ));
    }
    style->alert[style->reg.len_alert_cb] = NULL;
    style->alert = (_alert_st ** )gt_mem_realloc(style->alert, (style->reg.len_alert_cb) * sizeof(_alert_st * ));
    return true;
}

void gt_clock_start(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_clock_st * style = (_gt_clock_st * )obj;
    style->reg.is_running = 1;

    if (NULL == style->timer) {
        style->timer = _gt_timer_create(_gt_clock_timer_cb, _SECOND_PERIOD_MS, (void * )obj);
        return;
    }
    _gt_timer_set_paused(style->timer, false);
}

void gt_clock_stop(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_clock_st * style = (_gt_clock_st * )obj;
    style->reg.is_running = 0;

    if (style->timer) {
        _gt_timer_set_paused(style->timer, true);
    }
}

void gt_clock_toggle(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_clock_st * style = (_gt_clock_st * )obj;

    if (style->reg.is_running) {
        gt_clock_stop(obj);
        return;
    }
    gt_clock_start(obj);
}


void gt_clock_set_font_color(gt_obj_st * obj, gt_color_t color)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_clock_st * style = (_gt_clock_st * )obj;
    if (NULL == style->label) {
        return;
    }
    gt_label_set_font_color(style->label, color);
}

void gt_clock_set_font_size(gt_obj_st * obj, uint8_t size)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_clock_st * style = (_gt_clock_st * )obj;
    if (NULL == style->label) {
        return;
    }
    gt_label_set_font_size(style->label, size);
}

void gt_clock_set_font_gray(gt_obj_st * obj, uint8_t gray)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_clock_st * style = (_gt_clock_st * )obj;
    if (NULL == style->label) {
        return;
    }
    gt_label_set_font_gray(style->label, gray);
}

void gt_clock_set_font_align(gt_obj_st * obj, gt_align_et align)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_clock_st * style = (_gt_clock_st * )obj;
    if (NULL == style->label) {
        return;
    }
    gt_label_set_font_align(style->label, align);
}

void gt_clock_set_font_family_cn(gt_obj_st * obj, gt_family_t family)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_clock_st * style = (_gt_clock_st * )obj;
    if (NULL == style->label) {
        return;
    }
    gt_label_set_font_family_cn(style->label, family);
}

void gt_clock_set_font_family_en(gt_obj_st * obj, gt_family_t family)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_clock_st * style = (_gt_clock_st * )obj;
    if (NULL == style->label) {
        return;
    }
    gt_label_set_font_family_en(style->label, family);
}

void gt_clock_set_font_family_fl(gt_obj_st * obj, gt_family_t family)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_clock_st * style = (_gt_clock_st * )obj;
    if (NULL == style->label) {
        return;
    }
    gt_label_set_font_family_fl(style->label, family);
}

void gt_clock_set_font_family_numb(gt_obj_st * obj, gt_family_t family)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_clock_st * style = (_gt_clock_st * )obj;
    if (NULL == style->label) {
        return;
    }
    gt_label_set_font_family_numb(style->label, family);
}

void gt_clock_set_font_thick_en(gt_obj_st * obj, uint8_t thick)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_clock_st * style = (_gt_clock_st * )obj;
    if (NULL == style->label) {
        return;
    }
    gt_label_set_font_thick_en(style->label, thick);
}

void gt_clock_set_font_thick_cn(gt_obj_st * obj, uint8_t thick)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_clock_st * style = (_gt_clock_st * )obj;
    if (NULL == style->label) {
        return;
    }
    gt_label_set_font_thick_cn(style->label, thick);
}

void gt_clock_set_space(gt_obj_st * obj, uint8_t space_x, uint8_t space_y)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_clock_st * style = (_gt_clock_st * )obj;
    if (NULL == style->label) {
        return;
    }
    gt_label_set_space(style->label, space_x , space_y);
}

#endif /** #if GT_CFG_ENABLE_CLOCK */
/* end ------------------------------------------------------------------*/
