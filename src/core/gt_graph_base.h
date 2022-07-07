/**
 * @file gt_graph_base.h
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-05-11 15:07:30
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
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


/* typedef --------------------------------------------------------------*/
typedef enum {
    GT_BLEND_MODE_NORMAL,     /**< Simply mix according to the opacity value*/
    GT_BLEND_MODE_ADDITIVE,   /**< Add the respective color channels*/
    GT_BLEND_MODE_SUBTRACTIVE,/**< Subtract the foreground from the background*/
    GT_BLEND_MODE_MULTIPLY,   /**< Multiply the foreground and background*/
    GT_BLEND_MODE_REPLACE,    /**< Replace background with foreground in the area*/
}gt_blend_mode_et;

typedef struct _gt_attr_line_s
{
    gt_blend_mode_et blend_mode;
    gt_color_t fcolor;
    gt_color_t bcolor;
    gt_size_t width;

    gt_size_t x_1;
    gt_size_t y_1;

    gt_size_t x_2;
    gt_size_t y_2;
}gt_attr_line_st;

typedef struct _gt_attr_rect_s
{
    // gt_blend_mode_et blend_mode;

    gt_color_t fcolor;
    gt_color_t bcolor;

    gt_color_t border_color;
    gt_size_t border_width;

    gt_size_t radius;

    uint8_t is_fill:1;
}gt_attr_rect_st;


typedef struct _gt_attr_circle_s
{
    gt_color_t fcolor;
    gt_color_t bcolor;


    gt_size_t radius;

    gt_size_t border_width;
    gt_color_t border_color;

    uint8_t is_fill:1;
}gt_attr_circle_st;




typedef struct _gt_attr_arch_s
{
    gt_blend_mode_et blend_mode;
    gt_color_t fcolor;
    gt_color_t bcolor;

    gt_size_t radius;
    gt_size_t angle_start;
    gt_size_t angle_end;

    gt_size_t border_width;
}gt_attr_arch_st;

/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

gt_color_t * gt_graph_get_default(void);
void gt_graph_set_default(gt_color_t *);
void gt_graph_circle(gt_attr_circle_st * circle_attr, gt_area_st * area);

void gt_graph_line(gt_attr_line_st * line_attr, gt_area_st * area );
void gt_graph_rect(gt_attr_rect_st * rect_attr, gt_area_st * area, gt_color_t * color );
void gt_graph_arch(gt_attr_arch_st * arch_attr, gt_area_st * area );
void _gt_graph_dot_circle(gt_attr_circle_st * circle_attr, gt_area_st * area);
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_GRAPH_BASE_H_
