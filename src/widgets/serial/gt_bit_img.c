/**
 * @file gt_bit_img.c
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2024-10-12 09:40:57
 * @copyright Copyright (c) 2014-2024, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_bit_img.h"
#if GT_CFG_ENABLE_BIT_IMG
#include "../../core/gt_mem.h"
#include "../../others/gt_types.h"
#include "../../core/gt_draw.h"
#include "../../core/gt_disp.h"
#include "../../core/gt_fs.h"
#include "../../others/gt_log.h"
#include "../../core/gt_obj_pos.h"
#include "../../hal/gt_hal_tick.h"
#include "../../utils/gt_vector.h"
#include "../../core/gt_fs.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_BIT_IMG
#define MY_CLASS    &gt_bit_img_class

//
#define _NO_ANIM    (0)
#define _ANIM_0     (1)
#define _ANIM_1     (2)
#define _ANIM_ALL   (3)

/* private typedef ------------------------------------------------------*/
typedef struct _gt_bit_img_item_s {
    void * src;

#if GT_USE_FILE_HEADER
    gt_file_header_param_st fh_param;
#endif

#if GT_USE_DIRECT_ADDR
    gt_addr_t addr;
#endif
}_gt_bit_img_item_st;

typedef struct {
    uint16_t lock : 1;
    uint16_t real_time_update: 1;  ///< 1: real time update, 0: update when the bit_img is displayed

}_gt_bit_img_reg_st;


typedef struct _gt_bit_img_s {
    gt_obj_st obj;
    _gt_vector_st * vector0;
    _gt_vector_st * vector1;
    _gt_timer_st * timer;
    uint32_t last_tick;
    _gt_bit_img_reg_st reg;

    uint16_t value;
    uint16_t act;
    uint16_t move_gap;
    uint8_t disp_mode;
    uint8_t move_mode;
}_gt_bit_img_st;


/* static prototypes ----------------------------------------------------*/



/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

static GT_ATTRIBUTE_RAM_DATA const gt_obj_class_st gt_bit_img_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_bit_img_st)
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
/**
 * @brief Get the src object from the list item
 *
 * @param style
 * @return void*
 */
static inline void * _get_src(_gt_vector_st * vector) {
    _gt_bit_img_item_st * item = (_gt_bit_img_item_st * )_gt_vector_get_item(vector, _gt_vector_get_index(vector));
    GT_CHECK_BACK_VAL(item, NULL);
    return item->src;
}

#if GT_USE_FILE_HEADER
static inline gt_file_header_param_st * _get_file_header_param(_gt_vector_st * vector) {
    _gt_bit_img_item_st * item = (_gt_bit_img_item_st * )_gt_vector_get_item(vector, _gt_vector_get_index(vector));
    GT_CHECK_BACK_VAL(item, NULL);
    return &item->fh_param;
}
#endif

#if GT_USE_DIRECT_ADDR
static inline gt_addr_t _get_direct_addr_param(_gt_vector_st * vector) {
    _gt_bit_img_item_st * item = (_gt_bit_img_item_st * )_gt_vector_get_item(vector, _gt_vector_get_index(vector));
    GT_CHECK_BACK_VAL(item, GT_ADDR_INVALID);
    return item->addr;
}
#endif

static bool _is_position_retain(uint8_t act, gt_bit_img_move_mode_et mv_mode)
{
    if(!act && (MOVE_X_POSITION_NOT_RETAIN == mv_mode || MOVE_Y_POSITION_NOT_RETAIN == mv_mode)){
        return false;
    }
    return true;
}

static bool _is_move_dir_x(gt_bit_img_move_mode_et mv_mode)
{
    if(MOVE_Y_POSITION_RETAIN == mv_mode || MOVE_Y_POSITION_NOT_RETAIN == mv_mode){
        return false;
    }
    return true;
}

static _gt_vector_st* _get_vector(_gt_bit_img_st * style, uint8_t val)
{
    _gt_vector_st* vec = NULL;
    switch (style->disp_mode)
    {
    case BIN_VAR_DISP_MODE_0: vec = (val) ? style->vector1 : style->vector0; break;
    case BIN_VAR_DISP_MODE_1: vec = (val) ? NULL : style->vector0; break;
    case BIN_VAR_DISP_MODE_2: vec = (val) ? style->vector1 : style->vector0; break;
    case BIN_VAR_DISP_MODE_3: vec = (val) ? style->vector1 : NULL; break;
    case BIN_VAR_DISP_MODE_4: vec = (val) ? style->vector1 : NULL; break;
    case BIN_VAR_DISP_MODE_5: vec = (val) ? style->vector1 : style->vector0; break;
    case BIN_VAR_DISP_MODE_6: vec = (val) ? NULL : style->vector0; break;
    case BIN_VAR_DISP_MODE_7: vec = (val) ? style->vector1 : style->vector0; break;
    default:
        break;
    }
    return vec;
}

static int _is_use_anim(_gt_bit_img_st * style){
    int ret = _NO_ANIM;
    switch (style->disp_mode)
    {
    case BIN_VAR_DISP_MODE_2:
    case BIN_VAR_DISP_MODE_4:
        ret = (style->value) ? _ANIM_1 : _NO_ANIM;
        break;
    case BIN_VAR_DISP_MODE_5:
    case BIN_VAR_DISP_MODE_6:
        ret = ((style->value & 0xFFFF) == 0xFFFF) ? _NO_ANIM : _ANIM_0;
        break;
    case BIN_VAR_DISP_MODE_7:
        ret = _ANIM_ALL ;
        break;
    default:
        break;
    }

    return ret;
}

static inline bool _lock(_gt_bit_img_st * style) {
    if (NULL == style) {
        return false;
    }
    if (style->reg.lock) {
        return false;
    }
    style->reg.lock = true;
    return true;
}

/**
 * @brief Unlock the bit_img callback handler
 *
 * @param style
 * @return true Set unlock status success
 * @return false Unable to set unlock status
 */
static inline bool _unlock(_gt_bit_img_st * style) {
    if (NULL == style) {
        return false;
    }
    style->reg.lock = false;
    return true;
}

static void _init_cb(gt_obj_st * obj)
{
    _gt_bit_img_st * style = (_gt_bit_img_st * )obj;
    GT_CHECK_BACK(style->vector0);
    GT_CHECK_BACK(style->vector1);

    if (0 == _gt_vector_get_count(style->vector0) && 0 == _gt_vector_get_count(style->vector1)) {
        return;
    }

    //
    _unlock(style);

    uint16_t w = 0, h = 0;
    gt_area_st area = {0};
    area.x = obj->area.x;
    area.y = obj->area.y;

    _gt_vector_st* vector = NULL;
    int8_t v_bit = 0;
    uint8_t val = 0, act = 0;
    for(v_bit = 16; v_bit >= 0; --v_bit) {
        act = (style->act >> v_bit) & 0x01;
        val = (style->value >> v_bit) & 0x01;

        if(!_is_position_retain(act, style->move_mode)) {
            continue;
        }
        if(!act) goto _next;

        vector = _get_vector(style, val);

        if(vector) {
            gt_attr_rect_st dsc = {
                .bg_img_src = (void * )_get_src(vector),
                .bg_opa = obj->opa,
            };
#if GT_USE_FILE_HEADER
            dsc.file_header = gt_file_header_param_check_valid(_get_file_header_param(vector));
#endif

#if GT_USE_DIRECT_ADDR
            dsc.addr = _get_direct_addr_param(vector);
#endif
            /* start draw obj */
            draw_bg_img(obj->draw_ctx, &dsc, &area);
        }
_next:
        if(_is_move_dir_x(style->move_mode)){
            area.x += style->move_gap;
        }
        else {
            area.y += style->move_gap;
        }
    }
}


static void _destroy_timer(_gt_bit_img_st * style) {

    if(style->timer){
        _gt_timer_del(style->timer);
        style->timer = NULL;
    }
}

static void _deinit_cb(gt_obj_st * obj)
{
    if (NULL == obj) {
        return ;
    }
    _gt_bit_img_st * style = (_gt_bit_img_st * )obj;
    _destroy_timer(style);

    if (style->vector0) {
        _gt_vector_free(style->vector0);
        style->vector0 = NULL;
    }

    if (style->vector1) {
        _gt_vector_free(style->vector1);
        style->vector1 = NULL;
    }
}

static void _update_area(struct gt_obj_s * obj)
{
    _gt_bit_img_st * style = (_gt_bit_img_st * )obj;
    uint16_t w = 0, h = 0;
    gt_area_st area = {0};
    area.x = obj->area.x;
    area.y = obj->area.y;

    _gt_vector_st* vector = NULL;
    int8_t v_bit = 0;
    uint8_t val = 0, act = 0;
    for(v_bit = 16; v_bit >= 0; --v_bit) {
        act = (style->act >> v_bit) & 0x01;
        val = (style->value >> v_bit) & 0x01;

        if(!_is_position_retain(act, style->move_mode)) {
            continue;
        }

        vector = _get_vector(style, val);

        if(vector) {
            gt_fs_read_img_wh(_get_src(vector), &w, &h);

#if GT_USE_FILE_HEADER
            gt_fs_fh_read_img_wh(_get_file_header_param(vector), &w, &h);
#endif
#if GT_USE_DIRECT_ADDR
            GT_FS_RES_OK == gt_fs_direct_addr_read_img_wh(_get_direct_addr_param(vector), &w, &h);
#endif
        }

        if(_is_move_dir_x(style->move_mode)){
            area.w += (area.w == 0) ? w : style->move_gap;
            area.h = GT_MAX(area.h, h);
        }
        else {
            area.h += (area.h == 0) ? h : style->move_gap;
            area.w = GT_MAX(area.w, w);
        }
    }
    gt_obj_size_change(obj, &area);
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

static void _event_cb(struct gt_obj_s * obj, gt_event_st * e)
{
    gt_event_type_et code_val = gt_event_get_code(e);

    if (GT_EVENT_TYPE_DRAW_START == code_val) {
        gt_disp_invalid_area(obj);
    } else if (GT_EVENT_TYPE_UPDATE_STYLE == code_val) {
        gt_disp_invalid_area(obj);
    }
    else if(GT_EVENT_TYPE_UPDATE_VALUE == code_val){
        _update_area(obj);
    }
}


static void _turn_prev_item(gt_obj_st * obj, _gt_vector_st* vector) {
    // _gt_bit_img_st * style = (_gt_bit_img_st * )obj;
    GT_CHECK_BACK(vector);
    _gt_vector_turn_prev(vector);
}

static void _turn_next_item(gt_obj_st * obj, _gt_vector_st* vector) {
    // _gt_bit_img_st * style = (_gt_bit_img_st * )obj;
    GT_CHECK_BACK(vector);
    _gt_vector_turn_next(vector);
}

static inline void _change_item_handler(gt_obj_st * obj, bool is_next, _gt_vector_st* vector) {
    is_next ? _turn_next_item(obj, vector) : _turn_prev_item(obj, vector);
}

static void _change_timer_handler(gt_obj_st * obj)
{
    _gt_bit_img_st * style = (_gt_bit_img_st * )obj;

    int r_anim = _is_use_anim(style);
    if(_NO_ANIM == r_anim){
        if(!_gt_timer_get_paused(style->timer)){
            _gt_timer_set_paused(style->timer, true);
        }
    }
    else{
        bool is_paused = !((_gt_vector_get_count(style->vector0) < 2) && (_gt_vector_get_count(style->vector1) < 2));
        if(_gt_timer_get_paused(style->timer) && is_paused){
            _gt_timer_set_paused(style->timer, false);
        }
    }
}

static void _auto_timer_callback(struct _gt_timer_s * timer){
    gt_obj_st * obj = (gt_obj_st * )_gt_timer_get_user_data(timer);
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }

    _gt_bit_img_st * style = (_gt_bit_img_st * )obj;

    if (!_lock(style)) {
        /** The callback has been executed, waiting for it to be displayed */
        gt_disp_invalid_area(obj);
        return ;
    }

    int r_anim = _is_use_anim(style);


    gt_size_t count = 1;
    if (style->reg.real_time_update) {
        count = gt_tick_elapse(style->last_tick) / _gt_timer_get_period(timer);
    }

    while (count-- > 0) {
        if(_ANIM_ALL == r_anim){
            _change_item_handler(obj, true, style->vector0);
            _change_item_handler(obj, true, style->vector1);
        }
        else if(_ANIM_0 == r_anim){
            _change_item_handler(obj, true, style->vector0);
        }
        else if(_ANIM_1 == r_anim){
            _change_item_handler(obj, true, style->vector1);
        }
    }
    style->last_tick =  gt_tick_get();
}


static bool _free_item_cb(void * item) {
    _gt_bit_img_item_st * item_p = (_gt_bit_img_item_st * )item;
    GT_CHECK_BACK_VAL(item_p, false);
    if (item_p->src) {
        gt_mem_free(item_p->src);
        item_p->src = NULL;
    }
    gt_mem_free(item_p);
    return true;
}

static bool _equal_item_cb(void * item, void * target) {
    _gt_bit_img_item_st * item_p = (_gt_bit_img_item_st * )item;
    _gt_bit_img_item_st * tar_p = (_gt_bit_img_item_st * )target;
    GT_CHECK_BACK_VAL(item_p, false);
    GT_CHECK_BACK_VAL(tar_p, false);

    if ((tar_p->src || item_p->src) && tar_p->src == item_p->src) {
        return true;
    }
#if GT_USE_FILE_HEADER
    if (true == gt_file_header_param_is_equal(&tar_p->fh_param, &item_p->fh_param)) {
        return true;
    }
#endif

#if GT_USE_DIRECT_ADDR
    if (false == gt_hal_is_invalid_addr(tar_p->addr)) {
        if (item_p->addr == tar_p->addr) {
            return true;
        }
    }
#endif
    return false;
}

/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_bit_img_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    if (NULL == obj) {
        return obj;
    }

    _gt_bit_img_st * style = (_gt_bit_img_st * )obj;

    style->timer = _gt_timer_create(_auto_timer_callback, 500, (void * )obj);
    _gt_timer_set_paused(style->timer, true);

    style->vector0 = _gt_vector_create(_free_item_cb, _equal_item_cb);
    GT_CHECK_BACK_VAL(style->vector0, obj);

    style->vector1 = _gt_vector_create(_free_item_cb, _equal_item_cb);
    GT_CHECK_BACK_VAL(style->vector1, obj);


    return obj;
}

gt_size_t gt_bit_img_add_item0(gt_obj_st * obj, void * item, uint16_t item_byte_size)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return -1;
    }
    _gt_bit_img_st * style = (_gt_bit_img_st * )obj;
    GT_CHECK_BACK_VAL(style->vector0, -1);

    _gt_bit_img_item_st * new_item = gt_mem_malloc(sizeof(_gt_bit_img_item_st));
    GT_CHECK_BACK_VAL(new_item, -1);
    gt_memset(new_item, 0, sizeof(_gt_bit_img_item_st));

#if GT_USE_FILE_HEADER
    gt_file_header_param_init(&new_item->fh_param);
#endif

    new_item->src = gt_mem_malloc(item_byte_size + 1);
    if (NULL == new_item->src) {
        GT_CHECK_PRINT(new_item->src);
        goto item_lb;
    }
    gt_memcpy(new_item->src, item, item_byte_size);
    char * ptr = (char * )new_item->src;
    ptr[item_byte_size] = '\0';

    if (false == _gt_vector_add_item(style->vector0, new_item)) {
        goto src_lb;
    }
    return _gt_vector_get_count(style->vector0);

src_lb:
    gt_mem_free(new_item->src);
    new_item->src = NULL;
item_lb:
    gt_mem_free(new_item);
    new_item = NULL;
    return -1;
}

gt_size_t gt_bit_img_add_item1(gt_obj_st * obj, void * item, uint16_t item_byte_size)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return -1;
    }
    _gt_bit_img_st * style = (_gt_bit_img_st * )obj;
    GT_CHECK_BACK_VAL(style->vector1, -1);

    _gt_bit_img_item_st * new_item = gt_mem_malloc(sizeof(_gt_bit_img_item_st));
    GT_CHECK_BACK_VAL(new_item, -1);
    gt_memset(new_item, 0, sizeof(_gt_bit_img_item_st));

#if GT_USE_FILE_HEADER
    gt_file_header_param_init(&new_item->fh_param);
#endif

    new_item->src = gt_mem_malloc(item_byte_size + 1);
    if (NULL == new_item->src) {
        GT_CHECK_PRINT(new_item->src);
        goto item_lb;
    }
    gt_memcpy(new_item->src, item, item_byte_size);
    char * ptr = (char * )new_item->src;
    ptr[item_byte_size] = '\0';

    if (false == _gt_vector_add_item(style->vector1, new_item)) {
        goto src_lb;
    }
    return _gt_vector_get_count(style->vector1);

src_lb:
    gt_mem_free(new_item->src);
    new_item->src = NULL;
item_lb:
    gt_mem_free(new_item);
    new_item = NULL;
    return -1;
}

#if GT_USE_FILE_HEADER
gt_size_t gt_bit_img_add_item0_by_file_header(gt_obj_st * obj, gt_file_header_param_st * fh)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return -1;
    }
    if (NULL == fh) {
        return -1;
    }
    _gt_bit_img_st * style = (_gt_bit_img_st * )obj;
    if (fh->idx >= gt_file_header_get_count()) {
        return -1;
    }

    _gt_bit_img_item_st * new_item = gt_mem_malloc(sizeof(_gt_bit_img_item_st));
    GT_CHECK_BACK_VAL(new_item, _gt_vector_get_count(style->vector0));
    new_item->src = NULL;
    new_item->fh_param = *fh;
#if GT_USE_DIRECT_ADDR
    gt_hal_direct_addr_init(&new_item->addr);
#endif

    if (false == _gt_vector_add_item(style->vector0, new_item)) {
        gt_mem_free(new_item);
        new_item = NULL;
        return -1;
    }
    return _gt_vector_get_count(style->vector0);
}

gt_size_t gt_bit_img_add_item0_list_by_file_header(gt_obj_st * obj, gt_file_header_param_st const * const fh_array, uint16_t count)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return -1;
    }
    if (NULL == fh_array) {
        return -1;
    }
    _gt_bit_img_st * style = (_gt_bit_img_st * )obj;
    GT_CHECK_BACK_VAL(style->vector0, -1);
    _gt_vector_clear_all_items(style->vector0);

    _gt_bit_img_item_st * new_item = NULL;
    uint16_t instance = sizeof(_gt_bit_img_item_st);

    for (gt_size_t i = 0; i < count; ++i) {
        new_item = (_gt_bit_img_item_st * )gt_mem_malloc(instance);
        GT_CHECK_BACK_VAL(new_item, -1);
        new_item->src = NULL;
        new_item->fh_param = (gt_file_header_param_st)fh_array[i];
#if GT_USE_DIRECT_ADDR
        gt_hal_direct_addr_init(&new_item->addr);
#endif

        if (false == _gt_vector_add_item(style->vector0, new_item)) {
            gt_mem_free(new_item);
            new_item = NULL;
            break;
        }
    }
    return _gt_vector_get_count(style->vector0);
}

gt_size_t gt_bit_img_add_item1_by_file_header(gt_obj_st * obj, gt_file_header_param_st * fh)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return -1;
    }
    if (NULL == fh) {
        return -1;
    }
    _gt_bit_img_st * style = (_gt_bit_img_st * )obj;
    if (fh->idx >= gt_file_header_get_count()) {
        return -1;
    }

    _gt_bit_img_item_st * new_item = gt_mem_malloc(sizeof(_gt_bit_img_item_st));
    GT_CHECK_BACK_VAL(new_item, _gt_vector_get_count(style->vector1));
    new_item->src = NULL;
    new_item->fh_param = *fh;
#if GT_USE_DIRECT_ADDR
    gt_hal_direct_addr_init(&new_item->addr);
#endif

    if (false == _gt_vector_add_item(style->vector1, new_item)) {
        gt_mem_free(new_item);
        new_item = NULL;
        return -1;
    }
    return _gt_vector_get_count(style->vector1);
}

gt_size_t gt_bit_img_add_item1_list_by_file_header(gt_obj_st * obj, gt_file_header_param_st const * const fh_array, uint16_t count)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return -1;
    }
    if (NULL == fh_array) {
        return -1;
    }
    _gt_bit_img_st * style = (_gt_bit_img_st * )obj;
    GT_CHECK_BACK_VAL(style->vector1, -1);
    _gt_vector_clear_all_items(style->vector1);

    _gt_bit_img_item_st * new_item = NULL;
    uint16_t instance = sizeof(_gt_bit_img_item_st);

    for (gt_size_t i = 0; i < count; ++i) {
        new_item = (_gt_bit_img_item_st * )gt_mem_malloc(instance);
        GT_CHECK_BACK_VAL(new_item, -1);
        new_item->src = NULL;
        new_item->fh_param = (gt_file_header_param_st)fh_array[i];
#if GT_USE_DIRECT_ADDR
        gt_hal_direct_addr_init(&new_item->addr);
#endif

        if (false == _gt_vector_add_item(style->vector1, new_item)) {
            gt_mem_free(new_item);
            new_item = NULL;
            break;
        }
    }
    return _gt_vector_get_count(style->vector1);
}
#endif


#if GT_USE_DIRECT_ADDR
gt_size_t gt_bit_img_add_item0_by_direct_addr(gt_obj_st * obj, gt_addr_t addr)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return -1;
    }
    if (gt_hal_is_invalid_addr(addr)) {
        return -1;
    }
    _gt_bit_img_st * style = (_gt_bit_img_st * )obj;

    _gt_bit_img_item_st * new_item = gt_mem_malloc(sizeof(_gt_bit_img_item_st));
    GT_CHECK_BACK_VAL(new_item, _gt_vector_get_count(style->vector0));
    new_item->src = NULL;
    new_item->addr = addr;
#if GT_USE_FILE_HEADER
    gt_file_header_param_init(&new_item->fh_param);
#endif

    if (false == _gt_vector_add_item(style->vector0, new_item)) {
        gt_mem_free(new_item);
        new_item = NULL;
        return -1;
    }
    return _gt_vector_get_count(style->vector0);
}

gt_size_t gt_bit_img_add_item0_list_by_direct_addr(gt_obj_st * obj, gt_addr_t const * const addr_array, uint16_t count)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return -1;
    }
    if (NULL == addr_array) {
        return -1;
    }
    _gt_bit_img_st * style = (_gt_bit_img_st * )obj;
    GT_CHECK_BACK_VAL(style->vector0, -1);
    _gt_vector_clear_all_items(style->vector0);

    _gt_bit_img_item_st * new_item = NULL;
    uint16_t instance = sizeof(_gt_bit_img_item_st);

    for (gt_size_t i = 0; i < count; ++i) {
        if (gt_hal_is_invalid_addr(addr_array[i])) {
            continue;
        }
        new_item = (_gt_bit_img_item_st * )gt_mem_malloc(instance);
        GT_CHECK_BACK_VAL(new_item, -1);
        new_item->src = NULL;
        new_item->addr = addr_array[i];
#if GT_USE_FILE_HEADER
        gt_file_header_param_init(&new_item->fh_param);
#endif

        if (false == _gt_vector_add_item(style->vector0, new_item)) {
            gt_mem_free(new_item);
            new_item = NULL;
            break;
        }
    }
    return _gt_vector_get_count(style->vector0);
}

gt_size_t gt_bit_img_add_item1_by_direct_addr(gt_obj_st * obj, gt_addr_t addr)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return -1;
    }
    if (gt_hal_is_invalid_addr(addr)) {
        return -1;
    }
    _gt_bit_img_st * style = (_gt_bit_img_st * )obj;

    _gt_bit_img_item_st * new_item = gt_mem_malloc(sizeof(_gt_bit_img_item_st));
    GT_CHECK_BACK_VAL(new_item, _gt_vector_get_count(style->vector1));
    new_item->src = NULL;
    new_item->addr = addr;
#if GT_USE_FILE_HEADER
    gt_file_header_param_init(&new_item->fh_param);
#endif

    if (false == _gt_vector_add_item(style->vector1, new_item)) {
        gt_mem_free(new_item);
        new_item = NULL;
        return -1;
    }
    return _gt_vector_get_count(style->vector1);
}

gt_size_t gt_bit_img_add_item1_list_by_direct_addr(gt_obj_st * obj, gt_addr_t const * const addr_array, uint16_t count)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return -1;
    }
    if (NULL == addr_array) {
        return -1;
    }
    _gt_bit_img_st * style = (_gt_bit_img_st * )obj;
    GT_CHECK_BACK_VAL(style->vector1, -1);
    _gt_vector_clear_all_items(style->vector1);

    _gt_bit_img_item_st * new_item = NULL;
    uint16_t instance = sizeof(_gt_bit_img_item_st);

    for (gt_size_t i = 0; i < count; ++i) {
        if (gt_hal_is_invalid_addr(addr_array[i])) {
            continue;
        }
        new_item = (_gt_bit_img_item_st * )gt_mem_malloc(instance);
        GT_CHECK_BACK_VAL(new_item, -1);
        new_item->src = NULL;
        new_item->addr = addr_array[i];
#if GT_USE_FILE_HEADER
        gt_file_header_param_init(&new_item->fh_param);
#endif

        if (false == _gt_vector_add_item(style->vector1, new_item)) {
            gt_mem_free(new_item);
            new_item = NULL;
            break;
        }
    }
    return _gt_vector_get_count(style->vector1);
}
#endif

void gt_bit_img_remove_all_items(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_bit_img_st * style = (_gt_bit_img_st * )obj;

    if(style->vector0){
        _gt_vector_clear_all_items(style->vector0);
    }

    if(style->vector1){
        _gt_vector_clear_all_items(style->vector1);
    }
    _gt_timer_set_paused(style->timer, true);
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}


void gt_bit_img_set_active(gt_obj_st * obj, uint16_t act)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_bit_img_st * style = (_gt_bit_img_st * )obj;
    style->act = act;
    gt_event_send(obj, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

uint16_t gt_bit_img_get_active(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return 0;
    }
    _gt_bit_img_st * style = (_gt_bit_img_st * )obj;
    return style->act;
}

void gt_bit_img_set_value(gt_obj_st * obj, uint16_t value)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_bit_img_st * style = (_gt_bit_img_st * )obj;
    style->value = value;
    _change_timer_handler(obj);

    // if(style->vector0){
    //     _gt_vector_set_index(style->vector0  ,0);
    // }
    // if(style->vector1){
    //     _gt_vector_set_index(style->vector1 ,0);
    // }

    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_bit_img_set_value_bit(gt_obj_st* obj, uint8_t bit, bool value)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_bit_img_st * style = (_gt_bit_img_st * )obj;
    if (value) {
        style->value |= (1 << bit);
    } else {
        style->value &= ~(1 << bit);
    }
    _change_timer_handler(obj);
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

uint16_t gt_bit_img_get_value(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return 0;
    }
    _gt_bit_img_st * style = (_gt_bit_img_st * )obj;
    return style->value;
}

bool gt_bit_img_get_value_bit(gt_obj_st* obj, uint8_t bit)
{
    if( false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return false;
    }
    _gt_bit_img_st * style = (_gt_bit_img_st * )obj;
    return (style->value & (1 << bit)) ? true : false;
}

void gt_bit_img_set_move_gap(gt_obj_st * obj, uint16_t gap)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_bit_img_st * style = (_gt_bit_img_st * )obj;
    style->move_gap = gap;
    gt_event_send(obj, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

void gt_bit_img_set_move_mode(gt_obj_st * obj, gt_bit_img_move_mode_et mode)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_bit_img_st * style = (_gt_bit_img_st * )obj;
    style->move_mode = mode;
    gt_event_send(obj, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

void gt_bit_img_set_disp_mode(gt_obj_st * obj, gt_bit_img_disp_mode_et mode)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_bit_img_st * style = (_gt_bit_img_st * )obj;
    style->disp_mode = mode;
}

void gt_bit_img_set_time(gt_obj_st* obj, uint16_t time)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_bit_img_st * style = (_gt_bit_img_st * )obj;
    if(style->timer){
        _gt_timer_set_period(style->timer, time);
    }
}

/* end of file ----------------------------------------------------------*/
#endif /* GT_CFG_ENABLE_BIT_IMG */

