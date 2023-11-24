/**
 * @file gt_imgbtn.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-05-11 15:03:35
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_imgbtn.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../core/gt_fs.h"
#include "string.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"
#include "../core/gt_event.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_IMGBTN
#define MY_CLASS    &gt_imgbtn_class

/* private typedef ------------------------------------------------------*/
typedef struct _gt_imgbtn_s
{
    char * src;
    char * src_press;           ///< imgbtn src when press, It can be a virt file name/path or an array of C
    char * src_release;         ///< imgbtn src when release
    char * src_base;            ///< imgbtn src when nothing

    _gt_vector_st * imgs;

    uint8_t selected: 1;
    uint8_t reserved: 7;
}_gt_imgbtn_st;


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_imgbtn_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_imgbtn_st)
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static void _gt_imgbtn_set_src(gt_obj_st * imgbtn, char * src){
    _gt_imgbtn_st * style = imgbtn->style;

    if( src == NULL ){
        style->src = style->src_base;
    }else{
        style->src = src;
    }
}

static char * _gt_imgbtn_get_src(gt_obj_st * obj) {
    _gt_imgbtn_st * style = obj->style;
    return style->src;
}

/**
 * @brief obj init imgbtn widget call back
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");

    _gt_imgbtn_st * style = obj->style;
    void * img = style->src_press && style->selected ? style->src_press : style->src;
    gt_attr_rect_st dsc = {
        .bg_img_src = img,
        .bg_opa = obj->opa
    };

    /* start draw imgbtn */
    draw_bg_img(obj->draw_ctx, &dsc, &obj->area);
    style->selected = 0;

    // focus
    draw_focus(obj , 0);
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

    _gt_imgbtn_st ** style_p = (_gt_imgbtn_st ** )&obj->style;
    if (NULL == *style_p) {
        return ;
    }

    if (NULL != (*style_p)->imgs) {
        _gt_vector_free(&(*style_p)->imgs);
        (*style_p)->imgs = NULL;
    }

    if (NULL != (*style_p)->src_press) {
        gt_mem_free((*style_p)->src_press);
        (*style_p)->src_press = NULL;
    }

    if (NULL != (*style_p)->src_release) {
        gt_mem_free((*style_p)->src_release);
        (*style_p)->src_release = NULL;
    }

    if (NULL != (*style_p)->src_base) {
        gt_mem_free((*style_p)->src_base);
        (*style_p)->src_base = NULL;
    }

    (*style_p)->src = NULL;
    gt_mem_free(*style_p);
    *style_p = NULL;
}

static void _invalid_area(gt_obj_st * obj) {
    char * src = _gt_imgbtn_get_src(obj);
    uint16_t w = 0, h = 0;
    gt_fs_read_img_wh(src, &w, &h);
    obj->area.w = GT_MAX(obj->area.w, w);
    obj->area.h = GT_MAX(obj->area.h, h);
    gt_disp_invalid_area(obj);
}

static bool _turn_next_image(gt_obj_st * obj) {
    _gt_imgbtn_st * style = (_gt_imgbtn_st * )obj->style;
    if (NULL == style->imgs) {
        return false;
    }

    _gt_imgbtn_set_src(obj, (char * )_gt_vector_turn_next(style->imgs));
    return true;
}

/**
 * @brief obj event handler call back
 *
 * @param obj
 * @param e event
 */
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    _gt_imgbtn_st * style = obj->style;
    gt_event_type_et code = gt_event_get_code(e);
    switch(code) {
        case GT_EVENT_TYPE_DRAW_START:
            GT_LOGV(GT_LOG_TAG_GUI, "start draw");
            _invalid_area(obj);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;

        case GT_EVENT_TYPE_DRAW_END:
            GT_LOGV(GT_LOG_TAG_GUI, "end draw");
            break;

        case GT_EVENT_TYPE_CHANGE_CHILD_REMOVE: /* remove child from screen but not delete */
            GT_LOGV(GT_LOG_TAG_GUI, "child remove");
			break;

        case GT_EVENT_TYPE_CHANGE_CHILD_DELETE: /* delete child */
            GT_LOGV(GT_LOG_TAG_GUI, "child delete");
            break;
        case GT_EVENT_TYPE_INPUT_PRESSED:
        case GT_EVENT_TYPE_INPUT_PRESSING:   /* add clicking style and process clicking event */
            GT_LOGV(GT_LOG_TAG_GUI, "clicking");
            style->selected = 1;
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;
        case GT_EVENT_TYPE_UPDATE_VALUE:
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_SCROLL:
            GT_LOGV(GT_LOG_TAG_GUI, "scroll");
            break;

        case GT_EVENT_TYPE_INPUT_RELEASED: /* click event finish */
            GT_LOGV(GT_LOG_TAG_GUI, "processed");
            style->selected = 0;
            if (!_turn_next_image(obj)) {
                _gt_imgbtn_set_src(obj, style->src_base);
            }
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        default:
            break;
    }
}


static void _gt_imgbtn_init_style(gt_obj_st * imgbtn)
{
    _gt_imgbtn_st * style = (_gt_imgbtn_st * )imgbtn->style;

    gt_memset(style, 0, sizeof(_gt_imgbtn_st));

	style->selected = 0;
    style->src = NULL;
}

static bool _imgs_free_cb(void * item) {
    gt_mem_free(item);
    return true;
}

static bool _imgs_equal_cb(void * item, void * target) {
    return strcmp(item, target) ? false : true;
}

static void imgs_init(_gt_imgbtn_st * style) {
    if (NULL == style) {
        return ;
    }
    _gt_vector_add_free_item_cb(&style->imgs, _imgs_free_cb);
    _gt_vector_add_equal_item_cb(&style->imgs, _imgs_equal_cb);
}


/* global functions / API interface -------------------------------------*/

gt_obj_st * gt_imgbtn_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    _gt_imgbtn_init_style(obj);
    return obj;
}


void gt_imgbtn_style_set_selected(gt_obj_st * imgbtn, uint8_t selected)
{
    _gt_imgbtn_st * style = imgbtn->style;
    style->selected = selected ? 1 : 0;
    gt_event_send(imgbtn, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_imgbtn_set_src(gt_obj_st * imgbtn, char * src)
{
    _gt_imgbtn_st * style = (_gt_imgbtn_st *)imgbtn->style;
    if( NULL != style->src_base ){
        gt_mem_free(style->src_base);
    }
    uint16_t len = src == NULL ? 0 : strlen(src);

    style->src_base = gt_mem_malloc( len + 1 );
    strcpy(style->src_base, src);
    style->src_base[len] = 0;
    style->src = style->src_base;
    gt_event_send(imgbtn, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

void gt_imgbtn_set_src_press(gt_obj_st * imgbtn, char * src)
{
    _gt_imgbtn_st * style = (_gt_imgbtn_st *)imgbtn->style;
    if( NULL != style->src_press ){
        gt_mem_free(style->src_press);
    }
    uint16_t len = src == NULL ? 0 : strlen(src);

    style->src_press = gt_mem_malloc( len + 1 );
    strcpy(style->src_press, src);
    style->src_press[len] = 0;

    gt_event_send(imgbtn, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

void gt_imgbtn_set_src_release(gt_obj_st * imgbtn, char * src)
{
    _gt_imgbtn_st * style = (_gt_imgbtn_st *)imgbtn->style;
    if( NULL != style->src_release ){
        gt_mem_free(style->src_release);
    }
    uint16_t len = src == NULL ? 0 : strlen(src);
    style->src_release = gt_mem_malloc( len + 1 );
    strcpy(style->src_release, src);
    style->src_release[len] = 0;

    gt_event_send(imgbtn, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

bool gt_imgbtn_add_state_item(gt_obj_st * obj, char * src)
{
    if (NULL == obj) {
        return false;
    }
    _gt_imgbtn_st * style = (_gt_imgbtn_st *)obj->style;
    if (NULL == style) {
        return false;
    }
    if (NULL == src || !strlen(src)) {
        return false;
    }

    imgs_init(style);
    /** set first item src */
    if (_gt_vector_get_count(style->imgs) <= 0) {
        _gt_vector_add_item(&style->imgs, (void *)style->src_base, strlen(style->src_base) + 1);
    }
    return _gt_vector_add_item(&style->imgs, (void *)src, strlen(src) + 1);
}

bool gt_imgbtn_remove_state_item(gt_obj_st * obj, char * src)
{
    if (NULL == obj) {
        return false;
    }
    _gt_imgbtn_st * style = (_gt_imgbtn_st *)obj->style;
    if (NULL == style) {
        return false;
    }
    if (NULL == src || !strlen(src)) {
        return false;
    }
    return _gt_vector_remove_item(&style->imgs, (void * )src);
}

bool gt_imgbtn_clear_all_state_item(gt_obj_st * obj)
{
    if (NULL == obj) {
        return false;
    }
    _gt_imgbtn_st * style = (_gt_imgbtn_st *)obj->style;
    if (NULL == style) {
        return false;
    }
    return _gt_vector_clear_all_items(style->imgs);
}

int16_t bt_imgbtn_get_state_item_index(gt_obj_st * obj)
{
    if (NULL == obj) {
        return -1;
    }
    _gt_imgbtn_st * style = (_gt_imgbtn_st *)obj->style;
    if (NULL == style) {
        return -1;
    }
    return _gt_vector_get_index(style->imgs);
}

/* end ------------------------------------------------------------------*/
