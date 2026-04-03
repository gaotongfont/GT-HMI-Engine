/**
 * @file gt_chat.h
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2024-03-19 14:31:18
 * @copyright Copyright (c) 2014-2024, Company Genitop. Co., Ltd.
 */
#ifndef _GT_CHAT_H_
#define _GT_CHAT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_CHAT
#include "./gt_obj.h"
#include "../core/gt_style.h"
#include "../font/gt_font.h"
#include "../widgets/gt_btn.h"
#include "../widgets/gt_textarea.h"

/* define ---------------------------------------------------------------*/
#define GT_CHAT_MAX_MSG_CNT    (20)



/* typedef --------------------------------------------------------------*/
typedef enum {
    GT_CHAT_TYPE_TEXT = 0,
    GT_CHAT_TYPE_VOICE,
}gt_chat_type_te;

typedef struct gt_chat_item_cb_s
{
    gt_obj_st* (*create_cb)( gt_obj_st* );
    void (*set_text_by_len_cb)(gt_obj_st * , const char * , uint16_t );
    void (*set_text_cb)(gt_obj_st * , const char * , ...);
    void (*append_text_cb)(gt_obj_st * , const char * );
    void (*set_color_background_cb)(gt_obj_st * , gt_color_t );
    void (*set_color_border_cb)(gt_obj_st * , gt_color_t);
    void (*set_border_width_cb)(gt_obj_st * , uint8_t );
    void (*set_font_color_cb)(gt_obj_st * , gt_color_t );
    void (*set_font_size_cb)(gt_obj_st * , uint8_t );
    void (*set_font_gray_cb)(gt_obj_st * , uint8_t );
    void (*set_font_align_cb)(gt_obj_st * , gt_align_et );
#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
    void (*set_font_family_cn_cb)(gt_obj_st * , gt_family_t );
    void (*set_font_family_en_cb)(gt_obj_st * , gt_family_t );
    void (*set_font_family_fl_cb)(gt_obj_st * , gt_family_t );
    void (*set_font_family_numb_cb)(gt_obj_st * , gt_family_t );
#else
    void (*set_font_family_cb)(gt_obj_st * , gt_family_t );
    void (*set_font_cjk_cb)(gt_obj_st* , gt_font_cjk_et );
#endif
    void (*set_font_thick_en_cb)(gt_obj_st * , uint8_t );
    void (*set_font_thick_cn_cb)(gt_obj_st * , uint8_t );
    void (*set_font_encoding_cb)(gt_obj_st * , gt_encoding_et );
    void (*set_font_style_cb)(gt_obj_st * , gt_font_style_et );
    void (*set_space_cb)(gt_obj_st * , uint8_t , uint8_t );
    void (*set_indent_cb)(gt_obj_st * , uint16_t );
    void (*set_start_point_offset_cb)(gt_obj_st* , uint8_t , uint8_t );
    gt_split_line_ret_st (*get_font_split_line_numb_cb)(gt_obj_st* , gt_font_split_line_st* );
}gt_chat_item_cb_st;
/* macros ---------------------------------------------------------------*/
#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
#define GT_CHAT_BTN_ITEM_CB_INIT(_item) do{\
    _item.create_cb = gt_btn_create; \
    _item.set_text_by_len_cb = gt_btn_set_text_by_len; \
    _item.set_text_cb = gt_btn_set_text; \
    _item.append_text_cb = gt_btn_append_text; \
    _item.set_color_background_cb = gt_btn_set_color_background; \
    _item.set_color_border_cb = gt_btn_set_color_border; \
    _item.set_border_width_cb = gt_btn_set_border_width; \
    _item.set_font_color_cb = gt_btn_set_font_color; \
    _item.set_font_size_cb = gt_btn_set_font_size; \
    _item.set_font_gray_cb = gt_btn_set_font_gray; \
    _item.set_font_align_cb = gt_btn_set_font_align; \
    _item.set_font_family_cn_cb = gt_btn_set_font_family_cn; \
    _item.set_font_family_en_cb = gt_btn_set_font_family_en; \
    _item.set_font_family_fl_cb = gt_btn_set_font_family_fl; \
    _item.set_font_family_numb_cb = gt_btn_set_font_family_numb; \
    _item.set_font_thick_en_cb = gt_btn_set_font_thick_en; \
    _item.set_font_thick_cn_cb = gt_btn_set_font_thick_cn; \
    _item.set_font_encoding_cb = gt_btn_set_font_encoding; \
    _item.set_font_style_cb = gt_btn_set_font_style; \
    _item.set_space_cb = gt_btn_set_space; \
    _item.set_indent_cb = gt_btn_set_indent; \
    _item.set_start_point_offset_cb = NULL; \
    _item.get_font_split_line_numb_cb = gt_btn_get_font_split_line_numb; \
}while(0)

#else
/* btn */
#define GT_CHAT_BTN_ITEM_CB_INIT(_item) do{\
    _item.create_cb = gt_btn_create; \
    _item.set_text_by_len_cb = gt_btn_set_text_by_len; \
    _item.set_text_cb = gt_btn_set_text; \
    _item.append_text_cb = gt_btn_append_text; \
    _item.set_color_background_cb = gt_btn_set_color_background; \
    _item.set_color_border_cb = gt_btn_set_color_border; \
    _item.set_border_width_cb = gt_btn_set_border_width; \
    _item.set_font_color_cb = gt_btn_set_font_color; \
    _item.set_font_size_cb = gt_btn_set_font_size; \
    _item.set_font_gray_cb = gt_btn_set_font_gray; \
    _item.set_font_align_cb = gt_btn_set_font_align; \
    _item.set_font_family_cb = gt_btn_set_font_family; \
    _item.set_font_cjk_cb = gt_btn_set_font_cjk; \
    _item.set_font_thick_en_cb = gt_btn_set_font_thick_en; \
    _item.set_font_thick_cn_cb = gt_btn_set_font_thick_cn; \
    _item.set_font_encoding_cb = gt_btn_set_font_encoding; \
    _item.set_font_style_cb = gt_btn_set_font_style; \
    _item.set_space_cb = gt_btn_set_space; \
    _item.set_indent_cb = gt_btn_set_indent; \
    _item.set_start_point_offset_cb = NULL; \
    _item.get_font_split_line_numb_cb = gt_btn_get_font_split_line_numb; \
}while(0)

/* textarea */
#define GT_CHAT_TEXTAREA_ITEM_CB_INIT(_item) do{ \
    _item.create_cb = gt_textarea_create; \
    _item.set_text_by_len_cb = gt_textarea_set_text_by_len; \
    _item.set_text_cb = gt_textarea_set_text; \
    _item.append_text_cb = NULL; \
    _item.set_color_background_cb = gt_textarea_set_bg_color; \
    _item.set_color_border_cb = gt_textarea_set_border_color; \
    _item.set_border_width_cb = gt_textarea_set_border_width; \
    _item.set_font_color_cb = gt_textarea_set_font_color; \
    _item.set_font_size_cb = gt_textarea_set_font_size; \
    _item.set_font_gray_cb = gt_textarea_set_font_gray; \
    _item.set_font_align_cb = gt_textarea_set_font_align; \
    _item.set_font_family_cb = gt_textarea_set_font_family; \
    _item.set_font_cjk_cb = gt_textarea_set_font_cjk; \
    _item.set_font_thick_en_cb = gt_textarea_set_font_thick_en; \
    _item.set_font_thick_cn_cb = gt_textarea_set_font_thick_cn; \
    _item.set_font_encoding_cb = gt_textarea_set_font_encoding; \
    _item.set_font_style_cb = NULL; \
    _item.set_space_cb = gt_textarea_set_space; \
    _item.set_indent_cb = gt_textarea_set_indent; \
    _item.set_start_point_offset_cb = gt_textarea_set_start_point_offset; \
    _item.get_font_split_line_numb_cb = gt_textarea_get_font_split_line_numb; \
}while(0)

#endif

/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_chat_create(gt_obj_st * parent);

void gt_chat_set_bg_color(gt_obj_st * chat, gt_color_t color);

gt_obj_st * gt_chat_add_send_text_msg(gt_obj_st * chat, const char * msg);
gt_obj_st * gt_chat_add_send_text_msg_by_len(gt_obj_st * chat, const char * msg, uint16_t len);
gt_obj_st * gt_chat_add_send_voice_msg(gt_obj_st * chat, uint8_t tim_s);

gt_obj_st * gt_chat_add_received_text_msg(gt_obj_st * chat, const char * msg);
gt_obj_st * gt_chat_add_received_text_msg_by_len(gt_obj_st * chat, const char * msg, uint16_t len);
gt_obj_st * gt_chat_add_received_voice_msg(gt_obj_st * chat, uint8_t tim_s);

gt_obj_st * gt_chat_append_msg_text(gt_obj_st * chat, const char * msg);
gt_obj_st * gt_chat_append_msg_text_control_scroll(gt_obj_st * chat, const char * msg, bool is_scroll);
gt_obj_st * gt_chat_replace_msg_text(gt_obj_st * chat, const char * msg);
gt_obj_st * gt_chat_replace_msg_text_control_scroll(gt_obj_st * chat, const char * msg, bool is_scroll);

gt_obj_st * gt_chat_replace_msg_text_by_len(gt_obj_st * chat, const char * msg, uint16_t len);
gt_obj_st * gt_chat_replace_msg_text_by_len_control_scroll(gt_obj_st * chat, const char * msg, uint16_t len, bool is_scroll);

void gt_chat_clean_all_msg(gt_obj_st * chat);

void gt_chat_set_msg_radius(gt_obj_st * chat, gt_radius_t radius);
void gt_chat_set_msg_space(gt_obj_st * chat, uint8_t space_x, uint8_t space_y);

void gt_chat_set_send_msg_bg_color(gt_obj_st * chat, gt_color_t color);
void gt_chat_set_received_msg_bg_color(gt_obj_st * chat, gt_color_t color);
void gt_chat_set_send_msg_border_color(gt_obj_st * chat, gt_color_t color);
void gt_chat_set_received_msg_border_color(gt_obj_st * chat, gt_color_t color);
void gt_chat_set_send_msg_border_width(gt_obj_st * chat, uint8_t width);
void gt_chat_set_received_msg_border_width(gt_obj_st * chat, uint8_t width);
void gt_chat_set_send_msg_font_color(gt_obj_st * chat, gt_color_t color);
void gt_chat_set_received_msg_font_color(gt_obj_st * chat, gt_color_t color);

void gt_chat_set_font_size(gt_obj_st * chat, uint8_t size);
void gt_chat_set_font_gray(gt_obj_st * chat, uint8_t gray);

/**
 * @brief Set software scale target size for font rendering.
 *        The font library size (set by gt_chat_set_font_size) is used as source;
 *        the glyph is scaled to scale_size pixels before display.
 *        0 or equal to font size: no scaling.
 *        gray=1 uses nearest-neighbor; gray=2/4 uses bilinear interpolation.
 *
 * @param chat       chat obj
 * @param scale_size target display size in pixels, 0 = disabled
 */
void gt_chat_set_font_scale_size(gt_obj_st * chat, uint8_t scale_size);

#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
void gt_chat_set_font_family_cn(gt_obj_st * chat, gt_family_t font_family_cn);
void gt_chat_set_font_family_en(gt_obj_st * chat, gt_family_t font_family_en);
void gt_chat_set_font_family_fl(gt_obj_st * chat, gt_family_t font_family_fl);
void gt_chat_set_font_family_numb(gt_obj_st * chat, gt_family_t font_family_numb);
#else
void gt_chat_set_font_family(gt_obj_st * chat, gt_family_t font_family);
void gt_chat_set_font_cjk(gt_obj_st* chat, gt_font_cjk_et cjk);
#endif
void gt_chat_set_font_thick_en(gt_obj_st * chat, uint8_t thick);
void gt_chat_set_font_thick_cn(gt_obj_st * chat, uint8_t thick);
void gt_chat_set_font_style(gt_obj_st * chat, gt_font_style_et font_style);


void gt_chat_set_item_cb(gt_obj_st * chat, gt_chat_item_cb_st* cb);
void gt_chat_update_item_size(gt_obj_st * chat, bool is_scroll);

int32_t gt_chat_get_item_index(gt_obj_st* chat, gt_obj_st* item);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /** GT_CFG_ENABLE_CHATTING */

#endif //!_GT_CHAT_H_

/* end of file ----------------------------------------------------------*/

