/**
 * @file gt_gc.h
 * @author Feyoung
 * @brief The global call
 * @version 0.1
 * @date 2022-06-14 16:56:57
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_GC_H_
#define _GT_GC_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_ll.h"
#include "../core/gt_timer.h"
#include "../others/gt_list.h"
#include "../others/gt_color.h"
#include "../hal/gt_hal_disp.h"
#include "../hal/gt_hal_file_header.h"
#include "../core/gt_disp.h"

/* define ---------------------------------------------------------------*/

#if GT_USE_EXTRA_FULL_IMG_BUFFER

#ifndef GT_USE_EXTRA_FULL_IMG_OFFSET
    /**
     * @brief Using the full image buffer, set the offset of the buffer is valid.
     */
    #define GT_USE_EXTRA_FULL_IMG_OFFSET    0
#endif

/**
 * @brief Malloc or free by user.
 */
typedef struct _gt_gc_full_img_buffer_s {
    uint8_t * buffer;
    uint32_t len;
#if GT_USE_EXTRA_FULL_IMG_OFFSET
    uint32_t offset;
#endif
}_gt_gc_full_img_buffer_st;
#endif

typedef struct gt_gc_s {
    struct _gt_list_head _gt_anim_ll;
    struct _gt_list_head _gt_timer_ll;
    struct _gt_list_head _gt_img_decoder_ll;
    struct _gt_event_s * _gt_event_node_header_ll;

    gt_disp_st * disp;
    _gt_gc_scr_st scr_info;

#if GT_USE_EXTRA_FULL_IMG_BUFFER
    _gt_gc_full_img_buffer_st full_img_buffer;
#endif

#if GT_USE_FILE_HEADER
    _gt_file_header_ctl_st file_header_ctl;
#endif
}gt_gc_st;

extern gt_gc_st _gt_global_call;

#define _GT_GC_GET_ROOT(_name)  ((&_gt_global_call)->_name)

/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/

static inline int _gt_gc_is_ll_empty(const struct _gt_list_head * head) {
    return _gt_list_empty(head);
}


/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

gt_disp_st * _gt_gc_get_disp(void);
void _gt_gc_set_disp(gt_disp_st * disp);

_gt_gc_scr_st * _gt_gc_get_scr_info(void);

uint16_t _gt_gc_get_size(void);

#if GT_USE_EXTRA_FULL_IMG_BUFFER
/**
 * @brief Set external full image buffer, this buffer will be used by img reading
 *      from flash or SD nand flash, etc. Set with the maximum width and height of
 *      the material.
 *
 * @param buffer color 8 / 565 / 888 buffer, byte
 * @param len buffer length, must be equal or more than
 *          [max_width * max_height * (sizeof(gt_opa_t) + sizeof(gt_color_t))].
 */
void gt_gc_set_full_img_buffer(uint8_t * buffer, uint32_t len);

#if GT_USE_EXTRA_FULL_IMG_OFFSET
/**
 * @brief Using the full image buffer, set the offset of the buffer is valid.
 *
 * @param offset 0[defalut]: Buffer begin address
 */
void gt_gc_set_full_img_buffer_offset(uint32_t offset);
#endif

_gt_gc_full_img_buffer_st * _gt_gc_get_full_img_buffer(void);
#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_GC_H_
