/**
 * @file gt_img_decoder.h
 * @author Feyoung
 * @brief The image decoder which need to register the image file type by
 *      extra file type.
 * @version 0.1
 * @date 2022-08-10 19:52:48
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
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
/**
 * @brief Support extra image decoder type
 */
typedef enum {
    GT_IMG_DECODER_TYPE_NONE = 0,   ///< Not supported yet
    GT_IMG_DECODER_TYPE_PNG,        ///< supports png type image
    GT_IMG_DECODER_TYPE_SJPG,       ///< supports jpg / sjpg / jpeg type image
    GT_IMG_DECODER_TYPE_RAM,        ///< supports ram type image

    GT_IMG_DECODER_TYPE_COUNT,      ///< totally supports type count
}gt_img_decoder_type_em;

/**
 * @brief Image color format type
 */
typedef enum gt_img_color_format_e {
    GT_IMG_CF_UNKNOWN = 0,      ///< Unknown color format

    GT_IMG_CF_TRUE_COLOR,       ///< RGB888
    GT_IMG_CF_TRUE_COLOR_ALPHA, ///< ARGB8888 / RGBA 8888
}gt_img_color_format_et;

/**
 * @brief image header info
 */
typedef struct _gt_img_info_s {
    uint32_t color_format : 5;  // TODO adjust import @ref gt_img_color_format_et
    uint32_t reserved : 3;
    uint32_t w : 12;
    uint32_t h : 12;
    uint8_t type;               ///< record this decoder image type @ref gt_img_decoder_type_em
}_gt_img_info_st;

/**
 * @brief describe image info
 */
typedef struct _gt_img_dsc_s {
    struct _gt_img_decoder_s * decoder; ///< iamge decoder
    gt_fs_fp_st * fp;           ///< file object
    void * src;                 ///< path or destination of image
#if GT_USE_FILE_HEADER
    gt_file_header_param_st * file_header;  ///< Using file header mode to read image data
#endif
    uint8_t * img;              ///< temp buffer which is used to save image data
    gt_opa_t * alpha;           ///< temp buffer which is used to save alpha data
    void * customs_data;        ///< customs data
    gt_color_t fill_color;      ///< fill color
    _gt_img_info_st header;     ///< the header information of image
    gt_fs_type_et type;         ///< file driver type, such as: SD, Flash...
}_gt_img_dsc_st;

/**
 * @brief function pointer to decode image information, @ref _gt_img_info_st
 * @param decoder image decoder
 * @param src image file path
 * @param header Get the result which is image header information, from src
 * @return gt_res_t The return status
 */
typedef gt_res_t ( * gt_img_decoder_get_info_t)(struct _gt_img_decoder_s * decoder, const void * src, _gt_img_info_st * header);
/**
 * @brief function pointer to open image file
 */
typedef gt_res_t ( * gt_img_decoder_open_t)(struct _gt_img_decoder_s * decoder, struct _gt_img_dsc_s * dsc);
/**
 * @brief function pointer to read line data from image data
 */
typedef gt_res_t ( * gt_img_decoder_read_line_t)(struct _gt_img_dsc_s * dsc,
                                                gt_size_t x, gt_size_t y, gt_size_t len, uint8_t * buffer);
/**
 * @brief function pointer to close image file
 */
typedef gt_res_t ( * gt_img_decoder_close_t)(struct _gt_img_dsc_s * dsc);

#if GT_USE_FILE_HEADER
typedef gt_res_t ( * gt_img_decoder_fh_get_info_t)(struct _gt_img_decoder_s * decoder, gt_file_header_param_st const * const param, _gt_img_info_st * header);

typedef gt_res_t ( * gt_img_decoder_fh_open_t)(struct _gt_img_decoder_s * decoder, struct _gt_img_dsc_s * dsc);
#endif

/**
 * @brief callback function
 */
typedef struct _gt_img_decoder_s {
    struct _gt_list_head list;   // GUI system used, Do not modify!

    gt_img_decoder_get_info_t info_cb;          ///< get image base information
    gt_img_decoder_open_t open_cb;              ///< open image file and create file object
    gt_img_decoder_read_line_t read_line_cb;    ///< read image data
    gt_img_decoder_close_t close_cb;            ///< close image file object

#if GT_USE_FILE_HEADER
    gt_img_decoder_fh_get_info_t fh_info_cb;    ///< get image base information by file header
    gt_img_decoder_fh_open_t fh_open_cb;        ///< open image file and create file object by file header
#endif
}_gt_img_decoder_st;


/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief init decoder collection
 */
void _gt_img_decoder_init(void);

/**
 * @brief Create a gt_img decoder object
 *
 * @return _gt_img_decoder_st* decoder malloc memory pointer
 */
_gt_img_decoder_st * gt_img_decoder_create(void);

/**
 * @brief The image decoder is registered to the kernel task
 *
 * @param decoder need to be registered image decoder
 */
void gt_img_decoder_register(_gt_img_decoder_st * decoder);

/**
 * @brief Get the current file type of the image decoder
 *
 * @param header the _gt_img_dsc_st's header of the image decoder
 * @return gt_img_decoder_type_em The file type enum
 */
gt_img_decoder_type_em gt_img_decoder_get_type(_gt_img_info_st * header);

/**
 * @brief Get image decoder information
 *
 * @param name The full path of file name
 * @param header The struct to save image data.
 * @return gt_res_t The return status
 */
gt_res_t gt_img_decoder_get_info(const char * name, _gt_img_info_st * header);

/**
 * @brief Open image file and create a file object. The file object is a
 *      identifier for subsequent operations to the file.
 *
 * @param dsc The Description of the image file.
 * @param name The full path of file name
 * @return gt_res_t The return status
 */
gt_res_t gt_img_decoder_open(_gt_img_dsc_st * dsc, const char * const name);

/**
 * @brief Image data read a row of data
 *
 * @param dsc The Description of the image file.
 * @param x
 * @param y
 * @param len The data length need to read line
 * @param buf The buffer to save data which read from file content
 * @return gt_res_t The return status
 */
gt_res_t gt_img_decoder_read_line(_gt_img_dsc_st * dsc, gt_size_t x, gt_size_t y, gt_size_t len, uint8_t * buf);

/**
 * @brief Close an open file object. After the function successful be called, the
 *      file object is no longer valid and it can be discarded.
 *
 * @param dsc need to be closed file object
 * @return gt_res_t The return status
 */
gt_res_t gt_img_decoder_close(_gt_img_dsc_st * dsc);

#if GT_USE_FILE_HEADER
gt_res_t gt_img_decoder_fh_get_info(gt_file_header_param_st const * const param, _gt_img_info_st * header);
gt_res_t gt_img_decoder_fh_open(_gt_img_dsc_st * dsc, gt_file_header_param_st const * const param);
#endif

/**
 * @brief Set the information callback function pointer into image decoder object.
 *
 * @param decoder Image decoder, Which need to be init function and get information.
 *      information.
 * @param info_cb The callback function pointer
 */
void gt_img_decoder_set_info_cb(_gt_img_decoder_st * decoder, gt_img_decoder_get_info_t info_cb);

/**
 * @brief Set the open image file callback function pointer into image decoder object.
 *
 * @param decoder Image decoder, Which need to be init function by open file.
 * @param open_cb The callback function pointer, which open image file and
 *      create image file object.
 */
void gt_img_decoder_set_open_cb(_gt_img_decoder_st * decoder, gt_img_decoder_open_t open_cb);

/**
 * @brief Set the callback function pointer into image decoder object, which get image
 *      pre-line data.
 *
 * @param decoder Image decoder, Which need to be init function and read image line data.
 * @param read_line_cb The callback function pointer, which read image line data.
 */
void gt_img_decoder_set_read_line_cb(_gt_img_decoder_st * decoder, gt_img_decoder_read_line_t read_line_cb);

/**
 * @brief Set the callback function pointer into image decoder object, which close image
 *      object.
 *
 * @param decoder Image decoder, Which need to be init function and close the image file
 *      object.
 * @param close_cb The callback function pointer, which close image file.
 */
void gt_img_decoder_set_close_cb(_gt_img_decoder_st * decoder, gt_img_decoder_close_t close_cb);

#if GT_USE_FILE_HEADER
/**
 * @brief Set the information callback function pointer into image decoder object
 *      by file header.
 *
 * @param decoder Image decoder, Which need to be init function and get information.
 *      information.
 * @param fh_info_cb The file header control callback function pointer
 */
void gt_img_decoder_set_fh_info_cb(_gt_img_decoder_st * decoder, gt_img_decoder_fh_get_info_t fh_info_cb);

/**
 * @brief Set the open image file callback function pointer into image decoder object
 *      by file header.
 *
 * @param decoder Image decoder, Which need to be init function by open file.
 * @param fh_open_cb The file header control callback function pointer, which open image file and
 *      create image file object.
 */
void gt_img_decoder_set_fh_open_cb(_gt_img_decoder_st * decoder, gt_img_decoder_fh_open_t fh_open_cb);
#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_IMG_DECODER_H_
