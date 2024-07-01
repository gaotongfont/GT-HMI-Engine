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


/* define ---------------------------------------------------------------*/
#define GT_CHAT_MAX_MSG_CNT    (20)



/* typedef --------------------------------------------------------------*/
typedef enum {
    GT_CHAT_TYPE_TEXT = 0,
    GT_CHAT_TYPE_VOICE,
}gt_chat_type_te;


/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_chat_create(gt_obj_st * parent);

void gt_chat_set_bg_color(gt_obj_st * chat, gt_color_t color);

void gt_chat_add_send_text_msg(gt_obj_st * chat, const char * msg);
void gt_chat_add_send_voice_msg(gt_obj_st * chat, uint8_t tim_s);

void gt_chat_add_received_text_msg(gt_obj_st * chat, const char * msg);
void gt_chat_add_received_voice_msg(gt_obj_st * chat, uint8_t tim_s);

void gt_chat_msg_set_color_background(gt_obj_st * chat, gt_color_t color);
void gt_chat_msg_set_color_border(gt_obj_st * chat, gt_color_t color);
void gt_chat_msg_set_border_width(gt_obj_st * chat, uint8_t width);
void gt_chat_set_font_color(gt_obj_st * chat, gt_color_t color);
void gt_chat_set_font_size(gt_obj_st * chat, uint8_t size);
void gt_chat_set_font_gray(gt_obj_st * chat, uint8_t gray);
void gt_chat_set_font_family_cn(gt_obj_st * chat, gt_family_t font_family_cn);
void gt_chat_set_font_family_en(gt_obj_st * chat, gt_family_t font_family_en);
void gt_chat_set_font_family_fl(gt_obj_st * chat, gt_family_t font_family_fl);
void gt_chat_set_font_family_numb(gt_obj_st * chat, gt_family_t font_family_numb);
void gt_chat_set_font_thick_en(gt_obj_st * chat, uint8_t thick);
void gt_chat_set_font_thick_cn(gt_obj_st * chat, uint8_t thick);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /** GT_CFG_ENABLE_CHATTING */

#endif //!_GT_CHAT_H_

/* end of file ----------------------------------------------------------*/

