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
#include "../others/gt_effects.h"
#include "../widgets/gt_obj.h"


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
			gt_graph_get_gray(&_color, bcolor, fcolor, bit_mask, temp);
			GT_COLOR_SET(color[cnt], GT_COLOR_GET(_color));
			cnt++;
		}
		idx++;
	}
}

/* global functions / API interface -------------------------------------*/

uint32_t gt_graph_get_gray(gt_color_t * color_out, gt_color_t color_from, gt_color_t color_to, gt_size_t gray, gt_size_t step)
{
	color_out->ch.red   = (GT_COLOR_GET_R(color_to) - GT_COLOR_GET_R(color_from)) * step / gray + GT_COLOR_GET_R(color_from);
	color_out->ch.green = (GT_COLOR_GET_G(color_to) - GT_COLOR_GET_G(color_from)) * step / gray + GT_COLOR_GET_G(color_from);
	color_out->ch.blue  = (GT_COLOR_GET_B(color_to) - GT_COLOR_GET_B(color_from)) * step / gray + GT_COLOR_GET_B(color_from);

	return color_out->full;
}
gt_color_t * gt_graph_get_default()
{
	return color;
}

void gt_graph_set_default(gt_color_t * _color){
	color = _color;
}

/**
 * @brief Circle effect segmentation
 */
#define _RADIUS_SPLIT_SIZE 50

/**
 * @brief Fill points in rows from center to outline point
 *
 * @param buf
 * @param fcolor the front color of circle
 * @param coord the point of outline
 * @param w the width of display area
 * @param h the height of display area
 * @param r the radius of circle
 */
static inline void _gt_fill_circle_inside_line(
	gt_color_t * buf,
	gt_color_t fcolor,
	gt_point_st coord,
	gt_size_t w,
	gt_size_t h,
	gt_size_t r)
{
	uint16_t idx;

	if (coord.x < r) {
		idx = coord.y * w + coord.x;
		for (; coord.x < r; coord.x++) {
			GT_COLOR_SET(buf[idx++], GT_COLOR_GET(fcolor));
		}
	} else {
		idx = coord.y * w + coord.x;
		for (; coord.x >= r; coord.x--) {
			GT_COLOR_SET(buf[idx--], GT_COLOR_GET(fcolor));
		}
	}
}

/**
 * @brief handler 0 -> 45 degrees gray color
 *
 * @param buf	color buffer to save graph
 * @param area	display area
 * @param r		radius
 * @param coord locate coordinate point
 * @param last	last coordinate point
 * @param remark_col remark 0 degrees y value
 * @param count	the number of same col points
 */
static inline void _gt_graph_set_arc_0_to_45_direct_gray(
	gt_color_t * buf,
	gt_area_st * area,
	gt_size_t r,
	gt_point_st coord,
	gt_point_st last,
	gt_size_t remark_col,
	uint16_t count)
{
	gt_point_st from, to;
	gt_size_t size = r > _RADIUS_SPLIT_SIZE ? 2 : 1;
	// return ;
	if (last.y == coord.y) {
		--count;
	}

	// outside of outline
	if (last.x == remark_col) {
		// rightmost
		from.x = last.x;
		from.y = ( (r >> 4) ? (r >> 4) : 1 ) + last.y;
		to.x = from.x;
		to.y = r;
	}
	else {
		from.x = coord.x + 1;
		from.y = coord.y + size;
		to.x   = from.x;
		to.y   = from.y - count - size;
	}
	gt_anti_aliasing_div(buf, area, &from, &to);

	// inside of outline
	if (last.x == remark_col) {
		from.x = coord.x;
		from.y = coord.y;
		to.x = from.x;
		to.y = r;
	}
	else {
		from.x = coord.x;
		from.y = coord.y;
		to.x = from.x;
		to.y = from.y - count - size + 1;
	}
	gt_anti_aliasing_div(buf, area, &from, &to);
}

/**
 * @brief handler 45 -> 90 degrees gray color
 *
 * @param buf	color buffer to save graph
 * @param area	display area
 * @param r		radius
 * @param coord locate coordinate point
 * @param last	last coordinate point
 * @param remark_col remark 0 degrees y value
 * @param count	the number of same col points
 */
static inline void _gt_graph_set_arc_45_to_90_direct_gray(
	gt_color_t * buf,
	gt_area_st * area,
	gt_size_t r,
	gt_point_st coord,
	gt_point_st last,
	gt_size_t remark_row,
	uint16_t count)
{
	gt_point_st from, to;
	gt_size_t size = r > _RADIUS_SPLIT_SIZE ? 2 : 1;
	gt_size_t offset = r >= 10 ? 1 : 0;

	if (last.x == coord.x) {
		--count;
	}

	// outside of outline
	if (last.y == remark_row) {
		from.x = ( (r >> 4) ? (r >> 4) : 1 ) + last.x;
		from.y = last.y;
		to.x = r;
		to.y = from.y;
	}
	else {
		from.x = last.x + size + offset;
		from.y = last.y;
		to.x   = from.x - count - size;
		to.y   = from.y;
	}
	gt_anti_aliasing_div(buf, area, &from, &to);

	// inside of outline
	if (last.y == remark_row) {
		from.x = coord.x;
		from.y = coord.y;
		to.x = r;
		to.y = from.y;
	}
	else {
		from.x = coord.x;
		from.y = coord.y;
		to.x = from.x - count - size + 1;
		to.y = from.y;
	}
	gt_anti_aliasing_div(buf, area, &from, &to);
}

/**
 * @brief Fill points in rows from center to outline point
 *
 * @param buf
 * @param fcolor front color
 * @param coord outline point position
 * @param w the width of the display area
 * @param h the height of the display area
 * @param r the radius of the circle
 * @param is_fill fill inside of outline, true: fill inside
 */
static inline void _gt_graph_fill_line(
	gt_color_t * buf,
	gt_color_t fcolor,
	gt_point_st coord,
    gt_size_t w,
    gt_size_t h,
    gt_size_t r,
    bool is_fill)
{
	if (is_fill) {
		_gt_fill_circle_inside_line(buf, fcolor, coord, w, h, r);
	}
	GT_COLOR_SET_BUF(buf, coord.x, coord.y, w, h, GT_COLOR_GET(fcolor));
}

/**
 * @brief Fill the horizontal points
 *
 * @param buf
 * @param fcolor front color
 * @param coord coordinates of beginning of line
 * @param hor_count horizontal coordinates points
 * @param w	the width of display area buffer
 * @param h the height of display area buffer
 */
static inline void _gt_graph_fill_hor_points(
	gt_color_t * buf,
	gt_color_t fcolor,
	gt_point_st coord,
	gt_size_t hor_count,
    gt_size_t w,
    gt_size_t h
	)
{
	gt_size_t count = abs(hor_count);
	gt_size_t pre = hor_count > 0 ? 1 : -1;
	while(count--) {
		coord.x += pre;
		GT_COLOR_SET_BUF(buf, coord.x, coord.y, w, h, GT_COLOR_GET(fcolor));
	}
}

/**
 * @brief Draw radians from 0 to 45 degrees
 *
 * @param buf fill buffer
 * @param circle_attr circle attribute
 * @param area display area
 * @param start_angle start angle
 * @param end_angle end angle
 */
static void _gt_graph_arc_0_45(
	gt_color_t * buf,
	gt_attr_circle_st * circle_attr,
	gt_area_st * area,
	gt_size_t start_angle,
	gt_size_t end_angle)
{
	gt_size_t x, y, r, theta, div = 0;
	gt_point_st coord, last, tmp;
	gt_size_t w, h;
	gt_color_t fcolor;
	bool       is_fill = circle_attr->is_fill;
	gt_size_t  diff_x  = 0, diff_y = 0, remark_col;
	uint8_t    count   = 0;

	w            = area->w;
	h            = area->h;
	r            = circle_attr->radius;
	fcolor       = circle_attr->fcolor;
	remark_col   = GT_MATH_GET_TRIGON_VAL(gt_cos(0) * r) + r;

	if (45 == end_angle && r > _RADIUS_SPLIT_SIZE) {
		end_angle += 2;
	}

	for (theta = start_angle; theta < end_angle; theta++) {
		if (start_angle != theta) {
			last.x = coord.x;
			last.y = coord.y;
		}

		coord.x = GT_MATH_GET_TRIGON_VAL(gt_cos(theta) * r) + r;
		coord.y = GT_MATH_GET_TRIGON_VAL(gt_sin(theta) * r) + r;
		_gt_graph_fill_line(buf, fcolor, coord, w, h, r, is_fill);
		diff_x = coord.x - last.x;
		diff_y = coord.y - last.y;

		if (!diff_x && !diff_y) {
			continue;
		}

		diff_x = abs(diff_x);
		diff_y = abs(diff_y);
		// fill space points
		if (diff_y && start_angle != theta) {

			tmp.x = last.x;
			tmp.y = last.y + 1;

			if (!diff_x) { // the same col
				y = diff_y;
				while (y--) {
					_gt_graph_fill_line(buf, fcolor, tmp, w, h, r, is_fill);
					++tmp.y;
				}
			}
			else {
				div = diff_y / diff_x;
				div = div ? div : 1;

				--tmp.x;
				if (tmp.x != coord.x) {
					_gt_graph_fill_line(buf, fcolor, tmp, w, h, r, is_fill);
				}

				for (x = 0; x < diff_x; x++) {
					for (y = 1; y < div; y++) {
						++tmp.y;
						_gt_graph_fill_line(buf, fcolor, tmp, w, h, r, is_fill);
					}
					_gt_graph_set_arc_0_to_45_direct_gray(buf, area, r, tmp, last, remark_col, x ? div : count);

					last.x = tmp.x;
					last.y = tmp.y;
					--tmp.x;
				}
				diff_x = coord.x - last.x;
				count = diff_x ? div : 0;
			}
		}

		if ( diff_x && start_angle != theta ) {
			_gt_graph_set_arc_0_to_45_direct_gray(buf, area, r, coord, last, remark_col, count);
			count = 1;
		} else {
			count += diff_y;
		}
	}
}

#define _GT_GRAPH_FILL_POINTS(_step, _p1_x, _p1_y, _p2_x, _p2_y) \
	do {	\
		gt_graph_get_gray(&tmp_color,	\
			GT_COLOR_GET_OBJ(buf, coord.x, coord.y, w, h),	\
			bcolor,	\
			GT_GRAY_4, _step);	\
		GT_COLOR_SET_BUF(buf, _p1_x, _p1_y, w, h, GT_COLOR_GET(tmp_color));	\
		GT_COLOR_SET_BUF(buf, _p2_x, _p2_y, w, h, GT_COLOR_GET(tmp_color));	\
	} while (0)


/**
 * @brief Draw radians from 45 to 90 degrees
 *
 * @param buf fill buffer
 * @param circle_attr circle attribute
 * @param area display area
 * @param start_angle start angle
 * @param end_angle end angle
 */
static void _gt_graph_arc_45_90(
	gt_color_t * buf,
	gt_attr_circle_st * circle_attr,
	gt_area_st * area,
	gt_size_t start_angle,
	gt_size_t end_angle)
{
	gt_size_t x, y, r, theta, div = 0;
	gt_point_st coord, last, tmp;
	gt_size_t w, h;
	gt_color_t fcolor;
	bool       is_fill = circle_attr->is_fill;
	gt_size_t  diff_x  = 0, diff_y = 0, remark_row;
	uint8_t    count   = 0;

	w          = area->w;
	h          = area->h;
	r          = circle_attr->radius;
	fcolor     = circle_attr->fcolor;
	remark_row = GT_MATH_GET_TRIGON_VAL(gt_sin(90) * r) + r;

	for (theta = end_angle; theta > start_angle; theta--) {
		if (end_angle != theta) {
			last.x = coord.x;
			last.y = coord.y;
		}

		coord.x = GT_MATH_GET_TRIGON_VAL(gt_cos(theta) * r) + r;
		coord.y = GT_MATH_GET_TRIGON_VAL(gt_sin(theta) * r) + r;
		_gt_graph_fill_line(buf, fcolor, coord, w, h, r, is_fill);
		diff_x = coord.x - last.x;
		diff_y = coord.y - last.y;

		if (!diff_x && !diff_y) {
			continue;
		}

		diff_x = abs(diff_x);
		diff_y = abs(diff_y);
		// fill space points
		if (diff_x && end_angle != theta) {

			tmp.x = last.x + 1;
			tmp.y = last.y - 1;

			if (!diff_y) { // the same line
				_gt_graph_fill_hor_points(buf, fcolor, tmp, coord.x - last.x, w, h);
			}
			else {
				div = diff_x / diff_y;
				div = div ? div : 1;

				if (tmp.y != coord.y) {
					_gt_graph_fill_line(buf, fcolor, tmp, w, h, r, is_fill);
				}

				for (y = 0; y < diff_y; y++) {
					for (x = 1; x < div; x++) {
						++tmp.x;
						_gt_graph_fill_line(buf, fcolor, tmp, w, h, r, is_fill);
					}
					_gt_graph_set_arc_45_to_90_direct_gray(buf, area, r, tmp, last, remark_row, y ? div : count);
					last.x = tmp.x;
					last.y = tmp.y;
					--tmp.y;
				}

				diff_y = coord.y - last.y;
				count = diff_y ? div : 0;
			}
		}

		if (diff_y && end_angle != theta) {
			_gt_graph_set_arc_45_to_90_direct_gray(buf, area, r, coord, last, remark_row, count);
			count = 1;
		} else {
			count += diff_x;
		}
	}

	if (45 == start_angle) {
		coord.x = GT_MATH_GET_TRIGON_VAL(gt_cos(start_angle) * r) + r;
		coord.y = GT_MATH_GET_TRIGON_VAL(gt_sin(start_angle) * r) + r;

		gt_color_t tmp_color;
		gt_color_t bcolor = circle_attr->bcolor;

		if (_RADIUS_SPLIT_SIZE == r) {
			_GT_GRAPH_FILL_POINTS(1, coord.x, coord.y, coord.x + 1, coord.y - 1);
		}
		if (r > _RADIUS_SPLIT_SIZE) {
			_GT_GRAPH_FILL_POINTS(1, coord.x - 1, coord.y, coord.x - 1, coord.y - 1);
		}
		else {
			_GT_GRAPH_FILL_POINTS(3, coord.x, coord.y, coord.x + 1, coord.y);
		}
	}
}

/**
 * @brief draw a circle
 *
 * @param circle_attr circle attribute
 * @param area display area
 */
void gt_graph_circle(gt_attr_circle_st * circle_attr, gt_area_st * area)
{
	gt_color_t * buf   = gt_disp_graph_get_buf_default();
	gt_size_t x, y, r;
	gt_size_t w, h;
	gt_color_t color;

	w = area->w;
	h = area->h;
	r = circle_attr->radius;

	GT_COLOR_CLR_BUF(buf, w * h, GT_COLOR_GET(circle_attr->bcolor));

	_gt_graph_arc_0_45(buf, circle_attr, area, 0, 45);
	_gt_graph_arc_45_90(buf, circle_attr, area, 45, 90);

	// copy area
	for (y = r; y < h; y++) {
		for (x = 0; x < r; x++) {
			color = GT_COLOR_GET_OBJ(buf, r + x, y, w, h);
			// left bottom
			GT_COLOR_SET_BUF(buf, r - x, y, w, h, GT_COLOR_GET(color));
			// left top
			GT_COLOR_SET_BUF(buf, r - x, h - y, w, h, GT_COLOR_GET(color));
			// right top
			GT_COLOR_SET_BUF(buf, r + x, h - y, w, h, GT_COLOR_GET(color));
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

	for(y = area_color.y; y < (_h); y++){
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
