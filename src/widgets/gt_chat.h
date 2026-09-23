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
/**
 * @brief Audio playback segment of a chat message.
 */
typedef enum {
    GT_CHAT_TYPE_TEXT = 0,
    GT_CHAT_TYPE_VOICE,
}gt_chat_type_te;

typedef enum {
    GT_CHAT_MSG_TYPE_NONE = 0,
    GT_CHAT_MSG_TYPE_SEND,
    GT_CHAT_MSG_TYPE_RECV,
}gt_chat_msg_type_te;

typedef enum {
    GT_CHAT_PLAY_SOURCE = 0,
    GT_CHAT_PLAY_TRANSLATION,
} gt_chat_play_segment_te;

/**
 * @brief Event payload identifying the message segment being played.
 */
typedef struct {
    int32_t index;                         /**< legacy array index */
    uint32_t message_id;                   /**< stable message ID */
    gt_chat_play_segment_te segment;       /**< source or translation */
} gt_chat_play_event_st;

typedef struct gt_chat_item_cb_s
{
    gt_obj_st* (*create_cb)( gt_obj_st* );
    void (*set_text_by_len_cb)(gt_obj_st * , const char * , uint32_t );
    void (*set_text_cb)(gt_obj_st * , const char * , ...);
    void (*append_text_cb)(gt_obj_st * , const char * );
    void (*set_color_background_cb)(gt_obj_st * , gt_color_t );
    void (*set_color_border_cb)(gt_obj_st * , gt_color_t);
    void (*set_border_width_cb)(gt_obj_st * , uint8_t );
    void (*set_font_color_cb)(gt_obj_st * , gt_color_t );
    void (*set_font_size_cb)(gt_obj_st * , gt_font_size_t);
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
    _item.set_text_by_len_cb = (void(*)(gt_obj_st*, const char*, uint32_t))gt_btn_set_text_by_len; \
    _item.set_text_cb = gt_btn_set_text; \
    _item.append_text_cb = gt_btn_append_text; \
    _item.set_color_background_cb = gt_btn_set_color_background; \
    _item.set_color_border_cb = gt_btn_set_color_border; \
    _item.set_border_width_cb = gt_btn_set_border_width; \
    _item.set_font_color_cb = gt_btn_set_font_color; \
    _item.set_font_size_cb = (void(*)(gt_obj_st*, gt_font_size_t))gt_btn_set_font_size; \
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
    _item.set_text_by_len_cb = (void(*)(gt_obj_st*, const char*, uint32_t))gt_btn_set_text_by_len; \
    _item.set_text_cb = gt_btn_set_text; \
    _item.append_text_cb = gt_btn_append_text; \
    _item.set_color_background_cb = gt_btn_set_color_background; \
    _item.set_color_border_cb = gt_btn_set_color_border; \
    _item.set_border_width_cb = gt_btn_set_border_width; \
    _item.set_font_color_cb = gt_btn_set_font_color; \
    _item.set_font_size_cb = (void(*)(gt_obj_st*, gt_font_size_t))gt_btn_set_font_size; \
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

/**
 * @brief Add a sent message containing source text and an optional translation.
 * @param chat Chat object.
 * @param source Source text.
 * @param translation Optional translated text; may be NULL.
 * @return Created message object, or NULL on failure.
 */
gt_obj_st * gt_chat_add_send_translated_msg(gt_obj_st * chat, const char * source, const char * translation);
/**
 * @brief Add a received message containing source text and an optional translation.
 * @param chat Chat object.
 * @param source Source text.
 * @param translation Optional translated text; may be NULL.
 * @return Created message object, or NULL on failure.
 */
gt_obj_st * gt_chat_add_received_translated_msg(gt_obj_st * chat, const char * source, const char * translation);
/**
 * @brief Add a sent translated message and set its voice duration.
 * @param chat Chat object.
 * @param source Source text.
 * @param translation Optional translated text; may be NULL.
 * @param tim_s Voice duration in seconds.
 * @return Created message object, or NULL on failure.
 */
gt_obj_st * gt_chat_add_send_translated_msg_with_time(gt_obj_st * chat, const char * source, const char * translation, uint8_t tim_s);
/**
 * @brief Add a received translated message and set its voice duration.
 * @param chat Chat object.
 * @param source Source text.
 * @param translation Optional translated text; may be NULL.
 * @param tim_s Voice duration in seconds.
 * @return Created message object, or NULL on failure.
 */
gt_obj_st * gt_chat_add_received_translated_msg_with_time(gt_obj_st * chat, const char * source, const char * translation, uint8_t tim_s);
/**
 * @brief Replace the last text message translation.
 * @param chat Chat object.
 * @param translation New translated text; pass NULL to clear it.
 * @return Updated message object, or NULL on failure.
 */
gt_obj_st * gt_chat_set_last_msg_translation(gt_obj_st * chat, const char * translation);

/**
 * @brief Set the generic Tips text for a message.
 * @param chat Chat object.
 * @param message_id Stable message ID.
 * @param text Tips text; NULL or an empty string clears the Tips.
 * @return Tips button object, or NULL when the message does not exist,
 *         allocation fails, or the Tips is cleared.
 * @note The returned object is owned by the chat and is valid only while the
 *       message exists. The caller may use it to customize its style.
 */
gt_obj_st * gt_chat_set_message_tips(gt_obj_st * chat, uint32_t message_id, const char * text);

/**
 * @brief Set the generic Tips text using an explicit byte length.
 * @param chat Chat object.
 * @param message_id Stable message ID.
 * @param text Tips text buffer.
 * @param len Byte length of text, excluding any terminator.
 * @return Tips button object, or NULL on failure.
 */
gt_obj_st * gt_chat_set_message_tips_by_len(gt_obj_st * chat, uint32_t message_id, const char * text, uint16_t len);

/**
 * @brief Set the Tips text for the last message.
 * @param chat Chat object.
 * @param text Tips text; NULL or an empty string clears the Tips.
 * @return Tips button object, or NULL when there is no message or the Tips is cleared.
 */
gt_obj_st * gt_chat_set_last_msg_tips(gt_obj_st * chat, const char * text);

/**
 * @brief Clear the Tips attached to a message.
 * @param chat Chat object.
 * @param message_id Stable message ID.
 * @return true on success, otherwise false.
 */
bool gt_chat_clear_message_tips(gt_obj_st * chat, uint32_t message_id);

/**
 * @brief Clear the Tips attached to the last message.
 * @param chat Chat object.
 * @return true on success, otherwise false.
 */
bool gt_chat_clear_last_msg_tips(gt_obj_st * chat);

/**
 * @brief Get the Tips button object for a message.
 * @param chat Chat object.
 * @param message_id Stable message ID.
 * @return Tips object, or NULL when the message has no Tips.
 * @note The returned object is owned by the chat and becomes invalid when
 *       the message is removed, the chat is cleared, or the chat is destroyed.
 */
gt_obj_st * gt_chat_get_message_tips_obj(gt_obj_st * chat, uint32_t message_id);

/**
 * @brief Get the Tips button object for the last message.
 * @param chat Chat object.
 * @return Tips object, or NULL when there is no message or no Tips.
 */
gt_obj_st * gt_chat_get_last_msg_tips_obj(gt_obj_st * chat);
/**
 * @brief Replace a message translation by stable message ID.
 * @param chat Chat object.
 * @param message_id Stable message ID.
 * @param translation New translated text; pass NULL to clear it.
 * @return true on success, otherwise false.
 */
bool gt_chat_set_message_translation(gt_obj_st * chat, uint32_t message_id, const char * translation);
/**
 * @brief Set the voice duration of the last message.
 * @param chat Chat object.
 * @param tim_s Voice duration in seconds.
 */
void gt_chat_set_last_msg_audio_time(gt_obj_st * chat, uint8_t tim_s);
/**
 * @brief Set a message voice duration by stable message ID.
 * @param chat Chat object.
 * @param message_id Stable message ID.
 * @param tim_s Voice duration in seconds.
 * @return true on success, otherwise false.
 */
bool gt_chat_set_message_audio_time(gt_obj_st * chat, uint32_t message_id, uint8_t tim_s);
/**
 * @brief Set the voice duration of a specific message segment.
 * @param chat Chat object.
 * @param message_id Stable message ID.
 * @param segment Source or translation segment.
 * @param tim_s Voice duration in seconds.
 * @return true on success, otherwise false.
 */
bool gt_chat_set_message_segment_audio_time(gt_obj_st * chat, uint32_t message_id,
                                            gt_chat_play_segment_te segment, uint8_t tim_s);
/**
 * @brief Get the stable ID assigned to the last message.
 * @param chat Chat object.
 * @return Stable message ID, or zero when unavailable.
 */
uint32_t gt_chat_get_last_msg_id(gt_obj_st * chat);
/**
 * @brief Start playback for a specific message segment.
 *
 * A GT_EVENT_TYPE_INPUT_PLAY_START event is emitted when the animation starts;
 * register an event callback on the chat object to request audio data.
 * @param chat Chat object.
 * @param message_id Stable message ID.
 * @param segment Source or translation segment.
 * @return true when playback starts, otherwise false.
 */
bool gt_chat_play_message_segment(gt_obj_st * chat, uint32_t message_id, gt_chat_play_segment_te segment);
/**
 * @brief Stop the currently playing chat message segment.
 *
 * Emits GT_EVENT_TYPE_INPUT_PLAY_END for the active segment, allowing the
 * audio owner to stop decoding and release its playback resources.
 * @param chat Chat object.
 */
void gt_chat_stop_message_playback(gt_obj_st * chat);

gt_obj_st * gt_chat_append_msg_text(gt_obj_st * chat, const char * msg);
gt_obj_st * gt_chat_append_msg_text_control_scroll(gt_obj_st * chat, const char * msg, bool is_scroll);
gt_obj_st * gt_chat_replace_msg_text(gt_obj_st * chat, const char * msg);
gt_obj_st * gt_chat_replace_msg_text_control_scroll(gt_obj_st * chat, const char * msg, bool is_scroll);

gt_obj_st * gt_chat_replace_msg_text_by_len(gt_obj_st * chat, const char * msg, uint16_t len);
gt_obj_st * gt_chat_replace_msg_text_by_len_control_scroll(gt_obj_st * chat, const char * msg, uint16_t len, bool is_scroll);

gt_chat_msg_type_te gt_chat_get_last_msg_type(gt_obj_st * chat);

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

void gt_chat_set_font_size(gt_obj_st * chat, gt_font_size_t size);
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
void gt_chat_set_font_scale_size(gt_obj_st * chat, gt_font_size_t scale_size);

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
