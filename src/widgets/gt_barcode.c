/**
 * @file gt_barcode.c
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2022-12-09 13:40:14
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_barcode.h"

#if GT_CFG_ENABLE_BARCODE
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../font/gt_font.h"
#include "../others/gt_assert.h"
#include "../core/gt_disp.h"
#include "../core/gt_draw.h"


/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_BARCODE
#define MY_CLASS    &gt_barcode_class

//
#define BARCODE_READ_MODE       (0) // 1 -- read all  0 - read tow line
#define BARCODE_LINE_NUMB       (10)
/* private typedef ------------------------------------------------------*/

typedef struct _gt_barcode_s {
    gt_obj_st obj;
    barcode_st info;
}_gt_barcode_st;


/* static prototypes ----------------------------------------------------*/



/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_barcode_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_barcode_st)
};

/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static inline void _gt_barcode_init_widget(gt_obj_st * barcode) {
    _gt_barcode_st * widget = (_gt_barcode_st * )barcode;
    barcode_st * style = (barcode_st * )&widget->info;
    bar_uint32_t buf_size = 0;
    gt_area_st area;
    gt_size_t h = 0;

    h = style->mode_h;

#if (!BARCODE_READ_MODE)
    gt_size_t x = 0, y = 0, w  = 0, i = 0;
    if(h < BARCODE_LINE_NUMB) { h = BARCODE_LINE_NUMB; }
    style->mode_h = 1;
#endif
    // 获取长宽
    if(!gt_barcode_get_w_and_h(style))
    {
        if(style->ret_param.dot_w == 0 || style->ret_param.dot_h == 0)
        {
            GT_LOGE(GT_LOG_TAG_GUI , "get width err dot_w = %d , dot_h = %d" , style->ret_param.dot_w , style->ret_param.dot_h);
            goto RET_FREE ;
        }
    }
    //
    area.w = style->ret_param.dot_w;
    area.h = style->ret_param.dot_h;
    area.x = barcode->area.x;
    area.y = barcode->area.y;
    //
#if (BARCODE_READ_MODE)
    buf_size = (area.w * area.h) >> 3;
#else
    buf_size = (area.w * (area.h > BARCODE_LINE_NUMB ? area.h : BARCODE_LINE_NUMB)) >> 3;
#endif
    style->pdat = gt_mem_malloc(buf_size);

    if( NULL == style->pdat )
    {
        GT_LOGE(GT_LOG_TAG_GUI , "bracode malloc failed!!!");
        goto RET_FREE ;
    }
    style->pdat_size = buf_size;
    // GT_LOGD(GT_LOG_TAG_GUI , "area dot_w = %d , dot_h = %d , buf_size = %d" , area.w , area.h , style->pdat_size);

    // 获取数据
    if(!gt_barcode_get(style))
    {
        GT_LOGE(GT_LOG_TAG_GUI , "get barcode type = %d ,dot_w = %d , dot_h = %d" , style->type , style->ret_param.dot_w , style->ret_param.dot_h);
        goto RET_FREE ;
    }
    // 重新写入
    style->mode_h = h;

    /* base shape */
    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.reg.is_fill   = 1;
    rect_attr.radius        = 0;
    rect_attr.bg_opa        = GT_OPA_COVER;
    rect_attr.border_width  = 0;
    rect_attr.fg_color      = gt_color_white();
    rect_attr.border_color  = gt_color_black();

    // Draw background
    rect_attr.bg_color      = gt_color_white();
    barcode->area.w = area.w + 4;
    barcode->area.h = h;
    //
    draw_bg(barcode->draw_ctx, &rect_attr, &barcode->area);

    // set barcode color
    rect_attr.bg_color      = gt_color_black();
    rect_attr.data_gray = style->pdat;
    rect_attr.gray = 1;

    area.x = barcode->area.x + 2;
#if (BARCODE_READ_MODE)
    // 1:base shape
    draw_bg(barcode->draw_ctx, &rect_attr, &area);
#else

    // Barcode indent
    area.y = barcode->area.y + 2;
    h -=4;
    area.h = BARCODE_LINE_NUMB;

    for(i = 1 ; i < BARCODE_LINE_NUMB ; i++)
    {
        gt_memmove(&style->pdat[i * (area.w >> 3)] , &style->pdat[0] , area.w >> 3);
    }

    int count = h / BARCODE_LINE_NUMB;
    if(h%BARCODE_LINE_NUMB)
    {
        count++;
    }

    for(i = 0 ; i < count ; i++)
    {
        if((h%BARCODE_LINE_NUMB) && (i == count-1))
        {
            area.h = h%BARCODE_LINE_NUMB;
            draw_bg(barcode->draw_ctx, &rect_attr, &area);
            area.y += area.h;
            continue;
        }
        draw_bg(barcode->draw_ctx, &rect_attr, &area);
        area.y += BARCODE_LINE_NUMB;
    }

#endif

    // focus
    draw_focus(barcode , 0);

RET_FREE:
    if(NULL != style->pdat){
        gt_mem_free(style->pdat);
        style->pdat = NULL;
    }
    style->pdat_size = 0;
}

/**
 * @brief obj init label widget call back
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");

    _gt_barcode_init_widget(obj);
}

/**
 * @brief obj deinit call back
 *
 * @param obj
 */
static void _deinit_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "start deinit_cb");

    if (NULL == obj) {
        return ;
    }

    _gt_barcode_st * obj_p = (_gt_barcode_st * )obj;
    barcode_st * style_p = (barcode_st * )&obj_p->info;

    if(NULL != style_p->barcode){
        gt_mem_free(style_p->barcode);
        style_p->barcode = NULL;
    }

    if(NULL != style_p->pdat){
        gt_mem_free(style_p->pdat);
        style_p->pdat = NULL;
    }
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
            if( 0 ){
                gt_disp_invalid_area(obj);
            }
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
 * @brief create a barcode obj
 *
 * @param parent barcode's parent element
 * @return gt_obj_st* barcode obj
 */
gt_obj_st * gt_barcode_create(gt_obj_st * parent) {
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    if (NULL == obj) {
        return obj;
    }
    _gt_barcode_st * widget = (_gt_barcode_st * )obj;
    barcode_st * style = (barcode_st * )&widget->info;

    char *text = "1234567890128";
    style->barcode = gt_mem_malloc(strlen(text)+1);
    gt_memcpy(style->barcode, text , strlen(text));
    style->barcode[strlen(text)+1] = '\0';
    style->hri_type = GT_FAMILY_BARCODE_HRI_TYPE_DOWN;
    style->type = GT_FAMILY_BARCODE_TYPE_EAN_13;
    style->mode_w = 2;
    style->mode_h = 64;
    style->upc_e_sys = 0;
    style->pdat_size = 0;

    return obj;
}

void gt_barcode_set_code_text(gt_obj_st * barcode , char* code)
{
    if (false == gt_obj_is_type(barcode, OBJ_TYPE)) {
        return;
    }
    _gt_barcode_st * obj = (_gt_barcode_st * )barcode;
    barcode_st * style = (barcode_st * )&obj->info;
    if( NULL != style->barcode) {
        gt_mem_free(style->barcode);
    }
    uint16_t size = code == NULL ? 0 : strlen(code);
    style->barcode = gt_mem_malloc(size+1);
    gt_memcpy(style->barcode, code, size);
    style->barcode[size] = '\0';
    // gt_event_send(barcode, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_barcode_set_type(gt_obj_st * barcode , gt_family_t family)
{
    if (false == gt_obj_is_type(barcode, OBJ_TYPE)) {
        return;
    }
    _gt_barcode_st * style = (_gt_barcode_st * )barcode;
    style->info.type = family;
}

void gt_barcode_set_hri_type(gt_obj_st * barcode , gt_family_t family)
{
    if (false == gt_obj_is_type(barcode, OBJ_TYPE)) {
        return;
    }
    _gt_barcode_st * style = (_gt_barcode_st * )barcode;
    style->info.hri_type = family;
}

void gt_barcode_set_mode_w(gt_obj_st * barcode , uint8_t value)
{
    if (false == gt_obj_is_type(barcode, OBJ_TYPE)) {
        return;
    }
    _gt_barcode_st * style = (_gt_barcode_st * )barcode;
    style->info.mode_w = value;
}

void gt_barcode_set_mode_h(gt_obj_st * barcode , uint8_t value)
{
    if (false == gt_obj_is_type(barcode, OBJ_TYPE)) {
        return;
    }
    _gt_barcode_st * style = (_gt_barcode_st * )barcode;
    style->info.mode_h = value;
}

void gt_barcode_set_upc_e_sys_code(gt_obj_st * barcode , uint8_t value)
{
    if (false == gt_obj_is_type(barcode, OBJ_TYPE)) {
        return;
    }
    _gt_barcode_st * style = (_gt_barcode_st * )barcode;
    style->info.upc_e_sys = value;
}

#endif  /** GT_CFG_ENABLE_BARCODE */

/* end of file ----------------------------------------------------------*/


