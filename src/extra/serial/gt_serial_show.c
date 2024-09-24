/**
 * @file gt_serial_show.c
 * @author Yang (your@email.com)
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
#include "../../others/gt_log.h"
#include "gt_fs.h"
#include "gt_mem.h"
#include "gt_disp.h"

/* private define -------------------------------------------------------*/
#ifndef GT_SERIAL_UI_VAR_SIZE
    /* serial ui variable size */
    #define GT_SERIAL_UI_VAR_SIZE           (128 * 1024)
#endif

#ifndef GT_SERIAL_SHOW_FILE_ADDR
    /* show file addrese */
    #define GT_SERIAL_SHOW_FILE_ADDR        (0x50)//(0x380000)
#endif

#ifndef GT_SERIAL_SHOW_FILE_HEAD_SIZE
    /* show file head info size */
    #define GT_SERIAL_SHOW_FILE_HEAD_SIZE   (16)
#endif

#ifndef GT_SERIAL_SHOW_FILE_PAGE_INFO_ADDR
    /* show file page info addr */
    #define GT_SERIAL_SHOW_FILE_PAGE_INFO_ADDR  (GT_SERIAL_SHOW_FILE_ADDR + GT_SERIAL_SHOW_FILE_HEAD_SIZE)
#endif

#ifndef GT_SERIAL_SHOW_FILE_PAGE_INFO_SIZ
    /* show file page info size */
    #define GT_SERIAL_SHOW_FILE_PAGE_INFO_SIZ    (4)
#endif

#ifndef GT_SERIAL_UI_PAGE_STACK_DEPTH
    /* show file page stack depth */
    #define GT_SERIAL_UI_PAGE_STACK_DEPTH   (20)
#endif

#ifndef GT_SERIAL_UI_HOME_PAGE_INDEX
    /* show file home page index */
    #define GT_SERIAL_UI_HOME_PAGE_INDEX   (0)
#endif

#ifndef GT_SERIAL_UI_PAGE_CONTROL_SIZE
    #define GT_SERIAL_UI_PAGE_CONTROL_SIZE      (32)
#endif

#ifndef GT_SERIAL_UI_PAGE_CONTROL_MAX_NUMB
    #define GT_SERIAL_UI_PAGE_CONTROL_MAX_NUMB  (255)
#endif


// define control offset
#define _CONTROL_TYPE       (0x01)
#define _CONTROL_SP         (0x02)
#define _CONTROL_LEN        (0x04)
#define _CONTROL_VP         (0x06)
//

/* private typedef ------------------------------------------------------*/
typedef struct {
    uint8_t file_id;
    uint8_t sys_id[6];
    uint8_t page_mode;
    uint16_t page_numb;
    uint8_t reserve[6];
}gt_serial_show_file_head;

typedef struct {
    uint8_t numb;
    uint32_t addr;
    uint8_t control[GT_SERIAL_UI_PAGE_CONTROL_SIZE * GT_SERIAL_UI_PAGE_CONTROL_MAX_NUMB];
}gt_serial_page_info;

typedef struct
{
    gt_scr_list_st* page_list;
    gt_serial_show_file_head _show_file_head;
    gt_serial_page_info page_info;
    //
    volatile uint16_t page_index;
    bool lock;
}gt_serial_ui;

/* static prototypes ----------------------------------------------------*/


/* static variables -----------------------------------------------------*/
/**
 * @brief serial ui variable
 */
static GT_ATTRIBUTE_LARGE_RAM_ARRAY uint8_t _serial_ui_var[GT_SERIAL_UI_VAR_SIZE] = {0};

/**
 * @brief serial ui
 */
static gt_serial_ui _serial_ui = {
    .page_list = NULL,
    .lock = false,
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static gt_res_t _parse_show_header_file_id(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_show_file_head* head = (gt_serial_show_file_head*)user_data;
    head->file_id = buffer[0];
    return GT_RES_OK;
}

static gt_res_t _parse_show_header_sys_id(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_show_file_head* head = (gt_serial_show_file_head*)user_data;
    gt_memcpy(head->sys_id, buffer, len);
    return GT_RES_OK;
}

static gt_res_t _parse_show_header_page_mode(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_show_file_head* head = (gt_serial_show_file_head*)user_data;
    head->page_mode = buffer[0];
    return GT_RES_OK;
}

static gt_res_t _parse_show_header_page_numb(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_show_file_head* head = (gt_serial_show_file_head*)user_data;
    head->page_numb = (buffer[0] << 8) | buffer[1];
    return GT_RES_OK;
}

static gt_res_t _parse_show_header_reserve(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_show_file_head* head = (gt_serial_show_file_head*)user_data;
    gt_memcpy(head->reserve, buffer, len);
    return GT_RES_OK;
}

static gt_res_t _print_show_header_file_id(void const * const user_data, uint8_t * const buffer, uint32_t len) {
    gt_serial_show_file_head* head = (gt_serial_show_file_head*)user_data;
    buffer[0] = head->file_id;
    return GT_RES_OK;
}

static gt_res_t _print_show_header_sys_id(void const * const user_data, uint8_t * const buffer, uint32_t len) {
    gt_serial_show_file_head* head = (gt_serial_show_file_head*)user_data;
    gt_memcpy(buffer, head->sys_id, len);
    return GT_RES_OK;
}
static gt_res_t _print_show_header_page_mode(void const * const user_data, uint8_t * const buffer, uint32_t len) {
    gt_serial_show_file_head* head = (gt_serial_show_file_head*)user_data;
    buffer[0] = head->page_mode;
    return GT_RES_OK;
}
static gt_res_t _print_show_header_page_numb(void const * const user_data, uint8_t * const buffer, uint32_t len) {
    gt_serial_show_file_head* head = (gt_serial_show_file_head*)user_data;
    buffer[0] = (head->page_numb >> 8) & 0xFF;
    buffer[1] = head->page_numb & 0xFF;
    return GT_RES_OK;
}
static gt_res_t _print_show_header_reserve(void const * const user_data, uint8_t * const buffer, uint32_t len) {
    gt_serial_show_file_head* head = (gt_serial_show_file_head*)user_data;
    gt_memcpy(buffer, head->reserve, len);
    return GT_RES_OK;
}

static GT_ATTRIBUTE_LARGE_RAM_ARRAY const gt_bin_item_st _format_show_hand[] = {
    { 1, _parse_show_header_file_id,   _print_show_header_file_id },
    { 6, _parse_show_header_sys_id,    _print_show_header_sys_id },
    { 1, _parse_show_header_page_mode, _print_show_header_page_mode },
    { 2, _parse_show_header_page_numb, _print_show_header_page_numb },
    { 6, _parse_show_header_reserve,   _print_show_header_reserve },

    /** ... */
    { 0x0, (parse_cb_t)NULL, (print_cb_t)NULL },
};

static gt_res_t _parse_page_info_numb(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_page_info* info = (gt_serial_page_info*)user_data;
    info->numb = buffer[0];
    return GT_RES_OK;
}

static gt_res_t _parse_page_info_addr(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_page_info* info = (gt_serial_page_info*)user_data;
    info->addr = (buffer[0] << 16) | (buffer[1] << 8) | buffer[2];
    return GT_RES_OK;
}

static gt_res_t _print_page_info_numb(void const * const user_data, uint8_t * const buffer, uint32_t len) {
    gt_serial_page_info* info = (gt_serial_page_info*)user_data;
    buffer[0] = info->numb;
    return GT_RES_OK;
}

static gt_res_t _print_page_info_addr(void const * const user_data, uint8_t * const buffer, uint32_t len) {
    gt_serial_page_info* info = (gt_serial_page_info*)user_data;
    buffer[0] = (info->addr >> 16) & 0xFF;
    buffer[1] = (info->addr >> 8) & 0xFF;
    buffer[2] = info->addr & 0xFF;
    return GT_RES_OK;
}

static GT_ATTRIBUTE_LARGE_RAM_ARRAY const gt_bin_item_st _format_page_info[] = {
    { 1, _parse_page_info_numb,   _print_page_info_numb },
    { 3, _parse_page_info_addr,   _print_page_info_addr },

    /** ... */
    { 0x0, (parse_cb_t)NULL, (print_cb_t)NULL },
};

static void _read_show_file_head(gt_serial_show_file_head* head)
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

static void _read_show_file_page_control(gt_serial_page_info* info)
{
    gt_memset_0(info->control, GT_SERIAL_UI_PAGE_CONTROL_SIZE * GT_SERIAL_UI_PAGE_CONTROL_MAX_NUMB);
    gt_fs_read_direct_physical(info->addr, info->numb * GT_SERIAL_UI_PAGE_CONTROL_SIZE, info->control);
}

static void _read_show_file_page_info(gt_serial_page_info* info, uint16_t index)
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
    // read control
    _read_show_file_page_control(info);
}


static int _set_contorl_param(uint8_t* control, uint8_t reg, uint8_t* param, uint16_t len)
{
    return 0;
}

static int _set_contorl_value(uint8_t* control, uint8_t* value, uint16_t len)
{
    return _set_contorl_param(control, 0, value, len);
}


static void _create_control(uint8_t* control, uint32_t len, gt_obj_st* parent)
{

}

static uint8_t* _get_control(gt_serial_page_info* info, uint16_t index)
{
    return ((index >= info->numb) ? NULL : info->control + (index * GT_SERIAL_UI_PAGE_CONTROL_SIZE));
}

static void _addr_control_to_page(gt_serial_page_info* info, gt_obj_st* parent)
{
    // create control
    for(uint8_t i = 0; i < info->numb; i++){
        _create_control(_get_control(info, i), GT_SERIAL_UI_PAGE_CONTROL_SIZE, parent);
    }
}

static int16_t _find_controls_in_vp_or_sp(gt_serial_page_info* info, uint16_t reg, bool is_vp)
{
    uint8_t *p;
    int16_t index = -1;
    // vp
    if(is_vp){
        uint16_t vp = 0x0000;
        for(index = 0; index < info->numb; index++){
            p = _get_control(info, index);
            vp = p[_CONTROL_VP] << 8 | p[_CONTROL_VP + 1];
            if(vp == reg){
                return index;
            }
        }
        return -1;
    }

    // sp
    uint16_t sp = 0x0000, len = 0;
    for(index = 0; index < info->numb; index++){
        p = _get_control(info, index);
        sp = p[_CONTROL_SP] << 8 | p[_CONTROL_SP + 1];
        len = p[_CONTROL_LEN] << 8 | p[_CONTROL_LEN + 1];
        if(reg >= sp && reg < (sp + len)){
            return index;
        }
    }
    return -1;
}

static gt_obj_st* _gt_serial_init_screen(void)
{
    gt_obj_st* screen = gt_obj_create(NULL);
    // read page info
    _read_show_file_page_info(&_serial_ui.page_info, _serial_ui.page_index);

    _addr_control_to_page(&_serial_ui.page_info, screen);

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
    for(uint16_t i = 0; i < _serial_ui._show_file_head.page_numb; i++){
        _serial_ui.page_list[i].scr_id = i;
        _serial_ui.page_list[i].init_cb = _gt_serial_init_screen;
    }

    gt_scr_stack_register_id_list(_serial_ui.page_list, _serial_ui._show_file_head.page_numb, GT_SERIAL_UI_PAGE_STACK_DEPTH);
    gt_scr_stack_set_home_scr_id(GT_SERIAL_UI_HOME_PAGE_INDEX, false);
    gt_serial_load_page(GT_SERIAL_UI_HOME_PAGE_INDEX);

    GT_LOGD(GT_LOG_TAG_SERIAL , "serial ui init success, _serial_ui size: %d byte, page list size: %d byte, page count: %d",\
        sizeof(gt_serial_ui), sizeof(gt_scr_list_st) * _serial_ui._show_file_head.page_numb, _serial_ui._show_file_head.page_numb);
}
/* global functions / API interface -------------------------------------*/
void gt_serial_show_init(void)
{
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
    GT_LOGI(GT_LOG_TAG_SERIAL , "load page = %d", index);
    _serial_ui.page_index = index;
    gt_disp_stack_load_scr(_serial_ui.page_index);
}

int16_t gt_serial_go_back(int16_t step)
{
    if(!_serial_ui.lock) {return 0;}
    _serial_ui.page_index = gt_scr_stack_get_prev_id_by(step);
    GT_LOGI(GT_LOG_TAG_SERIAL , "go back to page = %d, step = %d", _serial_ui.page_index, step);
    return gt_disp_stack_go_back(step);
}

int gt_serial_control_value_set(uint16_t vp, uint8_t* value, uint16_t len)
{
    if(!_serial_ui.lock) {return -1;}

    int16_t index = _find_controls_in_vp_or_sp(&_serial_ui.page_info, vp, true);
    if(index < 0) {return -1;}

    return _set_contorl_value(_get_control(&_serial_ui.page_info, index), value, len);
}

int gt_serial_control_param_set(uint16_t sp, uint8_t* param, uint16_t len)
{
    if(!_serial_ui.lock) {return -1;}

    if(sp >= 0xFFFF) {return -1;}

    int16_t index = _find_controls_in_vp_or_sp(&_serial_ui.page_info, sp, false);
    if(index < 0) {return -1;}

    uint8_t* p = _get_control(&_serial_ui.page_info, index);
    uint8_t reg = sp - (p[_CONTROL_SP] << 8 | p[_CONTROL_SP + 1]);
    return _set_contorl_param(p, reg, param, len);
}

int gt_serial_control_set(uint16_t reg, uint8_t* param, uint16_t len)
{
    if(!_serial_ui.lock) {return -1;}

    int ret = gt_serial_control_value_set(reg, param, len);
    if(ret > 0) {return ret;}

    return gt_serial_control_param_set(reg, param, len);
}

int gt_serial_val_get(uint16_t reg, uint8_t* data, uint16_t len)
{
    if(reg >= 0xFFFF) {return -1;}
    memcpy(data, &_serial_ui_var[reg * 2], len);
    return 0;
}

int gt_serial_val_set(uint16_t reg, uint8_t* data, uint16_t len)
{
    if(reg >= 0xFFFF) {return -1;}
    memcpy(&_serial_ui_var[reg * 2], data, len);
    return 0;
}


/* end of file ----------------------------------------------------------*/
#endif


