/**
 * @file gt_imgbtn.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-05-11 15:03:35
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_imgbtn.h"

#if GT_CFG_ENABLE_IMGBTN
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
typedef struct _item_s {
    char * name;
#if GT_USE_FILE_HEADER
    gt_file_header_param_st fh;
#endif
#if GT_USE_DIRECT_ADDR
    gt_addr_t addr;
#endif
}_item_st;

typedef struct _gt_imgbtn_s {
    gt_obj_st obj;

    _item_st src;
    _item_st press;     ///< imgbtn src when press, It can be a virt file name/path or an array of C
    _item_st release;   ///< imgbtn src when release

    gt_point_st prev_size;

    _gt_vector_st * imgs;

    uint8_t selected: 1;
    uint8_t reserved: 7;
}_gt_imgbtn_st;


/* static variables -----------------------------------------------------*/
static void _imgbtn_init_cb(gt_obj_st * obj);
static void _imgbtn_deinit_cb(gt_obj_st * obj);
static void _imgbtn_event_cb(struct gt_obj_s * obj, gt_event_st * e);

static const gt_obj_class_st gt_imgbtn_class = {
    ._init_cb      = _imgbtn_init_cb,
    ._deinit_cb    = _imgbtn_deinit_cb,
    ._event_cb     = _imgbtn_event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_imgbtn_st)
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static void _gt_imgbtn_set_src(gt_obj_st * imgbtn, _item_st * src) {
    _gt_imgbtn_st * style = (_gt_imgbtn_st * )imgbtn;

    if (src == NULL) {
        style->src.name = NULL;
        return;
    }
    style->src.name = src->name;
}

static char * _gt_imgbtn_get_src(gt_obj_st * obj) {
    _gt_imgbtn_st * style = (_gt_imgbtn_st * )obj;
    return style->src.name;
}

/**
 * @brief obj init imgbtn widget call back
 *
 * @param obj
 */
static void _imgbtn_init_cb(gt_obj_st * obj) {
    _gt_imgbtn_st * style = (_gt_imgbtn_st * )obj;
    _item_st * img = style->press.name && style->selected ? &style->press : &style->src;
    gt_attr_rect_st dsc = {
        .bg_img_src = img->name,
        .bg_opa = obj->opa
    };
#if GT_USE_FILE_HEADER
    dsc.file_header = gt_file_header_param_check_valid(&img->fh);
#endif
#if GT_USE_DIRECT_ADDR
    dsc.addr = img->addr;
#endif

    // TODO pref time
    if (obj->area.w > style->prev_size.x) {
        style->prev_size.x = obj->area.w;
    }
    if (obj->area.h > style->prev_size.y) {
        style->prev_size.y = obj->area.h;
    }

    obj->area.w = GT_MAX(style->prev_size.x, obj->area.w);
    obj->area.h = GT_MAX(style->prev_size.y, obj->area.h);

    /* start draw imgbtn */
    draw_bg_img(obj->draw_ctx, &dsc, &obj->area);

    // focus
    draw_focus(obj , 0);
}

/**
 * @brief obj deinit call back
 *
 * @param obj
 */
static void _imgbtn_deinit_cb(gt_obj_st * obj) {
    _gt_imgbtn_st * style_p = (_gt_imgbtn_st * )obj;
    if (NULL != style_p->imgs) {
        _gt_vector_free(style_p->imgs);
        style_p->imgs = NULL;
    }

    if (NULL != style_p->press.name) {
        gt_mem_free(style_p->press.name);
        style_p->press.name = NULL;
    }

    if (NULL != style_p->release.name) {
        gt_mem_free(style_p->release.name);
        style_p->release.name = NULL;
    }

    style_p->src.name = NULL;
}

static void _invalid_area(gt_obj_st * obj) {
    char * src = _gt_imgbtn_get_src(obj);
    uint16_t w = 0, h = 0;
    if (GT_FS_RES_FAIL == gt_fs_read_img_wh(src, &w, &h)) {
        return ;
    }
    if (0 == w || 0 == h) {
        return ;
    }
    obj->area.w = GT_MAX(obj->area.w, w);
    obj->area.h = GT_MAX(obj->area.h, h);
    gt_disp_invalid_area(obj);
}

static bool _turn_next_image(gt_obj_st * obj) {
    _gt_imgbtn_st * style = (_gt_imgbtn_st * )obj;
    if (NULL == style->imgs) {
        return false;
    }

    _gt_imgbtn_set_src(obj, (_item_st * )_gt_vector_turn_next(style->imgs));
    return true;
}

/**
 * @brief obj event handler call back
 *
 * @param obj
 * @param e event
 */
static void _imgbtn_event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    _gt_imgbtn_st * style = (_gt_imgbtn_st * )obj;
    gt_event_type_et code_val = gt_event_get_code(e);
    switch(code_val) {
        case GT_EVENT_TYPE_DRAW_START:
            _invalid_area(obj);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_PRESSED:
        case GT_EVENT_TYPE_INPUT_PRESSING:   /* add clicking style and process clicking event */
            style->selected = 1;
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        case GT_EVENT_TYPE_UPDATE_VALUE:
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_RELEASED: /* click event finish */
            style->selected = 0;
            if (!_turn_next_image(obj)) {
                _gt_imgbtn_set_src(obj, (_item_st * )_gt_vector_get_item(style->imgs, 0));
            }
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_PRESS_LOST: {
            style->selected = 0;
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;
        }

        default:
            break;
    }
}


static bool _imgs_free_cb(void * item) {
    _item_st * item_p = (_item_st * )item;
    if (NULL == item_p) {
        return true;
    }
    if (item_p->name) {
        gt_mem_free(item_p->name);
        item_p->name = NULL;
    }
    gt_mem_free(item);
    return true;
}

static bool _imgs_equal_cb(void * item, void * target) {
    _item_st * item_p = (_item_st * )item;
    _item_st * target_p = (_item_st * )target;
#if GT_USE_FILE_HEADER
    if (GT_FILE_HEADER_INVALID_IDX != target_p->fh.idx) {
        if (item_p->fh.idx == target_p->fh.idx) {
            return true;
        }
    }
#endif
#if GT_USE_DIRECT_ADDR
    if (false == gt_hal_is_invalid_addr(target_p->addr)) {
        if (item_p->addr == target_p->addr) {
            return true;
        }
    }
#endif
    return strcmp(item_p->name, target_p->name) ? false : true;
}

/* global functions / API interface -------------------------------------*/

gt_obj_st * gt_imgbtn_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    if (NULL == obj) {
        return obj;
    }
    _gt_imgbtn_st * style = (_gt_imgbtn_st * )obj;
	style->selected = 0;
    style->src.name = NULL;

#if GT_USE_DIRECT_ADDR
    gt_hal_direct_addr_init(&style->src.addr);
    gt_hal_direct_addr_init(&style->press.addr);
    gt_hal_direct_addr_init(&style->release.addr);
#endif

    style->imgs = _gt_vector_create(_imgs_free_cb, _imgs_equal_cb);

    return obj;
}


void gt_imgbtn_style_set_selected(gt_obj_st * imgbtn, uint8_t selected)
{
    if (false == gt_obj_is_type(imgbtn, OBJ_TYPE)) {
        return ;
    }
    _gt_imgbtn_st * style = (_gt_imgbtn_st * )imgbtn;
    style->selected = selected ? 1 : 0;
    gt_event_send(imgbtn, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_imgbtn_set_src(gt_obj_st * imgbtn, char * src)
{
    if (false == gt_obj_is_type(imgbtn, OBJ_TYPE)) {
        return ;
    }
    _gt_imgbtn_st * style = (_gt_imgbtn_st *)imgbtn;
    uint16_t len = src == NULL ? 0 : strlen(src);
    _item_st * item = (_item_st * )gt_mem_malloc(sizeof(_item_st));
    GT_CHECK_BACK(item);

    item->name = (char * )gt_mem_malloc(len + 1);
    if (NULL == item->name) {
        GT_CHECK_PRINT(item->name);
        goto item_lb;
    }
    gt_memcpy(item->name, src, len);
    item->name[len] = '\0';

#if GT_USE_FILE_HEADER
    gt_file_header_param_init(&item->fh);
#endif

#if GT_USE_DIRECT_ADDR
    gt_hal_direct_addr_init(&item->addr);
#endif

    if (false == _gt_vector_replace_item(style->imgs, 0, (void * )item)) {
        if (false == _gt_vector_add_item(style->imgs, (void * )item)) {
            goto name_lb;
        }
    }
    _item_st * item_p = (_item_st *)_gt_vector_get_item(style->imgs, 0);
    GT_CHECK_BACK(item_p);
    style->src = *item_p;

    gt_event_send(imgbtn, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
    return;

name_lb:
    gt_mem_free(item->name);
    item->name = NULL;
item_lb:
    gt_mem_free(item);
    item = NULL;
}

void gt_imgbtn_set_src_press(gt_obj_st * imgbtn, char * src)
{
    if (false == gt_obj_is_type(imgbtn, OBJ_TYPE)) {
        return ;
    }
    _gt_imgbtn_st * style = (_gt_imgbtn_st *)imgbtn;
    uint16_t len = src == NULL ? 0 : strlen(src);

    style->press.name = style->press.name ? gt_mem_realloc(style->press.name, len + 1) : gt_mem_malloc(len + 1);
    if (NULL == style->press.name) {
        return ;
    }
    strcpy(style->press.name, src);
    style->press.name[len] = 0;
#if GT_USE_FILE_HEADER
    gt_file_header_param_init(&style->press.fh);
#endif

#if GT_USE_DIRECT_ADDR
    gt_hal_direct_addr_init(&style->press.addr);
#endif

    gt_event_send(imgbtn, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

void gt_imgbtn_set_src_release(gt_obj_st * imgbtn, char * src)
{
    if (false == gt_obj_is_type(imgbtn, OBJ_TYPE)) {
        return ;
    }
    _gt_imgbtn_st * style = (_gt_imgbtn_st *)imgbtn;
    uint16_t len = src == NULL ? 0 : strlen(src);

    style->release.name = style->release.name ? gt_mem_realloc(style->release.name, len + 1) : gt_mem_malloc(len + 1);
    if (NULL == style->release.name) {
        return ;
    }
    strcpy(style->release.name, src);
    style->release.name[len] = 0;
#if GT_USE_FILE_HEADER
    gt_file_header_param_init(&style->release.fh);
#endif

#if GT_USE_DIRECT_ADDR
    gt_hal_direct_addr_init(&style->release.addr);
#endif

    gt_event_send(imgbtn, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

bool gt_imgbtn_add_state_item(gt_obj_st * obj, char * src)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return false;
    }
    _gt_imgbtn_st * style = (_gt_imgbtn_st *)obj;
    uint16_t len = strlen(src);
    if (NULL == src || !len) {
        return false;
    }

    _item_st * item = (_item_st * )gt_mem_malloc(sizeof(_item_st));
    GT_CHECK_BACK_VAL(item, false);

    item->name = (char * )gt_mem_malloc(len + 1);
    if (NULL == item->name) {
        GT_CHECK_PRINT(item->name);
        goto item_lb;
    }
    gt_memcpy(item->name, src, len);
    item->name[len] = '\0';

#if GT_USE_FILE_HEADER
    gt_file_header_param_init(&item->fh);
#endif

#if GT_USE_DIRECT_ADDR
    gt_hal_direct_addr_init(&item->addr);
#endif

    if (false == _gt_vector_add_item(style->imgs, (void * )item)) {
        goto name_lb;
    }
    return true;

name_lb:
    gt_mem_free(item->name);
    item->name = NULL;
item_lb:
    gt_mem_free(item);
    item = NULL;
    return false;
}

bool gt_imgbtn_remove_state_item(gt_obj_st * obj, char * src)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return false;
    }
    _gt_imgbtn_st * style = (_gt_imgbtn_st *)obj;
    if (NULL == src || !strlen(src)) {
        return false;
    }
    _item_st item = {
        .name = src
    };
#if GT_USE_FILE_HEADER
    gt_file_header_param_init(&item.fh);
#endif

#if GT_USE_DIRECT_ADDR
    gt_hal_direct_addr_init(&item.addr);
#endif

    return _gt_vector_remove_item(style->imgs, &item);
}

#if GT_USE_FILE_HEADER
void gt_imgbtn_set_src_by_file_header(gt_obj_st * imgbtn, gt_file_header_param_st * fh)
{
    if (false == gt_obj_is_type(imgbtn, OBJ_TYPE)) {
        return ;
    }
    if (NULL == fh) {
        return ;
    }
    _gt_imgbtn_st * style = (_gt_imgbtn_st *)imgbtn;
    _item_st * item = (_item_st * )gt_mem_malloc(sizeof(_item_st));
    GT_CHECK_BACK(item);
    item->fh = *fh;
#if GT_USE_DIRECT_ADDR
    gt_hal_direct_addr_init(&item->addr);
#endif

    if (0 == _gt_vector_get_count(style->imgs)) {
        if (false == _gt_vector_add_item(style->imgs, (void * )item)) {
            goto item_lb;
        }
    } else {
        if (false == _gt_vector_replace_item(style->imgs, 0, (void * )item)) {
            goto item_lb;
        }
    }
    _item_st * item_p = (_item_st *)_gt_vector_get_item(style->imgs, 0);
    GT_CHECK_BACK(item_p);
    style->src = *item_p;

    gt_event_send(imgbtn, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
    return;

item_lb:
    gt_mem_free(item);
    item = NULL;
}

void gt_imgbtn_set_src_press_by_file_header(gt_obj_st * imgbtn, gt_file_header_param_st * fh)
{
    if (false == gt_obj_is_type(imgbtn, OBJ_TYPE)) {
        return ;
    }
    if (NULL == fh) {
        return ;
    }
    _gt_imgbtn_st * style = (_gt_imgbtn_st *)imgbtn;
    if( NULL != style->press.name ){
        gt_mem_free(style->press.name);
        style->press.name = NULL;
    }
    style->press.fh = *fh;
#if GT_USE_DIRECT_ADDR
    gt_hal_direct_addr_init(&style->press.addr);
#endif

    gt_event_send(imgbtn, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

void gt_imgbtn_set_src_release_by_file_header(gt_obj_st * imgbtn, gt_file_header_param_st * fh)
{
    if (false == gt_obj_is_type(imgbtn, OBJ_TYPE)) {
        return ;
    }
    if (NULL == fh) {
        return ;
    }
    _gt_imgbtn_st * style = (_gt_imgbtn_st *)imgbtn;
    if( NULL != style->release.name ){
        gt_mem_free(style->release.name);
        style->release.name = NULL;
    }
    style->release.fh = *fh;
#if GT_USE_DIRECT_ADDR
    gt_hal_direct_addr_init(&style->release.addr);
#endif

    gt_event_send(imgbtn, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

bool gt_imgbtn_add_state_item_by_file_header(gt_obj_st * obj, gt_file_header_param_st * fh)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return false;
    }
    if (NULL == fh) {
        return false;
    }
    _gt_imgbtn_st * style = (_gt_imgbtn_st *)obj;
    _item_st * item = (_item_st * )gt_mem_malloc(sizeof(_item_st));
    GT_CHECK_BACK_VAL(item, false);
    item->fh = *fh;
#if GT_USE_DIRECT_ADDR
    gt_hal_direct_addr_init(&item->addr);
#endif

    if (false == _gt_vector_add_item(style->imgs, (void * )item)) {
        gt_mem_free(item);
        item = NULL;
        return false;
    }
    return true;
}

bool gt_imgbtn_remove_state_item_by_file_header(gt_obj_st * obj, gt_file_header_param_st * fh)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return false;
    }
    if (NULL == fh) {
        return false;
    }
    _gt_imgbtn_st * style = (_gt_imgbtn_st *)obj;
    _item_st item = {
        .name = NULL,
        .fh = *fh,
    };

    return _gt_vector_remove_item(style->imgs, &item);
}
#endif

#if GT_USE_DIRECT_ADDR
void gt_imgbtn_set_src_by_direct_addr(gt_obj_st * imgbtn, gt_addr_t addr)
{
    if (false == gt_obj_is_type(imgbtn, OBJ_TYPE)) {
        return ;
    }
    if (gt_hal_is_invalid_addr(addr)) {
        return ;
    }

    _gt_imgbtn_st * style = (_gt_imgbtn_st *)imgbtn;
    _item_st * item = (_item_st * )gt_mem_malloc(sizeof(_item_st));
    GT_CHECK_BACK(item);
    item->addr = addr;
#if GT_USE_FILE_HEADER
    gt_file_header_param_init(&item->fh);
#endif
    if (0 == _gt_vector_get_count(style->imgs)) {
        if (false == _gt_vector_add_item(style->imgs, (void * )item)) {
            goto item_lb;
        }
    } else {
        if (false == _gt_vector_replace_item(style->imgs, 0, (void * )item)) {
            goto item_lb;
        }
    }
    _item_st * item_p = (_item_st *)_gt_vector_get_item(style->imgs, 0);
    GT_CHECK_BACK(item_p);
    style->src = *item_p;

    gt_event_send(imgbtn, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
    return;

item_lb:
    gt_mem_free(item);
    item = NULL;
}

void gt_imgbtn_set_src_press_by_direct_addr(gt_obj_st * imgbtn, gt_addr_t addr)
{
    if (false == gt_obj_is_type(imgbtn, OBJ_TYPE)) {
        return ;
    }
    _gt_imgbtn_st * style = (_gt_imgbtn_st *)imgbtn;
    if( NULL != style->press.name ){
        gt_mem_free(style->press.name);
        style->press.name = NULL;
    }
    style->press.addr = addr;
#if GT_USE_FILE_HEADER
    gt_file_header_param_init(&style->press.fh);
#endif

    gt_event_send(imgbtn, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

void gt_imgbtn_set_src_release_by_direct_addr(gt_obj_st * imgbtn, gt_addr_t addr)
{
    if (false == gt_obj_is_type(imgbtn, OBJ_TYPE)) {
        return ;
    }
    _gt_imgbtn_st * style = (_gt_imgbtn_st *)imgbtn;
    if( NULL != style->release.name ){
        gt_mem_free(style->release.name);
        style->release.name = NULL;
    }
    style->release.addr = addr;
#if GT_USE_FILE_HEADER
    gt_file_header_param_init(&style->release.fh);
#endif

    gt_event_send(imgbtn, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

bool gt_imgbtn_add_state_item_by_direct_addr(gt_obj_st * obj, gt_addr_t addr)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return false;
    }
    if (gt_hal_is_invalid_addr(addr)) {
        return false;
    }

    _gt_imgbtn_st * style = (_gt_imgbtn_st *)obj;
    _item_st * item = (_item_st * )gt_mem_malloc(sizeof(_item_st));
    GT_CHECK_BACK_VAL(item, false);
    item->addr = addr;
#if GT_USE_FILE_HEADER
    gt_file_header_param_init(&item->fh);
#endif

    if (false == _gt_vector_add_item(style->imgs, (void * )item)) {
        gt_mem_free(item);
        item = NULL;
        return false;
    }
    return true;
}

bool gt_imgbtn_remove_state_item_by_direct_addr(gt_obj_st * obj, gt_addr_t addr)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return false;
    }
    _gt_imgbtn_st * style = (_gt_imgbtn_st *)obj;
    _item_st item = {
        .name = NULL,
        .addr = addr,
    };

    return _gt_vector_remove_item(style->imgs, &item);
}
#endif

bool gt_imgbtn_clear_all_state_item(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return false;
    }
    _gt_imgbtn_st * style = (_gt_imgbtn_st *)obj;
    return _gt_vector_clear_all_items(style->imgs);
}

int16_t bt_imgbtn_get_state_item_index(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return -1;
    }
    _gt_imgbtn_st * style = (_gt_imgbtn_st *)obj;
    return _gt_vector_get_index(style->imgs);
}

#endif  /** GT_CFG_ENABLE_IMGBTN */
/* end ------------------------------------------------------------------*/
