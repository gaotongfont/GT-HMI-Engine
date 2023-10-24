/**
 * @file gt_sjpg.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2023-10-23 15:47:00
 * @copyright Copyright (c) 2014-2023, Company Genitop. Co., Ltd.
 */
/** ----------------------------------------------------------------------------------------------------------------------------------
*     Added normal JPG support [7/10/2020]
*     ----------
*     SJPEG is a custom created modified JPEG file format for small embedded platforms.
*     It will contain multiple JPEG fragments all embedded into a single file with a custom header.
*     This makes JPEG decoding easier using any JPEG library. Overall file size will be almost
*     similar to the parent jpeg file. We can generate sjpeg from any jpeg using a python script
*     provided along with this project.

*     SJPEG FILE STRUCTURE
*     --------------------------------------------------------------------------------------------------------------------------------
*     Bytes                       |   Value                                                                                           |
*     --------------------------------------------------------------------------------------------------------------------------------

*     0 - 7                       |   "_SJPG__" followed by '\0'

*     8 - 13                      |   "V1.00" followed by '\0'       [VERSION OF SJPG FILE for future compatibiliby]

*     14 - 15                     |   X_RESOLUTION (width)            [little endian]

*     16 - 17                     |   Y_RESOLUTION (height)           [little endian]

*     18 - 19                     |   TOTAL_FRAMES inside sjpeg       [little endian]

*     20 - 21                     |   JPEG BLOCK WIDTH (16 normally)  [little endian]

*     22 - [(TOTAL_FRAMES*2 )]    |   SIZE OF EACH JPEG SPLIT FRAGMENTS   (FRAME_INFO_ARRAY)

*    SJPEG data                   |   Each JPEG frame can be extracted from SJPEG data by parsing the FRAME_INFO_ARRAY one time.

* ----------------------------------------------------------------------------------------------------------------------------------
*                    JPEG DECODER
*                    ------------
*    We are using TJpgDec - Tiny JPEG Decompressor library from ELM-CHAN for decoding each split-jpeg fragments.
*    The tjpgd.c and tjpgd.h is not modified and those are used as it is. So if any update comes for the tiny-jpeg,
*    just replace those files with updated files.
* ---------------------------------------------------------------------------------------------------------------------------------*/

/* include --------------------------------------------------------------*/
#include "gt_png.h"
#include "../../core/gt_img_decoder.h"

#if GT_USE_SJPG

#include "./gt_sjpg.h"
#include "./tjpgd.h"

#include "../../others/gt_log.h"
#include "../../core/gt_mem.h"
#include "../../core/gt_fs.h"
#include "../../others/gt_color.h"

/* private define -------------------------------------------------------*/
#define TJPGD_WORK_BUFF_SIZE             4096    //Recommended by TJPGD library

//NEVER EDIT THESE OFFSET VALUES
#define SJPEG_VERSION_OFFSET            8
#define SJPEG_X_RES_OFFSET              14
#define SJPEG_y_RES_OFFSET              16
#define SJPEG_TOTAL_FRAMES_OFFSET       18
#define SJPEG_BLOCK_WIDTH_OFFSET        20
#define SJPEG_FRAME_INFO_ARRAY_OFFSET   22


/* private typedef ------------------------------------------------------*/
// enum io_source_type {
//     SJPEG_IO_SOURCE_C_ARRAY,
//     SJPEG_IO_SOURCE_DISK,
// };

typedef struct {
    // enum io_source_type type;
    gt_fs_fp_st * file;
    int16_t img_cache_x_res;
    int16_t img_cache_y_res;
    // uint32_t raw_sjpg_data_size;          //Num bytes pointed to by raw_sjpg_data.
    // uint32_t raw_sjpg_data_next_read_pos; //Used for all types.

    uint8_t * img_cache_buff;
    // uint8_t * raw_sjpg_data;              //Used when type==SJPEG_IO_SOURCE_C_ARRAY.
} _device_st;


typedef struct {
    uint8_t * sjpeg_data;
    uint32_t sjpeg_data_size;
    int16_t sjpeg_x_res;
    int16_t sjpeg_y_res;
    int16_t sjpeg_total_frames;
    int16_t sjpeg_single_frame_height;
    int16_t sjpeg_cache_frame_index;
    uint8_t ** frame_base_array;        //to save base address of each split frames upto sjpeg_total_frames.
    uint32_t * frame_base_offset;            //to save base offset for fseek
    uint8_t * frame_cache;
    uint8_t * work_buf;                    //JPG work buffer for jpeg library
    JDEC * jdec_hdc;
    _device_st dev;
} _sjpg_st;


/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static bool _is_jpg(const uint8_t * raw_data, size_t len) {
    uint8_t jpg_signature[] = {0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46, 0x49, 0x46};
    if(len < sizeof(jpg_signature)) {
        return false;
    }
    return (gt_memcmp(jpg_signature, raw_data, sizeof(jpg_signature)) == 0) ? true : false;
}

static inline void _sjpg_set_type(_gt_img_info_st * header) {
    header->type = GT_IMG_DECODER_TYPE_SJPG;
    header->color_format = GT_IMG_CF_TRUE_COLOR;
}

static void _sjpg_free(_sjpg_st * sjpg) {
    if (NULL == sjpg) {
        return ;
    }
    if (sjpg->jdec_hdc) {
        gt_mem_free(sjpg->jdec_hdc);
        sjpg->jdec_hdc = NULL;
    }
    if (sjpg->work_buf) {
        gt_mem_free(sjpg->work_buf);
        sjpg->work_buf = NULL;
    }
    if (sjpg->frame_cache) {
        gt_mem_free(sjpg->frame_cache);
        sjpg->frame_cache = NULL;
    }
    if (sjpg->frame_base_offset) {
        gt_mem_free(sjpg->frame_base_offset);
        sjpg->frame_base_offset = NULL;
    }
    gt_mem_free(sjpg);
}

static size_t input_func(JDEC * jdec,uint8_t * buff, size_t len) {
    _device_st * device = (_device_st *)jdec->device;
    if (!device) {
        return 0;
    }
    gt_fs_fp_st * fp = (gt_fs_fp_st *)device->file;
    uint32_t ret_len = len;
    if (!fp) {
        return 0;
    }
    gt_fs_res_et res = buff ? gt_fs_read(buff, len, &ret_len) : gt_fs_seek(len, GT_FS_SEEK_CUR);
    if (res) {
        return 0;
    }
    return (size_t)ret_len;
}

static int output_func(JDEC * jdec, void * bitmap, JRECT * rect) {
    uint8_t * buffer_p = (uint8_t * )bitmap;
    uint8_t color_depth = 3;
    _device_st * device = (_device_st *)jdec->device;
    int16_t x_res = device->img_cache_x_res;
    uint8_t * buffer_dst_p = device->img_cache_buff;
    uint32_t row_size = (rect->right - rect->left + 1) * color_depth;

    for (gt_size_t y = rect->top; y <= rect->bottom; y++) {
        gt_memcpy(&buffer_dst_p[(y * x_res + rect->left) * color_depth], buffer_p, row_size);
        buffer_p += row_size;
    }

    return 1;
}

static gt_res_t _gt_sjpg_info(struct _gt_img_decoder_s * decoder, const void * src, _gt_img_info_st * header)
{
    gt_fs_fp_st * fp = gt_fs_open(src, GT_FS_MODE_RD);
    gt_res_t ret = GT_RES_OK;
    uint32_t ret_len = 0;
    bool is_jpg_flag = false;
    uint8_t buffer[22] = {0};
    if (!fp) {
        return GT_RES_INV;
    }
    if (gt_fs_seek(0, GT_FS_SEEK_SET)) {
        goto err_lb;
    }
    if (gt_fs_read(buffer, sizeof(buffer), &ret_len)) {
        goto err_lb;
    }
    if (ret_len < 10) {
        ret = GT_RES_INV;
        goto err_lb;
    }

    const char * ext = gt_fs_get_ext(src);
    if (0 == gt_memcmp(ext, "sjpg", 4) ||
        0 == gt_memcmp(ext, "jpg",  4) ||
        0 == gt_memcmp(ext, "jpeg", 5)) {
        is_jpg_flag = true;
    }

    if (0 == gt_memcmp(buffer, "_SJPG__", strlen("_SJPG__"))) {
        header->w = (buffer[SJPEG_X_RES_OFFSET + 1] << 8) | buffer[SJPEG_X_RES_OFFSET];
        header->h = (buffer[SJPEG_y_RES_OFFSET + 1] << 8) | buffer[SJPEG_y_RES_OFFSET];
    }
    else if (_is_jpg(buffer, ret_len)) {
        uint8_t * work_buffer = gt_mem_malloc(TJPGD_WORK_BUFF_SIZE);
        if (!work_buffer) {
            ret = GT_RES_INV;
            goto err_lb;
        }
        if (gt_fs_seek(0, GT_FS_SEEK_SET)) {
            gt_mem_free(work_buffer);
            ret = GT_RES_INV;
            goto err_lb;
        }
        JDEC jdec_hdc = {0};
        _device_st dev = {
            .file = fp,
        };
        JRESULT j_ret = jd_prepare(&jdec_hdc, input_func, work_buffer, (size_t)TJPGD_WORK_BUFF_SIZE, (void *)&dev);
        gt_mem_free(work_buffer);
        if (JDR_OK != j_ret) {
            ret = GT_RES_INV;
            goto err_lb;
        }
        header->w = jdec_hdc.width;
        header->h = jdec_hdc.height;
    }
    else if (false == is_jpg_flag) {
        ret = GT_RES_INV;
        goto err_lb;
    }

    _sjpg_set_type(header);

err_lb:
    gt_fs_close();
    return ret;
}

static gt_res_t _gt_sjpg_open(struct _gt_img_decoder_s * decoder, struct _gt_img_dsc_s * dsc)
{
    gt_res_t ret = GT_RES_INV;
    const char * path = (char * )dsc->src;
    gt_fs_fp_st * fp = gt_fs_open(path, GT_FS_MODE_RD);
    uint32_t ret_len = 0;
    gt_fs_res_et res = GT_FS_RES_OK;
    uint8_t buffer[22] = {0};
    _sjpg_st * sjpg = NULL;

    if (!fp) {
        return GT_RES_INV;
    }
    if (gt_fs_seek(0, GT_FS_SEEK_SET)) {
        goto err_lb;
    }
    if (gt_fs_read(buffer, sizeof(buffer), &ret_len)) {
        goto err_lb;
    }
    if (ret_len < 10) {
        goto err_lb;
    }
    if (0 == gt_memcmp(buffer, "_SJPG__", strlen("_SJPG__"))) {
        // FIX: need to test
        sjpg = (_sjpg_st *)dsc->user_data;
        if (NULL == sjpg) {
            sjpg = gt_mem_malloc(sizeof(_sjpg_st));
            if (NULL == sjpg) {
                goto err_lb;
            }
            gt_memset(sjpg, 0, sizeof(_sjpg_st));
            dsc->user_data = sjpg;
            gt_fs_seek(0, GT_FS_SEEK_END);
            gt_fs_tell(&sjpg->sjpeg_data_size);
        }
        sjpg->sjpeg_x_res = (buffer[SJPEG_X_RES_OFFSET + 1] << 8) | buffer[SJPEG_X_RES_OFFSET];
        sjpg->sjpeg_y_res = (buffer[SJPEG_y_RES_OFFSET + 1] << 8) | buffer[SJPEG_y_RES_OFFSET];
        sjpg->sjpeg_total_frames = (buffer[SJPEG_TOTAL_FRAMES_OFFSET + 1] << 8) | buffer[SJPEG_TOTAL_FRAMES_OFFSET];
        sjpg->sjpeg_single_frame_height = (buffer[SJPEG_BLOCK_WIDTH_OFFSET + 1] << 8) | buffer[SJPEG_BLOCK_WIDTH_OFFSET];

        sjpg->frame_base_offset = gt_mem_malloc(sizeof(uint8_t * ) * sjpg->sjpeg_total_frames);
        if (NULL == sjpg->frame_base_offset) {
            goto err_lb;
        }
        sjpg->frame_base_offset[0] = (SJPEG_FRAME_INFO_ARRAY_OFFSET + (sjpg->sjpeg_total_frames << 1));
        for (int16_t i = 1; i < sjpg->sjpeg_total_frames; i++) {
            res = gt_fs_read(buffer, 2, &ret_len);
            if (GT_FS_RES_OK != res || 2 != ret_len) {
                goto err_lb;
            }
            sjpg->frame_base_offset[i] = (uint32_t)((buffer[1] << 8) | buffer[0]) + sjpg->frame_base_offset[i - 1];
        }
    }
    else if (_is_jpg(buffer, ret_len)) {
        sjpg = (_sjpg_st *)dsc->user_data;
        if (NULL == sjpg) {
            sjpg = gt_mem_malloc(sizeof(_sjpg_st));
            if (NULL == sjpg) {
                goto err_lb;
            }
            gt_memset(sjpg, 0, sizeof(_sjpg_st));
            dsc->user_data = sjpg;
            gt_fs_seek(0, GT_FS_SEEK_END);
            gt_fs_tell(&sjpg->sjpeg_data_size);
        }
        if (gt_fs_seek(0, GT_FS_SEEK_SET)) {
            goto err_lb;
        }

        uint8_t * work_buffer = gt_mem_malloc(TJPGD_WORK_BUFF_SIZE);
        if (!work_buffer) {
            goto err_lb;
        }
        JDEC jdec_hdc = {0};
        _device_st dev = {
            .file = fp,
        };
        JRESULT j_ret = jd_prepare(&jdec_hdc, input_func, work_buffer, (size_t)TJPGD_WORK_BUFF_SIZE, (void *)&dev);
        gt_mem_free(work_buffer);
        if (JDR_OK != j_ret) {
            goto err_lb;
        }
        sjpg->sjpeg_x_res = jdec_hdc.width;
        sjpg->sjpeg_y_res = jdec_hdc.height;
        sjpg->sjpeg_total_frames = 1;
        sjpg->sjpeg_single_frame_height = jdec_hdc.height;
        sjpg->frame_base_offset = gt_mem_malloc(sizeof(uint8_t * ) * sjpg->sjpeg_total_frames);
        if (NULL == sjpg->frame_base_offset) {
            goto err_lb;
        }
        sjpg->frame_base_offset[0] = 0;     /** img_frame_start_offset */
    } else {
        goto err_lb;
    }

    /** common */
    sjpg->sjpeg_cache_frame_index = -1;
    sjpg->frame_cache = gt_mem_malloc(sjpg->sjpeg_x_res * sjpg->sjpeg_single_frame_height * 3);
    if (NULL == sjpg->frame_cache) {
        goto err_lb;
    }
    sjpg->work_buf = gt_mem_malloc(TJPGD_WORK_BUFF_SIZE);
    if (NULL == sjpg->work_buf) {
        goto err_lb;
    }
    sjpg->jdec_hdc = gt_mem_malloc(sizeof(JDEC));
    if (NULL == sjpg->jdec_hdc) {
        goto err_lb;
    }
    sjpg->dev.img_cache_x_res = sjpg->sjpeg_x_res;
    sjpg->dev.img_cache_y_res = sjpg->sjpeg_y_res;
    sjpg->dev.img_cache_buff = sjpg->frame_cache;
    sjpg->dev.file = fp;

    return GT_RES_OK;
err_lb:
    _sjpg_free(sjpg);
    dsc->user_data = NULL;
    gt_fs_close();
    return ret;
}

static gt_res_t _gt_sjpg_read_line(struct _gt_img_dsc_s * dsc,
                                   gt_size_t x, gt_size_t y,
                                   gt_size_t len, uint8_t * buffer)
{
    _sjpg_st * sjpg = (_sjpg_st *)dsc->user_data;
    JRESULT j_ret;
    int16_t req_frame_index = y / sjpg->sjpeg_single_frame_height;
    gt_fs_fp_st * fp = sjpg->dev.file;
    if (NULL == fp) {
        return GT_RES_INV;
    }

    if (req_frame_index != sjpg->sjpeg_cache_frame_index) {
        gt_fs_seek((uint32_t)(sjpg->frame_base_offset[req_frame_index]), GT_FS_SEEK_SET);

        j_ret = jd_prepare(sjpg->jdec_hdc, input_func, sjpg->work_buf, (size_t)TJPGD_WORK_BUFF_SIZE, (void *)&sjpg->dev);
        if (JDR_OK != j_ret) {
            return GT_RES_INV;
        }
        j_ret = jd_decomp(sjpg->jdec_hdc, output_func, 0);
        if (JDR_OK != j_ret) {
            return GT_RES_INV;
        }
        sjpg->sjpeg_cache_frame_index = req_frame_index;
    }
    uint8_t * cache = (uint8_t *)&sjpg->frame_cache[(x + (y % sjpg->sjpeg_single_frame_height) * sjpg->sjpeg_x_res) * 3];

#if GT_COLOR_DEPTH == 32
    for (uint32_t i = 0; i < len; i++) {
        buffer[offset + 3] = 0xff;
        buffer[offset + 2] = *cache++;
        buffer[offset + 1] = *cache++;
        buffer[offset]     = *cache++;
        offset += 4;
    }
#elif GT_COLOR_DEPTH == 16
    uint16_t color = 0;
    for (uint32_t i = 0; i < len; i+=2) {
        color = (*cache++ & 0xf8) << 8;
        color |= (*cache++ & 0xfc) << 3;
        color |= (*cache++ >> 3);

        buffer[i] = color & 0xff;
        buffer[i + 1] = color >> 8;
    }
#elif GT_COLOR_DEPTH == 8
    uint8_t color = 0;
    for (uint32_t i = 0; i < len; i++) {
        color = (*cache++ & 0xc0);
        color |= (*cache++ & 0xe0) >> 2;
        color |= (*cache++ & 0xe0) >> 5;
        buffer[offset++] = color;
    }
#endif

    return GT_RES_OK;
}

static gt_res_t _gt_sjpg_close(struct _gt_img_dsc_s * dsc)
{
    _sjpg_st * sjpg = (_sjpg_st *)dsc->user_data;
    if (NULL == sjpg) {
        return GT_RES_OK;
    }

    _sjpg_free((_sjpg_st *)dsc->user_data);
    dsc->user_data = NULL;
    gt_fs_close();

    return GT_RES_OK;
}

/* global functions / API interface -------------------------------------*/

void gt_sjpg_init(void)
{
    _gt_img_decoder_st * decoder = gt_img_decoder_create();

    gt_img_decoder_set_info_cb(decoder, _gt_sjpg_info);
    gt_img_decoder_set_open_cb(decoder, _gt_sjpg_open);
    gt_img_decoder_set_read_line_cb(decoder, _gt_sjpg_read_line);
    gt_img_decoder_set_close_cb(decoder, _gt_sjpg_close);

    gt_img_decoder_register(decoder);
}


#endif /** GT_USE_SJPG */
/* end ------------------------------------------------------------------*/
