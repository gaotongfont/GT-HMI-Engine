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
#include "./gt_gradient.h"

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

typedef enum {
    GT_LINE_STYLE_DEFAULT = 0,  /** solid line */
    GT_LINE_STYLE_DASH,         /** such as: - - - - */
    GT_LINE_STYLE_DOT,          /** such as: . . . . (dash_width=1) */
}gt_line_style_et;


typedef struct gt_line_s {
    gt_color_t color;
    gt_opa_t opa;
    uint8_t width;
    uint8_t dash_width;  /** dash segment length in px; 0 = solid line */
    uint8_t dash_gap;    /** gap length between dashes in px; 0 = solid line */
    uint8_t type : 3;    /** @ref gt_graphs_type_et */
    uint8_t brush : 1;   /** @ref gt_brush_type_et */
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
    char ** point_value_str;
    gt_color_t point_value_font_color;
    gt_line_st line;
    uint16_t count_point;
    uint8_t font_line_space;    // Spacing between point value characters and lines
    /** point value display position, true: bottom of line; false[default]: top of line */
    uint8_t show_point_value : 1;
    uint8_t point_value_bottom : 1;
}gt_series_points_st;

typedef struct _gt_attr_graphs_s {
    gt_series_points_st * series_head_p;
    gt_axis_st axis;
    uint16_t count_line;
}_gt_attr_graphs_st;

typedef struct _gt_attr_rect_reg_s {
    uint8_t is_fill : 1;    ///< fill inside area by bg_color
    uint8_t ignore_img_decoder : 1; ///< Raw file mode ignore image decoder
    uint8_t use_opa_color : 1; ///< Use fg_color as tint: ignore image RGB, blend with alpha only
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
#if GT_USE_DIRECT_ADDR
    gt_addr_t addr;             ///< Using direct address mode to read image data
#endif
#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
    gt_direct_addr_custom_size_st * custom_addr;  ///< Using custom size direct address mode to read image data
#endif

    gt_attr_line_st * line;     ///< use straight line

    gt_area_st * limit_area;

    gt_area_st * base_area;

    gt_color_masking_st * masking;

    // Dot matrix data
    uint8_t * data_gray;
    uint8_t gray;

    gt_color_t fg_color;        ///< foreground color
    gt_color_t bg_color;        ///< background color

    // border
    gt_color_t border_color;    ///< border color
    uint8_t border_width;     ///< border width in pixels

    gt_size_t radius;           ///< radius value
    uint8_t corner_mask;        ///< @ref GT_MASK_CORNER_* bits; default GT_MASK_CORNER_ALL (set by gt_graph_init_rect_attr)

    gt_opt_t bg_img_opa;        ///< image opacity
    gt_opt_t bg_opa;            ///< background opacity @ref gt_color.h
    gt_opa_t border_opa;        ///< border opacity

#if GT_USE_GRADIENT
    gt_gradient_dsc_st * grad;  ///< Gradient descriptor; NULL = solid bg_color
#endif
#if GT_USE_SHADOW
    gt_shadow_dsc_st * shadow;  ///< Shadow descriptor; NULL = no shadow
#endif

    _gt_attr_rect_reg_st reg;
}gt_attr_rect_st;

typedef struct _gt_attr_arc_s{
    char* img;
#if GT_USE_FILE_HEADER
    gt_file_header_param_st * file_header;  ///< Using file header mode to read image data
#endif
#if GT_USE_DIRECT_ADDR
    gt_addr_t addr;
#endif
#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
    gt_direct_addr_custom_size_st * custom_addr;  ///< Using custom size direct address mode to read image data
#endif

    gt_color_t color;
    uint16_t start_angle;
    uint16_t end_angle;
    uint16_t width;

    gt_opa_t opa;

#if GT_USE_GRADIENT
    gt_gradient_dsc_st * grad;  ///< Angular gradient; NULL = solid color. Color varies along angle.
    uint16_t grad_ref_span;     ///< 0=auto(stretch): gradient mapped to arc_span. >0=fixed: gradient mapped to this span.
#endif
    uint8_t fade_inner_width;   ///< Inner edge fade width in pixels (0 = no fade)
    uint8_t fade_outer_width;   ///< Outer edge fade width in pixels (0 = no fade)

    uint8_t rounded : 1;
}gt_attr_arc_st;

#ifndef GT_POLYGON_MAX_VERTICES
    #define GT_POLYGON_MAX_VERTICES     32
#endif

/**
 * @brief polygon draw attribute
 */
typedef struct _gt_attr_polygon_s {
    gt_point_st const * points;  ///< vertex array (caller manages lifetime)
    uint16_t count;              ///< number of vertices (>= 3 to draw)
    gt_line_st line;             ///< stroke style; width=0: no stroke
    gt_color_t fill_color;       ///< fill color
    gt_opa_t fill_opa;           ///< fill opacity; GT_OPA_TRANSP: no fill
    /* vertices in pts[] are bbox-relative (origin = widget area top-left),
     * so gt_draw_polygon() adds coords->x/y directly — no bbox subtraction needed. */
} gt_attr_polygon_st;

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
