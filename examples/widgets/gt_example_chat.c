/**
 * @file gt_example_chat.c
 * @author Feyoung
 * @brief Chat widget example
 * @version 0.1
 * @date 2026-03-27
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_chat.h"

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/

static void _chat_play_start_cb(gt_event_st * e)
{
    /* Application audio/TTS integration point. The payload is backward
     * compatible with the old index-only event and now also carries the
     * stable message ID and source/translation segment. */
    const gt_chat_play_event_st * play = (const gt_chat_play_event_st *)e->param;
    if (!play) return;
    (void)play->index;
    (void)play->message_id;
    (void)play->segment;
}

static void _chat_play_end_cb(gt_event_st * e)
{
    const gt_chat_play_event_st * play = (const gt_chat_play_event_st *)e->param;
    if (!play) return;
    (void)play->index;
    (void)play->message_id;
    (void)play->segment;
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief Create a chat widget with sample messages.
 */
gt_obj_st * gt_examples_chat(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);
    uint32_t translated_id;

    gt_obj_st * chat = gt_chat_create(screen);
    gt_obj_set_pos(chat, 0, 0);
    gt_obj_set_size(chat, 320, 200);
    gt_chat_set_bg_color(chat, gt_color_hex(0xF0F0F0));
    /* Voice symbols are supplied at 16x16/24x24; 16 keeps them visible in
     * the simulator and gives CJK glyphs enough vertical metrics. */
    gt_chat_set_font_size(chat, 16);
    gt_chat_set_msg_radius(chat, 8);
    gt_chat_set_msg_space(chat, 0, 4);
    gt_chat_set_send_msg_bg_color(chat, gt_color_hex(0x95EC69));
    gt_chat_set_received_msg_bg_color(chat, gt_color_white());
    gt_chat_set_send_msg_font_color(chat, gt_color_black());
    gt_chat_set_received_msg_font_color(chat, gt_color_black());
    gt_chat_set_received_msg_border_color(chat, gt_color_hex(0xD0D0D0));
    gt_chat_set_received_msg_border_width(chat, 1);
    gt_chat_set_send_msg_border_color(chat, gt_color_hex(0x6CCB4A));
    gt_chat_set_send_msg_border_width(chat, 1);
    gt_obj_add_event_cb(chat, _chat_play_start_cb, GT_EVENT_TYPE_INPUT_PLAY_START, NULL);
    gt_obj_add_event_cb(chat, _chat_play_end_cb, GT_EVENT_TYPE_INPUT_PLAY_END, NULL);

    gt_chat_add_received_text_msg(chat, "Hello!");
    gt_chat_add_send_text_msg(chat, "Hi, how are you?");
    gt_chat_add_received_text_msg(chat, "I'm fine, thanks.");
    gt_chat_add_send_text_msg(chat, "Great to hear!");
    /* Translation can arrive asynchronously: create the source first, then
     * update the same message by its stable ID. */
    gt_chat_add_received_translated_msg_with_time(chat,
        "Do you know how to start a common topic with others?", NULL, 4);
    translated_id = gt_chat_get_last_msg_id(chat);
    gt_chat_set_message_translation(chat, translated_id,
        "你知道如何和他人开启一个共同的话题吗?");
    gt_chat_set_message_audio_time(chat, translated_id, 5);
    gt_chat_set_message_segment_audio_time(chat, translated_id, GT_CHAT_PLAY_SOURCE, 3);
    gt_chat_set_message_segment_audio_time(chat, translated_id, GT_CHAT_PLAY_TRANSLATION, 5);
    gt_chat_add_received_text_msg(chat, "I need to work right now.");
    gt_chat_add_send_text_msg(chat, "Nice! bye.");

    /* Both source and translation playback paths are available to the app
     * audio/TTS layer. */
    gt_chat_play_message_segment(chat, translated_id, GT_CHAT_PLAY_SOURCE);
    gt_chat_play_message_segment(chat, translated_id, GT_CHAT_PLAY_TRANSLATION);

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
