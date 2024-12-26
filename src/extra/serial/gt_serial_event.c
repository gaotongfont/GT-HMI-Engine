/**
 * @file gt_serial_event.c
 * @author Feyoung
 * @brief Add serial event by set callback function template
 * @version 0.1
 * @date 2024-10-29 17:50:39
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "./gt_serial_event.h"

#if GT_USE_SERIAL && GT_USE_BIN_CONVERT
#include "../../core/gt_fs.h"
#include "../../core/gt_mem.h"
#include "../../utils/gt_bin_convert.h"
#include "../../others/gt_log.h"
#include  "../../others/gt_anim.h"
#include "../../core/gt_disp.h"
#include "./gt_serial_var.h"
#include "./gt_serial_show.h"
#include "./gt_serial_resource.h"
#include "../../widgets/gt_btn.h"
#include "../../widgets/gt_input.h"
#include "../../widgets/gt_label.h"
#include "../../widgets/gt_btnmap.h"
#include "../../widgets/gt_chat.h"
#include "../../widgets/gt_rect.h"
#include "../../widgets/gt_clock.h"
#include "../../widgets/gt_radio.h"
#include "../../widgets/gt_player.h"
#include "../../widgets/gt_keypad.h"
#include "../../widgets/gt_wordart.h"
#include "../../widgets/gt_roller.h"
#include "../../widgets/gt_checkbox.h"
#include "../../widgets/gt_listview.h"
#include "../../widgets/gt_textarea.h"
#include "../../widgets/gt_input_number.h"
#include "../../widgets/gt_status_bar.h"


/* private define -------------------------------------------------------*/
#define _GT_SERIAL_FILE_CFG_EVENT_ID    0

#define _SERIAL_EVENT_FILE_ID_LEN       1
#define _SERIAL_EVENT_TITLE_BYTE_LEN    6
#define _SERIAL_EVENT_TOTAL_PAGE_LEN    2
#define _SERIAL_EVENT_PER_DATA_LEN      1
#define _SERIAL_EVENT_RESERVED_LEN      6

#define _SERIAL_EVENT_TOTAL_LEN         (_SERIAL_EVENT_FILE_ID_LEN + _SERIAL_EVENT_TITLE_BYTE_LEN + _SERIAL_EVENT_TOTAL_PAGE_LEN + _SERIAL_EVENT_PER_DATA_LEN + _SERIAL_EVENT_RESERVED_LEN)

#define _SERIAL_EVENT_ADDR_DATA_LEN     4

#define _SERIAL_EVENT_MAX_EVENT_COUNT   0xFF    // 256

#define _SERIAL_EVENT_INVALID_TRIGGER   0xFF
#define _SERIAL_EVENT_INVALID_TYPE      0xFF


#define _SE_OFFSET_SRC          0x00
#define _SE_OFFSET_TARGET       0x02
#define _SE_OFFSET_WIDGET_TYPE  0x04
#define _SE_OFFSET_TRIGGER      0x05
#define _SE_OFFSET_EV_TYPE      0x06
#define _SE_OFFSET_VP           0x07
#define _SE_OFFSET_DATA         0x09


#ifndef _GT_SERIAL_EVENT_LOG
    /**
     * @brief [default: 0]
     */
    #define _GT_SERIAL_EVENT_LOG    0
#endif

/* private typedef ------------------------------------------------------*/
typedef struct {
    uint32_t event_count : 8;
    uint32_t addr : 24;
}gt_serial_event_addr_data_st;

typedef struct {
    uint32_t addr;
    uint8_t * page_list;
    uint16_t page_count;
    uint8_t id;
    uint8_t title[_SERIAL_EVENT_TITLE_BYTE_LEN];
    uint8_t data_len;
    bool inited;
    gt_serial_event_addr_data_st page_event;
    uint8_t * src_data;     // data_len * 256
}gt_serial_event_file_header_st;

typedef struct {
    gt_obj_st * parent;
    uint16_t event_type;
    uint8_t * data;
    uint8_t len;
}gt_serial_event_packet_st;

typedef gt_res_t ( * _serial_init_event_cb)(gt_serial_event_packet_st * packet);

typedef struct {
    gt_obj_st * obj_src;
    gt_obj_st * obj_target;
    gt_event_type_et type;
    gt_res_t res;
}_serial_base_st;

typedef struct {
    gt_obj_st * tar;
    union {
        gt_color_t color;
        gt_point_st pos;
        gt_point_st size;
        gt_opa_t opa;
        uint32_t value;
        uint8_t cache[8];
    }data;
    uint16_t vp;
}_serial_event_data_st;

/* static variables -----------------------------------------------------*/
static gt_serial_event_file_header_st _event_fh = {
    .page_count = 0,
    .inited = false,
};

/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static uint32_t _calc_abs_addr(uint32_t addr, uint16_t page_index) {
    return page_index * _SERIAL_EVENT_ADDR_DATA_LEN + addr + _SERIAL_EVENT_TOTAL_LEN;
}

static gt_serial_event_addr_data_st _get_event_addr_data(uint32_t offset_addr) {
    gt_serial_event_addr_data_st ret = {0};
    uint8_t tmp[_SERIAL_EVENT_ADDR_DATA_LEN] = {0};
    gt_fs_read_direct_physical(offset_addr, _SERIAL_EVENT_ADDR_DATA_LEN, tmp);

    ret.event_count = tmp[0];
    ret.addr = gt_convert_parse_by_len(&tmp[1], 3);
    return ret;
}

/**
 * @brief
 *
 * @param val
 * @return gt_event_type_et
 *          GT_EVENT_TYPE_NONE: no event;
 *          _SERIAL_EVENT_INVALID_TRIGGER: invalid trigger, need second check;
 */
static gt_event_type_et _get_trigger_type(uint8_t val) {
    uint8_t type[] = {
        GT_EVENT_TYPE_INPUT_PRESSED,
        GT_EVENT_TYPE_INPUT_PRESSING,
        GT_EVENT_TYPE_INPUT_RELEASED,
        GT_EVENT_TYPE_INPUT_SCROLL,
        _SERIAL_EVENT_INVALID_TRIGGER,
        _SERIAL_EVENT_INVALID_TRIGGER,
        GT_EVENT_TYPE_UPDATE_VALUE,
        GT_EVENT_TYPE_CHANGE_CHILD_REMOVE,
        GT_EVENT_TYPE_CHANGE_CHILD_REMOVED,
        GT_EVENT_TYPE_CHANGE_CHILD_ADD,
        GT_EVENT_TYPE_CHANGE_CHILD_DELETE,
        GT_EVENT_TYPE_CHANGE_CHILD_DELETED,
    };
    if (_SERIAL_EVENT_INVALID_TRIGGER == val) {
        return GT_EVENT_TYPE_NONE;
    }
    return type[val];
}

/**
 * @brief get base data
 *
 * @param packet
 * @param ev_cb [default: NULL] check if the event has the same callback before add
 * @return _serial_base_st
 */
static _serial_base_st _get_base_data(gt_serial_event_packet_st * packet, gt_event_cb_t ev_cb) {
    _serial_base_st ret = {
        .res = GT_RES_OK,
    };
    uint16_t idx = gt_convert_parse_u16(&packet->data[_SE_OFFSET_SRC]);
    if (idx < packet->parent->cnt_child) {
        ret.obj_src = packet->parent->child[idx];
    }
    if (NULL == ret.obj_src) {
        ret.res = GT_RES_INV;
        return ret;
    }
    idx = gt_convert_parse_u16(&packet->data[_SE_OFFSET_TARGET]);
    if (idx < packet->parent->cnt_child) {
        ret.obj_target = packet->parent->child[idx];
    }

    ret.type = _get_trigger_type(packet->data[_SE_OFFSET_TRIGGER]);
    if (GT_EVENT_TYPE_NONE == ret.type) {
        ret.res = GT_RES_INV;
        return ret;
    }
    if (ev_cb && gt_event_has_the_same_cb(ret.obj_src, ev_cb, ret.type)) {
        ret.res = GT_RES_FAIL;
        return ret;
    }
    return ret;
}

static void _free_user_data_cb(gt_event_st * e) {
    if (NULL == e->user_data) {
        return;
    }
    gt_mem_free(e->user_data);
    e->user_data = NULL;
}

static void _free_user_data_by_deleted_event(gt_obj_st * tar, void * user_data) {
    gt_obj_add_event_cb(tar, _free_user_data_cb, GT_EVENT_TYPE_CHANGE_DELETED, user_data);
}

static gt_res_t _common_get_vp_data(gt_serial_event_packet_st * packet, gt_event_cb_t ev_cb) {
    _serial_base_st base = _get_base_data(packet, ev_cb);
    if (GT_RES_OK != base.res) {
        return base.res;
    }
    _serial_event_data_st * user_data = gt_mem_malloc(sizeof(_serial_event_data_st));
    if (NULL == user_data) {
        return GT_RES_INV;
    }
    user_data->vp = gt_convert_parse_u16(&packet->data[_SE_OFFSET_VP]);
    user_data->tar = base.obj_target;
    gt_obj_add_event_cb(base.obj_src, ev_cb, base.type, user_data);
    _free_user_data_by_deleted_event(base.obj_src, user_data);
    return GT_RES_OK;
}

/* ------------------- init callback begin ------------------- */
static void _back_ground_color_event_cb(gt_event_st * e) {
    _serial_event_data_st * user_data = (_serial_event_data_st * )e->user_data;
    gt_obj_st * obj = user_data->tar;

    switch (gt_obj_class_get_type(obj)) {
        case GT_TYPE_BTN: {
            gt_btn_set_color_background(obj, user_data->data.color);
            break;
        }
        case GT_TYPE_INPUT: {
            gt_input_set_bg_color(obj, user_data->data.color);
            break;
        }
        case GT_TYPE_RECT: {
            gt_rect_set_bg_color(obj, user_data->data.color);
            break;
        }
        case GT_TYPE_TEXTAREA: {
            gt_textarea_set_bg_color(obj, user_data->data.color);
            break;
        }
        case GT_TYPE_SCREEN: {
            gt_screen_set_bgcolor(obj, user_data->data.color);
            break;
        }
        default:
            break;
    }
}

static gt_res_t _back_ground_color_init_cb(gt_serial_event_packet_st * packet) {
    _serial_base_st base = _get_base_data(packet, _back_ground_color_event_cb);
    if (GT_RES_OK != base.res) {
        return base.res;
    }
    uint8_t buffer[3] = {0};
    _serial_event_data_st * user_data = gt_mem_malloc(sizeof(_serial_event_data_st));
    if (NULL == user_data) {
        return GT_RES_INV;
    }
    gt_serial_var_buffer_get_reg(gt_convert_parse_u16(&packet->data[_SE_OFFSET_VP]), buffer, 3);
    if (0x01 == packet->data[_SE_OFFSET_DATA]) {
        user_data->data.color.full = gt_convert_parse_u16(buffer);
    } else {
        user_data->data.color = gt_color_make(buffer[0], buffer[1], buffer[2]);
    }
    user_data->tar = base.obj_target;
    gt_obj_add_event_cb(base.obj_src, _back_ground_color_event_cb, base.type, user_data);
    _free_user_data_by_deleted_event(base.obj_src, user_data);
    return GT_RES_OK;
}

static void _font_color_event_cb(gt_event_st * e) {
    _serial_event_data_st * user_data = (_serial_event_data_st * )e->user_data;
    gt_obj_st * obj = user_data->tar;

    switch (gt_obj_class_get_type(obj)) {
        case GT_TYPE_BTN: {
            gt_btn_set_font_color(obj, user_data->data.color);
            break;
        }
        case GT_TYPE_INPUT: {
            gt_input_set_font_color(obj, user_data->data.color);
            break;
        }
        case GT_TYPE_LAB: {
            gt_label_set_font_color(obj, user_data->data.color);
            break;
        }
        case GT_TYPE_RADIO: {
            gt_radio_set_font_color(obj, user_data->data.color);
            break;
        }
        case GT_TYPE_ROLLER: {
            gt_roller_set_font_color(obj, user_data->data.color);
            break;
        }
        case GT_TYPE_BTNMAP: {
            gt_btnmap_set_font_color(obj, user_data->data.color);
            break;
        }
        case GT_TYPE_KEYPAD: {
            gt_keypad_set_font_color(obj, user_data->data.color);
            break;
        }
        case GT_TYPE_CHAT: {
            break;
        }
        case GT_TYPE_CLOCK: {
            gt_clock_set_font_color(obj, user_data->data.color);
            break;
        }
        case GT_TYPE_CHECKBOX: {
            gt_checkbox_set_font_color(obj, user_data->data.color);
            break;
        }
        case GT_TYPE_LISTVIEW: {
            gt_listview_set_font_color(obj, user_data->data.color);
            break;
        }
        case GT_TYPE_TEXTAREA: {
            gt_textarea_set_font_color(obj, user_data->data.color);
            break;
        }
        case GT_TYPE_INPUT_NUMBER: {
            gt_input_number_set_font_color(obj, user_data->data.color);
            break;
        }
        case GT_TYPE_STATUS_BAR: {
            gt_status_bar_set_font_color(user_data->data.color);
            break;
        }
        default:
            break;
    }
}
static gt_res_t _font_color_init_cb(gt_serial_event_packet_st * packet) {
    _serial_base_st base = _get_base_data(packet, _font_color_event_cb);
    if (GT_RES_OK != base.res) {
        return base.res;
    }
    uint8_t buffer[3] = {0};
    _serial_event_data_st * user_data = gt_mem_malloc(sizeof(_serial_event_data_st));
    if (NULL == user_data) {
        return GT_RES_INV;
    }
    gt_serial_var_buffer_get_reg(gt_convert_parse_u16(&packet->data[_SE_OFFSET_VP]), buffer, 3);
    if (0x01 == packet->data[_SE_OFFSET_DATA]) {
        user_data->data.color.full = gt_convert_parse_u16(buffer);
    } else {
        user_data->data.color = gt_color_make(buffer[0], buffer[1], buffer[2]);
    }

    user_data->tar = base.obj_target;
    gt_obj_add_event_cb(base.obj_src, _font_color_event_cb, base.type, user_data);
    _free_user_data_by_deleted_event(base.obj_src, user_data);
    return GT_RES_OK;
}

static void _pos_change_event_cb(gt_event_st * e) {
    _serial_event_data_st * udata = (_serial_event_data_st * )e->user_data;
    gt_obj_set_pos(udata->tar, udata->data.pos.x, udata->data.pos.y);
}

static gt_res_t _pos_change_init_cb(gt_serial_event_packet_st * packet) {
    _serial_base_st base = _get_base_data(packet, _pos_change_event_cb);
    if (GT_RES_OK != base.res) {
        return base.res;
    }
    _serial_event_data_st * user_data = gt_mem_malloc(sizeof(_serial_event_data_st));
    if (NULL == user_data) {
        return GT_RES_INV;
    }
    user_data->data.pos.x = gt_convert_parse_i16(&packet->data[_SE_OFFSET_DATA]);
    user_data->data.pos.y = gt_convert_parse_i16(&packet->data[_SE_OFFSET_DATA + 2]);
    user_data->tar = base.obj_target;
    gt_obj_add_event_cb(base.obj_src, _pos_change_event_cb, base.type, user_data);
    _free_user_data_by_deleted_event(base.obj_src, user_data);
    return GT_RES_OK;
}

static void _size_change_event_cb(gt_event_st * e) {
    _serial_event_data_st * udata = (_serial_event_data_st * )e->user_data;
    gt_obj_set_size(udata->tar, udata->data.size.x, udata->data.size.y);
}

static gt_res_t _size_change_init_cb(gt_serial_event_packet_st * packet) {
    _serial_base_st base = _get_base_data(packet, _size_change_event_cb);
    if (GT_RES_OK != base.res) {
        return base.res;
    }
    uint8_t buffer[4] = {0};
    _serial_event_data_st * user_data = gt_mem_malloc(sizeof(_serial_event_data_st));
    if (NULL == user_data) {
        return GT_RES_INV;
    }
    user_data->data.size.x = gt_convert_parse_u16(&packet->data[_SE_OFFSET_DATA]);
    user_data->data.size.y = gt_convert_parse_u16(&packet->data[_SE_OFFSET_DATA + 2]);
    user_data->tar = base.obj_target;
    gt_obj_add_event_cb(base.obj_src, _size_change_event_cb, base.type, user_data);
    _free_user_data_by_deleted_event(base.obj_src, user_data);
    return GT_RES_OK;
}

static void _opa_change_event_cb(gt_event_st * e) {
    _serial_event_data_st * udata = (_serial_event_data_st * )e->user_data;
    gt_obj_set_opa(udata->tar, udata->data.opa);
}

static gt_res_t _opa_change_init_cb(gt_serial_event_packet_st * packet) {
    _serial_base_st base = _get_base_data(packet, _opa_change_event_cb);
    if (GT_RES_OK != base.res) {
        return base.res;
    }
    uint8_t buffer[2] = {0};
    _serial_event_data_st * user_data = gt_mem_malloc(sizeof(_serial_event_data_st));
    if (NULL == user_data) {
        return GT_RES_INV;
    }
    user_data->data.opa = packet->data[_SE_OFFSET_DATA];
    user_data->tar = base.obj_target;
    gt_obj_add_event_cb(base.obj_src, _opa_change_event_cb, base.type, user_data);
    _free_user_data_by_deleted_event(base.obj_src, user_data);
    return GT_RES_OK;
}

static void _text_change_event_cb(gt_event_st * e) {
    _serial_event_data_st * udata = (_serial_event_data_st * )e->user_data;
    uint16_t vp = udata->vp;
    uint16_t len = gt_serial_var_buffer_get_len_by_eof(vp, 0x0000);
    uint16_t len_full = gt_serial_var_buffer_get_len_by_eof(vp, 0xFFFF);
    len = GT_MAX(len, len_full);
    if (0 == len) {
        return;
    }
    char * pointer = NULL;
    len = gt_serial_var_buffer_get_pointer(vp, (const uint8_t **)&pointer, len);
    switch (gt_obj_class_get_type(udata->tar)) {
        case GT_TYPE_BTN: {
            gt_btn_set_text_by_len(udata->tar, pointer, len);
            break;
        }
        case GT_TYPE_INPUT: {
            gt_input_set_value_by_len(udata->tar, pointer, len);
            break;
        }
        case GT_TYPE_LAB: {
            gt_label_set_text_by_len(udata->tar, pointer, len);
            break;
        }
        case GT_TYPE_RADIO: {
            gt_radio_set_text_by_len(udata->tar, pointer, len);
            break;
        }
        case GT_TYPE_CHECKBOX: {
            gt_checkbox_set_text_by_len(udata->tar, pointer, len);
            break;
        }
        case GT_TYPE_TEXTAREA: {
            gt_textarea_set_text_by_len(udata->tar, pointer, len);
            break;
        }
        case GT_TYPE_WORDART: {
            gt_wordart_set_text_by_len(udata->tar, pointer, len);
            break;
        }
        default:
            break;
    }
}

static gt_res_t _text_change_init_cb(gt_serial_event_packet_st * packet) {
    return _common_get_vp_data(packet, _text_change_event_cb);
}

static void _visible_change_event_cb(gt_event_st * e) {
    _serial_event_data_st * udata = (_serial_event_data_st * )e->user_data;
    if (udata->data.value > 0x02) {
        return;
    }
    if (0x02 == udata->data.value) {
        gt_obj_set_visible(udata->tar, gt_obj_get_visible(udata->tar) ? false : true);
        return;
    }
    gt_obj_set_visible(udata->tar, udata->data.value);
}

static gt_res_t _visible_change_init_cb(gt_serial_event_packet_st * packet) {
    _serial_base_st base = _get_base_data(packet, _visible_change_event_cb);
    if (GT_RES_OK != base.res) {
        return base.res;
    }
    _serial_event_data_st * user_data = gt_mem_malloc(sizeof(_serial_event_data_st));
    if (NULL == user_data) {
        return GT_RES_INV;
    }
    user_data->data.value = packet->data[_SE_OFFSET_DATA];
    user_data->tar = base.obj_target;
    gt_obj_add_event_cb(base.obj_src, _visible_change_event_cb, base.type, user_data);
    _free_user_data_by_deleted_event(base.obj_src, user_data);
    return GT_RES_OK;
}

static void _move_anim_event_cb(gt_event_st * e) {
    _serial_event_data_st * udata = (_serial_event_data_st * )e->user_data;
    uint16_t vp = udata->vp;
    char * pointer = NULL;
    gt_serial_var_buffer_get_pointer(vp, (const uint8_t **)&pointer, 12);

    gt_anim_param_st param;
    gt_anim_param_init(&param);
    gt_anim_param_set_dst_point(&param, gt_convert_parse_i16(pointer), gt_convert_parse_i16(pointer + 2));
    gt_anim_param_set_time(&param, gt_convert_parse_u32(pointer + 4), gt_convert_parse_u32(pointer + 8));
    gt_anim_param_set_path_type(&param, GT_ANIM_PATH_TYPE_OVERSHOOT);
    gt_anim_pos_move(udata->tar, &param);
}

static gt_res_t _move_anim_init_cb(gt_serial_event_packet_st * packet) {
    return _common_get_vp_data(packet, _move_anim_event_cb);
}

static void _get_progress_event_cb(gt_event_st * e) {
    _serial_event_data_st * udata = (_serial_event_data_st * )e->user_data;
    uint16_t value = (uint16_t)gt_player_get_percentage(e->origin);
    switch (gt_obj_class_get_type(udata->tar)) {
        case GT_TYPE_LAB: {
            gt_label_set_text(udata->tar, "%d%%", value);
            break;
        }
        case GT_TYPE_WORDART: {
            gt_wordart_set_text(udata->tar, "%d", value);
            break;
        }
        default:
            break;
    }
}

static gt_res_t _get_progress_init_cb(gt_serial_event_packet_st * packet) {
    return _common_get_vp_data(packet, _get_progress_event_cb);
}

static void _disable_widget_event_cb(gt_event_st * e) {
    _serial_event_data_st * udata = (_serial_event_data_st * )e->user_data;
    gt_obj_set_disabled(udata->tar, true);
}

static gt_res_t _disable_widget_init_cb(gt_serial_event_packet_st * packet) {
    return _common_get_vp_data(packet, _disable_widget_event_cb);
}

static void _enable_widget_event_cb(gt_event_st * e) {
    _serial_event_data_st * udata = (_serial_event_data_st * )e->user_data;
    gt_obj_set_disabled(udata->tar, false);
}

static gt_res_t _enable_widget_init_cb(gt_serial_event_packet_st * packet) {
    return _common_get_vp_data(packet, _enable_widget_event_cb);
}

static void _del_chars_event_cb(gt_event_st * e) {
    _serial_event_data_st * udata = (_serial_event_data_st * )e->user_data;
    gt_input_del_value(udata->tar);
}

static gt_res_t _del_chars_init_cb(gt_serial_event_packet_st * packet) {
    return _common_get_vp_data(packet, _del_chars_event_cb);
}

static void _append_chars_event_cb(gt_event_st * e) {
    _serial_event_data_st * udata = (_serial_event_data_st * )e->user_data;
    uint16_t vp = udata->vp;
    uint16_t len = gt_serial_var_buffer_get_len_by_eof(vp, 0x0000);
    uint16_t len_full = gt_serial_var_buffer_get_len_by_eof(vp, 0xFFFF);
    len = GT_MAX(len, len_full);
    if (0 == len) {
        return;
    }
    char * pointer = NULL;
    len = gt_serial_var_buffer_get_pointer(vp, (const uint8_t **)&pointer, len);

    gt_input_append_char(udata->tar, pointer[0]);
}

static gt_res_t _append_chars_init_cb(gt_serial_event_packet_st * packet) {
    return _common_get_vp_data(packet, _append_chars_event_cb);
}

static void _load_prev_event_cb(gt_event_st * e) {
    _serial_event_data_st * udata = (_serial_event_data_st * )e->user_data;
    gt_player_turn_prev(udata->tar);
}

static gt_res_t _load_prev_item_init_cb(gt_serial_event_packet_st * packet) {
    return _common_get_vp_data(packet, _load_prev_event_cb);
}

static void _load_next_event_cb(gt_event_st * e) {
    _serial_event_data_st * udata = (_serial_event_data_st * )e->user_data;
    gt_player_turn_next(udata->tar);
}

static gt_res_t _load_next_item_init_cb(gt_serial_event_packet_st * packet) {
    return _common_get_vp_data(packet, _load_next_event_cb);
}

static void _toggle_player_event_cb(gt_event_st * e) {
    _serial_event_data_st * udata = (_serial_event_data_st * )e->user_data;
    gt_player_toggle(udata->tar);
}

static gt_res_t _toggle_player_init_cb(gt_serial_event_packet_st * packet) {
    return _common_get_vp_data(packet, _toggle_player_event_cb);
}

static void _number_increase_event_cb(gt_event_st * e) {
    _serial_event_data_st * udata = (_serial_event_data_st * )e->user_data;
    gt_input_number_increase(udata->tar);
}

static gt_res_t _number_increase_init_cb(gt_serial_event_packet_st * packet) {
    return _common_get_vp_data(packet, _number_increase_event_cb);
}

static void _number_decrease_event_cb(gt_event_st * e) {
    _serial_event_data_st * udata = (_serial_event_data_st * )e->user_data;
    gt_input_number_decrease(udata->tar);
}

static gt_res_t _number_decrease_init_cb(gt_serial_event_packet_st * packet) {
    return _common_get_vp_data(packet, _number_decrease_event_cb);
}

static void _number_input_event_cb(gt_event_st * e) {
    _serial_event_data_st * udata = (_serial_event_data_st * )e->user_data;
    uint16_t vp = udata->vp;
    char * pointer = NULL;
    uint16_t len = gt_serial_var_buffer_get_pointer(vp, (const uint8_t **)&pointer, 8);
    gt_input_number_set_display_integer_length(udata->tar, udata->data.cache[0]);
    gt_input_number_set_display_decimal_length(udata->tar, udata->data.cache[1]);
    switch (udata->data.cache[2]) {
        case 0x00: {
            if (len < 2) { return; }
            gt_input_number_set_value(udata->tar, gt_convert_parse_i16(pointer));
            break;
        }
        case 0x01: {
            if (len < 4) { return; }
            gt_input_number_set_value(udata->tar, gt_convert_parse_i32(pointer));
            break;
        }
        case 0x02: {
            if (len < 1) { return; }
            gt_input_number_set_value(udata->tar, (uint8_t)pointer[0]);
            break;
        }
        case 0x03: {
            if (len < 2) { return; }
            gt_input_number_set_value(udata->tar, (uint8_t)pointer[1]);
            break;
        }
        case 0x04: {
            if (len < 8) { return; }
            gt_input_number_set_value(udata->tar, gt_convert_parse_i64(pointer));
            break;
        }
        case 0x05: {
            if (len < 2) { return; }
            gt_input_number_set_value(udata->tar, gt_convert_parse_u16(pointer));
            break;
        }
        case 0x06: {
            if (len < 4) { return; }
            gt_input_number_set_value(udata->tar, gt_convert_parse_u32(pointer));
            break;
        }
        case 0x07: {
            if (len < 4) { return; }
            gt_input_number_set_value(udata->tar, gt_convert_parse_float(pointer));
            break;
        }
        case 0x08: {
            if (len < 8) { return; }
            gt_input_number_set_value(udata->tar, gt_convert_parse_double(pointer));
            break;
        }
        default:
            break;
    }
}

static gt_res_t _number_input_init_cb(gt_serial_event_packet_st * packet) {
    _serial_base_st base = _get_base_data(packet, _number_input_event_cb);
    if (GT_RES_OK != base.res) {
        return base.res;
    }
    _serial_event_data_st * user_data = gt_mem_malloc(sizeof(_serial_event_data_st));
    if (NULL == user_data) {
        return GT_RES_INV;
    }
    user_data->vp = gt_convert_parse_u16(&packet->data[_SE_OFFSET_VP]);
    user_data->tar = base.obj_target;
    /** len_integer, len_decimal, value_type */
    gt_memcpy(user_data->data.cache, &packet->data[_SE_OFFSET_DATA], 3);
    gt_obj_add_event_cb(base.obj_src, _number_input_event_cb, base.type, user_data);
    _free_user_data_by_deleted_event(base.obj_src, user_data);
    return GT_RES_OK;
}

static void _next_second_event_cb(gt_event_st * e) {
    _serial_event_data_st * udata = (_serial_event_data_st * )e->user_data;
    gt_clock_turn_next_second(udata->tar);
}

static gt_res_t _next_second_init_cb(gt_serial_event_packet_st * packet) {
    return _common_get_vp_data(packet, _next_second_event_cb);
}

static void _prev_second_event_cb(gt_event_st * e) {
    _serial_event_data_st * udata = (_serial_event_data_st * )e->user_data;
    gt_clock_turn_prev_second(udata->tar);
}

static gt_res_t _prev_second_init_cb(gt_serial_event_packet_st * packet) {
    return _common_get_vp_data(packet, _prev_second_event_cb);
}

static void _start_clock_event_cb(gt_event_st * e) {
    _serial_event_data_st * udata = (_serial_event_data_st * )e->user_data;
    gt_clock_start(udata->tar);
}

static gt_res_t _start_clock_init_cb(gt_serial_event_packet_st * packet) {
    return _common_get_vp_data(packet, _start_clock_event_cb);
}

static void _pause_clock_event_cb(gt_event_st * e) {
    _serial_event_data_st * udata = (_serial_event_data_st * )e->user_data;
    gt_clock_stop(udata->tar);
}

static gt_res_t _pause_clock_init_cb(gt_serial_event_packet_st * packet) {
    return _common_get_vp_data(packet, _pause_clock_event_cb);
}

static void _toggle_clock_event_cb(gt_event_st * e) {
    _serial_event_data_st * udata = (_serial_event_data_st * )e->user_data;
    gt_clock_toggle(udata->tar);
}

static gt_res_t _toggle_clock_init_cb(gt_serial_event_packet_st * packet) {
    return _common_get_vp_data(packet, _toggle_clock_event_cb);
}

static void _change_clock_event_cb(gt_event_st * e) {
    _serial_event_data_st * udata = (_serial_event_data_st * )e->user_data;
    uint16_t vp = udata->vp;
    char * pointer = NULL;
    uint16_t len = gt_serial_var_buffer_get_pointer(vp, (const uint8_t **)&pointer, 4);
    gt_clock_set_time(udata->tar, pointer[0], pointer[1], pointer[2]);
}

static gt_res_t _change_clock_init_cb(gt_serial_event_packet_st * packet) {
    return _common_get_vp_data(packet, _change_clock_event_cb);
}

static void _select_prev_event_cb(gt_event_st * e) {
    _serial_event_data_st * udata = (_serial_event_data_st * )e->user_data;
    gt_roller_go_prev(udata->tar);
}

static gt_res_t _select_prev_init_cb(gt_serial_event_packet_st * packet) {
    return _common_get_vp_data(packet, _select_prev_event_cb);
}

static void _select_next_event_cb(gt_event_st * e) {
    _serial_event_data_st * udata = (_serial_event_data_st * )e->user_data;
    gt_roller_go_next(udata->tar);
}

static gt_res_t _select_next_init_cb(gt_serial_event_packet_st * packet) {
    return _common_get_vp_data(packet, _select_next_event_cb);
}

static void _slider_event_cb(gt_event_st * e) {
    _serial_event_data_st * udata = (_serial_event_data_st * )e->user_data;
    uint16_t vp = udata->vp;
    char * pointer = NULL;
    uint16_t len = gt_serial_var_buffer_get_pointer(vp, (const uint8_t **)&pointer, 2);
    uint16_t value = gt_convert_parse_u16(pointer);

    gt_event_type_et type[] = {
        GT_EVENT_TYPE_NONE,
        GT_EVENT_TYPE_INPUT_SCROLL_UP,
        GT_EVENT_TYPE_INPUT_SCROLL_DOWN,
        GT_EVENT_TYPE_INPUT_SCROLL_LEFT,
        GT_EVENT_TYPE_INPUT_SCROLL_RIGHT,
    };
    gt_event_send(udata->tar, value < 5 ? type[value] : type[0], NULL);
}

static gt_res_t _slider_init_cb(gt_serial_event_packet_st * packet) {
    return _common_get_vp_data(packet, _slider_event_cb);
}

static void _update_text_event_cb(gt_event_st * e) {
    _serial_event_data_st * udata = (_serial_event_data_st * )e->user_data;
    uint8_t buffer[128] = {0};
    if (false == gt_roller_get_selected_text(e->target, buffer, 128)) {
        return;
    }
    gt_label_set_text(udata->tar, buffer);
}

static gt_res_t _update_text_init_cb(gt_serial_event_packet_st * packet) {
    return _common_get_vp_data(packet, _update_text_event_cb);
}

static void _go_back_page_stack_event_cb(gt_event_st * e) {
    _serial_event_data_st * udata = (_serial_event_data_st * )e->user_data;
    gt_serial_go_back(udata->data.value);
}

static gt_res_t _go_back_page_stack_init_cb(gt_serial_event_packet_st * packet) {
    _serial_base_st base = _get_base_data(packet, _go_back_page_stack_event_cb);
    if (GT_RES_OK != base.res) {
        return base.res;
    }
    _serial_event_data_st * user_data = gt_mem_malloc(sizeof(_serial_event_data_st));
    if (NULL == user_data) {
        return GT_RES_INV;
    }
    user_data->vp = gt_convert_parse_u16(&packet->data[_SE_OFFSET_VP]);
    user_data->tar = base.obj_target;
    user_data->data.value = packet->data[_SE_OFFSET_DATA];
    gt_obj_add_event_cb(base.obj_src, _go_back_page_stack_event_cb, base.type, user_data);
    _free_user_data_by_deleted_event(base.obj_src, user_data);
    return GT_RES_OK;
}

// static void _show_event_cb(gt_event_st * e) {
//     _serial_event_data_st * udata = (_serial_event_data_st * )e->user_data;
// }

static gt_res_t _show_init_cb(gt_serial_event_packet_st * packet) {
    // return _common_get_vp_data(packet, _show_event_cb);
    return GT_RES_OK;
}

// static void _hide_event_cb(gt_event_st * e) {
//     _serial_event_data_st * udata = (_serial_event_data_st * )e->user_data;
// }

static gt_res_t _hide_init_cb(gt_serial_event_packet_st * packet) {
    // return _common_get_vp_data(packet, _hide_event_cb);
    return GT_RES_OK;
}

static gt_scr_anim_type_et _get_anim_type_by(uint8_t val) {
    gt_scr_anim_type_et arr[9] = {
        GT_SCR_ANIM_TYPE_NONE,
        GT_SCR_ANIM_TYPE_MOVE_LEFT,
        GT_SCR_ANIM_TYPE_MOVE_RIGHT,
        GT_SCR_ANIM_TYPE_MOVE_UP,
        GT_SCR_ANIM_TYPE_MOVE_DOWN,
        GT_SCR_ANIM_TYPE_COVER_LEFT,
        GT_SCR_ANIM_TYPE_COVER_RIGHT,
        GT_SCR_ANIM_TYPE_COVER_UP,
        GT_SCR_ANIM_TYPE_COVER_DOWN,
    };
    return val < 9 ? arr[val] : GT_SCR_ANIM_TYPE_NONE;
}

static void _load_page_event_cb(gt_event_st * e) {
    _serial_event_data_st * udata = (_serial_event_data_st * )e->user_data;
    uint16_t vp = udata->vp;
    char * pointer = NULL;
    uint16_t len = gt_serial_var_buffer_get_pointer(vp, (const uint8_t **)&pointer, 8);
    uint16_t total_ms = gt_convert_parse_u32(pointer);
    uint16_t delay_ms = gt_convert_parse_u32(pointer + 4);
    uint16_t scr_id = gt_convert_parse_u16(&udata->data.cache[0]);
    uint8_t free_prev = udata->data.cache[2] ? true : false;
    gt_scr_anim_type_et anim_type = _get_anim_type_by(udata->data.cache[3]);
    gt_serial_load_page_anim(scr_id, anim_type, total_ms, delay_ms, free_prev);
}

static gt_res_t _load_page_init_cb(gt_serial_event_packet_st * packet) {
    _serial_base_st base = _get_base_data(packet, _load_page_event_cb);
    if (GT_RES_OK != base.res) {
        return base.res;
    }
    _serial_event_data_st * user_data = gt_mem_malloc(sizeof(_serial_event_data_st));
    if (NULL == user_data) {
        return GT_RES_INV;
    }
    user_data->vp = gt_convert_parse_u16(&packet->data[_SE_OFFSET_VP]);
    gt_memcpy(user_data->data.cache, &packet->data[_SE_OFFSET_DATA], 4);
    gt_obj_add_event_cb(base.obj_src, _load_page_event_cb, base.type, user_data);
    _free_user_data_by_deleted_event(base.obj_src, user_data);
    return GT_RES_OK;
}

static void _load_wav_id_event_cb(gt_event_st * e) {
    _serial_event_data_st * udata = (_serial_event_data_st * )e->user_data;
    uint16_t wav_id = udata->data.value;
    // play wav here
}

static gt_res_t _load_wav_id_init_cb(gt_serial_event_packet_st * packet) {
    _serial_base_st base = _get_base_data(packet, _load_wav_id_event_cb);
    if (GT_RES_OK != base.res) {
        return base.res;
    }
    _serial_event_data_st * user_data = gt_mem_malloc(sizeof(_serial_event_data_st));
    if (NULL == user_data) {
        return GT_RES_INV;
    }
    user_data->vp = gt_convert_parse_u16(&packet->data[_SE_OFFSET_VP]);
    user_data->tar = base.obj_target;
    user_data->data.value = gt_convert_parse_u16(&packet->data[_SE_OFFSET_DATA]);
    gt_obj_add_event_cb(base.obj_src, _load_wav_id_event_cb, base.type, user_data);
    _free_user_data_by_deleted_event(base.obj_src, user_data);
    return GT_RES_OK;
}

static GT_ATTRIBUTE_LARGE_RAM_ARRAY const _serial_init_event_cb _serial_event_init_list_cb[] = {
    _back_ground_color_init_cb,
    _font_color_init_cb,
    _pos_change_init_cb,
    _size_change_init_cb,
    _opa_change_init_cb,
    _text_change_init_cb,
    _visible_change_init_cb,
    _move_anim_init_cb,
    _get_progress_init_cb,
    _disable_widget_init_cb,
    _enable_widget_init_cb,
    _del_chars_init_cb,
    _append_chars_init_cb,
    _load_prev_item_init_cb,
    _load_next_item_init_cb,
    _toggle_player_init_cb,
    _number_increase_init_cb,
    _number_decrease_init_cb,
    _number_input_init_cb,
    _next_second_init_cb,
    _prev_second_init_cb,
    _start_clock_init_cb,
    _pause_clock_init_cb,
    _toggle_clock_init_cb,
    _change_clock_init_cb,
    _select_prev_init_cb,
    _select_next_init_cb,
    _slider_init_cb,
    _update_text_init_cb,
    _go_back_page_stack_init_cb,
    _show_init_cb,
    _hide_init_cb,
    _load_page_init_cb,
    _load_wav_id_init_cb,
};

/* ------------------- init callback end ------------------- */

static gt_res_t _serial_add_event(gt_obj_st * parent, uint8_t * data, uint8_t len) {
    uint8_t event_type = data[_SE_OFFSET_EV_TYPE];
    if (_SERIAL_EVENT_INVALID_TYPE == event_type) {
        GT_LOG_A(GT_LOG_TAG_SERIAL, "invalid event type");
        return GT_RES_INV;
    }
    if (event_type >= sizeof(_serial_event_init_list_cb) / sizeof(_serial_event_init_list_cb[0])) {
        GT_LOGE(GT_LOG_TAG_SERIAL, "event type over list length: 0x%x", event_type);
        return GT_RES_INV;
    }
#if _GT_SERIAL_EVENT_LOG
    GT_LOG_A(GT_LOG_TAG_SERIAL, "Event type: 0x%x", event_type);
    GT_LOG_ARR(data, 0, len);
#endif
    gt_serial_event_packet_st packet = {
        .parent = parent,
        .event_type = event_type,
        .data = data,
        .len = len,
    };
    return _serial_event_init_list_cb[event_type](&packet);
}


/* global functions / API interface -------------------------------------*/
void gt_serial_event_init(void)
{
    uint8_t tmp[_SERIAL_EVENT_TOTAL_LEN] = {0};
    uint8_t cnt = 0;

    uint32_t addr = gt_serial_resource_get_addr_by(_GT_SERIAL_FILE_CFG_EVENT_ID);
    if (GT_SERIAL_RESOURCE_INVALID_ADDR == addr) {
        GT_LOGE(GT_LOG_TAG_SERIAL, "get event addr err");
        return;
    }
    _event_fh.addr = addr + GT_SERIAL_RESOURCE_EVENT_OFFSET;
#if GT_BOOTING_INFO_MSG
    GT_LOG_A(GT_LOG_TAG_SERIAL, "addr: 0x%x event addr: 0x%x", addr, _event_fh.addr);
#endif
    /** Read event header data */
    gt_fs_read_direct_physical(_event_fh.addr, _SERIAL_EVENT_TOTAL_LEN, tmp);

    _event_fh.id = tmp[cnt];
    cnt += _SERIAL_EVENT_FILE_ID_LEN;

    gt_memcpy(_event_fh.title, &tmp[cnt], 6);
    cnt += _SERIAL_EVENT_TITLE_BYTE_LEN;

    _event_fh.page_count = gt_convert_parse_u16(&tmp[cnt]);
    if (_event_fh.page_count > 4092) {
        _event_fh.page_count = 4092;
    }
    cnt += _SERIAL_EVENT_TOTAL_PAGE_LEN;

    uint8_t before_data_len = _event_fh.data_len;
    _event_fh.data_len = tmp[cnt];
    if (16 != _event_fh.data_len && 32 != _event_fh.data_len) {
        GT_LOGE(GT_LOG_TAG_SERIAL, "data len err: %d", _event_fh.data_len);
        return;
    }
    if (NULL == _event_fh.src_data) {
        _event_fh.src_data = (uint8_t*)gt_mem_malloc(_event_fh.data_len * _SERIAL_EVENT_MAX_EVENT_COUNT);
    } else if (before_data_len != _event_fh.data_len) {
        _event_fh.src_data = (uint8_t*)gt_mem_realloc(_event_fh.src_data, _event_fh.data_len * _SERIAL_EVENT_MAX_EVENT_COUNT);
    }
    if (NULL == _event_fh.src_data) {
        GT_LOGE(GT_LOG_TAG_SERIAL, "alloc event page data buffer err");
        return;
    }
#if GT_BOOTING_INFO_MSG
    GT_LOG_A(GT_LOG_TAG_SERIAL, "Event id: %d, data len: %d total: %d", _event_fh.id, _event_fh.data_len, _SERIAL_EVENT_TOTAL_LEN);
#endif
    _event_fh.inited = true;
}

gt_res_t gt_serial_event_init_by_page(uint16_t page_index, gt_obj_st * parent)
{
    if (false == _event_fh.inited) {
        return GT_RES_FAIL;
    }
    uint32_t address = _calc_abs_addr(_event_fh.addr, page_index);
#if _GT_SERIAL_EVENT_LOG
    GT_LOGD(GT_LOG_TAG_SERIAL, "_event_fh.addr: 0x%x, addr: 0x%x",  _event_fh.addr, address);
#endif
    gt_serial_event_addr_data_st addr_data = _get_event_addr_data(address);
    address = addr_data.addr + _event_fh.addr;
    // max event count: 0xff, 16 * 256 = 4096
    gt_fs_read_direct_physical(address, _event_fh.data_len * addr_data.event_count, _event_fh.src_data);
#if _GT_SERIAL_EVENT_LOG
    GT_LOGD(GT_LOG_TAG_SERIAL, "page: %d, event count: %d, address: 0x%x", page_index, addr_data.event_count, address);
#endif

    for (uint16_t i = 0, offset = 0; i < addr_data.event_count; ++i) {
        if (GT_RES_FAIL == _serial_add_event(parent, _event_fh.src_data + offset, _event_fh.data_len)) {
            GT_LOGE(GT_LOG_TAG_SERIAL, "add event failed, index: %d", i);
        }
        offset += _event_fh.data_len;
    }

    return GT_RES_OK;
}

/* end ------------------------------------------------------------------*/
#endif  /** GT_USE_SERIAL && GT_USE_BIN_CONVERT */
