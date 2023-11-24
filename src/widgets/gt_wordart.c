/**
 * @file gt_wordart.c
 * @author Yang (your@email.com)
 * @brief
 * @version 0.1
 * @date 2023-07-21 14:10:32
 * @copyright Copyright (c) 2014-2023, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_wordart.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../font/gt_font.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_WORDART
#define MY_CLASS    &gt_wordart_class

#if GT_CFG_ENABLE_WORDART
/* private typedef ------------------------------------------------------*/
typedef struct _gt_wordart_item_s{
    uint32_t coding;
    char * img;
}_gt_wordart_item_st;

typedef struct _gt_wordart_s{

    _gt_wordart_item_st *items;
    uint32_t total;
    uint32_t item_number;
    char *text;
    uint8_t space_x;
    uint8_t space_y;
}_gt_wordart_st;


/* static prototypes ----------------------------------------------------*/



/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_wordart_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_wordart_st)
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static int _gt_wordart_items_find(_gt_wordart_st * style , uint32_t coding)
{
    uint32_t i = 0;

    for(i = 0 ; i < style->item_number ; i++){

        if(coding == style->items[i].coding){
            return i;
        }
    }

    return -1;
}

static inline void _gt_wordart_init_widget(gt_obj_st * wordart)
{
    _gt_wordart_st * style = (_gt_wordart_st * )wordart->style;

    if(0 == wordart->area.w || 0 == wordart->area.h){
        wordart->area.w = 5;
        wordart->area.h = 5;
    }

    gt_area_st area = {
        .x = wordart->area.x ,
        .y = wordart->area.y ,
        .w = wordart->area.w ,
        .h = wordart->area.h
    };

    if(NULL == style->items ||  0 == style->item_number){
        return ;
    }

    gt_attr_rect_st dsc = {
        .bg_opa = wordart->opa,
    };

    uint32_t len = (NULL == style->text) ? 0 : strlen(style->text);
    uint32_t idx = 0 , coding = 0 ;
    int tmp = 0;

    for(idx = 0 ; idx < len ;){
        tmp = _gt_utf8_to_unicode(&style->text[idx] , &coding);
        if(0 == tmp){
            return ;
        }
        idx += tmp;

        tmp = _gt_wordart_items_find(style , coding);
        if(tmp < 0){
            continue;
        }

        dsc.bg_img_src = style->items[tmp].img;

        //
        draw_bg_img(wordart->draw_ctx, &dsc, &area);
        area.x += area.w + style->space_x;
    }
}

/**
 * @brief obj init wordart widget call back
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj)
{
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");

    _gt_wordart_init_widget(obj);
}

/**
 * @brief obj deinit call back
 *
 * @param obj
 */
static void _deinit_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "wordart deinit");
    if (NULL == obj) {
        return ;
    }

    _gt_wordart_st ** style_p = (_gt_wordart_st ** )&obj->style;
    if (NULL == *style_p) {
        return ;
    }

    if(NULL != (*style_p)->items){
        for(int i =  (*style_p)->item_number ; i >= 0 ; i--) {
            if(NULL == (*style_p)->items[i].img)
            {
                continue;
            }
            gt_mem_free((*style_p)->items[i].img);
            (*style_p)->items[i].img = NULL;
        }

        gt_mem_free((*style_p)->items);
        (*style_p)->items = NULL;
        (*style_p)->item_number = 0;
    }

    if(NULL != (*style_p)->text){
        gt_mem_free((*style_p)->text);
        (*style_p)->text = NULL;
    }

    gt_mem_free(*style_p);
    *style_p = NULL;
}

/**
 * @brief obj event handler call back
 *
 * @param obj
 * @param e event
 */
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code = gt_event_get_code(e);
    switch(code) {
        case GT_EVENT_TYPE_DRAW_START:
            GT_LOGV(GT_LOG_TAG_GUI, "start draw");
            gt_disp_invalid_area(obj);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;

        case GT_EVENT_TYPE_DRAW_END:
            GT_LOGV(GT_LOG_TAG_GUI, "end draw");
            break;

        case GT_EVENT_TYPE_UPDATE_VALUE:
            GT_LOGV(GT_LOG_TAG_GUI, "value update");
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        case GT_EVENT_TYPE_CHANGE_CHILD_REMOVE: /* remove child from screen but not delete */
            GT_LOGV(GT_LOG_TAG_GUI, "child remove");
			break;

        case GT_EVENT_TYPE_CHANGE_CHILD_DELETE: /* delete child */
            GT_LOGV(GT_LOG_TAG_GUI, "child delete");
            break;

        case GT_EVENT_TYPE_INPUT_PRESSING:   /* add clicking style and process clicking event */
            GT_LOGV(GT_LOG_TAG_GUI, "clicking");
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_SCROLL:
            GT_LOGV(GT_LOG_TAG_GUI, "scroll");
            break;

        case GT_EVENT_TYPE_INPUT_RELEASED: /* click event finish */
            GT_LOGV(GT_LOG_TAG_GUI, "processed");

            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        default:
            break;
    }
}


/* global functions / API interface -------------------------------------*/
/**
 * @brief create a wordart obj
 *
 * @param parent wordart's parent element
 * @return gt_obj_st* wordart obj
 */
gt_obj_st * gt_wordart_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);

    _gt_wordart_st * style = (_gt_wordart_st * )obj->style;
    gt_memset(style, 0, sizeof(_gt_wordart_st));

    style->total = 0;
    style->item_number = 0;
    style->space_x = 0;
    style->space_y = 0;

    return obj;
}


void gt_wordart_items_init(gt_obj_st* wordart , uint32_t total)
{
    _gt_wordart_st * style = (_gt_wordart_st * )wordart->style;

    if(NULL != style->items){
        return;
    }

    style->item_number = 0;
    style->items = (_gt_wordart_item_st * )gt_mem_malloc(total * sizeof(_gt_wordart_item_st));

    if(NULL == style->items){
        style->total = 0;
        return ;
    }

    style->total = total;
}


void gt_wordart_add_item(gt_obj_st* wordart , uint32_t coding , char* img)
{
    if(NULL == wordart || NULL == img){
        return ;
    }
    _gt_wordart_st * style = (_gt_wordart_st * )wordart->style;
    uint32_t len = 0;

    if(NULL == style->items || style->item_number >= style->total){
        return ;
    }
    // if(NULL == style->items){
    //     style->item_number = 0;
    //     style->items = (_gt_wordart_item_st * )gt_mem_malloc(20*sizeof(_gt_wordart_item_st));
    // }
    // else{
    //     // style->items = (_gt_wordart_item_st * )gt_mem_realloc( style->items , (style->item_number+1) * sizeof(_gt_wordart_item_st));
    // }

    len = (NULL == img) ? 1 : strlen(img) + 1;
    style->items[style->item_number].img = gt_mem_malloc(len);
    if(NULL == style->items[style->item_number].img){
        return ;
    }
    gt_memcpy(style->items[style->item_number].img , img, len-1);
    style->items[style->item_number].img[len] = '\0';
    style->items[style->item_number].coding = coding;
    style->item_number++;
}

void gt_wordart_set_text(gt_obj_st * wordart, const char * fmt, ...)
{
    char buffer[8] = {0};
    va_list args;
    va_list args2;
    va_start(args, fmt);
    va_copy(args2, args);

    _gt_wordart_st * style = (_gt_wordart_st *)wordart->style;

    if( NULL != style->text ){
        gt_mem_free(style->text);
    }
    uint16_t size = (NULL == fmt) ? 0 : (vsnprintf(buffer, sizeof(buffer), fmt, args) + 1);
    if (!size) {
        goto free_lb;
    }
    style->text = gt_mem_malloc(size);

    vsnprintf(style->text, size, fmt, args2);

    gt_event_send(wordart, GT_EVENT_TYPE_DRAW_START, NULL);

free_lb:
    va_end(args2);
    va_end(args);
}

char * gt_wordart_get_text(gt_obj_st * wordart)
{
    _gt_wordart_st * style = (_gt_wordart_st *)wordart->style;
    return style->text;
}

void gt_wordart_set_space(gt_obj_st * wordart, uint8_t space_x, uint8_t space_y)
{
    _gt_wordart_st * style = (_gt_wordart_st * )wordart->style;
    style->space_x = space_x;
    style->space_y = space_y;
}

#endif /* GT_CFG_ENABLE_WORDART */
/* end of file ----------------------------------------------------------*/


