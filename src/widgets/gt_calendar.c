/**
 * @file gt_calendar.c
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2025-01-02 10:21:43
 * @copyright Copyright (c) 2014-2025, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_calendar.h"

#if GT_CFG_ENABLE_CALENDAR
#include "../core/gt_mem.h"
#include "../hal/gt_hal_disp.h"
#include "../others/gt_log.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"
#include "../core/gt_event.h"
#include "../font/gt_symbol.h"
#include "../core/gt_indev.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_CALENDAR
#define MY_CLASS    &gt_calendar_class

/* private typedef ------------------------------------------------------*/

typedef struct _gt_calendar_reg_s {
    uint8_t fill: 1;
    uint8_t is_cn: 1;
    uint8_t reserved: 6;
}_gt_calendar_reg_st;

typedef struct {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t week;
}_gt_calendar_days_st;

typedef struct {
    uint32_t year;
    uint32_t month[12][2]; // 0: normal, 1: holiday 2: workday
}_gt_calendar_day_type_st;

typedef struct _gt_calendar_s {
    gt_obj_st obj;

    _gt_calendar_day_type_st day_type;
    _gt_calendar_days_st today;
    _gt_calendar_days_st show_days;
    _gt_calendar_days_st select_days;

    gt_font_info_st font_info;

    gt_color_t border_color;
    gt_color_t font_color_today;
    gt_color_t font_color_weekend;
    gt_color_t font_color_other_month;

    gt_align_et font_align;
    uint8_t border_width;

    _gt_calendar_reg_st reg;
}_gt_calendar_st;


/* static prototypes ----------------------------------------------------*/


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

static GT_ATTRIBUTE_RAM_DATA const gt_obj_class_st gt_calendar_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_calendar_st)
};

static GT_ATTRIBUTE_RAM_DATA const char* _week_str_en[7] = {
    "Sun","Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};
static GT_ATTRIBUTE_RAM_DATA const char* _week_str_cn[7] = {
    "日","一", "二", "三", "四", "五", "六",
};

static GT_ATTRIBUTE_RAM_DATA const char* _month_str_en[12] = {
    "Dec", "Jan","Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov"
};
static GT_ATTRIBUTE_RAM_DATA const char* _month_str_cn[12] = {
    "12月", "1月","2月", "3月", "4月", "5月", "6月", "7月", "8月", "9月", "10月", "11月"
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static GT_ATTRIBUTE_RAM_TEXT const char* _get_week_str(uint8_t week, uint8_t is_cn)
{
    return is_cn ? _week_str_cn[week % 7] : _week_str_en[week % 7];
}

static GT_ATTRIBUTE_RAM_TEXT const char* _get_month_str(uint8_t month, uint8_t is_cn)
{
    return is_cn ? _month_str_cn[month % 12] : _month_str_en[month % 12];
}

static GT_ATTRIBUTE_RAM_TEXT void _set_day_type(uint32_t* month, uint8_t day, gt_calendar_day_type_et flag)
{
    if(0 == day) return ;
    uint8_t pos = ((day - 1) << 1) >> 5;
    uint8_t bit = ((day - 1) << 1) % 32;
    month[pos] = (month[pos] & (~(3 << bit))) | ((flag & 3) << bit);
}

static GT_ATTRIBUTE_RAM_TEXT gt_calendar_day_type_et _get_day_type(uint32_t* month, uint8_t day)
{
    if(0 == day) return GT_CALENDAR_DAY_TYPE_NORMAL;
    uint8_t pos = ((day - 1) << 1) >> 5;
    uint8_t bit = ((day - 1) << 1) % 32;
    return (gt_calendar_day_type_et)((month[pos] >> bit) & 3);
}

static GT_ATTRIBUTE_RAM_TEXT bool _is_leap_year(uint16_t year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static GT_ATTRIBUTE_RAM_TEXT uint8_t _get_days_in_month(uint16_t year, uint8_t month)
{
    if(month == 2) {
        return _is_leap_year(year) ? 29 : 28;
    }
    else if(month == 4 || month == 6 || month == 9 || month == 11) {
        return 30;
    }
    else {
        return 31;
    }
}

static GT_ATTRIBUTE_RAM_TEXT uint8_t _get_week_day(uint16_t year, uint8_t month, uint8_t day)
{
    if(month < 3) {
        month += 12;
        year--;
    }
    int c = year / 100;
    int y = year % 100;
    int m = month;
    int d = day;
    int w = (y + y / 4 + c / 4 - 2 * c + (26 * (m + 1)) / 10 + d - 1) % 7;
    w = (w + 7) % 7;
    return w == 0 ? 7 : w;
}

static GT_ATTRIBUTE_RAM_TEXT void _get_prev_year(_gt_calendar_days_st* days)
{
    days->year--;
    days->week = _get_week_day(days->year, days->month, days->day);
}

static GT_ATTRIBUTE_RAM_TEXT void _get_next_year(_gt_calendar_days_st* days)
{
    days->year++;
    days->week = _get_week_day(days->year, days->month, days->day);
}

static GT_ATTRIBUTE_RAM_TEXT void _get_prev_month(_gt_calendar_days_st* days)
{
    if(days->month > 1) {
        days->month--;
    }
    else {
        days->month = 12;
        days->year--;
    }
    if(days->day > _get_days_in_month(days->year, days->month)){
        days->day = 1;
    }
    days->week = _get_week_day(days->year, days->month, days->day);
}

static GT_ATTRIBUTE_RAM_TEXT void _get_next_month(_gt_calendar_days_st* days)
{
    if(days->month < 12) {
        days->month++;
    }
    else {
        days->month = 1;
        days->year++;
    }
    if(days->day > _get_days_in_month(days->year, days->month)){
        days->day = 1;
    }
    days->week = _get_week_day(days->year, days->month, days->day);
}


static GT_ATTRIBUTE_RAM_TEXT void _get_prev_week(_gt_calendar_days_st* days)
{
    if(days->day > 7) {
        days->day -= 7;
    }
    else {
        days->month--;
        if(days->month == 0) {
            days->month = 12;
            days->year--;
        }
        days->day = _get_days_in_month(days->year, days->month) - (7 - days->day);
    }
    days->week = _get_week_day(days->year, days->month, days->day);
}

static GT_ATTRIBUTE_RAM_TEXT void _get_next_week(_gt_calendar_days_st* days)
{
    uint16_t days_munb = _get_days_in_month(days->year, days->month);

    if(days->day <= days_munb - 7){
        days->day += 7;
    }
    else {
        days->month++;
        if(days->month > 12) {
            days->month = 1;
            days->year++;
        }
        days->day = days->day + 7 - days_munb;
    }
    days->week = _get_week_day(days->year, days->month, days->day);
}

static GT_ATTRIBUTE_RAM_TEXT void _get_prev_days(_gt_calendar_days_st* days)
{
    if(days->day > 1) {
        days->day--;
    }
    else {
        days->month--;
        if(days->month == 0) {
            days->month = 12;
            days->year--;
        }
        days->day = _get_days_in_month(days->year, days->month);
    }
    days->week = _get_week_day(days->year, days->month, days->day);
}

static GT_ATTRIBUTE_RAM_TEXT void _get_next_days(_gt_calendar_days_st* days)
{
    uint16_t days_munb = _get_days_in_month(days->year, days->month);

    if(days->day < days_munb){
        days->day++;
    }
    else {
        days->day = 1;
        days->month++;
        if(days->month > 12) {
            days->month = 1;
            days->year++;
        }
    }
    days->week = _get_week_day(days->year, days->month, days->day);
}

static GT_ATTRIBUTE_RAM_TEXT void _get_obj_size_alig(gt_obj_st* obj)
{
    obj->area.w = (obj->area.w / 7) * 7;
    obj->area.h = (obj->area.h / 8) * 8;
}
static GT_ATTRIBUTE_RAM_TEXT gt_area_st _get_itme_area(gt_obj_st* obj, uint8_t row, uint8_t col)
{
    gt_area_st area = obj->area;
    area.x += col * (area.w / 7);
    area.y += row * (area.h / 8);
    area.w = area.w / 7;
    area.h = area.h / 8;
    return area;
}

static GT_ATTRIBUTE_RAM_TEXT _gt_calendar_days_st _get_start_day(_gt_calendar_days_st day)
{
    _gt_calendar_days_st start_day;
    uint8_t day_numb = 0;

    start_day.week = _get_week_day(day.year, day.month, 1);
    if(start_day.week != 1){
        if(day.month == 1){
            start_day.year = day.year - 1;
            start_day.month = 12;
        }
        else{
            start_day.year = day.year;
            start_day.month = day.month - 1;
        }
        day_numb = _get_days_in_month(start_day.year, start_day.month);
        start_day.day = day_numb - start_day.week + 2;
    }
    else{
        start_day.year = day.year;
        start_day.month = day.month;
        day_numb = _get_days_in_month(start_day.year, start_day.month);
        start_day.day = 1;
    }
    start_day.week = _get_week_day(start_day.year, start_day.month, start_day.day);

    return start_day;
}

// 从当前一天开始，计算加减多少天后的日期
static GT_ATTRIBUTE_RAM_TEXT _gt_calendar_days_st _get_days(_gt_calendar_days_st today, int8_t days)
{
    _gt_calendar_days_st result = today;
    if (days > 0) {
        for (int8_t i = 0; i < days; i++) {
            _get_next_days(&result);
        }
    } else {
        for (int8_t i = 0; i < -days; i++) {
            _get_prev_days(&result);
        }
    }
    return result;
}



static GT_ATTRIBUTE_RAM_TEXT void _draw_calendar_style(gt_obj_st* obj)
{
    _gt_calendar_st* style = (_gt_calendar_st*)obj;

    gt_attr_rect_st rect_attr = {0};
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.radius = obj->radius;
    rect_attr.bg_color = obj->bgcolor;
    rect_attr.bg_opa = obj->opa;
    rect_attr.border_color = style->border_color;
    rect_attr.border_width = style->border_width;
    rect_attr.border_opa = obj->opa;
    rect_attr.reg.is_fill = style->reg.fill;
    //
    draw_bg(obj->draw_ctx, &rect_attr, &obj->area);
}

static GT_ATTRIBUTE_RAM_TEXT void _draw_calendar_header(gt_obj_st* obj)
{
    _gt_calendar_st* style = (_gt_calendar_st*)obj;
    gt_font_st font = {
        .info = style->font_info,
        .res = NULL,
    };

    gt_attr_font_st font_attr = {
        .font = &font,
        .align = style->font_align,
        .space_x = 0,
        .space_y = 0,
        .font_color = font.info.palette,
        .opa = obj->opa,
        .reg = {
            .single_line = true,
        },
    };
    uint8_t buf[8] = {0};
    gt_area_st area = {0};
    gt_size_t offset = 0;
    //
    area = _get_itme_area(obj, 0, 0);
    font_attr.logical_area = area;
    sprintf((char*)buf, "%s", GT_SYMBOL_LEFT_SOLID);
    font.utf8 = (char*)buf;
    font.len = strlen((char*)buf);
    draw_text(obj->draw_ctx, &font_attr, &area);

    if(area.w > font.info.size){
        offset = (area.w - font.info.size) >> 1;
    }

    area = _get_itme_area(obj, 0, 1);
    area.x -= offset;
    area.w += offset << 1;
    font_attr.logical_area = area;
    sprintf((char*)buf, "%s", _get_month_str(style->show_days.month, style->reg.is_cn));
    font.utf8 = (char*)buf;
    font.len = strlen((char*)buf);
    draw_text(obj->draw_ctx, &font_attr, &area);

    area = _get_itme_area(obj, 0, 2);
    font_attr.logical_area = area;
    sprintf((char*)buf, "%s", GT_SYMBOL_RIGHT_SOLID);
    font.utf8 = (char*)buf;
    font.len = strlen((char*)buf);
    draw_text(obj->draw_ctx, &font_attr, &area);

    //
    area = _get_itme_area(obj, 0, 4);
    font_attr.logical_area = area;
    sprintf((char*)buf, "%s", GT_SYMBOL_LEFT_SOLID);
    font.utf8 = (char*)buf;
    font.len = strlen((char*)buf);
    draw_text(obj->draw_ctx, &font_attr, &area);

    if(area.w > font.info.size){
        offset = (area.w - font.info.size) >> 1;
    }

    area = _get_itme_area(obj, 0, 5);
    area.x -= offset;
    area.w += offset << 1;
    font_attr.logical_area = area;
    sprintf((char*)buf, "%d", style->show_days.year);
    font.utf8 = (char*)buf;
    font.len = strlen((char*)buf);
    draw_text(obj->draw_ctx, &font_attr, &area);

    area = _get_itme_area(obj, 0, 6);
    font_attr.logical_area = area;
    sprintf((char*)buf, "%s", GT_SYMBOL_RIGHT_SOLID);
    font.utf8 = (char*)buf;
    font.len = strlen((char*)buf);
    draw_text(obj->draw_ctx, &font_attr, &area);
}

static GT_ATTRIBUTE_RAM_TEXT void _draw_calendar_body(gt_obj_st* obj)
{
    _gt_calendar_st* style = (_gt_calendar_st*)obj;
    gt_font_st font = {
        .info = style->font_info,
        .res = NULL,
    };

    gt_font_st day_type_font = {
        .info = style->font_info,
        .res = NULL,
    };
    day_type_font.info.size = 12;
    gt_attr_rect_st rect_attr = {0};
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.bg_opa = obj->opa;
    //

    gt_attr_font_st font_attr = {
        .font = &font,
        .align = style->font_align,
        .space_x = 0,
        .space_y = 0,
        .font_color = font.info.palette,
        .opa = obj->opa,
        .reg = {
            .single_line = true,
        },
    };
    uint8_t buf[8] = {0};
    gt_area_st area = {0};
    for(uint8_t i = 1; i <= 7; ++i){
        area = _get_itme_area(obj, 1, i-1);
        font_attr.logical_area = area;
        sprintf((char*)buf, "%s", _get_week_str(i, style->reg.is_cn));
        font.utf8 = (char*)buf;
        font.len = strlen((char*)buf);

        if(6 == i || 7 == i){
            font_attr.font_color = style->font_color_weekend;
        }
        else{
            font_attr.font_color = style->font_info.palette;
        }
        draw_text(obj->draw_ctx, &font_attr, &area);
    }

    _gt_calendar_days_st start_day = {0};
    start_day = _get_start_day(style->show_days);

    uint8_t row = 2, col = 0;

    for(row = 2; row < 8; ++row)
    {
        for(col = 0; col < 7; ++col)
        {
            area = _get_itme_area(obj, row, col);
            area = gt_area_reduce(area, 1);
            font_attr.logical_area = area;
            sprintf((char*)buf, "%d", start_day.day);
            font.utf8 = (char*)buf;
            font.len = strlen((char*)buf);

            if(start_day.month != style->show_days.month){
                font_attr.font_color = style->font_color_other_month;
            }
            else if(start_day.day == style->today.day
                    && start_day.month == style->today.month
                    && start_day.year == style->today.year
            ){
                font_attr.font_color = style->font_color_today;
            }
            else if(6 == start_day.week || 7 == start_day.week){
                font_attr.font_color = style->font_color_weekend;
            }
            else{
                font_attr.font_color = style->font_info.palette;
            }

            if(start_day.day == style->select_days.day
                && start_day.month == style->select_days.month
                && start_day.year == style->select_days.year
            ){
                rect_attr.radius = 6; // round
                rect_attr.reg.is_fill = 0;
                rect_attr.border_width = 1;
                rect_attr.border_color = gt_color_hex(GT_FOCUS_COLOR_SELECT);
                draw_bg(obj->draw_ctx, &rect_attr, &area);
            }
            draw_text(obj->draw_ctx, &font_attr, &area);
            // day_type
            gt_calendar_day_type_et day_type = _get_day_type(style->day_type.month[start_day.month%12], start_day.day);
            if(GT_CALENDAR_DAY_TYPE_NORMAL != day_type && start_day.year == style->day_type.year){
                area.x = area.x + area.w - 16;
                area.y = area.y;
                area.w = 16;
                area.h = 16;
                area = gt_area_reduce(area, 1);

                if(GT_CALENDAR_DAY_TYPE_HOLIDAY == day_type){
                    sprintf((char*)buf, "%s", GT_SYMBOL_CALENDAR_HOLIDAY);
                    font_attr.font_color = gt_color_red();
                    rect_attr.bg_color = gt_color_hex(0x391515);
                }
                else{
                    sprintf((char*)buf, "%s", GT_SYMBOL_CALENDAR_WORKDAY);
                    font_attr.font_color = gt_color_green();
                    rect_attr.bg_color = gt_color_hex(0x12211F);
                }
                //
                rect_attr.border_width = 0;
                rect_attr.radius = area.w; // round
                rect_attr.reg.is_fill = 1;
                draw_bg(obj->draw_ctx, &rect_attr, &area);
                //
                day_type_font.utf8 = (char*)buf;
                day_type_font.len = strlen((char*)buf);
                font_attr.logical_area = area;
                font_attr.font = &day_type_font;
                draw_text(obj->draw_ctx, &font_attr, &area);
                //
                font_attr.font = &font;
            }

            _get_next_days(&start_day);
        }
    }
}


static void GT_ATTRIBUTE_RAM_TEXT _calendar_click_release(gt_obj_st * obj)
{
    _gt_calendar_st * style = (_gt_calendar_st *)obj;
    gt_point_st point = obj->process_attr.point;
    gt_area_st area = {0};
    int8_t row = 0, col = 0;

    area = _get_itme_area(obj, 0, 0);
    area.x -= obj->area.x;
    area.y -= obj->area.y;
    if(point.y > area.y && point.y < area.y + area.h){
        // prev month
        if(point.x > area.x && point.x < area.x + area.w){
            _get_prev_month(&style->show_days);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            return ;
        }
        // next month
        area = _get_itme_area(obj, 0, 2);
        area.x -= obj->area.x;
        if(point.x > area.x && point.x < area.x + area.w){
            _get_next_month(&style->show_days);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            return ;
        }

        // prev year
        area = _get_itme_area(obj, 0, 4);
        area.x -= obj->area.x;
        if(point.x > area.x && point.x < area.x + area.w){
            _get_prev_year(&style->show_days);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            return ;
        }

        // next year
        area = _get_itme_area(obj, 0, 6);
        area.x -= obj->area.x;
        if(point.x > area.x && point.x < area.x + area.w){
            _get_next_year(&style->show_days);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            return ;
        }
    }

    // row
    for(row = 2; row < 8; ++row)
    {
        area = _get_itme_area(obj, row, 0);
        area.y -= obj->area.y;
        if(point.y > area.y && point.y < area.y + area.h){
            break;
        }
    }
    if(row >= 8){
        return ;
    }

    // col
    for(col = 0; col < 7; ++col)
    {
        area = _get_itme_area(obj, row, col);
        area.x -= obj->area.x;
        if(point.x > area.x && point.x < area.x + area.w){
            break;
        }
    }
    if(col >= 7){
        return ;
    }

    _gt_calendar_days_st start_day = {0};
    start_day = _get_start_day(style->show_days);
    style->select_days = _get_days(start_day, ((row-2)*7 + col));
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

static void GT_ATTRIBUTE_RAM_TEXT _calendar_key_release(gt_obj_st * obj, gt_event_st * e)
{
    _gt_calendar_st * style = (_gt_calendar_st *)obj;
    uint32_t key = gt_indev_get_key();

    if(style->select_days.year != style->show_days.year
        && style->select_days.month != style->show_days.month
    )
    {
        style->select_days = style->show_days;
        style->select_days.day = 1;
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
        return ;
    }

    switch (key) {
    case GT_KEY_LEFT:
        _get_prev_days(&style->select_days);
        style->show_days = style->select_days;
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
        break;

    case GT_KEY_RIGHT:
        _get_next_days(&style->select_days);
        style->show_days = style->select_days;
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
        break;
    case GT_KEY_UP:
        _get_prev_week(&style->select_days);
        style->show_days = style->select_days;
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
        break;
    case GT_KEY_DOWN:
        _get_next_week(&style->select_days);
        style->show_days = style->select_days;
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
        break;
    default:
        break;
    }

}

static void _init_cb(gt_obj_st * obj)
{
    if(obj->area.w <= 0 || obj->area.h <= 0) {
        return;
    }

    _get_obj_size_alig(obj);

    _draw_calendar_style(obj);

    _draw_calendar_header(obj);

    _draw_calendar_body(obj);
}
static void _deinit_cb(gt_obj_st * obj)
{

}
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e)
{
    gt_event_type_et code_val = gt_event_get_code(e);

    switch(code_val) {
        case GT_EVENT_TYPE_DRAW_START:
        case GT_EVENT_TYPE_UPDATE_STYLE:
            gt_disp_invalid_area(obj);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_RELEASED: /* click event finish */
            _calendar_click_release(obj);
            break;

        case GT_EVENT_TYPE_INPUT_KEY:
        case GT_EVENT_TYPE_INPUT_KEY_PRESSING:
            _calendar_key_release(obj, e);
            break;

        default:
            break;
    }
}


/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_calendar_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    if (NULL == obj) {
        return obj;
    }

    obj->area.w = 320;
    obj->area.h = 240;
    obj->bgcolor = gt_color_black();
    obj->radius = 0;
    _gt_calendar_st* style = (_gt_calendar_st*)obj;
    gt_font_info_init(&style->font_info);
    style->reg.fill = 1;
    style->font_info.palette = gt_color_white();
    style->font_color_today = gt_color_red();
    style->font_color_other_month = gt_color_hex(0x404040);
    style->font_color_weekend = gt_color_hex(0x808080);
    style->font_align = GT_ALIGN_CENTER_MID;

    return obj;
}

void gt_calendar_set_cn(gt_obj_st * obj, bool is_cn)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_calendar_st * style = (_gt_calendar_st * )obj;
    style->reg.is_cn = is_cn;
}

void gt_calendar_set_today(gt_obj_st * obj, uint16_t year, uint8_t month, uint8_t day)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }

    _gt_calendar_st * style = (_gt_calendar_st * )obj;
    style->today.year = year;
    style->today.month = (month % 12) == 0 ? 12 : (month % 12);
    if(day > _get_days_in_month(style->today.year, style->today.month)) {
        day = _get_days_in_month(style->today.year, style->today.month);
    }
    style->today.day = day;
    style->today.week = _get_week_day(year, month, day);

    //
    style->show_days = style->today;
    //
    style->day_type.year = style->today.year;
}

void gt_calendar_set_day_type(gt_obj_st * obj, uint16_t year, uint8_t month, uint8_t day, gt_calendar_day_type_et day_type)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_calendar_st * style = (_gt_calendar_st * )obj;
    if(year != style->day_type.year) {
        return ;
    }

    _set_day_type(style->day_type.month[month%12], day, day_type);
}


void gt_calendar_set_border_width(gt_obj_st * obj, uint8_t width)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_calendar_st * style = (_gt_calendar_st * )obj;
    style->border_width = width;
}

void gt_calendar_set_border_color(gt_obj_st * obj, gt_color_t color)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_calendar_st * style = (_gt_calendar_st * )obj;
    style->border_color = color;
}

void gt_calendar_set_font_color_to_today(gt_obj_st * obj, gt_color_t color)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_calendar_st * style = (_gt_calendar_st * )obj;
    style->font_color_today = color;
}

void gt_calendar_set_font_color_to_weekend(gt_obj_st * obj, gt_color_t color)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_calendar_st * style = (_gt_calendar_st * )obj;
    style->font_color_weekend = color;
}

void gt_calendar_set_font_color_to_other_month(gt_obj_st * obj, gt_color_t color)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_calendar_st * style = (_gt_calendar_st * )obj;
    style->font_color_other_month = color;
}

void gt_calendar_set_font_color(gt_obj_st * obj, gt_color_t color)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_calendar_st * style = (_gt_calendar_st * )obj;
    style->font_info.palette = color;
}

void gt_calendar_set_font_align(gt_obj_st * obj, gt_align_et align)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_calendar_st * style = (_gt_calendar_st * )obj;
    style->font_align = align;
}

void gt_calendar_set_font_size(gt_obj_st * obj, uint8_t size)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_calendar_st * style = (_gt_calendar_st * )obj;
    style->font_info.size = size;
}

void gt_calendar_set_font_gray(gt_obj_st * obj, uint8_t gray)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_calendar_st * style = (_gt_calendar_st * )obj;
    style->font_info.gray = gray;
}

void gt_calendar_set_font_thick_en(gt_obj_st * obj, uint8_t thick)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_calendar_st * style = (_gt_calendar_st * )obj;
    style->font_info.thick_en = thick;
}
void gt_calendar_set_font_thick_cn(gt_obj_st * obj, uint8_t thick)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_calendar_st * style = (_gt_calendar_st * )obj;
    style->font_info.thick_cn = thick;
}

void gt_calendar_set_font_style(gt_obj_st * obj, gt_font_style_et font_style)
{
    if( false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_calendar_st * style = (_gt_calendar_st * )obj;
    style->font_info.style.all = font_style;
}

#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
void gt_calendar_set_font_family_cn(gt_obj_st * obj, gt_family_t font_family_cn)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_calendar_st * style = (_gt_calendar_st * )obj;
    style->font_info.style_cn = font_family_cn;
}

void gt_calendar_set_font_family_en(gt_obj_st * obj, gt_family_t font_family_en)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_calendar_st * style = (_gt_calendar_st * )obj;
    style->font_info.style_en = font_family_en;
}

void gt_calendar_set_font_family_fl(gt_obj_st * obj, gt_family_t font_family_fl)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_calendar_st * style = (_gt_calendar_st * )obj;
    style->font_info.style_fl = font_family_fl;
}

void gt_calendar_set_font_family_numb(gt_obj_st * obj, gt_family_t font_family_numb)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_calendar_st * style = (_gt_calendar_st * )obj;
    style->font_info.style_numb = font_family_numb;
}

#else
void gt_calendar_set_font_family(gt_obj_st * obj, gt_family_t font_family)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_calendar_st * style = (_gt_calendar_st * )obj;
    gt_font_set_family(&style->font_info, font_family);
}

void gt_calendar_set_font_cjk(gt_obj_st* obj, gt_font_cjk_et cjk)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_calendar_st * style = (_gt_calendar_st * )obj;
    style->font_info.cjk = cjk;
}
#endif


/* end of file ----------------------------------------------------------*/
#endif //!GT_CFG_ENABLE_CALENDAR

