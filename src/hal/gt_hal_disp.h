/**
 * @file gt_hal_disp.h
 * @author yongg
 * @brief GUI display hal layer driver
 * @version 0.1
 * @date 2022-05-13 09:51:55
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
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
 * @brief init screen callback function
 * @return gt_obj_st* screen object
 */
typedef gt_obj_st * (* gt_scr_init_func_cb_t)(void);

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

    /**
     * @brief display flush callback
     * @param drv display driver
     * @param area
     * @param color buffer
     */
    void (* flush_cb)(struct _gt_disp_drv_s * drv, gt_area_st * area, gt_color_t * color);

    uint8_t rotated: 2;    ///< display rotation @ref gt_rotated_et
    uint8_t busy:1;        ///< display busy state
    uint8_t reserved:5;    ///< reserved
}gt_disp_drv_st;

/**
 * @brief Backoff screen info, only one screen(or single stack depth) can be recorded.
 */
typedef struct _gt_disp_stack_s {
    gt_scr_init_func_cb_t init_func_cb;
    gt_obj_st * need_backoff_scr;
    uint32_t time;
    uint32_t delay;
}_gt_disp_stack_st;

/**
 * @brief display description
 */
typedef struct _gt_disp_s
{
    gt_disp_drv_st * drv;

    gt_obj_st *  scr_prev;      // Preview Screen,Only save one layer
    gt_obj_st *  scr_act;       // Active screen

    _gt_disp_stack_st * stack;  // Set backoff screen, only one screen can be recorded.

    gt_obj_st ** screens;       // Array of screen
    uint16_t cnt_scr;           // Count screens nub

    // round-robin queue
    struct {
        gt_area_st areas[_GT_REFR_AREA_MAX];    // ptr of area
        uint8_t joined[_GT_REFR_AREA_MAX];      // the area is not joined into before area: 1: is joined into before.
        uint8_t idx_w;                          // index of write now
        uint8_t idx_r;                          // index of read now
    }refr;

    /* flush */
    gt_area_st     area_act;            //Location of physical screen on virtual screen
    gt_area_st     area_disp;           //10 lines area, Objects in this area will be copied to vbd_color
    gt_area_abs_st area_max;            // max area of virtual screen
    gt_area_abs_st area_max_new_scr;    // loading screen max area of virtual screen
    gt_color_t     * vbd_color;         //save 10 lines visible pic, this will flush to phy screen

    uint8_t scr_anim_type : 4;          // @ref gt_scr_anim_type_et
    uint8_t reserved      : 4;
}gt_disp_st;


/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
void gt_disp_drv_init(gt_disp_drv_st * disp_drv);
// void gt_disp_drv_deinit(void);

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

/**
 * @brief reset the invalid redraw areas message queue
 */
void _gt_disp_refr_reset_areas(void);

/**
 * @brief Add invalid redraw areas into message queue
 *
 * @param area The redraw area
 */
void _gt_disp_refr_append_area(gt_area_st * area);
/**
 * @brief check disp need refresh area?
 *
 * @param disp disp
 * @return uint8_t 0:none, !0:yes
 */
uint8_t _gt_disp_refr_check(void);

/**
 * @brief Invalid redraw areas message queue set next index
 */
void _gt_disp_refr_area_push(void);

/**
 * @brief Invalid redraw areas message queue get the last index
 */
void _gt_disp_refr_area_pop(void);

/**
 * @brief Get the laster invalid redraw areas from message queue
 *
 * @return gt_area_st* The invalid redraw areas
 */
gt_area_st * _gt_disp_refr_get_area(void);

/**
 * @brief Update max screen can display area
 *
 * @param area widget area
 * @param is_ignore_calc is ignore calculate into the screen area size, @ref _gt_obj_is_ignore_calc_max_area()
 *           true: widget do not calculate into the screen area size, widget size will not be used as max area size
 *           false: widget calculate into the screen area size, widget size will be used as max area size;
 */
void _gt_disp_update_max_area(const gt_area_st * const area, bool is_ignore_calc);

void _gt_disp_reload_max_area(gt_obj_st * scr);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_HAL_DISP_H_
