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

#if 0
// TODO
typedef enum {
    GT_LINE_STYLE_DEFAULT = 0,
    GT_LINE_STYLE_STUB,             /** such as: - - - - */
}gt_line_style_et;
#endif


typedef struct gt_line_s {
    gt_color_t color;
    gt_opa_t opa;
    uint8_t width;
#if 0
    uint8_t type_sty;           /** line style @ref gt_line_style_et */
#endif

    uint8_t type : 3;   /** Such point/line/curve/bar @ref gt_graphs_type_et */
    uint8_t brush : 1;  /** brush type @ref gt_brush_type_et */
}gt_line_st;

typedef struct gt_axis_s {
    gt_range_st hor;
    gt_range_st ver;
    gt_float_t hor_unit;
    gt_float_t ver_unit;

    gt_line_st scale;
    gt_line_st grid;
}gt_axis_st;

typedef struct gt_attr_point_s {
    gt_point_st pos;
    gt_line_st line;
}gt_attr_point_st;

/**
 * @brief line attribute information
 */
typedef struct _gt_attr_line_s {
    gt_point_st start;
    gt_point_st end;
    gt_line_st line;
}gt_attr_line_st;

/**
 * @brief curve attribute information
 */
typedef struct _gt_attr_curve_s {
    gt_point_f_st p0;
    gt_point_f_st p1;
    gt_point_f_st p2;
    gt_point_f_st p3;
    gt_line_st line;
}gt_attr_curve_st;

typedef struct gt_series_points_s {
    struct gt_series_points_s * next_series_p;
    gt_float_t * x_series;
    gt_float_t * y_series;
    gt_line_st line;
    uint16_t count_point;
}gt_series_points_st;

typedef struct _gt_attr_graphs_s {
    gt_series_points_st * series_head_p;
    gt_axis_st axis;
    uint16_t count_line;
}_gt_attr_graphs_st;

typedef struct _gt_attr_rect_reg_s {
    uint8_t is_fill : 1;    ///< fill inside area by bg_color
    uint8_t reserved : 7;   ///< reserved
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
#if GT_USE_DIRECT_ADDR
    gt_addr_t addr;             ///< Using direct address mode to read image data
#endif

    gt_attr_line_st * line;     ///< use straight line

    gt_area_st * limit_area;

    gt_area_st * base_area;
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



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief init rect_attr
 *
 * @param rect_attr
 */
void gt_graph_init_rect_attr(gt_attr_rect_st * rect_attr);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_GRAPH_BASE_H_
