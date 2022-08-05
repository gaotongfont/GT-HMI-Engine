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

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_KEYPAD
#define MY_CLASS    &gt_keypad_class

/* private typedef ------------------------------------------------------*/


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class);
static void _deinit_cb(gt_obj_st * obj,const gt_obj_class_st * class);
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
    '7',{(24+12)*0+2,(24+12)*0+2, 32, 32},  '8',{(24+12)*1+2,(24+12)*0+2,32,32},  '9',{(24+12)*2+2,(24+12)*0+2,32,32}, '+',{(24+12)*3+2,(24+12)*0+2,32,32*2+4},
    '4',{(24+12)*0+2,(24+12)*1+2, 32, 32},  '5',{(24+12)*1+2,(24+12)*1+2,32,32},  '6',{(24+12)*2+2,(24+12)*1+2,32,32},
    '1',{(24+12)*0+2,(24+12)*2+2, 32, 32},  '2',{(24+12)*1+2,(24+12)*2+2,32,32},  '3',{(24+12)*2+2,(24+12)*2+2,32,32},
    
    '0',{(24+12)*0+2,(24+12)*3+2, 32*2+4, 32},  '.',{(24+12)*2+2,(24+12)*3+2,32,32},  '>',{(24+12)*3+2,(24+12)*2+2,32,32*2+4},
};


gt_keypad_map_st map_keypad[] = {
    #define SPACE   6
    #define W (((GT_SCREEN_WIDTH-4)/10)-SPACE)
    #define H ((((GT_SCREEN_HEIGHT>>1)-2)>>2)-SPACE)
    #define START   2
    'q',{START+(W+SPACE)*0, START+(H+SPACE)*0, W, H}, 
    'w',{START+(W+SPACE)*1, START+(H+SPACE)*0, W, H}, 
    'e',{START+(W+SPACE)*2, START+(H+SPACE)*0, W, H}, 
    'r',{START+(W+SPACE)*3, START+(H+SPACE)*0, W, H}, 
    't',{START+(W+SPACE)*4, START+(H+SPACE)*0, W, H}, 
    'y',{START+(W+SPACE)*5, START+(H+SPACE)*0, W, H}, 
    'u',{START+(W+SPACE)*6, START+(H+SPACE)*0, W, H}, 
    'i',{START+(W+SPACE)*7, START+(H+SPACE)*0, W, H}, 
    'o',{START+(W+SPACE)*8, START+(H+SPACE)*0, W, H}, 
    'p',{START+(W+SPACE)*9, START+(H+SPACE)*0, W, H},

    'a',{START+(W>>1)+(W+SPACE)*0, START+(H+SPACE)*1, W, H}, 
    's',{START+(W>>1)+(W+SPACE)*1, START+(H+SPACE)*1, W, H}, 
    'd',{START+(W>>1)+(W+SPACE)*2, START+(H+SPACE)*1, W, H}, 
    'f',{START+(W>>1)+(W+SPACE)*3, START+(H+SPACE)*1, W, H}, 
    'g',{START+(W>>1)+(W+SPACE)*4, START+(H+SPACE)*1, W, H}, 
    'h',{START+(W>>1)+(W+SPACE)*5, START+(H+SPACE)*1, W, H}, 
    'j',{START+(W>>1)+(W+SPACE)*6, START+(H+SPACE)*1, W, H}, 
    'k',{START+(W>>1)+(W+SPACE)*7, START+(H+SPACE)*1, W, H}, 
    'l',{START+(W>>1)+(W+SPACE)*8, START+(H+SPACE)*1, W, H}, 


    '.',{START+((W/3)<<1)+SPACE, START+(H+SPACE)<<1, (W/3)<<1, H}, //space
    'z',{START+(W>>1)+(W+SPACE)*1, START+(H+SPACE)<<1, (W), H}, 
    'x',{START+(W>>1)+(W+SPACE)*2, START+(H+SPACE)<<1, (W), H}, 
    'c',{START+(W>>1)+(W+SPACE)*3, START+(H+SPACE)<<1, (W), H}, 
    'v',{START+(W>>1)+(W+SPACE)*4, START+(H+SPACE)<<1, (W), H}, 
    'b',{START+(W>>1)+(W+SPACE)*5, START+(H+SPACE)<<1, (W), H}, 
    'n',{START+(W>>1)+(W+SPACE)*6, START+(H+SPACE)<<1, (W), H}, 
    'm',{START+(W>>1)+(W+SPACE)*7, START+(H+SPACE)<<1, (W), H}, 
    
    
    ',',{START+(W/2)+(W+SPACE)*7 + START+(W/2+SPACE)*2, START+(H+SPACE)<<1, W>>1, H}, 
    0x08,{START+W/2+(W+SPACE)*7 + START+(W/2+SPACE)*3, START+(H+SPACE)<<1, W, H}, 
		
    0x81,{START+0, START+(H+SPACE)*3, (W*1.5), H}, //space
    '<',{START+(W/2)+W+SPACE+(W+SPACE)*0, START+(H+SPACE)*3, (W*1.5), H},
    ' ',{START+(W)+W+SPACE+(W+SPACE)*1, START+(H+SPACE)*3, (W+SPACE)*4, H},
    '>',{START+(W/2)+W+SPACE+2+(W+SPACE)*5.5, START+(H+SPACE)*3, (W*1.5), H},
    0x0D,{START+(W/2)+W+SPACE+(W+SPACE)*7, START+(H+SPACE)*3, (W+SPACE)*1.5, H}, //space
};




/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static uint8_t _gt_keypad_get_click_code(gt_obj_st * obj){
    int idx = 0;
    int len = sizeof(map_keypad)/sizeof(gt_keypad_map_st);
    while( idx < len ){
        if( (map_keypad[idx].area.x < obj->process_attr->point.x) &&
            ((map_keypad[idx].area.x + map_keypad[idx].area.w) > obj->process_attr->point.x) &&
            (map_keypad[idx].area.y < obj->process_attr->point.y) &&
            ((map_keypad[idx].area.y + map_keypad[idx].area.h) > obj->process_attr->point.y) ){
            return map_keypad[idx].ch;
        }
        idx++;
    }
    return NULL;
}

static void _gt_keypad_set_code_selected(gt_obj_st * keypad, char code)
{
    _gt_style_keypad_st * style = keypad->style;
    style->code_selected = code;
}

static const unsigned char gImage_enter_16x16[128] = { /* 0X10,0X04,0X00,0X10,0X00,0X10, */
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0XF0,0X00,0X00,0XF0,0X00,0X00,0X00,0X00,0XF0,
    0X00,0X0F,0X30,0X00,0X00,0X00,0X00,0XF0,0X00,0XF8,0X00,0X00,0X00,0X00,0X03,0XF0,
    0X0F,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X40,0X00,0XF8,0X00,0X00,0X00,0X00,0X00,0X00,
    0X00,0X0F,0X30,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0XF0,0X00,0X00,0X00,0X00,0X00,
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
};
static const unsigned char gImage_back_16x16[128] = { /* 0X10,0X04,0X00,0X10,0X00,0X10, */
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0XFB,0X00,0X00,0X00,0X00,
    0X00,0X00,0X0F,0XF4,0X00,0X00,0X00,0X00,0X00,0X00,0XFD,0X60,0X00,0X00,0X00,0X00,
    0X00,0X0F,0XD5,0X00,0X00,0X00,0X00,0X00,0X00,0XFD,0XFF,0XFF,0XFF,0XFF,0XF0,0X00,
    0X00,0XFD,0XFF,0XFF,0XFF,0XFF,0XF0,0X00,0X00,0X0F,0XE5,0X00,0X00,0X00,0X00,0X00,
    0X00,0X00,0XFE,0X50,0X00,0X00,0X00,0X00,0X00,0X00,0X0F,0XF5,0X00,0X00,0X00,0X00,
    0X00,0X00,0X00,0XFF,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
};
static const unsigned char gImage_keypad_16x16[128] = { /* 0X10,0X04,0X00,0X10,0X00,0X10, */
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
    gt_size_t x, y, w, h;           //base attr
    int idx = 0, cnt_temp = 0;
	gt_color_t color, * color_widget_p = NULL;

    int cnt_btn_map = sizeof(map_keypad)/sizeof(gt_keypad_map_st);
    gt_area_st area_temp, area_valid;
    uint8_t fp[64];
    if( GT_CREATE_TYPE_PART == keypad->create_type ){
        gt_obj_get_valid_area(keypad, &keypad->area_part, &area_valid);
        area_temp.x = keypad->area.x;
        area_temp.w = keypad->area.w;
        area_temp.h = area_valid.h;

        if( area_valid.y >= 0 ){
            area_temp.y = keypad->area.y + area_valid.y;
        }else{
            area_temp.y = keypad->area.y;
        }
        gt_area_copy(&keypad->area_back, &keypad->area);
        gt_area_copy(&keypad->area, &area_temp);

        w = area_temp.w;
        h = area_valid.h;
        keypad->cache.cache_color = gt_mem_malloc( w * h * sizeof(gt_color_t) );
        color_widget_p = (gt_color_t *)keypad->cache.cache_color;
        gt_color_fill(color_widget_p , w*h, style->bcolor); 
        
        // foreach all key
        gt_color_t * color_key_p = NULL;
        idx = 0;
        gt_obj_st obj_key;
        gt_area_st area_color, area_src, area_dst;
        gt_font_st font = {
            .style_cn = 1,
            .style_en = 2,
            .size = GT_FONT_SIZE_16x16,
        };
        while( idx < cnt_btn_map ){
            obj_key.area.x = map_keypad[idx].area.x + keypad->area_back.x;
            obj_key.area.y = map_keypad[idx].area.y + keypad->area_back.y;
            obj_key.area.w = map_keypad[idx].area.w;
            obj_key.area.h = map_keypad[idx].area.h;

            // check key is visible
            if( !gt_obj_check_visible(&obj_key, keypad->area) ){
                idx++;
                continue;
            }
            
            w = map_keypad[idx].area.w;
            h = map_keypad[idx].area.h;
            color_key_p = gt_mem_malloc( w * h * sizeof(gt_color_t) );
            gt_color_fill(color_key_p, w * h, style->fcolor);
            // get font
            if( map_keypad[idx].ch == 0x0D || map_keypad[idx].ch == 0x08 || map_keypad[idx].ch == 0x81 ){
                gt_color_t * color_key = gt_mem_malloc(16*16*sizeof(gt_color_t));
                gt_color_t ___color;
                ___color.full = 0xDF3D;
                gt_color_fill(color_key_p, w * h, ___color);
                if( map_keypad[idx].ch == 0x0D ){//enter
                    extern void _gt_graph_gray_to_color(uint8_t *gray, gt_color_t *color,gt_size_t w, gt_size_t h, gt_color_t fcolor, gt_color_t bcolor, gt_size_t level_gray);
                    gt_color_t _kfcolor,_kbcolor;
                    GT_COLOR_SET(_kfcolor, GT_COLOR_GET(style->color_font));
                    GT_COLOR_SET(_kbcolor, 0xDF3D);
                    _gt_graph_gray_to_color(gImage_enter_16x16, color_key, 16, 16, _kfcolor, _kbcolor, 4);
                }else if( map_keypad[idx].ch == 0x08 ){//back
                    extern void _gt_graph_gray_to_color(uint8_t *gray, gt_color_t *color,gt_size_t w, gt_size_t h, gt_color_t fcolor, gt_color_t bcolor, gt_size_t level_gray);
                    gt_color_t _kfcolor,_kbcolor;
                    GT_COLOR_SET(_kfcolor, GT_COLOR_GET(style->color_font));
                    GT_COLOR_SET(_kbcolor, 0xDF3D);
                    _gt_graph_gray_to_color(gImage_back_16x16, color_key, 16, 16, _kfcolor, _kbcolor, 4);
                }else if( map_keypad[idx].ch == 0x81 ){//keypad
                    extern void _gt_graph_gray_to_color(uint8_t *gray, gt_color_t *color,gt_size_t w, gt_size_t h, gt_color_t fcolor, gt_color_t bcolor, gt_size_t level_gray);
                    gt_color_t _kfcolor,_kbcolor;
                    GT_COLOR_SET(_kfcolor, GT_COLOR_GET(style->color_font));
                    GT_COLOR_SET(_kbcolor, 0xDF3D);
                    _gt_graph_gray_to_color(gImage_keypad_16x16, color_key, 16, 16, _kfcolor, _kbcolor, 4);
                }
                area_src.x = 0;
                area_src.y = 0;
                area_src.w = 16;
                area_src.h = 16;

                area_dst.x = (map_keypad[idx].area.w - 16) >> 1;
                area_dst.y = (map_keypad[idx].area.h - 16) >> 1;
                area_dst.w = map_keypad[idx].area.w;
                area_dst.h = map_keypad[idx].area.h;
                
                gt_color_area_copy(color_key_p, area_dst, color_key, area_src, 16,16);
                gt_mem_free(color_key);
                goto lab_cpy;
            }

            font.utf8 = &map_keypad[idx].ch;
            font.len = 1;
            font.res = fp;

            gt_memset(fp, 0, sizeof(fp));

            gt_font_get_string_dot(&font);
            
            area_color.x = (w - gt_font_get_one_word_width(map_keypad[idx].ch, 16))>>1;
            area_color.y = (h - font.size)>>1;
            area_color.w = w;
            area_color.h = h;

            gt_graph_dot_draw_to_color(font.res, color_key_p, area_color, font.size, font.size, style->color_font);

            lab_cpy:
            gt_obj_get_valid_area(&obj_key, &keypad->area, &area_valid);

            area_dst.x = map_keypad[idx].area.x;
            area_dst.y = (obj_key.area.y + area_valid.y) - keypad->area.y;
            area_dst.w = keypad->area.w;
            area_dst.h = keypad->area.h;

            if( area_dst.y < 0 ){
                area_dst.y = 0;
            }

            area_src.x = area_valid.x;
            area_src.y = area_valid.y;
            area_src.w = w;
            area_src.h = h;
            gt_color_area_copy(color_widget_p, area_dst, color_key_p, area_src, area_valid.w, area_valid.h);
            gt_mem_free(color_key_p);
            idx++;
        }
    }else{
        w = keypad->area.w;
        h = keypad->area.h;
        if( keypad->cache.cache_color != NULL ){
            gt_mem_free(keypad->cache.cache_color);
        }
        keypad->cache.cache_color = gt_mem_malloc( w * h * sizeof(gt_color_t) );
        color_widget_p = (gt_color_t *)keypad->cache.cache_color;
        gt_color_fill(color_widget_p , w*h, color); 
    }
    
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
    gt_mem_free(obj->cache.cache_color);
    if( GT_CREATE_TYPE_PART == obj->create_type ){
        gt_area_copy(&obj->area, &obj->area_back);
        obj->create_type = GT_CREATE_TYPE_FULL;
    }
    GT_LOGV(GT_LOG_TAG_GUI, "end deinit_cb");
}


/**
 * @brief obj event handler cb
 *
 * @param obj
 * @param e event
 */
static void _event_cb(struct _gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code = gt_event_get_code(e);
    switch(code) {
        case GT_EVENT_TYPE_DRAW_START:
            GT_LOGD(GT_LOG_TAG_GUI, "start draw");
            if( gt_obj_check_scr(obj) ){
                // _gt_keypad_init_widget(obj);

                // extern void gt_disp_obj(gt_obj_st * obj);
                // gt_disp_obj(obj);
            }
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;

        case GT_EVENT_TYPE_DRAW_END:
            GT_LOGD(GT_LOG_TAG_GUI, "end draw");
            gt_mem_free(obj->cache.cache_color);
            break;

        case GT_EVENT_TYPE_CHANGE_CHILD_REMOVE: /* remove child from screen but not delete */
            GT_LOGD(GT_LOG_TAG_GUI, "child remove");
			break;

        case GT_EVENT_TYPE_CHANGE_CHILD_DELETE: /* delete child */
            GT_LOGD(GT_LOG_TAG_GUI, "child delete");
            break;

        case GT_EVENT_TYPE_INPUT_CLICKING:   /* add clicking style and process clicking event */
            GT_LOGD(GT_LOG_TAG_GUI, "clicking");

            break;

        case GT_EVENT_TYPE_INPUT_SCROLL:
            GT_LOGD(GT_LOG_TAG_GUI, "scroll");
            break;

        case GT_EVENT_TYPE_INPUT_PROCESS:   /* remove clicking style and process click event */
            GT_LOGD(GT_LOG_TAG_GUI, "process");
            /* code here */
            char code_click = 0;
            code_click = _gt_keypad_get_click_code(obj);
            _gt_keypad_set_code_selected(obj, code_click);
            printf("code:%c\n", code_click);
            gt_event_send(obj, GT_EVENT_TYPE_INPUT_PROCESSED, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_PROCESSED: /* click event finish */
            GT_LOGD(GT_LOG_TAG_GUI, "processed");
            break;

        default:
            break;
    }
}


static void _gt_keypad_init_style(gt_obj_st * keypad)
{
    _gt_style_keypad_st * style = keypad->style;
    int cnt_btn_map = sizeof(map_keypad)/sizeof(gt_keypad_map_st);
    int idx = 0;

    // cal keypad width and height
    while( idx < cnt_btn_map ){
        if( keypad->area.w < (map_keypad[idx].area.x + map_keypad[idx].area.w) ){
            keypad->area.w = (map_keypad[idx].area.x + map_keypad[idx].area.w);
        }
        if( keypad->area.h < (map_keypad[idx].area.y + map_keypad[idx].area.h) ){
            keypad->area.h = (map_keypad[idx].area.y + map_keypad[idx].area.h);
        }
        idx++;
    }
    keypad->area.w += 2;
    keypad->area.h += 2;

    GT_COLOR_SET(style->color_font, 0x0000);
    GT_COLOR_SET(style->fcolor, 0xFFFF);
    GT_COLOR_SET(style->bcolor, 0xF79E);
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
