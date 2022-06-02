/**
 * @file gt_graph_base.c
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-05-11 15:02:43
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_graph_base.h"
#include "../hal/gt_hal_disp.h"
#include "gt_mem.h"
#include "../others/gt_math.h"
#include "stdlib.h"


/* private define -------------------------------------------------------*/

/* private typedef ------------------------------------------------------*/

typedef enum{
	GT_GRAY_1 = 1,
	GT_GRAY_2 = 2,
	GT_GRAY_4 = 4,
	GT_GRAY_8 = 8,
}gt_gray_t;

/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
void _gt_graph_get_gray(gt_color_t * color_out, gt_color_t color1, gt_color_t color2, gt_gray_t gray, gt_size_t step){
	uint8_t r,g,b;	
	r = GT_COLOR_GET_R(color1) + (GT_COLOR_GET_R(color2) - GT_COLOR_GET_R(color1))*step/gray;
	g = GT_COLOR_GET_G(color1) + (GT_COLOR_GET_G(color2) - GT_COLOR_GET_G(color1))*step/gray;
	b = GT_COLOR_GET_B(color1) + (GT_COLOR_GET_B(color2) - GT_COLOR_GET_B(color1))*step/gray;
	color_out->ch.red = r;
	color_out->ch.green = g;
	color_out->ch.blue = b;
}

void _gt_graph_dot_to_color(uint8_t *dot, gt_color_t *color,gt_size_t w, gt_size_t h, gt_color_t fcolor, gt_color_t bcolor){
	int i,idx, len,cnt=0;
	idx = 0;
	len = (w/8*h);
	while( idx < len ){
		for(i=7; i>=0; i--){
			if( (dot[idx]>>i)&0x01 ){
				GT_COLOR_SET(color[cnt], GT_COLOR_GET(fcolor) );
			}else{
				GT_COLOR_SET(color[cnt], GT_COLOR_GET(bcolor) );
			}
			cnt++;
		}
		idx++;
	}
}


#include "lcd.h"
#include "delay.h"
void _gt_graph_dot_to_gray(uint8_t *dot, gt_size_t w, gt_size_t h, gt_gray_t level_gray){
	int len,idx,i,_x,_y,_w, _h;
	uint8_t val,color2b;
	int bit_idx;
	uint8_t bit_mask,bit_cnt,bit_step, _bit_cnt,bit_x,bit_y;
	int __x,__y;
	uint8_t *out=dot, *p;
	float d1,d2,weight1=0.5,weight2=0.5;

	bit_idx = 8;
	switch ( level_gray ){
		case GT_GRAY_1: bit_step = 1; bit_cnt = 8; bit_mask = 0x01; break;
		case GT_GRAY_2:	bit_step = 2; bit_cnt = 4; bit_mask = 0x03; break;
		case GT_GRAY_4:	bit_step = 4; bit_cnt = 2; bit_mask = 0x0F; break;
		case GT_GRAY_8:	bit_step = 8; bit_cnt = 1; bit_mask = 0xFF; break;
		default: break;
	}

	idx = 0;
	_x = 0;
	_y = 0;
	_w = (w*level_gray)/8;
	_h = h*level_gray;

	while(_y<_h){
		while(_x<_w){
			bit_idx = 8;
			for(bit_x=0; bit_x<bit_cnt; bit_x++){
				p = dot+(_y*_w);
				bit_idx = bit_idx-bit_step;
				d1 = 0;
				for(bit_y=0; bit_y<bit_cnt; bit_y++){
					d1 += ((*p>>bit_idx)&bit_mask)*weight1;
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


static unsigned char dep1[]={0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,5};//4bit//{0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4};//4bit
static unsigned char dep2[]={0,1,1,2};//2bit 
static unsigned char dep3[]={0,1,1,2,1,3,2,3};//3bit//dep3[]={0,1,1,2,1,3,2,5};//3bit  
void _gt_graph_dot_to_gray0(uint8_t *OutPutData ,gt_size_t width,gt_size_t High,unsigned char Grade)
{
	unsigned int oldwidth,newwidth,height,thick,width2;		//newwidth
	unsigned char *ptr1,*ptr2,*ptr3,*ptr4,*ptr5;	//    *ptr6,*ptr7,*ptr8,*ptr9;
	unsigned int  d1,d2,d3,d4,d5;   				//d6,d7,d8,d9;
	unsigned char width_mult;
	unsigned long ser;	//i=0;
		
	height 		= High;
	width_mult=	Grade;
	oldwidth 	=	width;
	width2=(width*Grade+7)/8;
	if(width_mult==2)	//2bit
	{
			oldwidth=(oldwidth+7)/8*2;//
			for(thick=0;thick<height;thick++)//逐行扫描 
			{
				ptr1=OutPutData+width2*thick*2;
				ptr2=ptr1+width2;
				ptr5=OutPutData+oldwidth*thick;		
				for(ser=0;ser<width2;ser++)
				{
					d1=dep2[((*ptr1>>6)&0x03)];
					d2=dep2[((*ptr2>>6)&0x03)];
					d5=d1+d2;
					if(d5>=3)d5=3;
					*ptr5=((*ptr5) & 0x3F)|(d5<<6);

					d1=dep2[((*ptr1>>4)&0x03)];
					d2=dep2[((*ptr2>>4)&0x03)];
					d5=d1+d2;
					if(d5>=3)d5=3;
					*ptr5=((*ptr5)&0xCF)|(d5<<4);
					
					d1=dep2[((*ptr1>>2)&0x03)];
					d2=dep2[((*ptr2>>2)&0x03)];
					d5=d1+d2;
					if(d5>=3)d5=3;
					*ptr5=((*ptr5)&0xF3)|(d5<<2);
					
					d1=dep2[((*ptr1)&0x03)];
					d2=dep2[((*ptr2)&0x03)];
					d5=d1+d2;
					if(d5>=3)d5=3;
					*ptr5=((*ptr5)&0xFC)|(d5);
					
					ptr1++;
					ptr2++;
					ptr5++;
				}
			}
			if(oldwidth>width2)
			{
				for(thick=0;thick<height;thick++)
				{
					for(ser=0;ser<(oldwidth-width2);ser++)
					OutPutData[oldwidth*thick+width2+ser]=0;
				}
			}
			newwidth=oldwidth*height;
	}
	else if(width_mult==4)	//4bit
	{
		oldwidth=(oldwidth+7)/8*4;//
		for(thick=0;thick<height;thick++)//逐行扫描 
		{
			ptr1=OutPutData+width2*thick*4;//
			ptr2=ptr1+width2;//
			ptr3=ptr2+width2;
			ptr4=ptr3+width2;
			ptr5=OutPutData+oldwidth*thick;
			for(ser=0;ser<width2;ser++)
			{
				d1=dep1[((*ptr1>>4)&0x0F)];
				d2=dep1[((*ptr2>>4)&0x0F)];
				d3=dep1[((*ptr3>>4)&0x0F)];
				d4=dep1[((*ptr4>>4)&0x0F)];
				d5=d1+d2+d3+d4;
				if(d5>=16)
					d5=15;
				*ptr5=(d5<<4)&0xF0;
				
				d1=dep1[(*ptr1&0x0F)];
				d2=dep1[(*ptr2&0x0F)];
				d3=dep1[(*ptr3&0x0F)];
				d4=dep1[(*ptr4&0x0F)];
				d5=d1+d2+d3+d4;
				if(d5>=16)
					d5=15;
				*ptr5=*ptr5|d5;
				
				ptr1++;
				ptr2++;
				ptr3++;
				ptr4++;
				ptr5++;
			}
		}
		if(oldwidth>width2)
		{
			for(thick=0;thick<height;thick++)
			{
				for(ser=0;ser<(oldwidth-width2);ser++)
				OutPutData[oldwidth*thick+width2+ser]=0;
			}
		}
		newwidth=oldwidth*height;
	}
}

void _gt_graph_gray_to_color(uint8_t *gray, gt_color_t *color,gt_size_t w, gt_size_t h, gt_color_t fcolor, gt_color_t bcolor, gt_gray_t level_gray){
	int i, idx, len, cnt=0;
	int bit_step, bit_cnt, bit_idx;
	uint8_t bit_mask;
	uint8_t r,g,b;
	float weight, temp;
	gt_color_t _color;
	idx = 0;
	bit_idx = 8;
	switch ( level_gray ){
		case GT_GRAY_1: bit_step = 1; bit_cnt = 8; bit_mask = 0x01; break;
		case GT_GRAY_2:	bit_step = 2; bit_cnt = 4; bit_mask = 0x03; break;
		case GT_GRAY_4:	bit_step = 4; bit_cnt = 2; bit_mask = 0x0F; break;
		case GT_GRAY_8:	bit_step = 8; bit_cnt = 1; bit_mask = 0xFF; break;
		default: break;
	}
	len = (w*h)/bit_cnt;
	r = GT_COLOR_GET_R(fcolor);	g = GT_COLOR_GET_G(fcolor);	b = GT_COLOR_GET_B(fcolor);

	while( idx < len ){
		bit_idx = 8;
		for(i=1; i<=bit_cnt; i++){
			bit_idx = bit_idx-bit_step;
			temp = (gray[idx]>>bit_idx) & bit_mask;
			_gt_graph_get_gray(&_color, bcolor, fcolor, bit_mask, temp);
			GT_COLOR_SET(color[cnt], GT_COLOR_GET(_color) );
			cnt++;
		}
		idx++;
	}
}

/* global functions / API interface -------------------------------------*/
#if 0
void gt_graph_circle(gt_attr_circle_st * circle_attr, gt_area_st * area){
	gt_disp_st * disp = gt_disp_get_default();
	if(!disp->drv->flush_cb){
		return;
	}

	void (*flush)(struct _gt_disp_drv_s * drv, gt_area_st * area, gt_color_t * color);
    flush = disp->drv->flush_cb;

	gt_color_t * bufa = gt_disp_graph_get_bufa();
	gt_color_t fcolor, bcolor, border_color, color_temp;

	GT_COLOR_SET(fcolor, GT_COLOR_GET(circle_attr->fcolor));
	GT_COLOR_SET(bcolor, GT_COLOR_GET(circle_attr->bcolor));
	GT_COLOR_SET(border_color, GT_COLOR_GET(circle_attr->border_color));

	float _x,_y,r, x2, y2, r2,border_width;
	int w,h, isfill, idx;

	border_width = circle_attr->border_width;
	
	r = circle_attr->radius;
	w = 2*r;
	h = 2*r;
	r2 = gt_square(r);

	isfill = circle_attr->isfill;

	GT_COLOR_CLR_BUF(bufa, w*h, GT_COLOR_GET(bcolor));
	
	for(_x=-r; _x<r; _x++){
		x2 = gt_square(_x);
		for(_y=-r; _y<r; _y++){	
			y2 = gt_square(_y);
			if( sqrt(x2+y2) > (sqrt(r2)-border_width) && x2+y2 < r2 ){
				GT_COLOR_SET_BUF(bufa, _x+r, _y+r, w, h, GT_COLOR_GET(border_color));				
			}else if( isfill && (x2+y2 < r2) ){
				GT_COLOR_SET_BUF(bufa, _x+r, _y+r, w, h, GT_COLOR_GET(fcolor));				
			}
		}
	}

	gt_color_t color1,color2,color3,color4;
	int gray = 2;
	uint32_t _r,_g,_b;
	for(_x=0; _x<(w/2); _x++){
		for(_y=0; _y<(h/2); _y++){
			GT_COLOR_SET(color1, GT_COLOR_GET_BUF( bufa, _x*2, _y*2, w, h ) );//1
			GT_COLOR_SET(color2, GT_COLOR_GET_BUF( bufa, _x*2+1, _y*2, w, h ) );//2
			GT_COLOR_SET(color3, GT_COLOR_GET_BUF( bufa, _x*2, (_y+1)*2, w, h ) );//3
			GT_COLOR_SET(color4, GT_COLOR_GET_BUF( bufa, _x*2+1, (_y+1)*2, w, h ) );//4
			// blend
			_r = GT_COLOR_GET_R(color1) + GT_COLOR_GET_R(color2) + GT_COLOR_GET_R(color3) + GT_COLOR_GET_R(color4);
			_g = GT_COLOR_GET_G(color1) + GT_COLOR_GET_G(color2) + GT_COLOR_GET_G(color3) + GT_COLOR_GET_G(color4);
			_b = GT_COLOR_GET_B(color1) + GT_COLOR_GET_B(color2) + GT_COLOR_GET_B(color3) + GT_COLOR_GET_B(color4);
			GT_COLOR_SET_RGB(color_temp, _r/4, _g/4, _b/4);

			GT_COLOR_SET_BUF(bufa, _x, _y, w/2, h/2, GT_COLOR_GET(color_temp));
		}
	}

	gt_area_st _area;
	_area.x = area->x;
	_area.y = area->y;
	_area.w = w/2;
	_area.h = h/2;
	flush(disp->drv, &_area, bufa);
}

void gt_graph_line(gt_attr_line_st * line_attr, gt_area_st * area){
	gt_disp_st * disp = gt_disp_get_default();
	if(!disp->drv->flush_cb){
		return;
	}

	void (*flush)(struct _gt_disp_drv_s * drv, gt_area_st * area, gt_color_t * color);
    flush = disp->drv->flush_cb;

	gt_color_t * bufa = gt_disp_graph_get_bufa();
	gt_color_t fcolor;
	gt_color_t bcolor; 
	GT_COLOR_SET(fcolor, GT_COLOR_GET(line_attr->fcolor));
	GT_COLOR_SET(bcolor, GT_COLOR_GET(line_attr->bcolor));
    int idx = 0, all = 0;

	float x1,x2,y1,y2;		//two point get a line

	x1 = line_attr->x_1;
	y1 = line_attr->y_1;

	x2 = line_attr->x_2;	
	y2 = line_attr->y_2;

	int w,h;
	w = gt_abs(x1-x2);
	h = gt_abs(y1-y2);
	float step_y = (h)/w;

	int direction_x = x1>x2?-1:1;
	int _x,_y,cnt_row;
	cnt_row = w;

	GT_COLOR_CLR_BUF(bufa, w*h, GT_COLOR_GET(bcolor));

	_y = 0;
	for(_x=0; _x<w; _x++){
		_y+=step_y;
		GT_COLOR_SET_BUF(bufa, _x,_y, w, h, GT_COLOR_GET(fcolor));		
	}

	gt_area_st _area;
	_area.x = area->x;	_area.y = area->y;
	_area.w = w;		_area.h = h;
	flush(disp->drv, &_area, bufa);
}

void gt_graph_rect(gt_attr_rect_st * rect_attr, gt_area_st * area){
    gt_disp_st * disp = gt_disp_get_default();
    if(!disp->drv->flush_cb){
        return;
    }

    void (*flush)(struct _gt_disp_drv_s * drv, gt_area_st * area, gt_color_t * color);
    flush = disp->drv->flush_cb;

    gt_color_t * bufa = gt_disp_graph_get_bufa();
    int idx = 0;
    

    // fill backgroup
    if(rect_attr->isfill){
        while(idx < area->w * area->h){
            GT_COLOR_SET(bufa[idx], GT_COLOR_GET(rect_attr->color) );
            idx++;
        }				
        flush(disp->drv, area, bufa);
    }

    // border
    if( rect_attr->border_width != 0){   
		GT_LOGV(GT_LOG_TAG_GUI, "start draw border:%x", rect_attr->border_color.full);
        idx = area->w > area->h? area->w : area->h;
        while(idx > 0){
            bufa[idx].full = rect_attr->border_color.full;
            idx--;
        }
        gt_area_st _area;
        
        //up
        _area.h = rect_attr->border_width; _area.w = area->w; _area.x = area->x; _area.y = area->y;
        flush(disp->drv, &_area, bufa);
        //right
        _area.h = area->h; _area.w = rect_attr->border_width; _area.x = area->x + area->w; _area.y = area->y;
        flush(disp->drv, &_area, bufa);
        //down
        _area.h = rect_attr->border_width; _area.w = area->w; _area.x = area->x; _area.y = area->y + area->h;
        flush(disp->drv, &_area, bufa);
        //left
        _area.h = area->h; _area.w = rect_attr->border_width; _area.x = area->x; _area.y = area->y;
        flush(disp->drv, &_area, bufa);
    }
}

void gt_graph_arch(gt_attr_arch_st * arch_attr, gt_area_st * area){
    float x,y,r,_x,_y,_r,x2,y2,r2,_rx,_ry, _y_temp;
	float angle_start,angle_end;
	float pix_h, pix_w;
    void (*flush)(struct _gt_disp_drv_s * drv, gt_area_st * area, gt_color_t * color);
	uint16_t _cnt = 0;
	gt_area_st _area;

    gt_disp_st * disp = gt_disp_get_default();

    if(!disp->drv->flush_cb){
		GT_LOGV(GT_LOG_TAG_GUI, "flush_cb is null, please init drv");
        return;
    }

    flush = disp->drv->flush_cb;    

    gt_color_t * bufa = gt_disp_graph_get_bufa();

    gt_color_t fcolor;
    gt_color_t bcolor;
    gt_color_t color;

	gt_size_t border_width = arch_attr->border_width;

    fcolor.full = arch_attr->fcolor.full;
    bcolor.full = arch_attr->bcolor.full;

	x = area->x;
	y = area->y;
	r = arch_attr->radius;

	angle_start = arch_attr->angle_start;
	angle_end = arch_attr->angle_end;

	r2 = r*r;
    int cnt_row = 2 * r, __x, __y;
    int idx = 0, all = gt_square(r*2);
    while(idx < all){
        GT_COLOR_SET( bufa[idx], GT_COLOR_GET(bcolor) );
		idx++;
    }

	if(angle_start == angle_end){return;}
	
	if(angle_end > 360)angle_end=360;

	if(angle_end < angle_start){
		GT_LOGW(GT_LOG_TAG_GUI, "angle_end is less than angle_satart");
		return;
	}

	if( angle_start <= 90 ){		
		pix_h = gt_sin(angle_start)*r;
		pix_w = sqrt(r*r - gt_square(pix_h));
		_x = -1 * pix_w;
		if(angle_end <= 90){
			pix_h = gt_sin(angle_end)*r;
			pix_w = sqrt(r2 - gt_square(pix_h));
			_rx = -1 * pix_w;
		}else{
			_rx = 0;
		}
		for(; _x<=_rx; _x++){
			x2 = gt_square(_x);
			for(_y=-r; _y<0; _y++){
				y2 = gt_square(_y);
				if( sqrt(x2+y2) >= (sqrt(r2)-border_width)  && ((x2+y2) < (r2)) ){
					__x = _x+r;
					__y = _y+r;
					idx = __y * cnt_row + __x;
                    GT_COLOR_SET(bufa[idx], GT_COLOR_GET(fcolor));
				}
			}
		}
	}

	if( angle_start <= 180 && angle_end > 90 ){
		if(angle_start <= 90){
			_x = 0;
		}else{
			pix_h = gt_sin(180-angle_start)*r;
			pix_w = sqrt(r2 - gt_square(pix_h));
			_x = pix_w;
		}

		if(angle_end <= 180){
			pix_h = gt_sin(180-angle_end)*r;
			pix_w = sqrt(r2 - gt_square(pix_h));
			_rx = pix_w;
		}else{
			_rx = r;
		}

		for(; _x<=_rx; _x++){
			x2 = _x*_x;
			for(_y=-r; _y<0; _y++){
				y2 = _y*_y;
				if( sqrt(x2+y2) >= (sqrt(r2)-border_width)  && ((x2+y2) < (r2)) ){
					__x = _x+r;
					__y = _y+r;
					idx = __y * cnt_row + __x;
                    GT_COLOR_SET(bufa[idx], GT_COLOR_GET(fcolor));
				}
			}
		}
	}

	if( angle_start <= 270 && angle_end > 180 ){	
		
		if(angle_start <= 180){
			_y=0;
			_y_temp = _y;
		}else{
			pix_h = gt_sin(angle_start-180)*r;
			_y_temp = pix_h;
		}

		if(angle_end >= 270){
			_ry=r;
		}else{
			pix_h = gt_sin(angle_end-180)*r;		
			_ry = pix_h;
		}		

		for(_x=0; _x<=r; _x++){
			x2 = _x*_x;
			for(_y = _y_temp; _y<_ry; _y++){
				y2 = _y*_y;
				if( sqrt(x2+y2) >= (sqrt(r2)-border_width)  && ((x2+y2) < (r2)) ){
					__x = _x+r;
					__y = _y+r;
					idx = __y * cnt_row + __x;
                    GT_COLOR_SET(bufa[idx], GT_COLOR_GET(fcolor));
				}
			}
		}
	}

	if( angle_start <= 360 && angle_end > 270 ){
		if(angle_start <= 270){
			_ry = r;
		}else{
			pix_h = gt_sin(360 - angle_start)*r;
			_ry = pix_h;
		}
		
		if(angle_end >= 360){
			_y=0;
			_y_temp = _y;
		}else{
			pix_h = gt_sin(360 - angle_end)*r;		
			_y_temp = pix_h;
		}

		for(_x=-r; _x<=0; _x++){
			x2 = gt_square(_x);
			for(_y = _y_temp; _y<_ry; _y++){
				y2 = gt_square(_y);
				if( sqrt(x2+y2) >= (sqrt(r2)-border_width)  && ((x2+y2) < (r2)) ){
					__x = _x+r;
					__y = _y+r;
					idx = __y * cnt_row + __x;
                    GT_COLOR_SET(bufa[idx], GT_COLOR_GET(fcolor));
				}
			}
		}
	}

	_area.x = area->x-r;
	_area.y = area->y-r;
	_area.w = 2*r;
	_area.h = 2*r;
    flush(disp->drv, &_area, bufa);
}

/* dot */
void _gt_graph_dot_circle(gt_attr_circle_st * circle_attr, gt_area_st * area){
	gt_disp_st * disp = gt_disp_get_default();
	if(!disp->drv->flush_cb){ return; }

	void (*flush)(struct _gt_disp_drv_s * drv, gt_area_st * area, gt_color_t * color);
    flush = disp->drv->flush_cb;

	gt_color_t * bufa = gt_disp_graph_get_bufa();
	gt_color_t fcolor, bcolor, border_color;

	GT_COLOR_SET(fcolor, GT_COLOR_GET(circle_attr->fcolor));
	GT_COLOR_SET(bcolor, GT_COLOR_GET(circle_attr->bcolor));
	GT_COLOR_SET(border_color, GT_COLOR_GET(circle_attr->border_color));

	int w,h,r,_x,_y,_r,_w,_h;
	int border_width, isfill, idx, gray;
	int x2, y2, r2;
	gray = GT_GRAY_4;

	border_width = circle_attr->border_width;
	isfill = circle_attr->isfill;
	r = circle_attr->radius;
	w = 2*r;
	h = 2*r;

	_r = r*gray;
	_w = w*gray;
	_h = h*gray;
	
	GT_COLOR_CLR_BUF(bufa, _w*_h, GT_COLOR_GET(bcolor));	
	
	uint8_t buf_dot[GT_SCREEN_HEIGHT * (GT_SCREEN_WIDTH/8)];
	
	// create circle dot data
	r2 = gt_square(_r);
	for(_y=-_r; _y<_r; _y++){	
		y2 = gt_square(_y);
		for(_x=-_r; _x<_r; _x++){
			x2 = gt_square(_x);
			if( sqrt(x2+y2) > (sqrt(r2)-border_width*gray) && (x2+y2) < r2 ){
				GT_DOT_SET_BIT(buf_dot, _x+_r, _y+_r, _w, _h, 1);
				LCD_DrawPoint(_x+r*gray, _y+r*gray);
			}else if( isfill && ( (x2+y2) < r2) ){
				LCD_DrawPoint(_x+r*gray, _y+r*gray);
				GT_DOT_SET_BIT(buf_dot, _x+_r, _y+_r, _w, _h, 1);
			}
		}
	}
	_gt_graph_dot_to_gray0(buf_dot, w, h, (gt_gray_t)gray);
	_gt_graph_gray_to_color(buf_dot, bufa, w, h, fcolor, bcolor, (gt_gray_t)gray);

	gt_area_st _area;
	_area.x = area->x;
	_area.y = area->y+150;
	_area.w = w;
	_area.h = h;
	flush(disp->drv, &_area, bufa);
}
#endif

/* cache in buf, no draw*/
void gt_graph_circle(gt_attr_circle_st * circle_attr, gt_area_st * area){
	uint8_t buf_dot[GT_SCREEN_WIDTH/8 * GT_SCREEN_HEIGHT];
	gt_color_t border_color, fcolor;
	gt_size_t _x, _y, r;
	uint16_t _w, _h, border_width;
	uint8_t isfill;
	float x2,y2,r2;
	gt_color_t * bufa = gt_disp_graph_get_bufa();
	
	_w = area->w;
	_h = area->h;

	r = circle_attr->radius;

	border_width = circle_attr->border_width;
	border_color = circle_attr->border_color;
	fcolor = circle_attr->fcolor;
	isfill = circle_attr->isfill;

	r2 = gt_square(r);
	for(_y=-r; _y<r; _y++){
		y2 = gt_square(_y);
		for(_x=-r; _x<r; _x++){	
			x2 = gt_square(_x);
			if( sqrt(x2+y2) > (sqrt(r2)-border_width) && x2+y2 < r2 ){
				GT_COLOR_SET_BUF(bufa, _x+r, _y+r, _w, _h, GT_COLOR_GET(border_color));
			}else if( isfill && (x2+y2 < r2) ){
				GT_COLOR_SET_BUF(bufa, _x+r, _y+r, _w, _h, GT_COLOR_GET(fcolor));
			}
		}
	}
}

void gt_graph_line(gt_attr_line_st * line_attr, gt_area_st * area){
	gt_color_t * bufa = gt_disp_graph_get_bufa();
	gt_color_t fcolor;
	gt_color_t bcolor;
	GT_COLOR_SET(fcolor, GT_COLOR_GET(line_attr->fcolor));
	GT_COLOR_SET(bcolor, GT_COLOR_GET(line_attr->bcolor));
    int idx = 0, all = 0;

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
	for(_x=x_start; _x<(_w+x_start); _x++){
		_y+=step_y;
		GT_COLOR_SET_BUF(bufa, _x,_y, w, h, GT_COLOR_GET(fcolor));		
	}

	for(_y=y_start; _y<(_h+y_start); _y++){
		_x+=step_x;
		GT_COLOR_SET_BUF(bufa, _x,_y, w, h, GT_COLOR_GET(fcolor));		
	}

}

void gt_graph_rect(gt_attr_rect_st * rect_attr, gt_area_st * area){
	gt_size_t isfill, radius, border_width;
	gt_color_t bcolor, fcolor, border_color;

	isfill = rect_attr->isfill;
	radius = rect_attr->radius;
	border_width = rect_attr->border_width;
	GT_COLOR_SET(border_color, GT_COLOR_GET(rect_attr->border_color));

	GT_COLOR_SET(bcolor, GT_COLOR_GET(rect_attr->bcolor));
	GT_COLOR_SET(fcolor, GT_COLOR_GET(rect_attr->fcolor));
	
}

/* end ------------------------------------------------------------------*/
