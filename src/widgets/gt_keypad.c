/**
 * @file gt_keypad.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-08-01 14:05:01
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

 /* include --------------------------------------------------------------*/
#include "gt_keypad.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../font/gt_font.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_KEYPAD
#define MY_CLASS    &gt_keypad_class

/* private typedef ------------------------------------------------------*/


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class);
static void _deinit_cb(gt_obj_st * obj, const gt_obj_class_st * class);
static void _event_cb(struct _gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_keypad_class = {
    ._init_cb = _init_cb,
    ._deinit_cb = _deinit_cb,
    ._event_cb = _event_cb,
    .type = OBJ_TYPE,
    .size_instance = (sizeof(gt_obj_st)),
    .size_style = (sizeof(_gt_style_keypad_st))
};


gt_keypad_map_st map_keypad_num[] = {
    // ch, area
    // 7 8 9
    // 4 5 6 +
    // 1 2 3
    // 0   . en
    '7',{(24 + 12) * 0 + 2,(24 + 12) * 0 + 2, 32, 32},  '8',{(24 + 12) * 1 + 2,(24 + 12) * 0 + 2,32,32},  '9',{(24 + 12) * 2 + 2,(24 + 12) * 0 + 2,32,32}, '+',{(24 + 12) * 3 + 2,(24 + 12) * 0 + 2,32,32 * 2 + 4},
    '4',{(24 + 12) * 0 + 2,(24 + 12) * 1 + 2, 32, 32},  '5',{(24 + 12) * 1 + 2,(24 + 12) * 1 + 2,32,32},  '6',{(24 + 12) * 2 + 2,(24 + 12) * 1 + 2,32,32},
    '1',{(24 + 12) * 0 + 2,(24 + 12) * 2 + 2, 32, 32},  '2',{(24 + 12) * 1 + 2,(24 + 12) * 2 + 2,32,32},  '3',{(24 + 12) * 2 + 2,(24 + 12) * 2 + 2,32,32},

    '0',{(24 + 12) * 0 + 2,(24 + 12) * 3 + 2, 32 * 2 + 4, 32},  '.',{(24 + 12) * 2 + 2,(24 + 12) * 3 + 2,32,32},  '>',{(24 + 12) * 3 + 2,(24 + 12) * 2 + 2,32,32 * 2 + 4},
};


gt_keypad_map_st map_keypad[] = {
    #define SPACE   6
    #define W (((GT_SCREEN_WIDTH-4)/10)-SPACE)
    #define H ((((GT_SCREEN_HEIGHT>>1)-2)>>2)-SPACE)
    #define START   2
    'q',{START + (W + SPACE) * 0, START + (H + SPACE) * 0, W, H},
    'w',{START + (W + SPACE) * 1, START + (H + SPACE) * 0, W, H},
    'e',{START + (W + SPACE) * 2, START + (H + SPACE) * 0, W, H},
    'r',{START + (W + SPACE) * 3, START + (H + SPACE) * 0, W, H},
    't',{START + (W + SPACE) * 4, START + (H + SPACE) * 0, W, H},
    'y',{START + (W + SPACE) * 5, START + (H + SPACE) * 0, W, H},
    'u',{START + (W + SPACE) * 6, START + (H + SPACE) * 0, W, H},
    'i',{START + (W + SPACE) * 7, START + (H + SPACE) * 0, W, H},
    'o',{START + (W + SPACE) * 8, START + (H + SPACE) * 0, W, H},
    'p',{START + (W + SPACE) * 9, START + (H + SPACE) * 0, W, H},

    'a',{START + (W >> 1) + (W + SPACE) * 0, START + (H + SPACE) * 1, W, H},
    's',{START + (W >> 1) + (W + SPACE) * 1, START + (H + SPACE) * 1, W, H},
    'd',{START + (W >> 1) + (W + SPACE) * 2, START + (H + SPACE) * 1, W, H},
    'f',{START + (W >> 1) + (W + SPACE) * 3, START + (H + SPACE) * 1, W, H},
    'g',{START + (W >> 1) + (W + SPACE) * 4, START + (H + SPACE) * 1, W, H},
    'h',{START + (W >> 1) + (W + SPACE) * 5, START + (H + SPACE) * 1, W, H},
    'j',{START + (W >> 1) + (W + SPACE) * 6, START + (H + SPACE) * 1, W, H},
    'k',{START + (W >> 1) + (W + SPACE) * 7, START + (H + SPACE) * 1, W, H},
    'l',{START + (W >> 1) + (W + SPACE) * 8, START + (H + SPACE) * 1, W, H},


    '.',{START + ((W / 3) << 1) + SPACE, START + (H + SPACE) << 1, (W / 3) << 1, H}, //space
    'z',{START + (W >> 1) + (W + SPACE) * 1, START + (H + SPACE) << 1, (W), H},
    'x',{START + (W >> 1) + (W + SPACE) * 2, START + (H + SPACE) << 1, (W), H},
    'c',{START + (W >> 1) + (W + SPACE) * 3, START + (H + SPACE) << 1, (W), H},
    'v',{START + (W >> 1) + (W + SPACE) * 4, START + (H + SPACE) << 1, (W), H},
    'b',{START + (W >> 1) + (W + SPACE) * 5, START + (H + SPACE) << 1, (W), H},
    'n',{START + (W >> 1) + (W + SPACE) * 6, START + (H + SPACE) << 1, (W), H},
    'm',{START + (W >> 1) + (W + SPACE) * 7, START + (H + SPACE) << 1, (W), H},


    ',',{START + (W / 2) + (W + SPACE) * 7 + START + (W / 2 + SPACE) * 2, START + (H + SPACE) << 1, W >> 1, H},
    0x08,{START + W / 2 + (W + SPACE) * 7 + START + (W / 2 + SPACE) * 3, START + (H + SPACE) << 1, W, H},

    0x81,{START + 0, START + (H + SPACE) * 3, (W * 1.5), H}, //space
    '<',{START + (W / 2) + W + SPACE + (W + SPACE) * 0, START + (H + SPACE) * 3, (W * 1.5), H},
    ' ',{START + (W)+W + SPACE + (W + SPACE) * 1, START + (H + SPACE) * 3, (W + SPACE) * 4, H},
    '>',{START + (W / 2) + W + SPACE + 2 + (W + SPACE) * 5.5, START + (H + SPACE) * 3, (W * 1.5), H},
    0x0D,{START + (W / 2) + W + SPACE + (W + SPACE) * 7, START + (H + SPACE) * 3, (W + SPACE) * 1.5, H}, //space
};


gt_keypad_map_st map_keypad3[] = {
    
    #define SPACE       6
    #define W           (((GT_SCREEN_WIDTH-4)/10) - 2)
    #define H           (((((GT_SCREEN_HEIGHT>>1))-4)/4)-2)
    #define WS          (W+SPACE)

    #define START_X_L1    2
    #define START_Y_L1    2
    #define START_X_L2    (2+(W>>1))
    #define START_Y_L2    (2 + H + SPACE)
    
    'q',  {START_X_L1 + WS * 0,  START_Y_L1,  W, H },
    'w',  {START_X_L1 + WS * 1,  START_Y_L1,  W, H },
    'e',  {START_X_L1 + WS * 2,  START_Y_L1,  W, H },
    'r',  {START_X_L1 + WS * 3,  START_Y_L1,  W, H },
    't',  {START_X_L1 + WS * 4,  START_Y_L1,  W, H },
    'y',  {START_X_L1 + WS * 5,  START_Y_L1,  W, H },
    'u',  {START_X_L1 + WS * 6,  START_Y_L1,  W, H },
    'i',  {START_X_L1 + WS * 7,  START_Y_L1,  W, H },
    'o',  {START_X_L1 + WS * 8,  START_Y_L1,  W, H },
    'p',  {START_X_L1 + WS * 9,  START_Y_L1,  W, H },

    'a',  {START_X_L2 + WS * 0,  START_Y_L2,  W, H },
    's',  {START_X_L2 + WS * 1,  START_Y_L2,  W, H },
    'd',  {START_X_L2 + WS * 2,  START_Y_L2,  W, H },
    'f',  {START_X_L2 + WS * 3,  START_Y_L2,  W, H },
    'g',  {START_X_L2 + WS * 4,  START_Y_L2,  W, H },
    'h',  {START_X_L2 + WS * 5,  START_Y_L2,  W, H },
    'j',  {START_X_L2 + WS * 6,  START_Y_L2,  W, H },
    'k',  {START_X_L2 + WS * 7,  START_Y_L2,  W, H },
    'l',  {START_X_L2 + WS * 8,  START_Y_L2,  W, H },
};

/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static uint8_t _gt_keypad_get_click_code(gt_obj_st * obj) {
    
    int idx = 0;
    int len = sizeof(map_keypad) / sizeof(gt_keypad_map_st);
    gt_area_st area_key;
    while (idx < len) {
        area_key = map_keypad[idx].area;
        if ((area_key.x < obj->process_attr.point.x) &&
            ((area_key.x + area_key.w) > obj->process_attr.point.x) &&
            (area_key.y < obj->process_attr.point.y) &&
            ((area_key.y + area_key.h) > obj->process_attr.point.y)) {

            return map_keypad[idx].ch;
        }
        idx++;
    }
    return 0;
}


static void _gt_keypad_set_code_selected(gt_obj_st * keypad, char code)
{
    _gt_style_keypad_st * style = keypad->style;
    style->code_selected = code;
}

static char _gt_keypad_get_code_selected(gt_obj_st * keypad)
{
    _gt_style_keypad_st * style = keypad->style;
    return style->code_selected;
}


gt_area_st _gt_keypad_get_code_area(char code)
{
    int idx = 0;
    gt_area_st area = {
        .x = 0,.y = 0,.w = 0,.h = 0
    };
    int cnt_btn_map = sizeof(map_keypad) / sizeof(gt_keypad_map_st);
    while (idx < cnt_btn_map) {
        if (code == map_keypad[idx].ch) {
            return map_keypad[idx].area;
        }
        idx++;
    }
    return area;
}

const unsigned char gImage_enter_16x16[128] = { /* 0X10,0X04,0X00,0X10,0X00,0X10, */
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0XF0,0X00,0X00,0XF0,0X00,0X00,0X00,0X00,0XF0,
    0X00,0X0F,0X30,0X00,0X00,0X00,0X00,0XF0,0X00,0XF8,0X00,0X00,0X00,0X00,0X03,0XF0,
    0X0F,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X40,0X00,0XF8,0X00,0X00,0X00,0X00,0X00,0X00,
    0X00,0X0F,0X30,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0XF0,0X00,0X00,0X00,0X00,0X00,
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
};
const unsigned char gImage_back_16x16[128] = { /* 0X10,0X04,0X00,0X10,0X00,0X10, */
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0XFB,0X00,0X00,0X00,0X00,
    0X00,0X00,0X0F,0XF4,0X00,0X00,0X00,0X00,0X00,0X00,0XFD,0X60,0X00,0X00,0X00,0X00,
    0X00,0X0F,0XD5,0X00,0X00,0X00,0X00,0X00,0X00,0XFD,0XFF,0XFF,0XFF,0XFF,0XF0,0X00,
    0X00,0XFD,0XFF,0XFF,0XFF,0XFF,0XF0,0X00,0X00,0X0F,0XE5,0X00,0X00,0X00,0X00,0X00,
    0X00,0X00,0XFE,0X50,0X00,0X00,0X00,0X00,0X00,0X00,0X0F,0XF5,0X00,0X00,0X00,0X00,
    0X00,0X00,0X00,0XFF,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
};
const unsigned char gImage_keypad_16x16[128] = { /* 0X10,0X04,0X00,0X10,0X00,0X10, */
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
    0X00,0XBB,0XBB,0XBB,0XBB,0XBB,0XBB,0X00,0X00,0XBF,0XFF,0XFF,0XFF,0XFF,0XFB,0X00,
    0X00,0XB2,0XFF,0X2B,0XFB,0XF2,0X2B,0X00,0X00,0XBF,0XFF,0XFF,0XFF,0XFF,0XFB,0X00,
    0X00,0XB2,0XF5,0XF5,0XF5,0X5F,0X2B,0X00,0X00,0XBF,0XFF,0XFF,0XFF,0XFF,0XFB,0X00,
    0X00,0XB2,0X29,0X22,0X22,0X82,0X2B,0X00,0X00,0XBB,0XBB,0XBB,0XBB,0XBB,0XBB,0X00,
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
};


static void _gt_keypad_init_widget(gt_obj_st * keypad)
{
    _gt_style_keypad_st * style = keypad->style;
    int idx = 0, cnt_temp = 0;
    gt_color_t color, * color_widget_p = NULL;


#if 1
    uint8_t fp[64];
    gt_font_st font = {
        .style_cn = 1,
        .style_en = 2,
        .size = GT_FONT_SIZE_16x16,
        .len = 1,
    };

    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.is_fill = 1;
    rect_attr.border_opa = GT_OPA_COVER;
    rect_attr.border_width = 0;
    rect_attr.radius = 4;
    rect_attr.bg_color = gt_color_make(0xA8,0xA8,0xA8);
    draw_bg(keypad->draw_ctx, &rect_attr, &keypad->area);

    gt_attr_font_st font_attr = {
        .font = &font,
        .space_x = 0,
        .space_y = 0,
        .align = GT_ALIGN_CENTER,
        .font_color = style->color_font,
    };

    gt_area_st area_key, area_bg;

    int cnt_btn_map = sizeof(map_keypad) / sizeof(gt_keypad_map_st);

    int y_flag = 0;
    uint8_t r, g, b;
    while (idx < cnt_btn_map) {

        rect_attr.bg_color = gt_color_make(0xFF, 0xFF, 0xFF);

        if (map_keypad[idx].ch == _gt_keypad_get_code_selected(keypad)) {
            r = GT_COLOR_GET_R(rect_attr.bg_color) >> 1;
            g = GT_COLOR_GET_G(rect_attr.bg_color) >> 1;
            b = GT_COLOR_GET_B(rect_attr.bg_color) >> 1;
            GT_COLOR_SET_RGB(rect_attr.bg_color, r, g, b);
        }

        area_key = map_keypad[idx].area;

        area_key.y += keypad->area.y;
        draw_bg(keypad->draw_ctx, &rect_attr, &area_key);


        font.utf8 = &map_keypad[idx].ch;
        draw_text(keypad->draw_ctx, &font_attr, &area_key);
        idx++;
    }
#endif
}

/**
 * @brief obj init keypad widget cb
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");
    /* code */
    _gt_keypad_init_widget(obj);
    GT_LOGV(GT_LOG_TAG_GUI, "end init_cb");
}

/**
 * @brief obj deinit cb
 *
 * @param obj
 */
static void _deinit_cb(gt_obj_st * obj, const gt_obj_class_st * class) {
    GT_LOGV(GT_LOG_TAG_GUI, "start deinit_cb");
    /* code */
    GT_LOGV(GT_LOG_TAG_GUI, "end deinit_cb");
}


/**
 * @brief obj event handler cb
 *
 * @param obj
 * @param e event
 */
static void _event_cb(struct _gt_obj_s * obj, gt_event_st * e) {
    extern void gt_disp_ref_area(gt_area_st * area);
    gt_event_type_et code = gt_event_get_code(e);

    static gt_area_st area;
    static char code_pre = 0;
    switch (code) {
    case GT_EVENT_TYPE_DRAW_START:
        GT_LOGD(GT_LOG_TAG_GUI, "start draw");
        
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
        break;

    case GT_EVENT_TYPE_DRAW_END:
        GT_LOGD(GT_LOG_TAG_GUI, "end draw");
        break;

    case GT_EVENT_TYPE_CHANGE_CHILD_REMOVE: /* remove child from screen but not delete */
        GT_LOGD(GT_LOG_TAG_GUI, "child remove");
        break;

    case GT_EVENT_TYPE_CHANGE_CHILD_DELETE: /* delete child */
        GT_LOGD(GT_LOG_TAG_GUI, "child delete");
        break;

    case GT_EVENT_TYPE_INPUT_CLICKING:   /* add clicking style and process clicking event */
        GT_LOGD(GT_LOG_TAG_GUI, "clicking");        
        if (gt_obj_check_scr(obj)) {            
            _gt_keypad_set_code_selected(obj, _gt_keypad_get_click_code(obj));
            area = _gt_keypad_get_code_area( _gt_keypad_get_code_selected(obj) );           
            area.y += obj->area.y;
            gt_disp_ref_area(&area);
        }
        break;

    case GT_EVENT_TYPE_INPUT_SCROLL:
        GT_LOGD(GT_LOG_TAG_GUI, "scroll");
        break;

    case GT_EVENT_TYPE_INPUT_PROCESS:   /* remove clicking style and process click event */
        GT_LOGD(GT_LOG_TAG_GUI, "process");
        /* code here */
        
        gt_event_send(obj, GT_EVENT_TYPE_INPUT_PROCESSED, NULL);
        break;

    case GT_EVENT_TYPE_INPUT_PROCESSED: /* click event finish */
        GT_LOGD(GT_LOG_TAG_GUI, "processed");
        if(gt_obj_check_scr(obj)) {
            _gt_keypad_set_code_selected(obj, 0);
            gt_disp_ref_area(&area);
        }
        break;

    default:
        break;
    }
}


static void _gt_keypad_init_style(gt_obj_st * keypad)
{
    _gt_style_keypad_st * style = keypad->style;
    int cnt_btn_map = sizeof(map_keypad) / sizeof(gt_keypad_map_st);
    int idx = 0;
    gt_area_st area_key;
    // cal keypad width and height
    while (idx < cnt_btn_map) {
        area_key = map_keypad[idx].area;
        if (keypad->area.w < (area_key.x + area_key.w)) {
            keypad->area.w = (area_key.x + area_key.w);
        }
        if (keypad->area.h < (area_key.y + area_key.h)) {
            keypad->area.h = (area_key.y + area_key.h);
        }
        idx++;
    }
    keypad->area.w += 2;
    keypad->area.h += 2;

    GT_COLOR_SET(style->color_font, 0x0000);
    GT_COLOR_SET(style->bg_color, 0xF79E);
}




/* global functions / API interface -------------------------------------*/

/**
 * @brief create a keypad obj
 *
 * @param parent keypad's parent element
 * @return gt_obj_st* keypad obj
 */
gt_obj_st * gt_keypad_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    _gt_keypad_init_style(obj);
    return obj;
}

/* end ------------------------------------------------------------------*/
