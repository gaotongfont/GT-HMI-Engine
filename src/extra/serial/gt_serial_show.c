/**
 * @file gt_serial_show.c
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2024-09-20 17:22:14
 * @copyright Copyright (c) 2014-2024, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_serial_show.h"
//
#if GT_USE_SERIAL_SHOW && GT_USE_BIN_CONVERT
#include "../../utils/gt_bin_convert.h"
#include "./gt_serial_system.h"
#include "../../others/gt_log.h"
#include "../../../gt.h"
//
#include "gt_serial_var.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/
typedef struct {
    uint8_t file_id;
    uint8_t sys_id[6];
    uint8_t page_mode;
    uint16_t page_numb;
    uint8_t reserve[6];
}gt_serial_show_file_head_st;

typedef struct {
    uint8_t numb;
    uint32_t addr;
    uint8_t control[GT_SERIAL_UI_PAGE_CONTROL_SIZE * GT_SERIAL_UI_PAGE_CONTROL_MAX_NUMB];
}gt_serial_page_info_st;

typedef struct
{
    gt_scr_list_st* page_list;
    gt_serial_show_file_head_st _show_file_head;
    gt_serial_page_info_st page_info;
    //
    volatile uint16_t page_index;
    bool lock;
}gt_serial_ui_st;

/* static prototypes ----------------------------------------------------*/


/* static variables -----------------------------------------------------*/

/**
 * @brief serial ui
 */
static gt_serial_ui_st _serial_ui = {
    .page_list = NULL,
    .lock = false,
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static gt_res_t _parse_show_head(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_show_file_head_st* head = (gt_serial_show_file_head_st*)user_data;
    head->file_id = buffer[0];
    gt_memcpy(head->sys_id, &buffer[1], 6);
    head->page_mode = buffer[7];
    head->page_numb = (buffer[8] << 8) | buffer[9];
    gt_memcpy(head->reserve, &buffer[10], 6);
    return GT_RES_OK;
}

static gt_res_t _print_show_head(void const * const user_data, uint8_t * const buffer, uint32_t len) {

    gt_serial_show_file_head_st* head = (gt_serial_show_file_head_st*)user_data;
    buffer[0] = head->file_id;
    gt_memcpy(buffer + 1, head->sys_id, 6);
    buffer[7] = head->page_mode;
    buffer[8] = (head->page_numb >> 8) & 0xFF;
    buffer[9] = head->page_numb & 0xFF;
    gt_memcpy(buffer + 10, head->reserve, 6);
    return GT_RES_OK;
}

static GT_ATTRIBUTE_LARGE_RAM_ARRAY const gt_bin_item_st _format_show_hand[] = {
    { 16, _parse_show_head,   _print_show_head },
    /** ... */
    { 0x0, (parse_cb_t)NULL, (print_cb_t)NULL },
};

static gt_res_t _parse_page_info(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_page_info_st* info = (gt_serial_page_info_st*)user_data;
    info->numb = buffer[0];
    info->addr = ((buffer[1] << 16) | (buffer[2] << 8) | buffer[3]) + GT_SERIAL_SHOW_FILE_ADDR;
    return GT_RES_OK;
}

static gt_res_t _print_page_info(void const * const user_data, uint8_t * const buffer, uint32_t len) {
    gt_serial_page_info_st* info = (gt_serial_page_info_st*)user_data;
    buffer[0] = info->numb;
    buffer[1] = (info->addr >> 16) & 0xFF;
    buffer[2] = (info->addr >> 8) & 0xFF;
    buffer[3] = info->addr & 0xFF;
}

static GT_ATTRIBUTE_LARGE_RAM_ARRAY const gt_bin_item_st _format_page_info[] = {
    { 4, _parse_page_info,   _print_page_info },
    /** ... */
    { 0x0, (parse_cb_t)NULL, (print_cb_t)NULL },
};

static void _read_show_file_head(gt_serial_show_file_head_st* head)
{
    uint8_t tmp[GT_SERIAL_SHOW_FILE_HEAD_SIZE] = {0};
    gt_fs_read_direct_physical(GT_SERIAL_SHOW_FILE_ADDR, GT_SERIAL_SHOW_FILE_HEAD_SIZE, tmp);

    gt_bin_convert_st bin = {
        .tar = head,
        .buffer = tmp,
        .byte_len = GT_SERIAL_SHOW_FILE_HEAD_SIZE,
    };
    gt_bin_res_st ret = {0};
    ret = gt_bin_convert_parse(&bin, _format_show_hand);
    if (GT_RES_OK != ret.res) {
        GT_LOG_A("", "Parse err code: %d, index: %d", ret.res, ret.index);
    }

    GT_LOGI(GT_LOG_TAG_SERIAL , "file_id = 0x%X, sys_id = [%s], page_mode = 0x%X, page_numb = 0x%X",\
                                head->file_id ,head->sys_id, head->page_mode,head->page_numb);
}

static void _read_show_file_page_control(gt_serial_page_info_st* info)
{
    gt_memset_0(info->control, GT_SERIAL_UI_PAGE_CONTROL_SIZE * GT_SERIAL_UI_PAGE_CONTROL_MAX_NUMB);
    gt_fs_read_direct_physical(info->addr, info->numb * GT_SERIAL_UI_PAGE_CONTROL_SIZE, info->control);
}

static void _read_show_file_page_info(gt_serial_page_info_st* info, uint16_t index)
{
    uint8_t tmp[GT_SERIAL_SHOW_FILE_PAGE_INFO_SIZ];
    gt_fs_read_direct_physical( GT_SERIAL_SHOW_FILE_PAGE_INFO_ADDR + (index * GT_SERIAL_SHOW_FILE_PAGE_INFO_SIZ),\
                                GT_SERIAL_SHOW_FILE_PAGE_INFO_SIZ, tmp);
    gt_bin_convert_st bin = {
        .tar = info,
        .buffer = tmp,
        .byte_len = GT_SERIAL_SHOW_FILE_PAGE_INFO_SIZ,
    };
    gt_bin_res_st ret = {0};
    ret = gt_bin_convert_parse(&bin, _format_page_info);
    if (GT_RES_OK != ret.res) {
        GT_LOG_A("", "Parse err code: %d, index: %d", ret.res, ret.index);
    }
    GT_LOGI(GT_LOG_TAG_SERIAL , "page index = %d, numb = 0x%X, addr = 0x%X, ",index, info->numb, info->addr);
    // read control
    _read_show_file_page_control(info);
}

static uint8_t* _get_control(gt_serial_page_info_st* info, uint16_t index)
{
    return ((index >= info->numb) ? NULL : info->control + (index * GT_SERIAL_UI_PAGE_CONTROL_SIZE));
}

static void _read_sp_param_to_var_buffer(gt_serial_show_file_head_st* head, gt_serial_page_info_st* info)
{
    uint16_t i = 0, n = 0;
    const gt_bin_item_st _format[] = {
        {8, gt_serial_var_head_parse, (print_cb_t)NULL,},
        { 0x0, (parse_cb_t)NULL, (print_cb_t)NULL },
    };
    gt_var_head_st var_head;
    gt_bin_convert_st bin = {
        .tar = &var_head,
        .byte_len = GT_SERIAL_UI_PAGE_CONTROL_SIZE,
    };
    gt_bin_res_st ret = {0};

    for(i = 0; i < head->page_numb; i++){
        _read_show_file_page_info(info, i);

        for(n = 0; n < info->numb; n++){
            bin.buffer = _get_control(info, n);
            ret = gt_bin_convert_parse(&bin, _format);
            if (GT_RES_OK != ret.res) {
                continue;
            }
            if(var_head.sp != 0xFFFF){
                gt_serial_var_buffer_set_reg(var_head.sp, bin.buffer + 6, var_head.len_dsc*2);
            }
        }
    }
}

static int _set_contorl_param(uint8_t* control, uint8_t reg, uint8_t* param, uint16_t len)
{
    return 0;
}

static int _set_contorl_value(uint8_t* control, uint8_t* value, uint16_t len)
{
    return _set_contorl_param(control, 0, value, len);
}


static gt_res_t _create_control(gt_var_st* var)
{
#if GT_USE_SERIAL_VAR
    return gt_serial_var_create(var);
#else
    return GT_RES_OK;
#endif
}


static gt_res_t _addr_control_to_page(gt_serial_page_info_st* info, gt_obj_st* parent)
{
    // create control
    gt_var_st var;
    for(uint8_t i = 0; i < info->numb; i++){
        var.buffer = _get_control(info, i);
        var.len = GT_SERIAL_UI_PAGE_CONTROL_SIZE;
        var.obj = parent;
        var.id = i;
        if (GT_RES_FAIL == _create_control(&var)) {
            GT_LOGE(GT_LOG_TAG_SERIAL , "create control failed!, index: %d", i);
            return GT_RES_FAIL;
        }
    }
    return GT_RES_OK;
}
#define TEST_UI 1
#if TEST_UI
#define TEST_IS_VP   (0)
gt_obj_st* btn1 = NULL;
gt_obj_st* btn2 = NULL;
static uint16_t index_test = 0;
static uint16_t vp_test = 0x1000;
static uint16_t sp_test = 0x2001;
static uint8_t vp_buf[4] = {0};
static uint8_t sp_buf[4] = {0};
static void _btn1_cb(gt_event_st* event)
{
#if TEST_IS_VP
    ++index_test;
    gt_convert_print_u16(vp_buf, vp_test);
    gt_convert_print_u16(vp_buf + 2, index_test);

    GT_LOGD(GT_LOG_TAG_SERIAL , ">> send vp = 0x%X param = %d",vp_test, index_test);
    gt_serial_set_value(vp_buf, sizeof(vp_buf));
#else
    index_test += 10;
    gt_convert_print_u16(sp_buf, sp_test);
    gt_convert_print_u16(sp_buf + 2, index_test);

    GT_LOGD(GT_LOG_TAG_SERIAL , ">> send sp = 0x%X param = %d",sp_test, index_test);
    gt_serial_set_value(sp_buf, sizeof(sp_buf));
#endif

}
static void _btn2_cb(gt_event_st* event)
{
#if TEST_IS_VP
    --index_test;
    gt_convert_print_u16(vp_buf, vp_test);
    gt_convert_print_u16(vp_buf + 2, index_test);

    GT_LOGD(GT_LOG_TAG_SERIAL , ">> send vp = 0x%X param = %d",vp_test, index_test);
    gt_serial_set_value(vp_buf, sizeof(vp_buf));
#else
    index_test -= 10;
    gt_convert_print_u16(sp_buf, sp_test);
    gt_convert_print_u16(sp_buf + 2, index_test);

    GT_LOGD(GT_LOG_TAG_SERIAL , ">> send sp = 0x%X param = %d",sp_test, index_test);
    gt_serial_set_value(sp_buf, sizeof(sp_buf));
#endif
}
#endif
static gt_obj_st* _gt_serial_init_screen(void)
{
    gt_obj_st* screen = gt_obj_create(NULL);
    // read page info
    _read_show_file_page_info(&_serial_ui.page_info, _serial_ui.page_index);

    if(GT_RES_OK != _addr_control_to_page(&_serial_ui.page_info, screen)){
        gt_obj_destroy(screen);
        GT_LOGE(GT_LOG_TAG_SERIAL , "create control failed!");
        return NULL;
    }

#if TEST_UI
    btn1 = gt_btn_create(screen);
    gt_obj_set_pos(btn1, 10, 10);
    gt_obj_set_size(btn1, 100, 60);
    gt_btn_set_text(btn1, "test++");
    gt_obj_add_event_cb(btn1, _btn1_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    btn2 = gt_btn_create(screen);
    gt_obj_set_pos(btn2, 120, 10);
    gt_obj_set_size(btn2, 100, 60);
    gt_btn_set_text(btn2, "test--");
    gt_obj_add_event_cb(btn2, _btn2_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);
#endif

    return screen;
}


static void _gt_serial_ui_init(void)
{
    if(_serial_ui.lock){
        GT_LOGW(GT_LOG_TAG_SERIAL , "serial ui is already init");
        return;
    }
    // read file head
    _read_show_file_head(&_serial_ui._show_file_head);
    // create page list
    _serial_ui.page_list = (gt_scr_list_st*)gt_mem_malloc(sizeof(gt_scr_list_st) * _serial_ui._show_file_head.page_numb);
    if(!_serial_ui.page_list){
        GT_LOGE(GT_LOG_TAG_SERIAL , "malloc page list failed");
        return;
    }
    _serial_ui.lock = true;

#if GT_USE_SERIAL_VAR
    gt_serial_var_init();
    _read_sp_param_to_var_buffer(&_serial_ui._show_file_head, &_serial_ui.page_info);
#endif

    for(uint16_t i = 0; i < _serial_ui._show_file_head.page_numb; i++){
        _serial_ui.page_list[i].scr_id = i;
        _serial_ui.page_list[i].init_cb = _gt_serial_init_screen;
    }

    gt_scr_stack_register_id_list(_serial_ui.page_list, _serial_ui._show_file_head.page_numb, GT_SERIAL_UI_PAGE_STACK_DEPTH);
    gt_scr_stack_set_home_scr_id(GT_SERIAL_UI_HOME_PAGE_INDEX, false);
    gt_serial_load_page(GT_SERIAL_UI_HOME_PAGE_INDEX);

    GT_LOGD(GT_LOG_TAG_SERIAL , "serial ui init success, _serial_ui size: %d byte, page list size: %d byte, page count: %d",\
        sizeof(gt_serial_ui_st), sizeof(gt_scr_list_st) * _serial_ui._show_file_head.page_numb, _serial_ui._show_file_head.page_numb);
}
/* global functions / API interface -------------------------------------*/
void gt_serial_show_init(void)
{
    GT_LOGI(GT_LOG_TAG_SERIAL , "serial show init!");
    _gt_serial_ui_init();
}

uint16_t gt_serial_page_count_get(void)
{
    if(!_serial_ui.lock) {return 0;}
    GT_LOGI(GT_LOG_TAG_SERIAL , "page count = %d", _serial_ui._show_file_head.page_numb);
    return _serial_ui._show_file_head.page_numb;
}

uint16_t gt_serial_page_index_get(void)
{
    if(!_serial_ui.lock) {return 0;}
    GT_LOGI(GT_LOG_TAG_SERIAL , "page id = %d", _serial_ui.page_index);
    return _serial_ui.page_index;
}

void gt_serial_load_page(uint16_t index)
{
    if(!_serial_ui.lock) {return ;}
    _serial_ui.page_index = index;
    GT_LOGI(GT_LOG_TAG_SERIAL , "load page = %d", _serial_ui.page_index);
    gt_disp_stack_load_scr(_serial_ui.page_index);
}

int16_t gt_serial_go_back(int16_t step)
{
    if(!_serial_ui.lock) {return 0;}
    _serial_ui.page_index = gt_scr_stack_get_prev_id_by(step);
    GT_LOGI(GT_LOG_TAG_SERIAL , "go back to page = %d, step = %d", _serial_ui.page_index, step);
    return gt_disp_stack_go_back(step);
}

void gt_serial_set_value(uint8_t *value, uint16_t len)
{
    if(!_serial_ui.lock) {return ;}
    int ret = -1;
    uint16_t index = 0;
    gt_var_st var;
    gt_var_value_st var_value;
    var_value.buffer = value;
    var_value.len = len;

    if(0 != gt_serial_var_buffer_set(&var_value) ) {return ;}

    if(0 == gt_serial_var_check_value_is_sys_config(&var_value)) {
        /* Operation sys config */
        gt_serial_system_set_value(value, len);
        return ;
    }

    for(index = 0; index < _serial_ui.page_info.numb;index++){
        var.buffer = _get_control(&_serial_ui.page_info, index);
        var.len = GT_SERIAL_UI_PAGE_CONTROL_SIZE;
        var.id = index;
#if GT_USE_SERIAL_VAR
        ret = gt_serial_var_check_value_is_set_this(&var, &var_value);
#endif
        if(ret < 0) {continue;}

        var.id = index;
        var.obj = gt_obj_find_by_id(var.id);
        if(var.obj == NULL) {continue;}
#if GT_USE_SERIAL_VAR
        gt_serial_var_set_value(&var, &var_value);
#endif
    }
}


/* end of file ----------------------------------------------------------*/
#endif


