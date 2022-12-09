/**
 * @file gt_label.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-05-11 15:03:35
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_label.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../font/gt_font.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_LAB
#define MY_CLASS    &gt_label_class

/* private typedef ------------------------------------------------------*/


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct _gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_label_class = {
    ._init_cb = _init_cb,
    ._deinit_cb = _deinit_cb,
    ._event_cb = _event_cb,
    .type = OBJ_TYPE,
    .size_instance = (sizeof(gt_obj_st)),
    .size_style = (sizeof(_gt_style_label_st))
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static inline void _gt_label_init_widget(gt_obj_st * label) {
    _gt_style_label_st * style = label->style;

    if( 0 == label->area.w || 0 == label->area.h ){
        label->area.h = 20;
        gt_font_st font = {
            .utf8   = (uint8_t *)style->text,
            .len    = strlen(style->text),
            .size   = style->font_size
        };
        label->area.w =  gt_font_get_string_width(&font) + 20;
    }

    uint8_t fp[64];
    gt_memset(fp, 0, 64);
    gt_font_st font = {
        .style_cn   = style->font_family_cn,
        .style_en   = style->font_family_en,
        .res        = fp,
        .utf8       = (uint8_t *)style->text,
        .len        = strlen(style->text),
        .size       = style->font_size,
    };

    /*draw font*/
    gt_attr_font_st font_attr = {
        .font       = &font,
        .font_color = style->font_color,
        .space_x    = 0,
        .space_y    = 0,
        .align      = GT_ALIGN_LEFT,
        .opa        = label->opa,
    };
    draw_text(label->draw_ctx, &font_attr, &label->area);

}

/**
 * @brief obj init label widget cb
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");

    _gt_label_init_widget(obj);
}

/**
 * @brief obj deinit cb
 *
 * @param obj
 */
static void _deinit_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "label deinit");
    if (NULL == obj) {
        return ;
    }

    _gt_style_label_st ** style_p = (_gt_style_label_st ** )&obj->style;
    if (NULL == *style_p) {
        return ;
    }

    if (NULL != (*style_p)->text) {
        gt_mem_free((*style_p)->text);
        (*style_p)->text = NULL;
    }

    gt_mem_free(*style_p);
    *style_p = NULL;
}


/**
 * @brief obj event handler cb
 *
 * @param obj
 * @param e event
 */
static void _event_cb(struct _gt_obj_s * obj, gt_event_st * e) {
    extern void _gt_disp_refr_append_area(gt_area_st * area);
    gt_event_type_et code = gt_event_get_code(e);
    switch(code) {
        case GT_EVENT_TYPE_DRAW_START:
            GT_LOGD(GT_LOG_TAG_GUI, "start draw");
            if( 0 && gt_obj_check_scr(obj) ){
                _gt_disp_refr_append_area(&obj->area);
            }
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;

        case GT_EVENT_TYPE_DRAW_END:
            GT_LOGD(GT_LOG_TAG_GUI, "end draw");
            break;

        case GT_EVENT_TYPE_UPDATE_VALUE:
            GT_LOGD(GT_LOG_TAG_GUI, "value update");
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        case GT_EVENT_TYPE_CHANGE_CHILD_REMOVE: /* remove child from screen but not delete */
            GT_LOGD(GT_LOG_TAG_GUI, "child remove");
			break;

        case GT_EVENT_TYPE_CHANGE_CHILD_DELETE: /* delete child */
            GT_LOGD(GT_LOG_TAG_GUI, "child delete");
            break;

        case GT_EVENT_TYPE_INPUT_CLICKING:   /* add clicking style and process clicking event */
            GT_LOGD(GT_LOG_TAG_GUI, "clicking");
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
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

            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        default:
            break;
    }
}


static void _gt_label_init_style(gt_obj_st * label)
{
    _gt_style_label_st * style = label->style;

    gt_memset(style, 0, sizeof(_gt_style_label_st));

    style->text = gt_mem_malloc(sizeof("label"));
    gt_memcpy(style->text, "label\0", sizeof("label"));

    style->font_color        = gt_color_hex(0x00);

    style->font_family_cn    = GT_CFG_DEFAULT_FONT_FAMILY_CN;
    style->font_family_en    = GT_CFG_DEFAULT_FONT_FAMILY_EN;
    style->font_size         = GT_CFG_DEFAULT_FONT_SIZE;
}




/* global functions / API interface -------------------------------------*/

/**
 * @brief create a label obj
 *
 * @param parent label's parent element
 * @return gt_obj_st* label obj
 */
gt_obj_st * gt_label_create(gt_obj_st * parent) {
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    _gt_label_init_style(obj);
    return obj;
}


void gt_label_style_set_text(gt_obj_st * label, char * text)
{

    _gt_style_label_st * style = label->style;
    if( NULL != style->text ){
        gt_mem_free(style->text);
    }
    uint16_t size = strlen(text);
    style->text = gt_mem_malloc(size+1);
    gt_memset(style->text, 0, size+1);
    gt_memcpy(style->text, text, size);
    gt_event_send(label, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

char * gt_label_style_get_text(gt_obj_st * label)
{
    GT_ASSERT_NULL(label);
    _gt_style_label_st * style = label->style;
    return style->text;
}

void gt_label_style_set_font_color(gt_obj_st * label, gt_color_t color)
{
    _gt_style_label_st * style = label->style;
    style->font_color = color;
    gt_event_send(label, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_label_style_set_font_family_cn(gt_obj_st * label, gt_family_t family)
{
    _gt_style_label_st * style = label->style;
    style->font_family_cn = family;
}
void gt_label_style_set_font_family_en(gt_obj_st * label, gt_family_t family)
{
    _gt_style_label_st * style = label->style;
    style->font_family_en = family;
}

/* end ------------------------------------------------------------------*/
