/**
 * @file gt_graph_base.c
 * @author yongg
 * @brief Low-level rendering interface
 * @version 0.1
 * @date 2022-05-11 15:02:43
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
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

#if 0
typedef enum{
	GT_GRAY_1 = 1,
	GT_GRAY_2 = 2,
	GT_GRAY_4 = 4,
	GT_GRAY_8 = 8,

	GT_GRAY_MAX_COUNT,
}gt_gray_t;
#endif

/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

#if 0
/**
 * @brief dot data change to color
 *
 * @param dot input dot array
 * @param color output color array
 * @param w array width
 * @param h array height
 * @param fg_color foreground
 * @param bg_color background
 */
static void _gt_graph_dot_to_color(uint8_t * dot, gt_color_t * color,gt_size_t w, gt_size_t h, gt_color_t fg_color, gt_color_t bg_color)
{
	int i,idx, len,cnt=0;
	idx = 0;
	len = (w/8*h);
	while( idx < len ) {
		for(i=7; i>=0; i--) {
			if( (dot[idx]>>i)&0x01 ) {
				GT_COLOR_SET(color[cnt], GT_COLOR_GET(fg_color));
			}else{
				GT_COLOR_SET(color[cnt], GT_COLOR_GET(bg_color));
			}
			cnt++;
		}
		idx++;
	}
}

/**
 * @brief dot change to color(no set background)
 *
 * @param dot input dot array
 * @param color output color array
 * @param w array width
 * @param h array height
 * @param fg_color foreground
 */
static void _gt_graph_dot_to_color_no_bg_color(uint8_t *dot, gt_color_t *color,gt_size_t w, gt_size_t h, gt_color_t fg_color)
{
	int i,idx, len,cnt=0;
	idx = 0;
	len = (w/8*h);
	while( idx < len ) {
		for(i=7; i>=0; i--) {
			if( (dot[idx]>>i)&0x01 ) {
				GT_COLOR_SET(color[cnt], GT_COLOR_GET(fg_color));
			}
			cnt++;
		}
		idx++;
	}
}


static void _gt_graph_gray_to_color(uint8_t *gray, gt_color_t *color,gt_size_t w, gt_size_t h, gt_color_t fg_color, gt_color_t bg_color, gt_size_t level_gray)
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
			gt_graph_get_gray(&_color, bg_color, fg_color, bit_mask, temp);
			GT_COLOR_SET(color[cnt], GT_COLOR_GET(_color));
			cnt++;
		}
		idx++;
	}
}

/**
 * @brief Circle effect segmentation
 */
#define _RADIUS_SPLIT_SIZE 50

/**
 * @brief Fill points in rows from center to outline point
 *
 * @param buf
 * @param fg_color the front color of circle
 * @param coord the point of outline
 * @param w the width of display area
 * @param h the height of display area
 * @param r the radius of circle
 */
static inline void _gt_fill_circle_inside_line(
	gt_color_t * buf,
	gt_attr_circle_st * circle_attr,
	gt_point_st coord,
	gt_size_t w,
	gt_size_t h)
{
	uint16_t idx;
	gt_size_t x = coord.x;
	gt_size_t y = coord.y;
	gt_size_t r = circle_attr->radius;
	gt_color_t color = circle_attr->fg_color;

	idx = y * w + x;
	if (x < r) {
		for (; x < r; x++) {
			GT_COLOR_SET(buf[idx++], GT_COLOR_GET(color));
		}
	} else {
		for (; x >= r; x--) {
			GT_COLOR_SET(buf[idx--], GT_COLOR_GET(color));
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
	gt_size_t border_width,
	gt_size_t remark_col,
	uint16_t count)
{
	gt_point_st from, to;
	gt_size_t size = r > _RADIUS_SPLIT_SIZE ? 2 : 1;

	--border_width;

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
		from.x = coord.x - border_width;
		from.y = coord.y;
		to.x = from.x;
		to.y = r;
	}
	else {
		from.x = coord.x - border_width;
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
	gt_size_t border_width,
	gt_size_t remark_row,
	uint16_t count)
{
	gt_point_st from, to;
	gt_size_t size = r > _RADIUS_SPLIT_SIZE ? 2 : 1;
	gt_size_t cover = border_width > 1 ? 0 : 1;

	--border_width;

	// if (coord.x == last.x) {
	// 	--count;
	// }
	// inside of outline
#if 01
	if (coord.y == remark_row) {
		from.x = last.x;
		from.y = last.y - border_width;
		to.x = r - 1;
		to.y = from.y;
	} else {
		from.x = last.x - 1;
		from.y = last.y - border_width - cover;
		to.x = from.x + count;
		to.y = from.y;
	}
#else
	if (coord.y == remark_row) {
		from.x = last.x;
		from.y = last.y - border_width;
		to.x = r - 1;
		to.y = from.y;
	} else {
		from.x = last.x;
		from.y = last.y - border_width;
		to.x = from.x + count;
		to.y = from.y;
	}
#endif
	gt_anti_aliasing_div(buf, area, &from, &to);

	// outside of outline
#if 01
	if (coord.y == remark_row) {
		from.x = coord.x + ( (r >> 4) ? (r >> 4) : 1 );
		from.y = coord.y;
		to.x = r;
		to.y = from.y;
	} else {
		from.x = last.x;
		from.y = last.y;
		to.x = from.x + count + size;
		to.y = from.y;
	}
#else
	if (coord.y == remark_row) {
		from.x = last.x + ( (r >> 4) ? (r >> 4) : 1 );
		from.y = coord.y;
		to.x = r;
		to.y = from.y;
	}
	else {
		from.x = coord.x;
		from.y = coord.y;
		to.x = coord.x + count + size;
		to.y = from.y;
	}
#endif
	gt_anti_aliasing_div(buf, area, &from, &to);
}

static void _gt_graph_fill_ver_points(
	gt_color_t * buf,
	gt_attr_circle_st * circle_attr,
	gt_point_st coord,
	gt_size_t ver_count,
    gt_size_t w,
    gt_size_t h)
{
	gt_size_t count = abs(ver_count);
	gt_size_t pre = ver_count > 0 ? 1 : -1;
	gt_color_t color = circle_attr->border_color;
	while(count--) {
		GT_COLOR_SET_BUF(buf, coord.x, coord.y, w, h, GT_COLOR_GET(color));
		coord.y += pre;
	}
}

/**
 * @brief Fill the horizontal points
 *
 * @param buf
 * @param fg_color front color
 * @param coord coordinates of beginning of line
 * @param hor_count horizontal coordinates points
 * @param w	the width of display area buffer
 * @param h the height of display area buffer
 */
static void _gt_graph_fill_hor_points(
	gt_color_t * buf,
	gt_attr_circle_st * circle_attr,
	gt_point_st coord,
	gt_size_t hor_count,
    gt_size_t w,
    gt_size_t h)
{
	gt_size_t count = abs(hor_count);
	gt_size_t pre = hor_count > 0 ? 1 : -1;
	gt_color_t color = circle_attr->border_color;
	while(count--) {
		GT_COLOR_SET_BUF(buf, coord.x, coord.y, w, h, GT_COLOR_GET(color));
		coord.x += pre;
	}
}

/**
 * @brief Fill points in rows from center to outline point
 *
 * @param buf
 * @param coord outline point position
 * @param w the width of the display area
 * @param h the height of the display area
 * @param is_hor horizontal or vertical
 */
static inline void _gt_graph_fill_line(
	gt_color_t * buf,
	gt_attr_circle_st * circle_attr,
	gt_point_st coord,
    gt_size_t w,
    gt_size_t h,
	bool is_hor)
{
	if (circle_attr->reg.is_fill) {
		_gt_fill_circle_inside_line(buf, circle_attr, coord, w, h);
	}
	if (is_hor) {
		_gt_graph_fill_hor_points(buf, circle_attr, coord, -circle_attr->border_width, w, h);
	} else {
		_gt_graph_fill_ver_points(buf, circle_attr, coord, -circle_attr->border_width, w, h);
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
	gt_size_t  diff_x  = 0, diff_y = 0, remark_col;
	uint8_t border_width = circle_attr->border_width;
	uint8_t    count   = 0;

	w            = area->w;
	h            = area->h;
	r            = circle_attr->radius;
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
		_gt_graph_fill_line(buf, circle_attr, coord, w, h, true);
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
					_gt_graph_fill_line(buf, circle_attr, tmp, w, h, true);
					++tmp.y;
				}
			}
			else {
				div = diff_y / diff_x;
				div = div ? div : 1;

				if (tmp.x != coord.x) {
					_gt_graph_fill_line(buf, circle_attr, tmp, w, h, true);
				}
				--tmp.x;

				for (x = 0; x < diff_x; x++) {
					for (y = 1; y < div; y++) {
						++tmp.y;
						_gt_graph_fill_line(buf, circle_attr, tmp, w, h, true);
					}
					_gt_graph_set_arc_0_to_45_direct_gray(buf, area, r, tmp, last, border_width, remark_col, x ? div : count);

					last.x = tmp.x;
					last.y = tmp.y;
					--tmp.x;
				}
				diff_x = coord.x - last.x;
				count = diff_x ? div : 0;
			}
		}

		if ( diff_x && start_angle != theta ) {
			_gt_graph_set_arc_0_to_45_direct_gray(buf, area, r, coord, last, border_width, remark_col, count);
			count = 1;
		} else {
			count += diff_y;
		}
	}
}

#define _GT_GRAPH_FILL_POINTS(_step, _p1_x, _p1_y, _p2_x, _p2_y) \
	do {	\
		gt_graph_get_gray(&tmp_color,	\
			circle_attr->border_color,	\
			circle_attr->bg_color,	\
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
	gt_size_t  diff_x  = 0, diff_y = 0, remark_row;
	uint8_t border_width = circle_attr->border_width;
	uint8_t    count   = 0;

	w          = area->w;
	h          = area->h;
	r          = circle_attr->radius;
	remark_row = GT_MATH_GET_TRIGON_VAL(gt_sin(90) * r) + r;

	for (theta = start_angle; theta < end_angle; theta++) {
		if (start_angle != theta) {
			last.x = coord.x;
			last.y = coord.y;
		}

		coord.x = GT_MATH_GET_TRIGON_VAL(gt_cos(theta) * r) + r;
		coord.y = GT_MATH_GET_TRIGON_VAL(gt_sin(theta) * r) + r;
		_gt_graph_fill_line(buf, circle_attr, coord, w, h, false);
		diff_x = coord.x - last.x;
		diff_y = coord.y - last.y;

		if (!diff_x && !diff_y) {
			continue;
		}

		diff_x = abs(diff_x);
		diff_y = abs(diff_y);
		// fill space points
		if (remark_row == coord.y) {
			x = coord.x - r + 1;
			tmp.x = coord.x;
			tmp.y = coord.y;
			while (x--) {
				_gt_graph_fill_line(buf, circle_attr, tmp, w, h, false);
				--tmp.x;
			}
			_gt_graph_set_arc_45_to_90_direct_gray(buf, area, r, coord, last, border_width, remark_row, count);
			break;
		}
		else if (diff_x && start_angle != theta) {
			tmp.x = last.x - 1;
			tmp.y = last.y;

			if (!diff_y) { // the same line
				x = abs(coord.x - last.x);
				while (x--) {
					_gt_graph_fill_line(buf, circle_attr, tmp, w, h, false);
					--tmp.x;
				}
			}
			else {
				div = diff_x / diff_y;
				div = div ? div : 1;
				++tmp.y;

				if (tmp.y != coord.y) {
					_gt_graph_fill_line(buf, circle_attr, tmp, w, h, false);
				}

				for (y = 0; y < diff_y; y++) {
					for (x = 1; x < div; x++) {
						--tmp.x;
						_gt_graph_fill_line(buf, circle_attr, tmp, w, h, false);
					}

					_gt_graph_set_arc_45_to_90_direct_gray(buf, area, r, tmp, last, border_width, remark_row, y ? div : count);
					last.x = tmp.x;
					last.y = tmp.y;
					++tmp.y;
				}

				diff_y = coord.y - last.y;
				count = diff_y ? div : 0;
			}
		}

		if (diff_y && start_angle != theta) {
			_gt_graph_set_arc_45_to_90_direct_gray(buf, area, r, coord, last, border_width, remark_row, count);
			count = 1;
		} else {
			count += diff_x;
		}
	}

	if (45 == start_angle) {
		coord.x = GT_MATH_GET_TRIGON_VAL(gt_cos(start_angle) * r) + r;
		coord.y = GT_MATH_GET_TRIGON_VAL(gt_sin(start_angle) * r) + r;

		gt_color_t tmp_color;
		if (_RADIUS_SPLIT_SIZE == r) {
			_GT_GRAPH_FILL_POINTS(0, coord.x, coord.y, coord.x - 1, coord.y);
		}
		else if (r > _RADIUS_SPLIT_SIZE) {
			_GT_GRAPH_FILL_POINTS(1, coord.x - 1, coord.y, coord.x - 1, coord.y - 1);
		}
		else if (r < 10) {
			_GT_GRAPH_FILL_POINTS(3, coord.x + 1, coord.y, coord.x + 1, coord.y);
		}
	}
}
#endif

/* global functions / API interface -------------------------------------*/

uint32_t gt_graph_get_gray(gt_color_t * color_out, gt_color_t color_from, gt_color_t color_to, gt_size_t gray, gt_size_t step)
{
	*color_out = gt_color_make(
		(GT_COLOR_GET_R(color_to) - GT_COLOR_GET_R(color_from)) * step / gray + GT_COLOR_GET_R(color_from),
		(GT_COLOR_GET_G(color_to) - GT_COLOR_GET_G(color_from)) * step / gray + GT_COLOR_GET_G(color_from),
		(GT_COLOR_GET_B(color_to) - GT_COLOR_GET_B(color_from)) * step / gray + GT_COLOR_GET_B(color_from)
	);

	return color_out->full;
}

void gt_graph_init_rect_attr(gt_attr_rect_st * rect_attr)
{
	gt_memset(rect_attr, 0, sizeof(gt_attr_rect_st));
	rect_attr->bg_color = gt_color_black();
	rect_attr->bg_opa = GT_OPA_COVER;
	rect_attr->border_color = gt_color_black();
	rect_attr->border_opa = GT_OPA_COVER;
}
void gt_graph_init_line_attr(gt_attr_line_st * line_attr)
{
	gt_memset(line_attr, 0, sizeof(gt_attr_line_st));
}
void gt_graph_init_arch_attr(gt_attr_arch_st * arch_attr)
{
	gt_memset(arch_attr, 0, sizeof(gt_attr_arch_st));
}
void gt_graph_init_circle_attr(gt_attr_circle_st * circle_attr)
{
	gt_memset(circle_attr, 0, sizeof(gt_attr_circle_st));
}

#if 0
void gt_graph_circle(gt_attr_circle_st * circle_attr, gt_area_st * area, gt_color_t * color_buf)
{
	gt_color_t * buf = color_buf;
	gt_size_t x, y, r;
	gt_size_t w, h;
	gt_color_t color;

	w = area->w;
	h = area->h;
	r = circle_attr->radius;

	GT_COLOR_CLR_BUF(buf, w * h, GT_COLOR_GET(circle_attr->bg_color));

	_gt_graph_arc_0_45(buf, circle_attr, area, 0, 45);
	// _gt_graph_arc_45_90(buf, circle_attr, area, 45, 90);

	// 45 ~ 90 degrees
	for (y = r; y < h; y++) {
		for (x = w - y; x < w; x++) {
			color = GT_COLOR_GET_OBJ(buf, x, y, w, h);
			GT_COLOR_SET_BUF(buf, y, x, w, h, GT_COLOR_GET(color));
		}
	}

	// copy area
	for (y = r; y < h; y++) {
		for (x = 0; x < r; x++) {
			color = GT_COLOR_GET_OBJ(buf, r + x, y, w, h);
			// left bottom
			GT_COLOR_SET_BUF(buf, r - x - 1, y, w, h, GT_COLOR_GET(color));
			// left top
			GT_COLOR_SET_BUF(buf, r - x - 1, h - y - 1, w, h, GT_COLOR_GET(color));
			// right top
			GT_COLOR_SET_BUF(buf, r + x, h - y - 1, w, h, GT_COLOR_GET(color));
		}
	}
}

void gt_graph_line(gt_attr_line_st * line_attr, gt_area_st * area)
{
	gt_color_t * bufa = gt_disp_graph_get_buf_default();
	gt_color_t fg_color;

	GT_COLOR_SET(fg_color, GT_COLOR_GET(line_attr->fg_color));

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
		GT_COLOR_SET_BUF(bufa, _x,_y, w, h, GT_COLOR_GET(fg_color));
	}

	for(_y=y_start; _y<(_h+y_start); _y++) {
		_x+=step_x;
		GT_COLOR_SET_BUF(bufa, _x,_y, w, h, GT_COLOR_GET(fg_color));
	}

}

void gt_bresenham_line(gt_size_t x0, gt_size_t y0, gt_size_t x1, gt_size_t y1,gt_color_t fg_color)
{
	gt_color_t * buf = gt_disp_graph_get_buf_default();

	gt_size_t dx =  abs(x1-x0);
	gt_size_t dy = -abs(y1-y0);
	int32_t err = dx + dy, e2 = 0; /* error value e_xy */
	int8_t sx = x0 < x1 ? 1 : -1;
	int8_t sy = y0 < y1 ? 1 : -1;

	while(true) {
		GT_COLOR_SET_BUF(buf, x0, y0, GT_SCREEN_WIDTH, GT_SCREEN_HEIGHT, GT_COLOR_GET(fg_color));

		if (x0==x1 && y0==y1){
			break;
		}
		e2 = err << 1;

		if (e2 >= dy) { /* e_xy+e_x > 0 */
			err += dy;
			x0 += sx;
		}
		if (e2 <= dx) {/* e_xy+e_y < 0 */
			err += dx;
			y0 += sy;
		}
	}
}
#endif


/* end ------------------------------------------------------------------*/
