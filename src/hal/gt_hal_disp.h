/**
 * @file gt_hal_disp.h
 * @author yongg
 * @brief GUI display hal layer driver
 * @version 0.1
 * @date 2022-05-13 09:51:55
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_HAL_DISP_H_
#define _GT_HAL_DISP_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../widgets/gt_obj.h"
#include "../others/gt_color.h"

/* define ---------------------------------------------------------------*/


/* typedef --------------------------------------------------------------*/
/**
 * @brief rotation angle
 */
typedef enum {
    GT_ROTATED_0,   ///< normally angle
    GT_ROTATED_90,  ///< 90 degrees rotation angle
    GT_ROTATED_180, ///< 180 degrees rotation angle
    GT_ROTATED_270  ///< 270 degrees rotation angle
}gt_rotated_et;

/**
 * @brief display driver information
 */
typedef struct _gt_disp_drv_s
{
    uint16_t res_hor;   ///< horizontal resolution
    uint16_t res_ver;   ///< vertical resolution

    gt_color_t bg_color;    ///< background color
    gt_color_t fg_color;    ///< foreground color

    /// @brief display flush callback
    void (* flush_cb)(struct _gt_disp_drv_s * drv, gt_area_st * area, gt_color_t * color);

    uint16_t rotated: 2;    ///< display rotation @ref gt_rotated_et
    uint16_t busy:1;        ///< display busy state
}gt_disp_drv_st;

/**
 * @brief display description
 */
typedef struct _gt_disp_s
{
    gt_disp_drv_st * drv;

    gt_obj_st ** screens;       //Array of screen
    gt_obj_st *  scr_act;       //Active screen
#if 0
    // unused
    gt_obj_st *  scr_pre;       //Preview Screen,Only save one layer
    gt_obj_st *  scr_next;      //Next Screen,Only save one layer
#endif
    uint16_t cnt_scr;           //Count screens nub
    uint16_t status;            //

    // round-robin queue
    struct {
        gt_area_st areas[_GT_REFR_AREA_MAX];    // ptr of area
        uint8_t joined[_GT_REFR_AREA_MAX];      //the area is not joined into before area: 1: is joined into before.
        uint8_t idx_w;                          // index of write now
        uint8_t idx_r;                          // index of read now
    }refr;

    /* flush */
    gt_area_st area_act;        //Location of physical screen on virtual screen
    gt_area_st area_disp;       //10 lines area, Objects in this area will be copied to vbd_color
    gt_color_t * vbd_color;     //save 10 lines visible pic, this will flush to phy screen
}gt_disp_st;


/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
void gt_disp_drv_init(gt_disp_drv_st * disp_drv);
void gt_disp_drv_deinit(void);

void gt_disp_drv_register(gt_disp_drv_st *drv);
gt_disp_st * gt_disp_get_default(void);

/**
 * @brief Get horizontal resolution value
 *
 * @param disp The display structure
 * @return uint16_t horizontal resolution value
 */
uint16_t gt_disp_get_res_hor(gt_disp_st * disp);

/**
 * @brief Get vertical resolution value
 *
 * @param disp The display structure
 * @return uint16_t vertical resolution value
 */
uint16_t gt_disp_get_res_ver(gt_disp_st * disp);

/**
 * @brief Register display screen buffer
 *
 * @param buf1 display screen buffer
 * @param buf2 backup display screen buffer
 * @param buf_all display screen full buffer
 */
void gt_disp_graph_buf_init(gt_color_t *buf1, gt_color_t *buf2, gt_color_t *buf_all);

/**
 * @brief get default using buffer
 *
 * @return gt_color_t* The buffer which active display screen
 */
gt_color_t * gt_disp_graph_get_buf_default(void);

/**
 * @brief Get backup color buffer, if used double screen buffer.
 *
 * @return gt_color_t* The buffer which save display color
 */
gt_color_t * gt_disp_graph_get_buf_backup(void);


void _gt_disp_refr_reset_areas(void);

void _gt_disp_refr_append_area(gt_area_st * area);
/**
 * @brief check disp need refresh area?
 *
 * @param disp disp
 * @return uint8_t 0:none, !0:yes
 */
uint8_t _gt_disp_refr_check(void);
void _gt_disp_refr_area_push(void);
void _gt_disp_refr_area_pop(void);
gt_area_st * _gt_disp_refr_get_area(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_HAL_DISP_H_
