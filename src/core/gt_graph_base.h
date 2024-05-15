/**
 * @file gt_graph_base.h
 * @author yongg
 * @brief Low-level rendering interface
 * @version 0.1
 * @date 2022-05-11 15:07:30
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_GRAPH_BASE_H_
#define _GT_GRAPH_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "stdint.h"
#include "../others/gt_color.h"
#include "../others/gt_types.h"
#include "../widgets/gt_obj.h"
#include "./gt_img_decoder.h"

/* define ---------------------------------------------------------------*/
#define GT_SET_BIT(byte, bit)	( byte |= (bit) )
#define GT_CLR_BIT(byte, bit)	( byte &= ~(bit) )

#define GT_DOT_SET_BIT(buf,x,y,w,h,bit_val)				    \
	do{													    \
		if(bit_val)										    \
			GT_SET_BIT(buf[(y)*((w)>>3)+((x)>>3)], 1<<(7-((x)%8)) );	\
		else											                \
			GT_CLR_BIT(buf[(y)*((w)>>3)+((x)>>3)], 1<<(7-((x)%8)) );	\
	}while(0);											                \

#define GT_DOT_GET_BIT(buf,x,y,w,h)		((buf[(y)*((w)>>3)+((x)>>3)]>>(7-((x)%8)))&0x01)

#define GT_RADIUS_MIN   2
/* typedef --------------------------------------------------------------*/

/**
 * @brief line attribute information
 */
typedef struct _gt_attr_line_s
{
    gt_color_t fg_color;            ///< fore color
    gt_color_t bg_color;            ///< background color
    gt_size_t width;                ///< line width

    gt_size_t x_1;                  ///< start point position
    gt_size_t y_1;                  ///< start point position

    gt_size_t x_2;                  ///< end point position
    gt_size_t y_2;                  ///< end point position
}gt_attr_line_st;

typedef struct _gt_attr_rect_reg_s {
    uint8_t is_fill : 1;    ///< fill inside area by bg_color
    uint8_t is_line : 1;    ///< use straight line
    uint8_t reserved : 6;   ///< reserved
}_gt_attr_rect_reg_st;

/**
 * @brief rectangle attribute information
 */
typedef struct _gt_attr_rect_s {
    void * bg_img_src;          ///< image source; NULL: using RAM image data @ref raw_img
    _gt_img_dsc_st * raw_img;   ///< RAM image data, when bg_img_src is NULL
#if GT_USE_FILE_HEADER
    gt_file_header_param_st * file_header;  ///< Using file header mode to read image data
#endif

    gt_attr_line_st * line;     ///< use straight line

    gt_area_st* base_area;
    // Dot matrix data
    uint8_t * data_gray;
    uint8_t gray;

    gt_color_t fg_color;        ///< foreground color
    gt_color_t bg_color;        ///< background color

    // border
    gt_color_t border_color;    ///< border color
    uint8_t border_width;     ///< border width in pixels

    gt_size_t radius;           ///< radius value

    gt_opt_t bg_img_opa;        ///< image opacity
    gt_opt_t bg_opa;            ///< background opacity @ref gt_color.h
    gt_opa_t border_opa;        ///< border opacity

    _gt_attr_rect_reg_st reg;
}gt_attr_rect_st;

typedef struct _gt_attr_circle_reg_s {
    uint8_t is_fill : 1;    ///< fill inside area by fg_color
    uint8_t reserved : 7;   ///< reserved
}_gt_attr_circle_reg_st;

/**
 * @brief circle attribute information
 */
typedef struct _gt_attr_circle_s
{
    gt_color_t fg_color;        ///< fore color
    gt_color_t bg_color;        ///< background color

    gt_size_t radius;           ///< radius value

    gt_color_t border_color;    ///< border color
    uint8_t border_width;     ///< border width

    _gt_attr_circle_reg_st reg;
}gt_attr_circle_st;

/**
 * @brief arch attribute information
 */
typedef struct _gt_attr_arch_s
{
    gt_color_t fg_color;            ///< fore color
    gt_color_t bg_color;            ///< background color

    gt_size_t radius;               ///< radius value
    gt_size_t angle_start;          ///< The start angle
    gt_size_t angle_end;            ///< The end angle

    uint8_t border_width;         ///< border width
}gt_attr_arch_st;

/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief init rect_attr
 *
 * @param rect_attr
 */
void gt_graph_init_rect_attr(gt_attr_rect_st * rect_attr);

/**
 * @brief
 *
 * @param line_attr
 */
void gt_graph_init_line_attr(gt_attr_line_st * line_attr);

/**
 * @brief init line attribute object
 *
 * @param arch_attr
 */
void gt_graph_init_arch_attr(gt_attr_arch_st * arch_attr);

/**
 * @brief init circle attribute object
 *
 * @param circle_attr
 */
void gt_graph_init_circle_attr(gt_attr_circle_st * circle_attr);

/**
 * @brief Get gary between two colors
 *
 * @param color_out The return color value
 * @param color_from
 * @param color_to
 * @param gray
 * @param step The level of gray value
 * @return uint32_t The color value
 */
uint32_t gt_graph_get_gray(gt_color_t * color_out, gt_color_t color_from, gt_color_t color_to, gt_size_t gray, gt_size_t step);

#if 0

/**
 * @brief draw a circle
 *
 * @param circle_attr circle attribute
 * @param area display area
 * @param color_buf buffer to save data
 */
void gt_graph_circle(gt_attr_circle_st * circle_attr, gt_area_st * area, gt_color_t * color_buf);

/**
 * @brief draw a line
 *
 * @param line_attr line attribute
 * @param area  display area
 */
void gt_graph_line(gt_attr_line_st * line_attr, gt_area_st * area );

/**
 * @brief use bresenham algorithm create a line obj
 *
 * @param x0 start point
 * @param y0 start point
 * @param x1 end point
 * @param y1 end point
 * @param fg_color line color
 */
void gt_bresenham_line(gt_size_t x0, gt_size_t y0, gt_size_t x1, gt_size_t y1,gt_color_t fg_color);
#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_GRAPH_BASE_H_
