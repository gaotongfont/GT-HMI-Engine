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

/* global functions / API interface -------------------------------------*/

/**
 * @brief Create a chat widget with sample messages.
 */
gt_obj_st * gt_examples_chat(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);

    gt_obj_st * chat = gt_chat_create(screen);
    gt_obj_set_pos(chat, 0, 0);
    gt_obj_set_size(chat, 320, 200);
    gt_chat_set_bg_color(chat, gt_color_hex(0xF0F0F0));
    gt_chat_set_font_size(chat, 14);
    gt_chat_set_msg_radius(chat, 8);
    gt_chat_set_msg_space(chat, 0, 4);
    gt_chat_set_send_msg_bg_color(chat, gt_color_hex(0x95EC69));
    gt_chat_set_received_msg_bg_color(chat, gt_color_white());
    gt_chat_set_send_msg_font_color(chat, gt_color_black());
    gt_chat_set_received_msg_font_color(chat, gt_color_black());

    gt_chat_add_received_text_msg(chat, "Hello!");
    gt_chat_add_send_text_msg(chat, "Hi, how are you?");
    gt_chat_add_received_text_msg(chat, "I'm fine, thanks.");
    gt_chat_add_send_text_msg(chat, "Great to hear!");
    gt_chat_add_received_text_msg(chat, "I need to work right now.");
    gt_chat_add_send_text_msg(chat, "Nice! bye.");

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
