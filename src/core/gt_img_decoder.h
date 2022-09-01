/**
 * @file gt_img_decoder.h
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2022-08-10 19:52:48
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_IMG_DECODER_H_
#define _GT_IMG_DECODER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../gt_conf.h"
#include "../others/gt_list.h"
#include "../others/gt_types.h"
#include "../hal/gt_hal_fs.h"
#include "../others/gt_color.h"


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/

typedef enum gt_img_color_format_e {
    GT_IMG_CF_UNKNOWN = 0,

    GT_IMG_CF_TRUE_COLOR,       // RGB888
    GT_IMG_CF_TRUE_COLOR_ALPHA, // ARGB8888 / RGBA 8888
}gt_img_color_format_et;

/**
 * @brief image header info
 */
typedef struct _gt_img_info_s {
    uint32_t color_format : 5;  /* @ref gt_img_color_format_et */
    uint32_t reserved : 3;
    uint32_t w : 12;
    uint32_t h : 12;
}_gt_img_info_st;

/**
 * @brief describe image info
 */
typedef struct _gt_img_dsc_s {

    struct _gt_img_decoder_s * decoder;

    void * src;

    _gt_img_info_st header;

    gt_size_t frame_id;

    gt_fs_type_et type;

    uint8_t * img;

    void * user_data;

    char * msg;

}_gt_img_dsc_st;

struct _gt_img_decoder_s;

typedef gt_res_t ( * gt_img_decoder_get_info_t)(struct _gt_img_decoder_s * decoder, const void * src, _gt_img_info_st * header);
typedef gt_res_t ( * gt_img_decoder_open_t)(struct _gt_img_decoder_s * decoder, struct _gt_img_dsc_s * dsc);
typedef gt_res_t ( * gt_img_decoder_read_line_t)(struct _gt_img_decoder_s * decoder, struct _gt_img_dsc_s * dsc,
                                                gt_size_t x, gt_size_t y, gt_size_t len, uint8_t * buffer);
typedef gt_res_t ( * gt_img_decoder_close_t)(struct _gt_img_decoder_s * decoder, struct _gt_img_dsc_s * dsc);

/**
 * @brief callback function
 */
typedef struct _gt_img_decoder_s {
    gt_img_decoder_get_info_t info_cb;
    gt_img_decoder_open_t open_cb;
    gt_img_decoder_read_line_t read_line_cb;
    gt_img_decoder_close_t close_cb;

    struct gt_list_head list;
}_gt_img_decoder_st;


/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

void _gt_img_decoder_init(void);

_gt_img_decoder_st * gt_img_decoder_create(void);
void gt_img_decoder_register(_gt_img_decoder_st * decoder);

gt_res_t gt_img_decoder_get_info(const char * name, _gt_img_info_st * header);
gt_res_t gt_img_decoder_open(_gt_img_dsc_st * dsc, const char * const name, gt_size_t frame_id);
gt_res_t gt_img_decoder_read_line(_gt_img_dsc_st * dsc, gt_size_t x, gt_size_t y, gt_size_t len, uint8_t * buf);
gt_res_t gt_img_decoder_close(_gt_img_dsc_st * dsc);

void gt_img_decoder_set_info_cb(_gt_img_decoder_st * decoder, gt_img_decoder_get_info_t info_cb);
void gt_img_decoder_set_open_cb(_gt_img_decoder_st * decoder, gt_img_decoder_open_t open_cb);
void gt_img_decoder_set_read_line_cb(_gt_img_decoder_st * decoder, gt_img_decoder_read_line_t read_line_cb);
void gt_img_decoder_set_close_cb(_gt_img_decoder_st * decoder, gt_img_decoder_close_t close_cb);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_IMG_DECODER_H_
