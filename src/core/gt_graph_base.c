/**
 * @file gt_graph_base.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-05-11 15:02:43
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "stdlib.h"
#include "stdbool.h"

#include "gt_graph_base.h"
#include "../hal/gt_hal_disp.h"
#include "gt_mem.h"
#include "../others/gt_math.h"
#include "../others/gt_log.h"


/* private define -------------------------------------------------------*/

/* private typedef ------------------------------------------------------*/

typedef enum{
	GT_GRAY_1 = 1,
	GT_GRAY_2 = 2,
	GT_GRAY_4 = 4,
	GT_GRAY_8 = 8,

	GT_GRAY_MAX_COUNT,
}gt_gray_t;

/* static variables -----------------------------------------------------*/
static gt_color_t * color;


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
void _gt_graph_get_gray(gt_color_t * color_out, gt_color_t color_from, gt_color_t color_to, gt_gray_t gray, gt_size_t step)
{
	uint8_t r,g,b;
	r = GT_COLOR_GET_R(color_from) + (GT_COLOR_GET_R(color_to) - GT_COLOR_GET_R(color_from))*step/gray;
	g = GT_COLOR_GET_G(color_from) + (GT_COLOR_GET_G(color_to) - GT_COLOR_GET_G(color_from))*step/gray;
	b = GT_COLOR_GET_B(color_from) + (GT_COLOR_GET_B(color_to) - GT_COLOR_GET_B(color_from))*step/gray;
	color_out->ch.red = r;
	color_out->ch.green = g;
	color_out->ch.blue = b;
}

void _gt_graph_dot_to_color(uint8_t *dot, gt_color_t *color,gt_size_t w, gt_size_t h, gt_color_t fcolor, gt_color_t bcolor)
{
	int i,idx, len,cnt=0;
	idx = 0;
	len = (w/8*h);
	while( idx < len ) {
		for(i=7; i>=0; i--) {
			if( (dot[idx]>>i)&0x01 ) {
				GT_COLOR_SET(color[cnt], GT_COLOR_GET(fcolor));
			}else{
				GT_COLOR_SET(color[cnt], GT_COLOR_GET(bcolor));
			}
			cnt++;
		}
		idx++;
	}
}

void _gt_graph_dot_to_color_no_bcolor(uint8_t *dot, gt_color_t *color,gt_size_t w, gt_size_t h, gt_color_t fcolor)
{
	int i,idx, len,cnt=0;
	idx = 0;
	len = (w/8*h);
	while( idx < len ) {
		for(i=7; i>=0; i--) {
			if( (dot[idx]>>i)&0x01 ) {
				GT_COLOR_SET(color[cnt], GT_COLOR_GET(fcolor));
			}
			cnt++;
		}
		idx++;
	}
}


void _gt_graph_dot_to_gray(uint8_t *dot, gt_size_t w, gt_size_t h, gt_gray_t level_gray)
{
	int _x,_y,_w, _h;
	int bit_idx;
	uint8_t bit_mask,bit_cnt,bit_step,bit_x,bit_y;
	uint8_t *out=dot, *p;
	float d1,weight=0.5;

	bit_idx = 8;
	switch ( level_gray ) {
		case GT_GRAY_1: bit_step = 1; bit_cnt = 8; bit_mask = 0x01; break;
		case GT_GRAY_2:	bit_step = 2; bit_cnt = 4; bit_mask = 0x03; break;
		case GT_GRAY_4:	bit_step = 4; bit_cnt = 2; bit_mask = 0x0F; break;
		case GT_GRAY_8:	bit_step = 8; bit_cnt = 1; bit_mask = 0xFF; break;
		default: break;
	}

	_x = 0;
	_y = 0;
	_w = (w*level_gray)/8;
	_h = h*level_gray;

	while(_y<_h) {
		while(_x<_w) {
			bit_idx = 8;
			for(bit_x=0; bit_x<bit_cnt; bit_x++) {
				p = dot+(_y*_w);
				bit_idx = bit_idx-bit_step;
				d1 = 0;
				for(bit_y=0; bit_y<bit_cnt; bit_y++) {
					d1 += ((*p>>bit_idx)&bit_mask)*weight;
					p+=_w;
				}
				*out = *out&(~bit_mask<<bit_idx) | ((int)d1<<bit_idx);
			}
			out++;
			_x++;
		}
		_y+=level_gray;
	}
}


void _gt_graph_gray_to_color(uint8_t *gray, gt_color_t *color,gt_size_t w, gt_size_t h, gt_color_t fcolor, gt_color_t bcolor, gt_gray_t level_gray)
{
	int i, idx, len, cnt=0;
	int bit_step, bit_cnt, bit_idx;
	uint8_t bit_mask;

	float temp;
	gt_color_t _color;
	idx = 0;
	bit_idx = 8;
	switch ( level_gray ) {
		case GT_GRAY_1: bit_step = 1; bit_cnt = 8; bit_mask = 0x01; break;
		case GT_GRAY_2:	bit_step = 2; bit_cnt = 4; bit_mask = 0x03; break;
		case GT_GRAY_4:	bit_step = 4; bit_cnt = 2; bit_mask = 0x0F; break;
		case GT_GRAY_8:	bit_step = 8; bit_cnt = 1; bit_mask = 0xFF; break;
		default: break;
	}
	len = (w*h)/bit_cnt;

	while( idx < len ) {
		bit_idx = 8;
		for(i=1; i<=bit_cnt; i++) {
			bit_idx = bit_idx-bit_step;
			temp = (gray[idx]>>bit_idx) & bit_mask;
			_gt_graph_get_gray(&_color, bcolor, fcolor, (gt_gray_t)bit_mask, temp);
			GT_COLOR_SET(color[cnt], GT_COLOR_GET(_color));
			cnt++;
		}
		idx++;
	}
}

/* global functions / API interface -------------------------------------*/

gt_color_t * gt_graph_get_default()
{
	return color;
}

void gt_graph_set_default(gt_color_t * _color){
	color = _color;
}

void gt_graph_circle(gt_attr_circle_st * circle_attr, gt_area_st * area)
{
	gt_size_t x, y, r, theta;
	gt_size_t w, h, idx;
	gt_color_t fcolor;
	gt_color_t * buf = gt_disp_graph_get_buf_default();
	bool is_fill = circle_attr->is_fill;

	w = area->w;
	h = area->h;

	gt_memset(buf, 0xff, w * h << 1);

	r = circle_attr->radius;
	fcolor = circle_attr->fcolor;

	for (theta = 0; theta < 360; theta++) {
		x = GT_MATH_GET_TRIGON_VAL(gt_cos(theta) * r) + r;
		y = GT_MATH_GET_TRIGON_VAL(gt_sin(theta) * r) + r;
		GT_COLOR_SET_BUF(buf, x, y, w, h, GT_COLOR_GET(fcolor));

		if (!is_fill) {
			continue;
		}

		if (x < r) {
			idx = y * w + x;
			for (; x < r; x++) {
				GT_COLOR_SET(buf[idx++], GT_COLOR_GET(fcolor));
			}
		} else {
			idx = y * w + x;
			for (; x >= r; x--) {
				GT_COLOR_SET(buf[idx--], GT_COLOR_GET(fcolor));
			}
		}
	}
}

void gt_graph_line(gt_attr_line_st * line_attr, gt_area_st * area)
{
	gt_color_t * bufa = gt_disp_graph_get_buf_default();
	gt_color_t fcolor;

	GT_COLOR_SET(fcolor, GT_COLOR_GET(line_attr->fcolor));

	float x1,x2,y1,y2;		//two point get a line

	x1 = line_attr->x_1;
	y1 = line_attr->y_1;

	x2 = line_attr->x_2;
	y2 = line_attr->y_2;

	int w,h,_w,_h,x_start,y_start;
	w = area->w;
	h = area->h;

	_w = gt_abs(x1-x2);
	_h = gt_abs(y1-y2);
	float step_y = (_h)/(_w), step_x = _w/_h;

	int _x,_y;
	y_start = y1>y2?y2:y1;
	x_start = x1>x2?x2:x1;

	_y = y_start;
	for(_x=x_start; _x<(_w+x_start); _x++) {
		_y+=step_y;
		GT_COLOR_SET_BUF(bufa, _x,_y, w, h, GT_COLOR_GET(fcolor));
	}

	for(_y=y_start; _y<(_h+y_start); _y++) {
		_x+=step_x;
		GT_COLOR_SET_BUF(bufa, _x,_y, w, h, GT_COLOR_GET(fcolor));
	}

}

void gt_graph_rect(gt_attr_rect_st * rect_attr, gt_area_st * area, gt_color_t * color)
{
	gt_size_t is_fill, border_width;
	gt_color_t fcolor, border_color;

	is_fill = rect_attr->is_fill;

	border_width = rect_attr->border_width;
	GT_COLOR_SET(border_color, GT_COLOR_GET(rect_attr->border_color));

	GT_COLOR_SET(fcolor, GT_COLOR_GET(rect_attr->fcolor));

	gt_size_t w,h;
	w = area->w;
	h = area->h;

	// if( color != NULL ){
	// 	gt_mem_free(color);
	// }
	// color = gt_mem_malloc( w * h * sizeof(gt_color_t) );

	uint32_t x,y, x1,y1, x2,y2, idx = 0, all = w * h, x21, idxb1;
	x1 = border_width;
	x2 = w - border_width;

	y1 = border_width;
	y2 = h - border_width;
	x21 = x2 - x1;

	//draw center
	if(is_fill){
		while(idx < all){
			GT_COLOR_SET(color[idx++], GT_COLOR_GET(fcolor));
		}
	}
	return;

	/* FIXME */

	//draw up border
	y = 0;
	idx = 0;
	idxb1 = border_width * w;
	while(idx < idxb1){
		GT_COLOR_SET(color[idx], GT_COLOR_GET(border_color));
		idx++;
	}
	//draw bottom border
	y = y2;
	idx = w * y2;

	while( idx < all ){
		GT_COLOR_SET(color[idx], GT_COLOR_GET(border_color));
		idx++;
	}
	y = y1;
	idx = border_width * w;
	while( y < y2 ){
		//draw left border
		x = 0;
		while( x < border_width ){
			GT_COLOR_SET(color[idx], GT_COLOR_GET(border_color));
			idx++;
			x++;
		}
		//draw right border
		x = 0;
		idx += x21;
		while( x < border_width ){
			GT_COLOR_SET(color[idx], GT_COLOR_GET(border_color));
			idx++;
			x++;
		}
		y++;
	}
}


void gt_graph_dot_draw_to_color(uint8_t * dot, gt_color_t * color, gt_area_st area_color, uint8_t w, uint8_t h, gt_color_t fcolor)
{
	uint16_t y, x, idx = 0, temp;
	uint16_t _h, _w;
	int8_t j = 0;
	_h = area_color.y + h;
	_w = w >> 3;

	for(y = area_color.y; y < _h; y++){
		temp = 0;
		for(x = 0; x < _w; x++){
			for(j = 7; j >= 0; j--){			
				if( (dot[idx] >> j) & 0x01 ){
					GT_COLOR_SET_BUF(color, area_color.x + temp, y, area_color.w, area_color.h, GT_COLOR_GET(fcolor));
				}
				++temp;
			}
			++idx;
		}
	}
}

/* end ------------------------------------------------------------------*/
