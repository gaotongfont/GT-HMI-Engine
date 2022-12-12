/**
 * @file barcode.c
 * @author Yang (your@email.com)
 * @brief 
 * @version 0.1
 * @date 2022-06-16 13:44:09
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../extra/barcode/gt_barcode_src.h"
#include "string.h"
#include "stdio.h"

/* private define -------------------------------------------------------*/
#define BARCODE_MIN_H       (32)    // 最小高度
#define BARCODE_BLANK_SIZE  (4)     // 上下留白
#define FONT_SIZE_H         (8)     // 文字高度

/* private typedef ------------------------------------------------------*/


// 一行数据
typedef struct
{
    bar_uint16_t pos;
    bar_uint16_t row_byte;
    bar_uint8_t *pdat;
}_row_st;

// 条码参数
typedef struct
{
    barcode_st *p_code;

    bar_uint16_t left_blank_none;      // 左空白区
    bar_uint16_t right_blank_none;     // 又空白区

    bar_uint16_t start;                // 起始行
    bar_uint16_t end;                  // 结束行

    bar_uint16_t start_code;           // 起始符
    bar_uint16_t left_data_code;       // 左侧数据符
    bar_uint16_t separator;            // 中间间隔符
    bar_uint16_t right_data_code;      // 右侧数据符
    bar_uint16_t check_code;           // 校验符
    bar_uint16_t end_code;             // 结束符

    bar_uint16_t itf_bearer_bar;       // ITF 条码保护框
    bar_uint16_t interval_code;        // 字符间隔 ，在某些条码中用到 如 OCDABAR

    bar_uint16_t one_code_size;        // 数据符一个模块大小

    bar_uint8_t len;                  // 条码长度 

    bar_uint8_t wide_module;          // 宽模块
    bar_uint8_t narrow_module;        // 窄模块

}_code_st;
static _code_st _barcode;
static _code_st* _p_barcode = &_barcode;
static _row_st row_para;
/* static prototypes ----------------------------------------------------*/
const static bar_uint8_t _wide_module_size[] = {2 , 2 , 5 , 8 , 10 , 13 , 15};


/* static variables -----------------------------------------------------*/
static const bar_uint8_t _ASCII_5x7[] = 
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x20, 0x00, 
    0x50, 0x50, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x50, 0xF8, 0x50, 0xF8, 0x50, 0x50, 0x00, 
    0x20, 0x78, 0xA0, 0x70, 0x28, 0xF0, 0x20, 0x00, 0xC0, 0xC8, 0x10, 0x20, 0x40, 0x98, 0x18, 0x00, 
    0x60, 0x90, 0xA0, 0x40, 0xA8, 0x90, 0x68, 0x00, 0x60, 0x20, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x10, 0x20, 0x40, 0x40, 0x40, 0x20, 0x10, 0x00, 0x40, 0x20, 0x10, 0x10, 0x10, 0x20, 0x40, 0x00, 
    0x00, 0x20, 0xA8, 0x70, 0xA8, 0x20, 0x00, 0x00, 0x00, 0x20, 0x20, 0xF8, 0x20, 0x20, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x60, 0x20, 0x40, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x00, 
    0x70, 0x88, 0x98, 0xA8, 0xC8, 0x88, 0x70, 0x00, 0x20, 0x60, 0x20, 0x20, 0x20, 0x20, 0x70, 0x00, 
    0x70, 0x88, 0x08, 0x10, 0x20, 0x40, 0xF8, 0x00, 0xF8, 0x10, 0x20, 0x10, 0x08, 0x88, 0x70, 0x00, 
    0x10, 0x30, 0x50, 0x90, 0xF8, 0x10, 0x10, 0x00, 0xF8, 0x80, 0xF0, 0x08, 0x08, 0x88, 0x70, 0x00, 
    0x30, 0x40, 0x80, 0xF0, 0x88, 0x88, 0x70, 0x00, 0xF8, 0x08, 0x10, 0x20, 0x40, 0x40, 0x40, 0x00, 
    0x70, 0x88, 0x88, 0x70, 0x88, 0x88, 0x70, 0x00, 0x70, 0x88, 0x88, 0x78, 0x08, 0x10, 0x60, 0x00, 
    0x00, 0x60, 0x60, 0x00, 0x60, 0x60, 0x00, 0x00, 0x00, 0x60, 0x60, 0x00, 0x60, 0x20, 0x40, 0x00, 
    0x10, 0x20, 0x40, 0x80, 0x40, 0x20, 0x10, 0x00, 0x00, 0x00, 0xF8, 0x00, 0xF8, 0x00, 0x00, 0x00, 
    0x40, 0x20, 0x10, 0x08, 0x10, 0x20, 0x40, 0x00, 0x70, 0x88, 0x08, 0x10, 0x20, 0x00, 0x20, 0x00, 
    0x70, 0x88, 0x08, 0x68, 0xA8, 0xA8, 0x70, 0x00, 0x70, 0x88, 0x88, 0x88, 0xF8, 0x88, 0x88, 0x00, 
    0xF0, 0x88, 0x88, 0xF0, 0x88, 0x88, 0xF0, 0x00, 0x70, 0x88, 0x80, 0x80, 0x80, 0x88, 0x70, 0x00, 
    0xE0, 0x90, 0x88, 0x88, 0x88, 0x90, 0xE0, 0x00, 0xF8, 0x80, 0x80, 0xF0, 0x80, 0x80, 0xF8, 0x00, 
    0xF8, 0x80, 0x80, 0xF0, 0x80, 0x80, 0x80, 0x00, 0x70, 0x88, 0x80, 0xB8, 0x88, 0x88, 0x78, 0x00, 
    0x88, 0x88, 0x88, 0xF8, 0x88, 0x88, 0x88, 0x00, 0x70, 0x20, 0x20, 0x20, 0x20, 0x20, 0x70, 0x00, 
    0x38, 0x10, 0x10, 0x10, 0x10, 0x90, 0x60, 0x00, 0x88, 0x90, 0xA0, 0xC0, 0xA0, 0x90, 0x88, 0x00, 
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xF8, 0x00, 0x88, 0xD8, 0xA8, 0xA8, 0x88, 0x88, 0x88, 0x00, 
    0x88, 0x88, 0xC8, 0xA8, 0x98, 0x88, 0x88, 0x00, 0x70, 0x88, 0x88, 0x88, 0x88, 0x88, 0x70, 0x00, 
    0xF0, 0x88, 0x88, 0xF0, 0x80, 0x80, 0x80, 0x00, 0x70, 0x88, 0x88, 0x88, 0xA8, 0x90, 0x68, 0x00, 
    0xF0, 0x88, 0x88, 0xF0, 0xA0, 0x90, 0x88, 0x00, 0x78, 0x80, 0x80, 0x70, 0x08, 0x08, 0xF0, 0x00, 
    0xF8, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x70, 0x00, 
    0x88, 0x88, 0x88, 0x88, 0x88, 0x50, 0x20, 0x00, 0x88, 0x88, 0x88, 0xA8, 0xA8, 0xA8, 0x50, 0x00, 
    0x88, 0x88, 0x50, 0x20, 0x50, 0x88, 0x88, 0x00, 0x88, 0x88, 0x88, 0x50, 0x20, 0x20, 0x20, 0x00, 
    0xF8, 0x08, 0x10, 0x20, 0x40, 0x80, 0xF8, 0x00, 0x70, 0x40, 0x40, 0x40, 0x40, 0x40, 0x70, 0x00, 
    0x00, 0x80, 0x40, 0x20, 0x10, 0x08, 0x00, 0x00, 0x70, 0x10, 0x10, 0x10, 0x10, 0x10, 0x70, 0x00, 
    0x20, 0x50, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 
    0x80, 0x40, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x08, 0x78, 0x88, 0x78, 0x00, 
    0x80, 0x80, 0x80, 0xF0, 0x88, 0x88, 0xF0, 0x00, 0x00, 0x00, 0x78, 0x80, 0x80, 0x80, 0x78, 0x00, 
    0x08, 0x08, 0x08, 0x78, 0x88, 0x88, 0x78, 0x00, 0x00, 0x00, 0x70, 0x88, 0xF8, 0x80, 0x78, 0x00, 
    0x10, 0x28, 0x20, 0x70, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x78, 0x88, 0x78, 0x08, 0xF0, 0x00, 
    0x80, 0x80, 0x80, 0xF0, 0x88, 0x88, 0x88, 0x00, 0x00, 0x20, 0x00, 0x20, 0x20, 0x20, 0x20, 0x00, 
    0x10, 0x00, 0x10, 0x10, 0x10, 0x90, 0x60, 0x00, 0x80, 0x80, 0x90, 0xA0, 0xC0, 0xA0, 0x90, 0x00, 
    0x60, 0x20, 0x20, 0x20, 0x20, 0x20, 0x70, 0x00, 0x00, 0x00, 0xD8, 0xA8, 0xA8, 0xA8, 0x88, 0x00, 
    0x00, 0x00, 0xB0, 0xC8, 0x88, 0x88, 0x88, 0x00, 0x00, 0x00, 0x70, 0x88, 0x88, 0x88, 0x70, 0x00, 
    0x00, 0x00, 0xF0, 0x88, 0xF0, 0x80, 0x80, 0x00, 0x00, 0x00, 0x78, 0x88, 0x78, 0x08, 0x08, 0x00, 
    0x00, 0x00, 0xB0, 0xC8, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x78, 0x80, 0x70, 0x08, 0xF0, 0x00, 
    0x20, 0x20, 0xF8, 0x20, 0x20, 0x28, 0x10, 0x00, 0x00, 0x00, 0x88, 0x88, 0x88, 0x88, 0x70, 0x00, 
    0x00, 0x00, 0x88, 0x88, 0x88, 0x50, 0x20, 0x00, 0x00, 0x00, 0x88, 0x88, 0xA8, 0xA8, 0x50, 0x00, 
    0x00, 0x00, 0x88, 0x50, 0x20, 0x50, 0x88, 0x00, 0x00, 0x00, 0x88, 0x50, 0x20, 0x20, 0x40, 0x00, 
    0x00, 0x00, 0xF8, 0x10, 0x20, 0x40, 0xF8, 0x00, 0x30, 0x40, 0x40, 0x80, 0x40, 0x40, 0x30, 0x00, 
    0x20, 0x20, 0x20, 0x00, 0x20, 0x20, 0x20, 0x00, 0x60, 0x10, 0x10, 0x08, 0x10, 0x10, 0x60, 0x00, 
    0x08, 0x70, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    
};

/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
/**
 * @brief 得到文字
 * 
 * @param fontcode  文字编码
 * @param buf       文字buf
 * @return int 
 */
static int _get_font(bar_uint8_t fontcode , bar_uint8_t* buf)
{
    if( fontcode >= 0x20 && fontcode <= 0x7E )
    {
        memmove(buf , &_ASCII_5x7[(fontcode - 0x20)*8] , 8);
        return 1;
    }
    return 0;
}

static int _is_number(char c)
{
    if(c >= 0x30  && c <= 0x39)
        return 1;
    
    return 0;
}

/**
 * @brief 字符串是数字组成
 * 
 * @param str 字符串
 * @return int 
 */
static int _is_number_string(char* str)
{
    bar_uint32_t i  , ret = 0;
    for(i = 0 ; i < strlen(str) ; i++)
    {
        if(_is_number(str[i]))
        {
            ret = 1;
        }
        else
        {
            ret = 0;
            break;
        }
    }
    return ret;
}

static int _is_code93_or_39_char(char c)
{
    if((c >= 0x30  && c <= 0x39)||(c >= 65  && c <=90)||
        (c==32)||(c==36)||(c==37)||(c==42||(c==43)|(c>=45	&&	c<=47)))
        return 1;
    
    return 0;
}

static int _is_code93_or_39_string(char* str)
{
    int i  , ret = 0;
    for(i = 0 ; i < strlen(str) ; i++)
    {
        
        if(_is_code93_or_39_char(str[i]))
        {
            ret = 1;
        }
        else
        {
            ret = 0;
            break;
        }
    }
    return ret;
}

/**
 * @brief 设置黑点
 * 
 * @param p_row     行参数
 * @param point     点的位置
 * @return int 
 */
static int _set_point( _row_st* p_row , bar_uint16_t point)
{
    // if(NULL == p_row)
    //     return 0;
    
    // if(NULL == p_row->pdat)
    //     return 0;

    if(point > (p_row->row_byte << 3))
        return 0;

    p_row->pdat[ point >> 3 ] |= (0x01 << (7 - (point % 8)));

    return 1;
}

/**
 * @brief 设置白点
 * 
 * @param p_row 
 * @param point 
 * @return int 
 */
static int _reset_point( _row_st* p_row , bar_uint16_t point)
{

    // if(NULL == p_row)
    //     return 0;
    
    // if(point > (p_row->row_byte << 3))
    //     return 0;

    if(point > (p_row->row_byte << 3))
        return 0;

    p_row->pdat[ point >> 3 ] &= ~(0x01 << (7 - (point % 8)));

    return 1;
}

/**
 * @brief 得到起始行与结束行
 * 
 * @param p_barcode 
 * @return int 
 */
static int _get_bracode_scope(_code_st* p_barcode)
{
    bar_uint8_t offset = 0;
    if(NULL == p_barcode)
        return 0;

    if(p_barcode->p_code->ret_param.is_two_lines_data)
    {
        p_barcode->start = 0;
        p_barcode->end = 1;
        return 1;
    }

    switch (p_barcode->p_code->hri_type)
    {
    case GT_FAMILY_BARCODE_HRI_TYPE_NONE:
        p_barcode->start = BARCODE_BLANK_SIZE;
        p_barcode->end = p_barcode->p_code->ret_param.dot_h - BARCODE_BLANK_SIZE;
        break;
    case GT_FAMILY_BARCODE_HRI_TYPE_DOWN:
        p_barcode->start = BARCODE_BLANK_SIZE;
        p_barcode->end = p_barcode->p_code->ret_param.dot_h - BARCODE_BLANK_SIZE - FONT_SIZE_H;
        break;
    case GT_FAMILY_BARCODE_HRI_TYPE_UP:
        p_barcode->start = BARCODE_BLANK_SIZE + FONT_SIZE_H;
        p_barcode->end = p_barcode->p_code->ret_param.dot_h - BARCODE_BLANK_SIZE;
        break;
    case GT_FAMILY_BARCODE_HRI_TYPE_ALL:
        p_barcode->start = BARCODE_BLANK_SIZE + FONT_SIZE_H;
        p_barcode->end = p_barcode->p_code->ret_param.dot_h - BARCODE_BLANK_SIZE - FONT_SIZE_H;
        break;
    default:
        p_barcode->start = BARCODE_BLANK_SIZE;
        p_barcode->end = p_barcode->p_code->ret_param.dot_h - BARCODE_BLANK_SIZE - FONT_SIZE_H;
        break;
    }

    if( GT_FAMILY_BARCODE_TYPE_ITF == p_barcode->p_code->type)
    {
        offset = p_barcode->itf_bearer_bar;
    }
    p_barcode->start += offset;
    p_barcode->end -= offset;

    return 1;
}

/**
 * @brief 左侧空白区设置
 * 
 * @param p_barcode 
 * @param p_row 
 * @return int 
 */
static int _left_blank_none_set(_code_st * p_barcode , _row_st* p_row)
{
    bar_uint32_t i ;
    p_row->pos = 0;
    for(i = 0; i < p_barcode->left_blank_none ; i++)
    {
        if(0 == _reset_point(p_row , p_row->pos++))
            return 0;
    }
    return 1;
}

/**
 * @brief 右侧空白区设置
 * 
 * @param p_barcode 
 * @param p_row 
 * @return int 
 */
static int _right_blank_none_set(_code_st * p_barcode , _row_st* p_row)
{
    bar_uint32_t i ;

    for(i = 0 ; i < p_barcode->right_blank_none ; ++i)
    {
        if(0 == _reset_point(p_row , p_row->pos++))
            return 0;
    }

    return 1;
}

/**
 * @brief 起始符设置
 * 
 * @param p_barcode 
 * @param p_row 
 * @param dat 
 * @return int 
 */
static int _start_code_set(_code_st * p_barcode , _row_st* p_row , bar_uint32_t dat)
{
    bar_uint32_t i , j;
    for(i = 1 ; i <= p_barcode->start_code ; i++)
    {
        for(j = 0 ; j < p_barcode->p_code->mode_w ; j++)
        {
            if( 0 == ((dat>>(p_barcode->start_code-i))&0x0001 ? _set_point(p_row , p_row->pos++) : _reset_point(p_row , p_row->pos++)) )
                return 0;
        }
    }

    return 1;
}

/**
 * @brief 结束符设置
 * 
 * @param p_barcode 
 * @param p_row 
 * @param dat 
 * @return int 
 */
static int _end_code_set(_code_st * p_barcode , _row_st* p_row , bar_uint32_t dat)
{
    bar_uint32_t i , j;
    for(i = 1 ; i <= p_barcode->end_code ; i++)
    {
        for(j = 0 ; j < p_barcode->p_code->mode_w ; j++)
        {
            if( 0 == ((dat>>(p_barcode->end_code-i))&0x0001 ? _set_point(p_row , p_row->pos++) : _reset_point(p_row , p_row->pos++)) )
                return 0;
        }
    }

    return 1;
}

/**
 * @brief 中间间隔符设置
 * 
 * @param p_barcode 
 * @param p_row 
 * @param dat 
 * @return int 
 */
static int _separator_code_set(_code_st * p_barcode , _row_st* p_row , bar_uint32_t dat)
{
    bar_uint32_t i , j;
    for(i = 1 ; i <= p_barcode->separator ; i++)
    {
        for(j = 0 ; j < p_barcode->p_code->mode_w ; j++)
        {
            if( 0 == ((dat>>(p_barcode->separator-i))&0x0001 ? _set_point(p_row , p_row->pos++) : _reset_point(p_row , p_row->pos++)) )
                return 0;
        }
    }
    return 1;
}

/**
 * @brief 数据设置
 * 
 * @param p_barcode 
 * @param p_row 
 * @param dat 
 * @return int 
 */
static int _data_set(_code_st * p_barcode , _row_st* p_row , bar_uint32_t dat)
{
    bar_uint32_t i , j;
    for(i = 1; i <= p_barcode->one_code_size; i++)
    {
        for(j = 0 ; j < p_barcode->p_code->mode_w ; j++)
        {
            if( 0 == ((dat>>(p_barcode->one_code_size-i))&0x0001 ? _set_point(p_row , p_row->pos++) : _reset_point(p_row , p_row->pos++)) )
                return 0;
        }
    }
    return 1;
}

static int _2_of_5_data_set(_code_st * p_barcode , _row_st* p_row , bar_uint32_t dat1 , bar_uint32_t dat2)
{
    bar_uint32_t i , j  , max_count = 5;
    for(i = 1 ; i <= max_count ; i++)
    {
        // dat1
        for(j = 0 ; j < p_barcode->p_code->mode_w ; j++)
        {
            if((dat1 >> (max_count - i)) & 0x0001 )
            {
                if(!_set_point(p_row , p_row->pos++))
                    return 0;
                if(!_set_point(p_row , p_row->pos++))
                    return 0;
            }
            else
            {
                if(!_set_point(p_row , p_row->pos++))
                    return 0;
            }
        }
        // dat2
        for(j = 0 ; j < p_barcode->p_code->mode_w ; j++)
        {
            if((dat2 >> (max_count - i)) & 0x0001 )
            {
                if(!_reset_point(p_row , p_row->pos++))
                    return 0;
                if(!_reset_point(p_row , p_row->pos++))
                    return 0;
            }
            else
            {
                if(!_reset_point(p_row , p_row->pos++))
                    return 0;
            }
        }
    }
    return 1;
}

static int _codxx_data_set(_code_st * p_barcode , _row_st* p_row , bar_uint32_t dat1 , bar_uint8_t count)
{
    bar_uint32_t i , j ;
    
    int (*p_fun)(_row_st* , bar_uint16_t);

    for(i = 1 ; i <= count ; i++)
    {
        p_fun = (i % 2) ? _set_point : _reset_point ;

        if((dat1 >> (count-i)) & 0x0001)
        {
            for(j = 0 ; j < p_barcode->wide_module ; j++)
            {
                if(!p_fun(p_row , p_row->pos++))
                    return 0;
            }
        }
        else
        {
            for(j = 0 ; j < p_barcode->narrow_module ; j++)
            {
                if(!p_fun(p_row , p_row->pos++))
                    return 0;
            }
        }
    
    }
    return 1;
}

static int _interval_code_set(_code_st * p_barcode , _row_st* p_row)
{
    bar_uint32_t i ;

    for(i = 0 ; i < p_barcode->interval_code * p_barcode->narrow_module ; ++i)
    {
        if(0 == _reset_point(p_row , p_row->pos++))
            return 0;
    }

    return 1;
}

/**
 * @brief 文字设置
 * 
 * @param p_row 
 * @param w 
 * @param h 
 * @param font_buf 
 * @return int 
 */
static int _font_point_put(_row_st* p_row , bar_uint16_t w , bar_uint16_t h , bar_uint8_t *font_buf)
{
    int ret = 1;
    bar_uint32_t i , n , j ,pos = 0;
    pos = p_row->pos;

    for(i = 0 ; i < h; i++)
    {
        for(n = 0 ; n < (w + 7)/8 ; n++)
        {
            for(j = 0 ; j < 8 ;j++)
            {
                // 超出宽度不打点
                if((n+1)*j >= w)    
                    break;
                ret = (font_buf[i*((w + 7)/8) + n] << j) & 0x80 ? _set_point(p_row , pos++) : _reset_point(p_row , pos++);
                if(0 == ret)
                    return 0;
            }
        }
        pos = p_row->pos;
        p_row->pdat = p_row->pdat + p_row->row_byte;
    }

    return 1;
}

/**
 * @brief 得到字符串中的字符
 * 
 * @param str 
 * @param numb 
 * @return char 
 */
static char _get_string_char(char* str , bar_uint32_t numb)
{
    return (numb >= strlen(str)) ? 0 : str[numb] - 0x30;
}

static char _get_code93_or_39_string_index(char* str , bar_uint32_t numb)
{
    if(numb >= strlen(str))
        return 0;
    if(str[numb] >= 0x30 && str[numb] <= 0x39)
    {
        return str[numb] - 0x30;
    }
    else if(str[numb] >= 0x41 && str[numb] <= 0x5A)
    {
        //A-Z
        return str[numb] - 0x41 + 10;
    }
    else if( str[numb] >= 0x2D && str[numb] <= 0x2E )
    {
        //- .
        return  str[numb] - 0x2D + 36;
    }
    else if(str[numb] == 0x20)
    {
        //空格
        return  38;
    }
    else if(str[numb] == 0x24)
    {
        //$
        return 39;
    } 
    else if(str[numb] == 0x2F)
    {
        // /
        return 40;
    } 
    else if(str[numb] == 0x2B)
    { 
        // +
        return 41;
    }
    else if(str[numb] == 0x25)
    {
        // %
        return 42;
    }

    return 0;
}

/* ean13 ---------------------------------------------------------------*/
#if EAN13_BARCODE_EN
// static const bar_uint8_t _ean_mode_size[8] = { 11 , 3 , 42 , 5 , 35 , 7 , 3 , 7};
static const bar_uint8_t _ean13_logic_type[10] =  {0x3F, 0x34, 0x32, 0x31, 0x2C, 0x26, 0x23, 0x2A, 0x29, 0x25};
static const bar_uint8_t _ean13_code_type_a[10] = {0x0D, 0x19, 0x13, 0x3D, 0x23, 0x31, 0x2F, 0x3B, 0x37, 0x0B};
static const bar_uint8_t _ean13_code_type_b[10] = {0x27, 0x33, 0x1B, 0x21, 0x1D, 0x39, 0x05, 0x11, 0x09, 0x17};
static const bar_uint8_t _ean13_code_type_c[10] = {0x72, 0x66, 0x6C, 0x42, 0x5C, 0x4E, 0x50, 0x44, 0x48, 0x74};

/**
 * @brief ean13 文字设置
 * 
 * @param p_barcode 
 * @param p_row 
 * @param y 
 * @return int 
 */
static int _ean13_font_set(_code_st * p_barcode , _row_st* p_row ,bar_uint32_t y)
{
    bar_uint32_t i , w , h , pos = 0;
    bar_uint8_t temp_buf[16] , c = 0;

    w = 5;
    h = 8;

    // 超出显示区域
    if((y + h) > p_barcode->p_code->ret_param.dot_h)
        return 0;

    // 前置码
    c = _get_string_char(p_barcode->p_code->barcode , 0);
    if(!_get_font(c + 0x30 , temp_buf))
        return 0;
    p_row->pos = p_barcode->left_blank_none - w - p_barcode->p_code->mode_w;
    p_row->pdat = p_barcode->p_code->pdat + (y * p_row->row_byte);
    if(!_font_point_put(p_row , w , h , temp_buf))
        return 0;

    pos = p_row->pos;
    p_row->pos += p_barcode->start_code * p_barcode->p_code->mode_w + w + p_barcode->p_code->mode_w*2; 
    // 左数据
    for(i = 1; i <= 6; i++)
    {
        c = _get_string_char(p_barcode->p_code->barcode , i);
        if(!_get_font(c + 0x30 , temp_buf))
            return 0; 
        p_row->pdat = p_barcode->p_code->pdat + (y * p_row->row_byte);
        if(!_font_point_put(p_row , w , h , temp_buf))
            return 0;
        p_row->pos += p_barcode->left_data_code / p_barcode->one_code_size * p_barcode->p_code->mode_w + p_barcode->p_code->mode_w;

    }

    p_row->pos = pos + (p_barcode->left_data_code + p_barcode->separator) * p_barcode->p_code->mode_w;
    pos = p_row->pos;
    // 右数据 + 校验码
    for(i = 7 ; i <= 12;i++)
    {
        if(i == 12)
            c = p_barcode->p_code->ret_param.check_numb;
        else
            c = _get_string_char(p_barcode->p_code->barcode , i);
        p_row->pdat = p_barcode->p_code->pdat + (y * p_row->row_byte);
        if(!_get_font(c + 0x30 , temp_buf))
            return 0;
        if(!_font_point_put(p_row , w , h , temp_buf))
            return 0;
        p_row->pos += (p_barcode->right_data_code + p_barcode->check_code)/p_barcode->one_code_size * p_barcode->p_code->mode_w + p_barcode->p_code->mode_w;
    }

    return 1;
}

/**
 * @brief ean13 文字位置设置
 * 
 * @param p_barcode 
 * @param p_row 
 * @return int 
 */
static int _ean13_hri_type_set(_code_st * p_barcode , _row_st* p_row)
{
    bar_uint32_t i;
    // 上
    if( GT_FAMILY_BARCODE_HRI_TYPE_UP == p_barcode->p_code->hri_type || 
        GT_FAMILY_BARCODE_HRI_TYPE_ALL == p_barcode->p_code->hri_type)
    {
        p_row->pdat = p_barcode->p_code->pdat + ((p_barcode->start-BARCODE_BLANK_SIZE) * p_row->row_byte);
        p_row->pos = p_barcode->left_blank_none;
        if(!_start_code_set(p_barcode , p_row , 0x05))
            return 0;
        p_row->pos += p_barcode->left_data_code * p_barcode->p_code->mode_w;

        if(!_separator_code_set(p_barcode , p_row , 0x0A))
            return 0;
        p_row->pos += (p_barcode->right_data_code+p_barcode->check_code) * p_barcode->p_code->mode_w;
        if(!_end_code_set(p_barcode , p_row , 0x05))
            return 0;
        
        for(i = 1 ; i < BARCODE_BLANK_SIZE ; i++)
        {
            memmove(p_row->pdat + (i * p_row->row_byte) , p_row->pdat , p_row->row_byte);
        }

        if(!_ean13_font_set( p_barcode , p_row , BARCODE_BLANK_SIZE>>1))
            return 0;

    }
    // 下
    if( GT_FAMILY_BARCODE_HRI_TYPE_DOWN == p_barcode->p_code->hri_type ||
        GT_FAMILY_BARCODE_HRI_TYPE_ALL == p_barcode->p_code->hri_type)
    {
        p_row->pdat = p_barcode->p_code->pdat + (p_barcode->end * p_row->row_byte);

        p_row->pos = p_barcode->left_blank_none;
        if(!_start_code_set(p_barcode , p_row , 0x05))
            return 0;
        p_row->pos += p_barcode->left_data_code * p_barcode->p_code->mode_w;

        if(!_separator_code_set(p_barcode , p_row , 0x0A))
            return 0;
        p_row->pos += (p_barcode->right_data_code+p_barcode->check_code) * p_barcode->p_code->mode_w;
        if(!_end_code_set(p_barcode , p_row , 0x05))
            return 0;
        
        for(i = 1 ; i < BARCODE_BLANK_SIZE ; i++)
        {
            memmove(p_row->pdat + (i * p_row->row_byte) , p_row->pdat , p_row->row_byte);
        }

        if(!_ean13_font_set( p_barcode , p_row , p_barcode->end+(BARCODE_BLANK_SIZE>>1)))
            return 0;
    }

    return 1;
}

/**
 * @brief ean13 校验码设置
 * 
 * @param p_barcode 
 * @param p_row 
 * @return int 
 */
static int _ean13_check_code_set(_code_st * p_barcode , _row_st* p_row)
{
    bar_uint32_t i = 12 , c1 = 0 , c2 = 0 ,cc = 0 ;
    bar_uint8_t code_type = 0;

    while(i)
    {
        if(i%2)
        {
            --i;
            c1 += _get_string_char(p_barcode->p_code->barcode , i);
        }
        else
        {
            --i;
            c2 += _get_string_char(p_barcode->p_code->barcode , i);
        }
    }

    c2 = c2 * 3;
    cc = (c1 + c2) % 10;
    p_barcode->p_code->ret_param.check_numb = 10 - cc;
    if(10 == p_barcode->p_code->ret_param.check_numb)
        p_barcode->p_code->ret_param.check_numb = 0;
    
    code_type = _ean13_code_type_c[p_barcode->p_code->ret_param.check_numb];

    if(!_data_set(p_barcode , p_row , code_type))
        return 0;
    
    return 1;
}

/**
 * @brief ean13 左侧数据符设置
 * 
 * @param p_barcode 
 * @param p_row 
 * @return int 
 */
static int _ean13_left_data_code_set(_code_st * p_barcode , _row_st* p_row)
{
    bar_uint32_t c_n = 0 , tmp = 0;
    bar_uint8_t type = 0 , code_type = 0;
    char c = _get_string_char(p_barcode->p_code->barcode , c_n++);

    type = _ean13_logic_type[c];

    tmp = p_barcode->left_data_code / p_barcode->one_code_size;

    while (tmp)
    {
        --tmp;
        c = _get_string_char(p_barcode->p_code->barcode , c_n++);
        code_type = (type >> tmp) & 0x01 ? _ean13_code_type_a[c] : _ean13_code_type_b[c];

        if(!_data_set( p_barcode , p_row , code_type ))
            return 0;
    }
    return 1;
}

/**
 * @brief ean13 右侧数据符设置
 * 
 * @param p_barcode 
 * @param p_row 
 * @return int 
 */
static int _ean13_right_data_code_set(_code_st * p_barcode , _row_st* p_row)
{
    bar_uint32_t c_n = 7 , tmp = 0;
    bar_uint8_t code_type = 0;

    tmp = p_barcode->right_data_code / p_barcode->one_code_size;

    while (tmp)
    {
        --tmp;
        code_type = _ean13_code_type_c[ _get_string_char(p_barcode->p_code->barcode , c_n++) ];
        
        if(!_data_set( p_barcode , p_row , code_type ))
            return 0;
    }

    return 1;
}

/**
 * @brief ean13 条码处理
 * 
 * @param p_barcode 
 * @return int 
 */
static int _ean13_handler(_code_st * p_barcode)
{
    bar_uint32_t i;
    _row_st* p_row = &row_para;

    if(NULL == p_barcode)
        goto ERR_RET;
    
    _get_bracode_scope(p_barcode);

    memset(p_row , 0 , sizeof(_row_st));
    p_row->row_byte =  p_barcode->p_code->ret_param.dot_w >> 3 ;
    p_row->pos = 0;
    p_row->pdat = p_barcode->p_code->pdat + (p_barcode->start * p_row->row_byte);
    if(NULL == p_row->pdat)
        goto ERR_RET;
    
    // 左空白
    if(!_left_blank_none_set(p_barcode , p_row))
        goto ERR_RET;

    // 起始符
    if(!_start_code_set(p_barcode , p_row , 0x05))
        goto ERR_RET;

    // 左数据
    if(!_ean13_left_data_code_set(p_barcode , p_row))
        goto ERR_RET;

    // 间隔符
    if(!_separator_code_set(p_barcode , p_row , 0x0A))
        goto ERR_RET;

    // 右数据
    if(!_ean13_right_data_code_set(p_barcode , p_row))
        goto ERR_RET;

    // 校验符
    if(!_ean13_check_code_set(p_barcode , p_row))
        goto ERR_RET;

    // 结束符
    if(!_end_code_set(p_barcode , p_row , 0x05))
        goto ERR_RET;

    // 右空白
    if(!_right_blank_none_set(p_barcode , p_row))
        goto ERR_RET;

    // 第二行数据
    if(p_barcode->p_code->ret_param.is_two_lines_data)
    {
        p_row->pdat = p_barcode->p_code->pdat + ( p_barcode->end * p_row->row_byte );
        // 起始符
        p_row->pos = p_barcode->left_blank_none;
        if(!_start_code_set(p_barcode , p_row , 0x05))
            return 0;
        // 中间间隔符
        p_row->pos += p_barcode->left_data_code * p_barcode->p_code->mode_w;
        if(!_separator_code_set(p_barcode , p_row , 0x0A))
            return 0;
        // 结束符
        p_row->pos += (p_barcode->right_data_code+p_barcode->check_code) * p_barcode->p_code->mode_w;
        if(!_end_code_set(p_barcode , p_row , 0x05))
            return 0;
        return 1;
    }

    // 绘制范围类的数据
    for(i = p_barcode->start ; i < p_barcode->end;i++)
    {
        memmove(p_barcode->p_code->pdat + (i * p_row->row_byte) , p_row->pdat ,  p_row->row_byte);
    }

    // 文字
    if(!_ean13_hri_type_set(p_barcode , p_row))
        goto ERR_RET;

    return 1;
ERR_RET:
    p_barcode->p_code->ret_param.dot_w = 0;
    p_barcode->p_code->ret_param.dot_h = 0;
    return 0;
}

/**
 * @brief 得到 ean13条码
 * 
 * @param p_barcode 
 * @return int 
 */
static int _get_ean13(_code_st * p_barcode)
{
    bar_uint32_t temp = 0;
    if(NULL == p_barcode)
        goto ERR_RET;
    if( 0 == (p_barcode->len = strlen(p_barcode->p_code->barcode)))
        goto ERR_RET;
    if( !_is_number_string(p_barcode->p_code->barcode))
        goto ERR_RET;

    p_barcode->left_blank_none = 11;
    p_barcode->start_code = 3;
    p_barcode->left_data_code = 42;
    p_barcode->separator = 5;
    p_barcode->right_data_code = 35;
    p_barcode->check_code = 7;
    p_barcode->end_code = 3;
    p_barcode->right_blank_none = 7;
    temp = 113; // 实际值为上述变量相加

    p_barcode->one_code_size = 7;

    p_barcode->p_code->mode_w = p_barcode->p_code->mode_w ? p_barcode->p_code->mode_w : 2 ;
    
    // 计算显示宽高
    p_barcode->p_code->ret_param.dot_w =  ((p_barcode->p_code->mode_w * temp + 7 ) >> 3 ) << 3;
    p_barcode->p_code->ret_param.dot_h =  p_barcode->p_code->mode_h;
    // 限制高度 未达到特定高度传输两行数据
    if(p_barcode->p_code->mode_h < BARCODE_MIN_H)
    {
        p_barcode->p_code->ret_param.dot_h = 2;
        p_barcode->p_code->ret_param.is_two_lines_data = 1;
    }

    // 需要的字节数超出buf
    if((( p_barcode->p_code->ret_param.dot_w *  p_barcode->p_code->ret_param.dot_h) >> 3) >  p_barcode->p_code->pdat_size)
        return 0;

    p_barcode->left_blank_none = p_barcode->left_blank_none*p_barcode->p_code->mode_w;
    p_barcode->right_blank_none = p_barcode->right_blank_none*p_barcode->p_code->mode_w;
    temp = temp*p_barcode->p_code->mode_w;
    if( p_barcode->p_code->ret_param.dot_w > temp)
    {
        temp =  p_barcode->p_code->ret_param.dot_w - temp;
        p_barcode->left_blank_none += (temp - ((temp >> 1)));
        p_barcode->right_blank_none += (temp >> 1);
    }

    memset(p_barcode->p_code->pdat , 0 , p_barcode->p_code->pdat_size);

    // return _ean13_handler(p_barcode);
    return 1;


ERR_RET:
    p_barcode->p_code->ret_param.dot_w = 0;
    p_barcode->p_code->ret_param.dot_h = 0;
    return 0;
}
#endif

/* ean8 ---------------------------------------------------------------*/
#if EAN8_BARCODE_EN
// static const bar_uint8_t _ean8_logic_type[10] =  {0x3F, 0x34, 0x32, 0x31, 0x2C, 0x26, 0x23, 0x2A, 0x29, 0x25};
static const bar_uint8_t _ean8_code_type_a[10] = {0x0D, 0x19, 0x13, 0x3D, 0x23, 0x31, 0x2F, 0x3B, 0x37, 0x0B};
// static const bar_uint8_t _ean8_code_type_b[10] = {0x27, 0x33, 0x1B, 0x21, 0x1D, 0x39, 0x05, 0x11, 0x09, 0x17};
static const bar_uint8_t _ean8_code_type_c[10] = {0x72, 0x66, 0x6C, 0x42, 0x5C, 0x4E, 0x50, 0x44, 0x48, 0x74};

/**
 * @brief ean8 左侧数据符设置
 * 
 * @param p_barcode 
 * @param p_row 
 * @return int 
 */
static int _ean8_left_data_code_set(_code_st * p_barcode , _row_st* p_row)
{
    bar_uint32_t i , tmp = 0;
    bar_uint8_t  code_type = 0;
    char c ;

    tmp = p_barcode->left_data_code / p_barcode->one_code_size;

    for(i = 0 ; i < tmp ; i++)
    {
        c = _get_string_char(p_barcode->p_code->barcode ,i);
        code_type = _ean8_code_type_a[c];
        if(!_data_set( p_barcode , p_row , code_type ))
            return 0;
    }

    return 1;
}

/**
 * @brief ean8 右侧数据符设置
 * 
 * @param p_barcode 
 * @param p_row 
 * @return int 
 */
static int _ean8_right_data_code_set(_code_st * p_barcode , _row_st* p_row)
{
    bar_uint32_t  c_n = 4 , tmp = 0;
    bar_uint8_t code_type = 0;

    tmp = p_barcode->right_data_code / p_barcode->one_code_size;

    while (tmp)
    {
        --tmp;
        code_type = _ean8_code_type_c[ _get_string_char(p_barcode->p_code->barcode , c_n++) ];
        
        if(!_data_set( p_barcode , p_row , code_type ))
            return 0;
    }

    return 1;
}

/**
 * @brief ean8 校验码设置
 * 
 * @param p_barcode 
 * @param p_row 
 * @return int 
 */
static int _ean8_check_code_set(_code_st * p_barcode , _row_st* p_row)
{
    bar_uint32_t i  , c1 = 0 , c2 = 0 ,cc = 0 ;
    bar_uint8_t code_type = 0;

    for(i = 0 ; i < 8;i++)
    {
        if(i%2)
        {
            c1 += _get_string_char(p_barcode->p_code->barcode , i);
        }
        else
        {
            c2 += _get_string_char(p_barcode->p_code->barcode , i);
        }
    }

    c2 = c2 * 3;
    cc = (c1 + c2) % 10;
    p_barcode->p_code->ret_param.check_numb = 10 - cc;
    if(10 == p_barcode->p_code->ret_param.check_numb)
        p_barcode->p_code->ret_param.check_numb = 0;
    
    code_type = _ean8_code_type_c[p_barcode->p_code->ret_param.check_numb];

    if(!_data_set(p_barcode , p_row , code_type))
        return 0;
    
    return 1;
}

/**
 * @brief ean8 文字设置
 * 
 * @param p_barcode 
 * @param p_row 
 * @param y 
 * @return int 
 */
static int _ean8_font_set(_code_st * p_barcode , _row_st* p_row ,bar_uint32_t y)
{
    bar_uint32_t i , w , h , pos = 0;
    bar_uint8_t temp_buf[16] , c = 0;

    w = 5;
    h = 8;

    // 超出显示区域
    if((y + h) > p_barcode->p_code->ret_param.dot_h)
        return 0;

    p_row->pos = p_barcode->left_blank_none + p_barcode->start_code*p_barcode->p_code->mode_w + p_barcode->p_code->mode_w;
    pos = p_row->pos;
    // 左数据
    for(i = 0; i < 4; i++)
    {
        c = _get_string_char(p_barcode->p_code->barcode , i);
        if(!_get_font(c + 0x30 , temp_buf))
            return 0; 
        p_row->pdat = p_barcode->p_code->pdat + (y * p_row->row_byte);
        if(!_font_point_put(p_row , w , h , temp_buf))
            return 0;
        
        p_row->pos += p_barcode->one_code_size * p_barcode->p_code->mode_w;
    }

    p_row->pos = pos + (p_barcode->left_data_code + p_barcode->separator) * p_barcode->p_code->mode_w;
    pos = p_row->pos;
    // 右数据 + 校验码
    for(i = 4 ; i < 8;i++)
    {
        if(i == 7)
            c = p_barcode->p_code->ret_param.check_numb;
        else
            c = _get_string_char(p_barcode->p_code->barcode , i);
        p_row->pdat = p_barcode->p_code->pdat + (y * p_row->row_byte);
        if(!_get_font(c + 0x30 , temp_buf))
            return 0;
        if(!_font_point_put(p_row , w , h , temp_buf))
            return 0;
        p_row->pos += p_barcode->one_code_size * p_barcode->p_code->mode_w;
    }

    return 1;
}

/**
 * @brief ean8 文字位置设置
 * 
 * @param p_barcode 
 * @param p_row 
 * @return int 
 */
static int _ean8_hri_type_set(_code_st * p_barcode , _row_st* p_row)
{
    
    bar_uint32_t i;
    // 上
    if( GT_FAMILY_BARCODE_HRI_TYPE_UP == p_barcode->p_code->hri_type || 
        GT_FAMILY_BARCODE_HRI_TYPE_ALL == p_barcode->p_code->hri_type)
    {
        p_row->pdat = p_barcode->p_code->pdat + ((p_barcode->start-BARCODE_BLANK_SIZE) * p_row->row_byte);
        p_row->pos = p_barcode->left_blank_none;
        if(!_start_code_set(p_barcode , p_row , 0x05))
            return 0;
        p_row->pos += p_barcode->left_data_code * p_barcode->p_code->mode_w;

        if(!_separator_code_set(p_barcode , p_row , 0x0A))
            return 0;
        p_row->pos += (p_barcode->right_data_code+p_barcode->check_code) * p_barcode->p_code->mode_w;
        if(!_end_code_set(p_barcode , p_row , 0x05))
            return 0;
        
        for(i = 1 ; i < BARCODE_BLANK_SIZE ; i++)
        {
            memmove(p_row->pdat + (i * p_row->row_byte) , p_row->pdat , p_row->row_byte);
        }

        if(!_ean8_font_set( p_barcode , p_row , BARCODE_BLANK_SIZE>>1))
            return 0;
    }
    // 下
    if( GT_FAMILY_BARCODE_HRI_TYPE_DOWN == p_barcode->p_code->hri_type ||
        GT_FAMILY_BARCODE_HRI_TYPE_ALL == p_barcode->p_code->hri_type)
    {
        p_row->pdat = p_barcode->p_code->pdat + (p_barcode->end * p_row->row_byte);

        p_row->pos = p_barcode->left_blank_none;
        if(!_start_code_set(p_barcode , p_row , 0x05))
            return 0;

        p_row->pos += p_barcode->left_data_code * p_barcode->p_code->mode_w;
        if(!_separator_code_set(p_barcode , p_row , 0x0A))
            return 0;

        p_row->pos += (p_barcode->right_data_code+p_barcode->check_code) * p_barcode->p_code->mode_w;
        if(!_end_code_set(p_barcode , p_row , 0x05))
            return 0;

        for(i = 1 ; i < BARCODE_BLANK_SIZE ; i++)
        {
            memmove(p_row->pdat + (i * p_row->row_byte) , p_row->pdat , p_row->row_byte);
        }

        if(!_ean8_font_set( p_barcode , p_row , p_barcode->end+(BARCODE_BLANK_SIZE>>1)))
            return 0;
    }

    return 1;
}

/**
 * @brief ean8 条码处理
 * 
 * @param p_barcode 
 * @return int 
 */
static int _ean8_handler(_code_st * p_barcode)
{
    bar_uint32_t i;
    _row_st* p_row = &row_para;

    if(NULL == p_barcode)
        goto ERR_RET;
    
    _get_bracode_scope(p_barcode);

    memset(p_row , 0 , sizeof(_row_st));
    p_row->row_byte =  p_barcode->p_code->ret_param.dot_w >> 3 ;
    p_row->pos = 0;
    p_row->pdat = p_barcode->p_code->pdat + (p_barcode->start * p_row->row_byte);
    if(NULL == p_row->pdat)
        goto ERR_RET;

     // 左空白
    if(!_left_blank_none_set(p_barcode , p_row))
        goto ERR_RET;

    // 起始符
    if(!_start_code_set(p_barcode , p_row , 0x05))
        goto ERR_RET;

    // 左数据
    if(!_ean8_left_data_code_set(p_barcode , p_row))
        goto ERR_RET;

    // 间隔符
    if(!_separator_code_set(p_barcode , p_row , 0x0A))
        goto ERR_RET;

    // 右数据
    if(!_ean8_right_data_code_set(p_barcode , p_row))
        goto ERR_RET;

    // 校验符
    if(!_ean8_check_code_set(p_barcode , p_row))
        goto ERR_RET;

    // 结束符
    if(!_end_code_set(p_barcode , p_row , 0x05))
        goto ERR_RET;

    // 右空白
    if(!_right_blank_none_set(p_barcode , p_row))
        goto ERR_RET;

    if(p_barcode->p_code->ret_param.is_two_lines_data)
    {
        p_row->pdat = p_barcode->p_code->pdat + (p_barcode->end * p_row->row_byte);
        // 起始符
        p_row->pos = p_barcode->left_blank_none;
        if(!_start_code_set(p_barcode , p_row , 0x05))
            return 0;
        // 中间间隔符
        p_row->pos += p_barcode->left_data_code * p_barcode->p_code->mode_w;
        if(!_separator_code_set(p_barcode , p_row , 0x0A))
            return 0;
        // 结束符
        p_row->pos += (p_barcode->right_data_code+p_barcode->check_code) * p_barcode->p_code->mode_w;
        if(!_end_code_set(p_barcode , p_row , 0x05))
            return 0;

        return 1;
    }

    // 绘制范围类的数据
    for(i = p_barcode->start ; i < p_barcode->end;i++)
    {
        memmove(p_barcode->p_code->pdat + (i * p_row->row_byte) , p_row->pdat ,  p_row->row_byte);
    }

    // 文字
    if(!_ean8_hri_type_set(p_barcode , p_row))
        goto ERR_RET;

    return 1;
ERR_RET:
    p_barcode->p_code->ret_param.dot_w = 0;
    p_barcode->p_code->ret_param.dot_h = 0;
    return 0;
}

/**
 * @brief 得到 ean8 条码
 * 
 * @param p_barcode 
 * @return int 
 */
static int _get_ean8(_code_st * p_barcode)
{
    bar_uint32_t temp = 0;
    if(NULL == p_barcode)
        goto ERR_RET;
    if( 0 == (p_barcode->len = strlen(p_barcode->p_code->barcode)))
        goto ERR_RET;
    if( !_is_number_string(p_barcode->p_code->barcode))
        goto ERR_RET;

    p_barcode->left_blank_none = 7;
    p_barcode->start_code = 3;
    p_barcode->left_data_code = 28;
    p_barcode->separator = 5;
    p_barcode->right_data_code = 21;
    p_barcode->check_code = 7;
    p_barcode->end_code = 3;
    p_barcode->right_blank_none = 7;
    temp = 81; // 实际值为上述变量相加

    p_barcode->one_code_size = 7;

    p_barcode->p_code->mode_w = p_barcode->p_code->mode_w ? p_barcode->p_code->mode_w : 2 ;
    
    // 计算显示宽高
    p_barcode->p_code->ret_param.dot_w =  ((p_barcode->p_code->mode_w * temp + 7 ) >> 3 ) << 3;
    p_barcode->p_code->ret_param.dot_h = p_barcode->p_code->mode_h;
    
    if(p_barcode->p_code->mode_h < BARCODE_MIN_H)
    {
        p_barcode->p_code->ret_param.dot_h = 2;
        p_barcode->p_code->ret_param.is_two_lines_data = 1;
    }

    // 需要的字节数超出buf
    if((( p_barcode->p_code->ret_param.dot_w *  p_barcode->p_code->ret_param.dot_h) >> 3) >  p_barcode->p_code->pdat_size)
        return 0;

    p_barcode->left_blank_none = p_barcode->left_blank_none*p_barcode->p_code->mode_w;
    p_barcode->right_blank_none = p_barcode->right_blank_none*p_barcode->p_code->mode_w;
    temp = temp*p_barcode->p_code->mode_w;
    if( p_barcode->p_code->ret_param.dot_w > temp)
    {
        temp =  p_barcode->p_code->ret_param.dot_w - temp;
        p_barcode->left_blank_none += (temp - ((temp >> 1)));
        p_barcode->right_blank_none += (temp >> 1);
    }

    memset(p_barcode->p_code->pdat , 0 , p_barcode->p_code->pdat_size );

    // return _ean8_handler(p_barcode);
    return 1;

ERR_RET:
    p_barcode->p_code->ret_param.dot_w = 0;
    p_barcode->p_code->ret_param.dot_h = 0;
    return 0;
}

#endif

/* UPC-A ---------------------------------------------------------------*/
#if UPC_A_BARCODE_EN

// static const bar_uint8_t _upca_logic_type[10] =  {0x3F, 0x34, 0x32, 0x31, 0x2C, 0x26, 0x23, 0x2A, 0x29, 0x25};
static const bar_uint8_t _upca_code_type_a[10] = {0x0D, 0x19, 0x13, 0x3D, 0x23, 0x31, 0x2F, 0x3B, 0x37, 0x0B};
// static const bar_uint8_t _ean8_code_type_b[10] = {0x27, 0x33, 0x1B, 0x21, 0x1D, 0x39, 0x05, 0x11, 0x09, 0x17};
static const bar_uint8_t _upca_code_type_c[10] = {0x72, 0x66, 0x6C, 0x42, 0x5C, 0x4E, 0x50, 0x44, 0x48, 0x74};

static int _upca_left_data_code_set(_code_st * p_barcode , _row_st* p_row)
{
    bar_uint32_t i , tmp = 0;
    bar_uint8_t code_type = 0;
    char c ;
    bar_uint32_t len = 0;

    tmp = p_barcode->left_data_code / p_barcode->one_code_size;

    len = p_barcode->len < 11 ? 11 - p_barcode->len : 0 ;
    
    for(i = 0 ; i < tmp ; i++)
    {
        if(i < len)
        {
            c = 0 ;
        }
        else
        {
            c = _get_string_char(p_barcode->p_code->barcode ,i - len);
        }
        code_type = _upca_code_type_a[c];
        if(!_data_set( p_barcode , p_row , code_type ))
            return 0;
    }

    return 1;
}

static int _upca_right_data_code_set(_code_st * p_barcode , _row_st* p_row)
{
    bar_uint32_t i , c_n = 6 , tmp = 0;
    bar_uint8_t  code_type = 0;
    char c ;
    bar_uint32_t len = 0;
    tmp = p_barcode->right_data_code / p_barcode->one_code_size;
    
    len = p_barcode->len < 11 ? 11 - p_barcode->len : 0 ;

    for(i = c_n; i < tmp+c_n ; i++)
    {
        if(i < len)
        {
            c = 0 ;
        }
        else
        {
            c = _get_string_char(p_barcode->p_code->barcode ,i - len);
        }
        code_type = _upca_code_type_c[c];
        if(!_data_set( p_barcode , p_row , code_type ))
            return 0;
    }
    return 1;
}

static int _upca_check_code_set(_code_st * p_barcode , _row_st* p_row)
{
    bar_uint32_t i = 11 , c1 = 0 , c2 = 0 ,cc = 0 ;
    bar_uint8_t code_type = 0;
    bar_uint32_t len = 0;

    len = p_barcode->len < 11 ? 11 - p_barcode->len : 0 ;

    for(i = 0 ; i < 11 ; i++)
    {
        if(i < len) 
            continue;
        if(i%2)
        {
            c2 += _get_string_char(p_barcode->p_code->barcode , i-len);
        }
        else
        {
            c1 += _get_string_char(p_barcode->p_code->barcode , i-len);
        }
    }

    c1 = c1 * 3;
    cc = (c1 + c2) % 10;
    p_barcode->p_code->ret_param.check_numb = 10 - cc;
    if(10 == p_barcode->p_code->ret_param.check_numb)
        p_barcode->p_code->ret_param.check_numb = 0;
    
    code_type = _upca_code_type_c[p_barcode->p_code->ret_param.check_numb];

    if(!_data_set(p_barcode , p_row , code_type))
        return 0;
    
    return 1;
}

static int _upca_font_set(_code_st * p_barcode , _row_st* p_row ,bar_uint32_t y)
{
    bar_uint32_t i , w , h , pos = 0;
    bar_uint8_t temp_buf[16] , c = 0;
    bar_uint32_t len = 0;

    len = p_barcode->len < 11 ? 11 - p_barcode->len : 0 ;

    w = 5;
    h = 8;

    // 超出显示区域
    if((y + h) > p_barcode->p_code->ret_param.dot_h)
        return 0;

    // 系统码
    c = 0 < len ? 0 : _get_string_char(p_barcode->p_code->barcode , 0);
    if(!_get_font(c + 0x30 , temp_buf))
        return 0;
    p_row->pos = p_barcode->left_blank_none - w - p_barcode->p_code->mode_w;
    p_row->pdat = p_barcode->p_code->pdat + (y * p_row->row_byte);
    if(!_font_point_put(p_row , w , h , temp_buf))
        return 0;

    p_row->pos = p_barcode->left_blank_none + (p_barcode->start_code+p_barcode->one_code_size)*p_barcode->p_code->mode_w + p_barcode->p_code->mode_w;
    pos = p_row->pos;
    // 左数据
    for(i = 1; i < 6; i++)
    {
        c = i < len ? 0 : _get_string_char(p_barcode->p_code->barcode , i-len);
        if(!_get_font(c + 0x30 , temp_buf))
            return 0; 
        p_row->pdat = p_barcode->p_code->pdat + (y * p_row->row_byte);
        if(!_font_point_put(p_row , w , h , temp_buf))
            return 0;
        
        p_row->pos += p_barcode->one_code_size * p_barcode->p_code->mode_w;
    }

    p_row->pos = pos + (p_barcode->left_data_code + p_barcode->separator) * p_barcode->p_code->mode_w - (p_barcode->one_code_size * p_barcode->p_code->mode_w);
    pos = p_row->pos;
    // 右数据
    for(i = 6 ; i < 11;i++)
    {
        c = i < len ? 0 : _get_string_char(p_barcode->p_code->barcode , i-len);
        p_row->pdat = p_barcode->p_code->pdat + (y * p_row->row_byte);
        if(!_get_font(c + 0x30 , temp_buf))
            return 0;
        if(!_font_point_put(p_row , w , h , temp_buf))
            return 0;
        p_row->pos += p_barcode->one_code_size * p_barcode->p_code->mode_w;
    }

    p_row->pos = pos + (p_barcode->right_data_code + p_barcode->check_code+p_barcode->end_code)*p_barcode->p_code->mode_w + p_barcode->p_code->mode_w;
    pos = p_row->pos;
    p_row->pdat = p_barcode->p_code->pdat + (y * p_row->row_byte);
    // 校验码
    c = p_barcode->p_code->ret_param.check_numb;
    if(!_get_font(c + 0x30 , temp_buf))
        return 0;
    if(!_font_point_put(p_row , w , h , temp_buf))
        return 0;

    return 1;
}

static int _upca_hri_type_set(_code_st * p_barcode , _row_st* p_row)
{
    
    bar_uint32_t i , len = 0;
    bar_uint8_t code_type , c = 0;
    len = p_barcode->len < 11 ? 11 - p_barcode->len : 0 ;

    // 上
    if( GT_FAMILY_BARCODE_HRI_TYPE_UP == p_barcode->p_code->hri_type ||
        GT_FAMILY_BARCODE_HRI_TYPE_ALL == p_barcode->p_code->hri_type)
    {
        p_row->pdat = p_barcode->p_code->pdat + ((p_barcode->start-BARCODE_BLANK_SIZE) * p_row->row_byte);
        p_row->pos = p_barcode->left_blank_none;
        if(!_start_code_set(p_barcode , p_row , 0x05))
            return 0;
        // 第一个字符
        c = 0 < len ? 0 : _get_string_char(p_barcode->p_code->barcode , 0);
        code_type = _upca_code_type_a[c];
        if(!_data_set( p_barcode , p_row , code_type ))
            return 0;
        p_row->pos -= p_barcode->one_code_size * p_barcode->p_code->mode_w;
        // 
        p_row->pos += p_barcode->left_data_code * p_barcode->p_code->mode_w;
        if(!_separator_code_set(p_barcode , p_row , 0x0A))
            return 0;

        // 校验符
        p_row->pos += (p_barcode->right_data_code) * p_barcode->p_code->mode_w;
        c = p_barcode->p_code->ret_param.check_numb;
        code_type = _upca_code_type_c[c];
        if(!_data_set( p_barcode , p_row , code_type ))
            return 0;

        // p_row->pos += (p_barcode->right_data_code+p_barcode->check_code) * p_barcode->p_code->mode_w;
        if(!_end_code_set(p_barcode , p_row , 0x05))
            return 0;

        for(i = 1 ; i < BARCODE_BLANK_SIZE ; i++)
        {
            memmove(p_row->pdat + (i * p_row->row_byte) , p_row->pdat , p_row->row_byte);
        }

        if(!_upca_font_set( p_barcode , p_row , BARCODE_BLANK_SIZE>>1))
            return 0;
    }
    // 下
    if( GT_FAMILY_BARCODE_HRI_TYPE_DOWN == p_barcode->p_code->hri_type ||
        GT_FAMILY_BARCODE_HRI_TYPE_ALL == p_barcode->p_code->hri_type)
    {
        p_row->pdat = p_barcode->p_code->pdat + (p_barcode->end * p_row->row_byte);

        p_row->pos = p_barcode->left_blank_none;
        if(!_start_code_set(p_barcode , p_row , 0x05))
            return 0;

        // 第一个字符
        c = 0 < len ? 0 : _get_string_char(p_barcode->p_code->barcode , 0);
        code_type = _upca_code_type_a[c];
        if(!_data_set( p_barcode , p_row , code_type ))
            return 0;
        p_row->pos -= p_barcode->one_code_size * p_barcode->p_code->mode_w;
        // 
        p_row->pos += p_barcode->left_data_code * p_barcode->p_code->mode_w;
        if(!_separator_code_set(p_barcode , p_row , 0x0A))
            return 0;

        // 校验符
        p_row->pos += (p_barcode->right_data_code) * p_barcode->p_code->mode_w;
        c = p_barcode->p_code->ret_param.check_numb;
        code_type = _upca_code_type_c[c];
        if(!_data_set( p_barcode , p_row , code_type ))
            return 0;

        // p_row->pos += (p_barcode->right_data_code+p_barcode->check_code) * p_barcode->p_code->mode_w;
        if(!_end_code_set(p_barcode , p_row , 0x05))
            return 0;

        for(i = 1 ; i < BARCODE_BLANK_SIZE ; i++)
        {
            memmove(p_row->pdat + (i * p_row->row_byte) , p_row->pdat , p_row->row_byte);
        }

        if(!_upca_font_set( p_barcode , p_row , p_barcode->end+(BARCODE_BLANK_SIZE>>1)))
            return 0;
    }

    return 1;
}

static int _upc_a_handler(_code_st* p_barcode)
{
    bar_uint32_t i , len = 0;
    _row_st* p_row = &row_para;
    bar_uint8_t code_type , c = 0;
    len = p_barcode->len < 11 ? 11 - p_barcode->len : 0 ;

    if(NULL == p_barcode)
        goto ERR_RET;
    
    _get_bracode_scope(p_barcode);

    memset(p_row , 0 , sizeof(_row_st));
    p_row->row_byte =  p_barcode->p_code->ret_param.dot_w >> 3 ;
    p_row->pos = 0;
    p_row->pdat = p_barcode->p_code->pdat + (p_barcode->start * p_row->row_byte);
    if(NULL == p_row->pdat)
        goto ERR_RET;

    // 左空白
    if(!_left_blank_none_set(p_barcode , p_row))
        goto ERR_RET;

    // 起始符
    if(!_start_code_set(p_barcode , p_row , 0x05))
        goto ERR_RET;

    // 左数据
    if(!_upca_left_data_code_set(p_barcode , p_row))
        goto ERR_RET;

    // 间隔符
    if(!_separator_code_set(p_barcode , p_row , 0x0A))
        goto ERR_RET;

    // 右数据
    if(!_upca_right_data_code_set(p_barcode , p_row))
        goto ERR_RET;

    // 校验符
    if(!_upca_check_code_set(p_barcode , p_row))
        goto ERR_RET;

    // 结束符
    if(!_end_code_set(p_barcode , p_row , 0x05))
        goto ERR_RET;

    // 右空白
    if(!_right_blank_none_set(p_barcode , p_row))
        goto ERR_RET;
    
    // 第二行数据
    if(p_barcode->p_code->ret_param.is_two_lines_data)
    {
        p_row->pdat = p_barcode->p_code->pdat + ( p_barcode->end * p_row->row_byte );

        p_row->pos = p_barcode->left_blank_none;
        if(!_start_code_set(p_barcode , p_row , 0x05))
            return 0;
        // 第一个字符
        c = 0 < len ? 0 : _get_string_char(p_barcode->p_code->barcode , 0);
        code_type = _upca_code_type_a[c];
        if(!_data_set( p_barcode , p_row , code_type ))
            return 0;
        p_row->pos -= p_barcode->one_code_size * p_barcode->p_code->mode_w;
        // 
        p_row->pos += p_barcode->left_data_code * p_barcode->p_code->mode_w;
        if(!_separator_code_set(p_barcode , p_row , 0x0A))
            return 0;

        // 校验符
        p_row->pos += (p_barcode->right_data_code) * p_barcode->p_code->mode_w;
        c = p_barcode->p_code->ret_param.check_numb;
        code_type = _upca_code_type_c[c];
        if(!_data_set( p_barcode , p_row , code_type ))
            return 0;

        if(!_end_code_set(p_barcode , p_row , 0x05))
            return 0;
        
        return 1;
    }
    // 绘制范围类的数据
    for(i = p_barcode->start ; i < p_barcode->end;i++)
    {
        memmove(p_barcode->p_code->pdat + (i * p_row->row_byte) , p_row->pdat ,  p_row->row_byte);
    }

    // 文字
    if(!_upca_hri_type_set(p_barcode , p_row))
        goto ERR_RET;

    return 1;
ERR_RET:
    p_barcode->p_code->ret_param.dot_w = 0;
    p_barcode->p_code->ret_param.dot_h = 0;
    return 0;
}

static int _get_upc_a(_code_st* p_barcode)
{
    bar_uint32_t temp = 0;
    if(NULL == p_barcode)
        goto ERR_RET;
    if( 0 == (p_barcode->len = strlen(p_barcode->p_code->barcode)))
        goto ERR_RET;
    if( !_is_number_string(p_barcode->p_code->barcode))
        goto ERR_RET;

    p_barcode->left_blank_none = 9;
    p_barcode->start_code = 3;
    p_barcode->left_data_code = 42;
    p_barcode->separator = 5;
    p_barcode->right_data_code = 35;
    p_barcode->check_code = 7;
    p_barcode->end_code = 3;
    p_barcode->right_blank_none = 9;
    temp = 113; // 实际值为上述变量相加

    p_barcode->one_code_size = 7;

    p_barcode->p_code->mode_w = p_barcode->p_code->mode_w ? p_barcode->p_code->mode_w : 2 ;
    
    // 计算显示宽高
    p_barcode->p_code->ret_param.dot_w =  ((p_barcode->p_code->mode_w * temp + 7 ) >> 3 ) << 3;
    p_barcode->p_code->ret_param.dot_h =  p_barcode->p_code->mode_h;
    // 限制高度 未达到特定高度传输两行数据
    if(p_barcode->p_code->mode_h < BARCODE_MIN_H)
    {
        p_barcode->p_code->ret_param.dot_h = 2;
        p_barcode->p_code->ret_param.is_two_lines_data = 1;
    }

    // 需要的字节数超出buf
    if((( p_barcode->p_code->ret_param.dot_w *  p_barcode->p_code->ret_param.dot_h) >> 3) >  p_barcode->p_code->pdat_size)
        return 0;

    p_barcode->left_blank_none = p_barcode->left_blank_none*p_barcode->p_code->mode_w;
    p_barcode->right_blank_none = p_barcode->right_blank_none*p_barcode->p_code->mode_w;
    temp = temp*p_barcode->p_code->mode_w;
    if( p_barcode->p_code->ret_param.dot_w > temp)
    {
        temp =  p_barcode->p_code->ret_param.dot_w - temp;
        p_barcode->left_blank_none += (temp - ((temp >> 1)));
        p_barcode->right_blank_none += (temp >> 1);
    }

    memset(p_barcode->p_code->pdat , 0 , p_barcode->p_code->pdat_size);

    // return _upc_a_handler(p_barcode);
    return 1;

ERR_RET:
    p_barcode->p_code->ret_param.dot_w = 0;
    p_barcode->p_code->ret_param.dot_h = 0;
    return 0;
}
#endif

/* UPC-E ---------------------------------------------------------------*/
#if UPC_E_BARCODE_EN
static const bar_uint8_t _upce_logic_type[10] =  {0x07, 0x0B, 0x0D, 0x0E, 0x13, 0x19, 0x1C, 0x15, 0x16, 0x1A};
static const bar_uint8_t _upce_code_type_a[10] = {0x0D, 0x19, 0x13, 0x3D, 0x23, 0x31, 0x2F, 0x3B, 0x37, 0x0B};
static const bar_uint8_t _upce_code_type_b[10] = {0x27, 0x33, 0x1B, 0x21, 0x1D, 0x39, 0x05, 0x11, 0x09, 0x17};
// static const bar_uint8_t _upce_code_type_c[10] = {0x72, 0x66, 0x6C, 0x42, 0x5C, 0x4E, 0x50, 0x44, 0x48, 0x74};

static int _upce_get_check_numb(_code_st * p_barcode)
{
    bar_uint32_t i , c1 = 0 , c2 = 0 , cc = 0;

    c1 = p_barcode->p_code->upc_e_sys;

    for(i = 0 ; i < 6 ; i++)
    {
        if(i % 2)
        {
            c1 += _get_string_char(p_barcode->p_code->barcode , i);
        } 
        else
        {
            c2 += _get_string_char(p_barcode->p_code->barcode , i);
        }
    }
    c1 = c1 * 3;
    cc = (c1 + c2) % 10;

    p_barcode->p_code->ret_param.check_numb = (10 - cc)%10;

    return 1;
}

static int _upce_data_code_set(_code_st * p_barcode , _row_st* p_row)
{
    bar_uint32_t i ,  tmp = 0;
    bar_uint8_t type = 0 , code_type = 0 , c = 0;
    
    // 计算check numb
    _upce_get_check_numb(p_barcode);

    type = _upce_logic_type[p_barcode->p_code->ret_param.check_numb];
    type = p_barcode->p_code->upc_e_sys ? ~type : type;
    tmp = p_barcode->left_data_code / p_barcode->one_code_size;

    for(i = 0 ; i < tmp; i++)
    {
        c = _get_string_char(p_barcode->p_code->barcode , i);
        code_type = (type >> (tmp - (i+1))) & 0x01 ? _upce_code_type_a[c] : _upce_code_type_b[c];

        if(!_data_set( p_barcode , p_row , code_type ))
            return 0;
    }

    return 1;
}

static int _upce_font_set(_code_st * p_barcode , _row_st* p_row ,bar_uint32_t y)
{
    bar_uint32_t i , w , h , pos = 0;
    bar_uint8_t temp_buf[16] , c = 0;

    w = 5;
    h = 8;

    // 超出显示区域
    if((y + h) > p_barcode->p_code->ret_param.dot_h)
        return 0;

    // 系统码
    p_row->pos = p_barcode->left_blank_none - w - p_barcode->p_code->mode_w;
    p_row->pdat = p_barcode->p_code->pdat + (y * p_row->row_byte);
    c = p_barcode->p_code->upc_e_sys;
    if(!_get_font(c + 0x30 , temp_buf))
        return 0;
    if(!_font_point_put(p_row , w , h , temp_buf))
        return 0;

    p_row->pos += p_barcode->start_code * p_barcode->p_code->mode_w + w + p_barcode->p_code->mode_w*2; 
    pos = p_row->pos;
    // 左数据
    for(i = 0; i < 6; i++)
    {
        p_row->pdat = p_barcode->p_code->pdat + (y * p_row->row_byte);
        c = _get_string_char(p_barcode->p_code->barcode , i);
        if(!_get_font(c + 0x30 , temp_buf))
            return 0; 
        if(!_font_point_put(p_row , w , h , temp_buf))
            return 0;
        p_row->pos += p_barcode->left_data_code / p_barcode->one_code_size * p_barcode->p_code->mode_w + p_barcode->p_code->mode_w;

    }
    // 校验码
    p_row->pdat = p_barcode->p_code->pdat + (y * p_row->row_byte);
    p_row->pos = pos + (p_barcode->left_data_code + p_barcode->end_code) * p_barcode->p_code->mode_w;
    pos = p_row->pos;
    c = p_barcode->p_code->ret_param.check_numb;
    if(!_get_font(c + 0x30 , temp_buf))
        return 0;
    if(!_font_point_put(p_row , w , h , temp_buf))
        return 0;

    return 1;
}

static int _upce_hri_type_set(_code_st * p_barcode , _row_st* p_row)
{
    bar_uint32_t i;
    // 上
    if( GT_FAMILY_BARCODE_HRI_TYPE_UP == p_barcode->p_code->hri_type || 
        GT_FAMILY_BARCODE_HRI_TYPE_ALL == p_barcode->p_code->hri_type)
    {
        p_row->pdat = p_barcode->p_code->pdat + ((p_barcode->start-BARCODE_BLANK_SIZE) * p_row->row_byte);
        p_row->pos = p_barcode->left_blank_none;
        if(!_start_code_set(p_barcode , p_row , 0x05))
            return 0;
        p_row->pos += p_barcode->left_data_code * p_barcode->p_code->mode_w;

        if(!_end_code_set(p_barcode , p_row , 0x15))
            return 0;
        
        for(i = 1 ; i < BARCODE_BLANK_SIZE ; i++)
        {
            memmove(p_row->pdat + (i * p_row->row_byte) , p_row->pdat , p_row->row_byte);
        }

        if(!_upce_font_set( p_barcode , p_row , BARCODE_BLANK_SIZE>>1))
            return 0;

    }
    // 下
    if( GT_FAMILY_BARCODE_HRI_TYPE_DOWN == p_barcode->p_code->hri_type ||
        GT_FAMILY_BARCODE_HRI_TYPE_ALL == p_barcode->p_code->hri_type)
    {
        p_row->pdat = p_barcode->p_code->pdat + (p_barcode->end * p_row->row_byte);

        p_row->pos = p_barcode->left_blank_none;
        if(!_start_code_set(p_barcode , p_row , 0x05))
            return 0;
        
        p_row->pos += p_barcode->left_data_code * p_barcode->p_code->mode_w;
        if(!_end_code_set(p_barcode , p_row , 0x15))
            return 0;
        
        for(i = 1 ; i < BARCODE_BLANK_SIZE ; i++)
        {
            memmove(p_row->pdat + (i * p_row->row_byte) , p_row->pdat , p_row->row_byte);
        }

        if(!_upce_font_set( p_barcode , p_row , p_barcode->end+(BARCODE_BLANK_SIZE>>1)))
            return 0;
    }

    return 1;
}

static int _upce_handler(_code_st * p_barcode)
{
    bar_uint32_t i;
    _row_st* p_row = &row_para;

    if(NULL == p_barcode)
        goto ERR_RET;
    
    _get_bracode_scope(p_barcode);

    memset(p_row , 0 , sizeof(_row_st));
    p_row->row_byte =  p_barcode->p_code->ret_param.dot_w >> 3 ;
    p_row->pos = 0;
    p_row->pdat = p_barcode->p_code->pdat + (p_barcode->start * p_row->row_byte);
    if(NULL == p_row->pdat)
        goto ERR_RET;
    
    // 左空白
    if(!_left_blank_none_set(p_barcode , p_row))
        goto ERR_RET;

    // 起始符
    if(!_start_code_set(p_barcode , p_row , 0x05))
        goto ERR_RET;

    // 数据
    if(!_upce_data_code_set(p_barcode , p_row))
        goto ERR_RET;
    
    // 结束符
    if(!_end_code_set(p_barcode , p_row , 0x15))
        goto ERR_RET;

    // 右空白
    if(!_right_blank_none_set(p_barcode , p_row))
        goto ERR_RET;
    
    // 第二行数据
    if(p_barcode->p_code->ret_param.is_two_lines_data)
    {
        p_row->pdat = p_barcode->p_code->pdat + ( p_barcode->end * p_row->row_byte );

        p_row->pos = p_barcode->left_blank_none;
        if(!_start_code_set(p_barcode , p_row , 0x05))
            return 0;
        
        p_row->pos += p_barcode->left_data_code * p_barcode->p_code->mode_w;
        if(!_end_code_set(p_barcode , p_row , 0x15))
            return 0;
        return 1;
    }

    // 绘制范围类的数据
    for(i = p_barcode->start ; i < p_barcode->end;i++)
    {
        memmove(p_barcode->p_code->pdat + (i * p_row->row_byte) , p_row->pdat ,  p_row->row_byte);
    }

    // 文字
    if(!_upce_hri_type_set(p_barcode , p_row))
        goto ERR_RET;

    return 1;
ERR_RET:
    p_barcode->p_code->ret_param.dot_w = 0;
    p_barcode->p_code->ret_param.dot_h = 0;
    return 0;
}

static int _get_upc_e(_code_st* p_barcode)
{
    bar_uint32_t temp = 0;
    if(NULL == p_barcode)
        goto ERR_RET;
    if( 0 == (p_barcode->len = strlen(p_barcode->p_code->barcode)))
        goto ERR_RET;
    if( !_is_number_string(p_barcode->p_code->barcode))
        goto ERR_RET;

    p_barcode->left_blank_none = 9;
    p_barcode->start_code = 3;
    p_barcode->left_data_code = 42;
    p_barcode->separator = 0;
    p_barcode->right_data_code = 0;
    p_barcode->check_code = 0;
    p_barcode->end_code = 6;
    p_barcode->right_blank_none = 9;
    temp = 69; // 实际值为上述变量相加
    
    p_barcode->one_code_size = 7;

    // 系统字符
    p_barcode->p_code->upc_e_sys = p_barcode->p_code->upc_e_sys ? 1 : 0 ;

    p_barcode->p_code->mode_w = p_barcode->p_code->mode_w ? p_barcode->p_code->mode_w : 2 ;
    // 计算显示宽高
    p_barcode->p_code->ret_param.dot_w =  ((p_barcode->p_code->mode_w * temp + 7 ) >> 3 ) << 3;
    p_barcode->p_code->ret_param.dot_h =  p_barcode->p_code->mode_h;
    // 限制高度 未达到特定高度传输两行数据
    if(p_barcode->p_code->mode_h < BARCODE_MIN_H)
    {
        p_barcode->p_code->ret_param.dot_h = 2;
        p_barcode->p_code->ret_param.is_two_lines_data = 1;
    }

    // 需要的字节数超出buf
    if((( p_barcode->p_code->ret_param.dot_w *  p_barcode->p_code->ret_param.dot_h) >> 3) >  p_barcode->p_code->pdat_size)
        return 0;

    // 计算空白区域宽度
    p_barcode->left_blank_none = p_barcode->left_blank_none*p_barcode->p_code->mode_w;
    p_barcode->right_blank_none = p_barcode->right_blank_none*p_barcode->p_code->mode_w;
    temp = temp*p_barcode->p_code->mode_w;
    if( p_barcode->p_code->ret_param.dot_w > temp)
    {
        temp =  p_barcode->p_code->ret_param.dot_w - temp;
        p_barcode->left_blank_none += (temp - ((temp >> 1)));
        p_barcode->right_blank_none += (temp >> 1);
    }
    // 清空 buf
    memset(p_barcode->p_code->pdat , 0 , p_barcode->p_code->pdat_size);

    // return _upce_handler(p_barcode);
    return 1;

ERR_RET:
    p_barcode->p_code->ret_param.dot_w = 0;
    p_barcode->p_code->ret_param.dot_h = 0;
    return 0;
}
#endif

/* ITF ---------------------------------------------------------------*/
#if ITF_BARCODE_EN

static const bar_uint8_t _itf_code_type_a[10] = {0x06, 0x11, 0x09, 0x18, 0x05, 0x14, 0x0C, 0x03, 0x12, 0x0A};

static int _itf_data_code_set(_code_st * p_barcode , _row_st* p_row)
{
    bar_uint32_t i , offset = 0 , dat1 , dat2; 
    bar_uint8_t c1 , c2;
    offset = p_barcode->len % 2;

    for(i = 0 ; i < p_barcode->len + offset ; i+=2)
    {
        if(offset && i == 0)
            dat1 = 0;

        c1 = offset && (i == 0) ? 0 : _get_string_char(p_barcode->p_code->barcode , i - offset);
        c2 = _get_string_char(p_barcode->p_code->barcode , i + 1 - offset);

        dat1 = _itf_code_type_a[c1];
        dat2 = _itf_code_type_a[c2];

        if(!_2_of_5_data_set(p_barcode , p_row , dat1 , dat2))
            return 0;
    }

    return 1;
}

static int _itf_bearer_bar_set(_code_st * p_barcode , _row_st* p_row)
{
    bar_uint32_t i ; 

    if(p_barcode->start < p_barcode->itf_bearer_bar)
        return 0;
    if(p_barcode->p_code->ret_param.dot_h < p_barcode->end + p_barcode->itf_bearer_bar)
        return 0;
    // 左
    for(i = 0 ; i < p_barcode->itf_bearer_bar ; i++)
    {
        if(!_set_point(p_row , i ))
            return 0;
    }
    // 右
    for(i = 0 ; i < p_barcode->itf_bearer_bar ; i++)
    {
        if(!_set_point(p_row , p_barcode->p_code->ret_param.dot_w - 1 - i))
            return 0;
    }
    // 上
    for(i = 0 ; i < p_barcode->itf_bearer_bar ; i++)
    {
        p_row->pdat = p_barcode->p_code->pdat + ((p_barcode->start - p_barcode->itf_bearer_bar + i) * p_row->row_byte);
        memset(p_row->pdat , 0xFF , p_row->row_byte);
    }

    // 下
    for(i = 0 ; i < p_barcode->itf_bearer_bar ; i++)
    {
        p_row->pdat = p_barcode->p_code->pdat + ((p_barcode->end + i) * p_row->row_byte);
        memset(p_row->pdat , 0xFF , p_row->row_byte);
    }

    return 1;
}

static int _itf_font_set(_code_st * p_barcode , _row_st* p_row ,bar_uint32_t y)
{
    bar_uint32_t i , w , h , offset = 0;
    bar_uint8_t temp_buf[16] , c = 0;

    w = 5;
    h = 8;

    // 超出显示区域
    if((y + h) > p_barcode->p_code->ret_param.dot_h)
        return 0;

    offset = p_barcode->len % 2 ;

    p_row->pdat = p_barcode->p_code->pdat + (y * p_row->row_byte);

    // 文字居中
    p_row->pos = (p_barcode->p_code->ret_param.dot_w >> 1) - (((p_barcode->len + offset)*(w+2)) >> 1) ;

    for(i = 0 ; i < p_barcode->len + offset ; i++)
    {
        p_row->pdat = p_barcode->p_code->pdat + (y * p_row->row_byte);

        c = offset&&(i== 0) ? 0 : _get_string_char(p_barcode->p_code->barcode , i - offset);
        if(!_get_font(c + 0x30 , temp_buf))
            return 0;
        if(!_font_point_put(p_row , w , h , temp_buf))
            return 0;
        
        p_row->pos += w + 2 ;
    }

    return 1;
}

static int _itf_hri_type_set(_code_st * p_barcode , _row_st* p_row)
{
    // 上
    if( GT_FAMILY_BARCODE_HRI_TYPE_UP == p_barcode->p_code->hri_type || 
        GT_FAMILY_BARCODE_HRI_TYPE_ALL == p_barcode->p_code->hri_type)
    {

        if(!_itf_font_set( p_barcode , p_row , BARCODE_BLANK_SIZE>>1))
            return 0;

    }
    // 下
    if( GT_FAMILY_BARCODE_HRI_TYPE_DOWN == p_barcode->p_code->hri_type ||
        GT_FAMILY_BARCODE_HRI_TYPE_ALL == p_barcode->p_code->hri_type)
    {
        if(!_itf_font_set( p_barcode , p_row , p_barcode->end + (BARCODE_BLANK_SIZE>>1) + p_barcode->itf_bearer_bar ))
            return 0;
    }

    return 1;
}

static int _itf_handler(_code_st * p_barcode)
{
    bar_uint32_t i;
    _row_st* p_row = &row_para;

    if(NULL == p_barcode)
        goto ERR_RET;
    
    _get_bracode_scope(p_barcode);

    memset(p_row , 0 , sizeof(_row_st));
    p_row->row_byte =  p_barcode->p_code->ret_param.dot_w >> 3 ;
    p_row->pos = 0;
    p_row->pdat = p_barcode->p_code->pdat + (p_barcode->start * p_row->row_byte);
    if(NULL == p_row->pdat)
        goto ERR_RET;

    // 左空白区
    if(!_left_blank_none_set(p_barcode , p_row))
        goto ERR_RET;

    // 起始符
    if(!_start_code_set(p_barcode , p_row , 0x0A))
        goto ERR_RET;

    // 数据符
    if(!_itf_data_code_set(p_barcode , p_row))
        goto ERR_RET;

    // 结束符
    if(!_end_code_set(p_barcode , p_row , 0x0D))
        goto ERR_RET;
    // 右空白区
    if(!_right_blank_none_set(p_barcode , p_row))
        goto ERR_RET;
    
    // 
    if(p_barcode->p_code->ret_param.is_two_lines_data)
    {
        for(i = p_barcode->start + 1 ; i < p_barcode->p_code->ret_param.dot_h ; i++)
        {
            memmove(p_barcode->p_code->pdat + (i * p_row->row_byte) , p_row->pdat , p_row->row_byte);
        }
        return 1;
    }

    // 设置左右边框
    if(!_itf_bearer_bar_set(p_barcode , p_row))
        goto ERR_RET;

    // 绘制范围类的数据
    for(i = p_barcode->start+1; i < p_barcode->end ; i++)
    {
        memmove(p_barcode->p_code->pdat + (i * p_row->row_byte) , p_barcode->p_code->pdat + (p_barcode->start*p_row->row_byte) ,  p_row->row_byte);
    }

    // 文字
    if(!_itf_hri_type_set(p_barcode , p_row))
        goto ERR_RET;
    return 1;

ERR_RET:
    p_barcode->p_code->ret_param.dot_w = 0;
    p_barcode->p_code->ret_param.dot_h = 0;
    return 0;
}

static int _get_itf(_code_st* p_barcode)
{
    bar_uint32_t temp = 0;
    if(NULL == p_barcode)
        goto ERR_RET;
    if( 0 == (p_barcode->len = strlen(p_barcode->p_code->barcode)))
        goto ERR_RET;
    if( !_is_number_string(p_barcode->p_code->barcode))
        goto ERR_RET;

    // 保护框
    p_barcode->itf_bearer_bar = 3;

    p_barcode->one_code_size = 7;

    p_barcode->left_blank_none = 10 + p_barcode->itf_bearer_bar;
    p_barcode->start_code = 4;
    p_barcode->left_data_code = p_barcode->len % 2 ? (p_barcode->len + 1) * p_barcode->one_code_size : p_barcode->len * p_barcode->one_code_size;
    p_barcode->separator = 0;
    p_barcode->right_data_code = 0;
    p_barcode->check_code = 0;
    p_barcode->end_code = 4;
    p_barcode->right_blank_none = 10 +  p_barcode->itf_bearer_bar;
    // 实际值为上述变量相加
    temp = p_barcode->left_blank_none + p_barcode->start_code + p_barcode->left_data_code + p_barcode->end_code + p_barcode->right_blank_none;
    
    p_barcode->p_code->mode_w = p_barcode->p_code->mode_w ? p_barcode->p_code->mode_w : 2 ;
    // 计算显示宽高
    p_barcode->p_code->ret_param.dot_w =  ((p_barcode->p_code->mode_w * temp + 7 ) >> 3 ) << 3;
    p_barcode->p_code->ret_param.dot_h =  p_barcode->p_code->mode_h;
    // 限制高度 未达到特定高度传输两行数据
    if(p_barcode->p_code->mode_h < BARCODE_MIN_H)
    {
        p_barcode->p_code->ret_param.dot_h = 2;
        p_barcode->p_code->ret_param.is_two_lines_data = 1;
    }
    
    // 需要的字节数超出buf
    if((( p_barcode->p_code->ret_param.dot_w *  p_barcode->p_code->ret_param.dot_h) >> 3) >  p_barcode->p_code->pdat_size)
        return 0;

    // 计算空白区域宽度
    p_barcode->left_blank_none = p_barcode->left_blank_none*p_barcode->p_code->mode_w;
    p_barcode->right_blank_none = p_barcode->right_blank_none*p_barcode->p_code->mode_w;
    temp = temp*p_barcode->p_code->mode_w;
    if( p_barcode->p_code->ret_param.dot_w > temp)
    {
        temp =  p_barcode->p_code->ret_param.dot_w - temp;
        p_barcode->left_blank_none += (temp - ((temp >> 1)));
        p_barcode->right_blank_none += (temp >> 1);
    }
    // 清空 buf
    memset(p_barcode->p_code->pdat , 0 , p_barcode->p_code->pdat_size);

    // return _itf_handler(p_barcode);
    return 1;


ERR_RET:
    p_barcode->p_code->ret_param.dot_w = 0;
    p_barcode->p_code->ret_param.dot_h = 0;
    return 0;
}
#endif

/* CODABAR ---------------------------------------------------------------*/
#if CODABAR_BARCODE_EN
static const char _codabar_char[20] = {0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x2D,0x24,0x3A,0x2F,0x2E,0x2B,0x41,0x42,0x43,0x44};
static const bar_uint8_t _codabar_code_type_a[20] = {
    0x03,0x06,0x09,0x60,0x12,0x42,0x21,0x24,0x30,0x48,
    0x0C,0x18,0x45,0x51,0x54,0x15,0x1A,0x29,0x0B,0x0E
};

static int _get_codabar_char(char *str , bar_uint32_t numb)
{
    bar_uint32_t i ;

    for(i = 0 ; i < sizeof(_codabar_char) ; i++)
    {
        if(str[numb] == _codabar_char[i])
            return i;
    }
    return 0;
}

static int _is_ABCD(char c)
{
    if(c >= 0x41 && c <= 0x44)
        return 1;
    
    return 0;
}

static int _is_symbol_codabar(char c)
{
    const char tmp[] = {0x2D,0x24,0x3A,0x2F,0x2E,0x2B};
    bar_uint16_t i = 0;
    for(i = 0 ; i < sizeof(tmp) ; i++)
    {
        if(c == tmp[i])
            return 1;
    }
    return 0;
}
static int _is_string_ture_of_barocde(char* str , bar_uint16_t* p_wide_count , bar_uint16_t* p_narrow_count )
{
    bar_uint32_t i;
    *p_wide_count = 0;
    *p_narrow_count = 0;

    for(i = 0 ; i < strlen(str); i++)
    {
        if(_is_number(str[i]))
        {
            *p_wide_count += 2;
            *p_narrow_count += 5;
            continue;
        }
        else if(_is_ABCD(str[i]))
        {
            *p_wide_count += 3;
            *p_narrow_count += 4;
            continue;
        }
        else if(_is_symbol_codabar(str[i]))
        {

            if(str[i] == 0x3A || str[i] == 0x2F || str[i] == 0x2E || str[i] == 0x2B)
            {
                *p_wide_count += 3;
                *p_narrow_count += 4;
            }
            else
            {
                *p_wide_count += 2;
                *p_narrow_count += 5;
            }
            continue;
        }
        else
        {
            return 0;
        }
    }

    return 1;
}

static int _codabar_data_code_set(_code_st * p_barcode , _row_st* p_row)
{
    bar_uint32_t i ; 
    bar_uint8_t c1 , code_type= 0;

    for(i = 0 ; i < p_barcode->len  ; i++)
    {
        c1 = _get_codabar_char(p_barcode->p_code->barcode , i);
        code_type = _codabar_code_type_a[c1];

        if(!_codxx_data_set(p_barcode , p_row , code_type , 7))
            return 0;
        if(i < p_barcode->len -1)
        {
            if(!_interval_code_set(p_barcode , p_row))
                return 0;
        }
        
    }

    return 1;
}

static int _codabar_font_set(_code_st * p_barcode , _row_st* p_row ,bar_uint32_t y)
{
    bar_uint32_t i , w , h ;
    bar_uint8_t temp_buf[16] ;

    w = 5;
    h = 8;

    // 超出显示区域
    if((y + h) > p_barcode->p_code->ret_param.dot_h)
        return 0;

    p_row->pdat = p_barcode->p_code->pdat + (y * p_row->row_byte);

    // 文字居中
    p_row->pos = (p_barcode->p_code->ret_param.dot_w >> 1) - ((p_barcode->len*(w+2)) >> 1) ;

    for(i = 0 ; i < p_barcode->len  ; i++)
    {
        p_row->pdat = p_barcode->p_code->pdat + (y * p_row->row_byte);

        if(!_get_font( p_barcode->p_code->barcode[i], temp_buf))
            return 0;
        if(!_font_point_put(p_row , w , h , temp_buf))
            return 0;
        
        p_row->pos += w + 2 ;
    }

    return 1;
}

static int _codabar_hri_type_set(_code_st * p_barcode , _row_st* p_row)
{
    // 上
    if( GT_FAMILY_BARCODE_HRI_TYPE_UP == p_barcode->p_code->hri_type || 
        GT_FAMILY_BARCODE_HRI_TYPE_ALL == p_barcode->p_code->hri_type)
    {
        if(!_codabar_font_set( p_barcode , p_row , BARCODE_BLANK_SIZE>>1))
            return 0;
    }
    // 下
    if( GT_FAMILY_BARCODE_HRI_TYPE_DOWN == p_barcode->p_code->hri_type ||
        GT_FAMILY_BARCODE_HRI_TYPE_ALL == p_barcode->p_code->hri_type)
    {
        if(!_codabar_font_set( p_barcode , p_row , p_barcode->end + (BARCODE_BLANK_SIZE>>1) + p_barcode->itf_bearer_bar ))
            return 0;
    }

    return 1;
}

static int _codabar_handler(_code_st * p_barcode)
{
    bar_uint32_t i;
    _row_st* p_row = &row_para;

    if(NULL == p_barcode)
        goto ERR_RET;
    
    _get_bracode_scope(p_barcode);

    memset(p_row , 0 , sizeof(_row_st));
    p_row->row_byte =  p_barcode->p_code->ret_param.dot_w >> 3 ;
    p_row->pos = 0;
    p_row->pdat = p_barcode->p_code->pdat + (p_barcode->start * p_row->row_byte);
    if(NULL == p_row->pdat)
        goto ERR_RET;
    
    // 左空白区
    if(!_left_blank_none_set(p_barcode , p_row))
        goto ERR_RET;
    
    // 数据符
    if(!_codabar_data_code_set(p_barcode , p_row))
        goto ERR_RET;

    // 右空白区
    if(!_right_blank_none_set(p_barcode , p_row))
        goto ERR_RET;

    // 
    if(p_barcode->p_code->ret_param.is_two_lines_data)
    {
        for(i = p_barcode->start + 1 ; i < p_barcode->p_code->ret_param.dot_h ; i++)
        {
            memmove(p_barcode->p_code->pdat + (i * p_row->row_byte) , p_row->pdat , p_row->row_byte);
        }
        return 1;
    }
    
    // 绘制范围类的数据
    for(i = p_barcode->start+1; i < p_barcode->end ; i++)
    {
        memmove(p_barcode->p_code->pdat + (i * p_row->row_byte) , p_barcode->p_code->pdat + (p_barcode->start*p_row->row_byte) ,  p_row->row_byte);
    }

    // 文字
    if(!_codabar_hri_type_set(p_barcode , p_row))
        goto ERR_RET;
    return 1;

ERR_RET:
    p_barcode->p_code->ret_param.dot_w = 0;
    p_barcode->p_code->ret_param.dot_h = 0;
    return 0;
}

static int _get_codabar(_code_st* p_barcode)
{
    bar_uint32_t temp = 0 ;
    bar_uint16_t wide_count = 0 , narrow_count = 0;

    if(NULL == p_barcode)
        goto ERR_RET;
    if( 0 == (p_barcode->len = strlen(p_barcode->p_code->barcode)))
        goto ERR_RET;
    if( !_is_string_ture_of_barocde(p_barcode->p_code->barcode , &wide_count , &narrow_count))
        goto ERR_RET;
    // 检查起始和结束
    if(!_is_ABCD(p_barcode->p_code->barcode[0]))
        goto ERR_RET;
    if(!_is_ABCD(p_barcode->p_code->barcode[p_barcode->len-1]))
        goto ERR_RET;
    // 超宽 强制为 2
    if(p_barcode->p_code->mode_w > 6)
        p_barcode->p_code->mode_w = 2;
    
    p_barcode->p_code->mode_w = p_barcode->p_code->mode_w ? p_barcode->p_code->mode_w : 2 ;
    
    // 模块宽度 双模块使用
    p_barcode->narrow_module = p_barcode->p_code->mode_w;
    p_barcode->wide_module = _wide_module_size[p_barcode->p_code->mode_w];

    p_barcode->interval_code = 1;

    p_barcode->left_blank_none = 8 * p_barcode->narrow_module;
    p_barcode->start_code = 0;
    p_barcode->left_data_code =  wide_count * p_barcode->wide_module + narrow_count * p_barcode->narrow_module;
    p_barcode->separator = 0;
    p_barcode->right_data_code = 0;
    p_barcode->check_code = 0;
    p_barcode->end_code = 0;
    p_barcode->right_blank_none = 7 * p_barcode->narrow_module;

    temp =  (p_barcode->left_blank_none +  p_barcode->left_data_code) +  p_barcode->right_blank_none + (p_barcode->len - 1) * p_barcode->narrow_module * p_barcode->interval_code;
    // 计算显示宽高
    p_barcode->p_code->ret_param.dot_w =  ((temp + 7 ) >> 3 ) << 3;
    p_barcode->p_code->ret_param.dot_h =  p_barcode->p_code->mode_h;

    // 限制高度 未达到特定高度传输两行数据
    if(p_barcode->p_code->mode_h < BARCODE_MIN_H)
    {
        p_barcode->p_code->ret_param.dot_h = 2;
        p_barcode->p_code->ret_param.is_two_lines_data = 1;
    }

    // 需要的字节数超出buf
    if((( p_barcode->p_code->ret_param.dot_w *  p_barcode->p_code->ret_param.dot_h) >> 3) >  p_barcode->p_code->pdat_size)
        return 0;

    // 计算空白区域宽度
    if( p_barcode->p_code->ret_param.dot_w > temp)
    {
        temp =  p_barcode->p_code->ret_param.dot_w - temp;
        p_barcode->left_blank_none += (temp - ((temp >> 1)));
        p_barcode->right_blank_none += (temp >> 1);
    }

    // 清空 buf
    memset(p_barcode->p_code->pdat , 0 , p_barcode->p_code->pdat_size);

    // return _codabar_handler(p_barcode);
    return 1;


ERR_RET:
    p_barcode->p_code->ret_param.dot_w = 0;
    p_barcode->p_code->ret_param.dot_h = 0;
    return 0;
}
#endif

/* code128  ---------------------------------------------------------------*/
#if CODE128_BARCODE_EN
const static bar_uint16_t _code128_code_type_a[] = {
    0x06CC,0x066C,0x0666,0x0498,0x048C,0x044C,0x04C8,0x04C4,0x0464,0x0648,0x0644,0x0624,0x059C,0x04DC,0x04CE,0x05CC,
    0x04EC,0x04E6,0x0672,0x065C,0x064E,0x06E4,0x0674,0x076E,0x074C,0x072C,0x0726,0x0764,0x0734,0x0732,0x06D8,0x06C6,
    0x0636,0x0518,0x0458,0x0446,0x0588,0x0468,0x0462,0x0688,0x0628,0x0622,0x05B8,0x058E,0x046E,0x05D8,0x05C6,0x0476,
    0x0776,0x068E,0x062E,0x06E8,0x06E2,0x06EE,0x0758,0x0746,0x0716,0x0768,0x0762,0x071A,0x077A,0x0642,0x078A,0x0530,
    0x050C,0x04B0,0x0486,0x042C,0x0426,0x0590,0x0584,0x04D0,0x04C2,0x0434,0x0432,0x0612,0x0650,0x07BA,0x0614,0x047A,
    0x053C,0x04BC,0x049E,0x05E4,0x04F4,0x04F2,0x07A4,0x0794,0x0792,0x06DE,0x06F6,0x07B6,0x0578,0x051E,0x045E,0x05E8,
    0x05E2,0x07A8,0x07A2,0x05DE,0x05EE,0x075E,0x07AE,0x0684,0x0690,0x069C,0x18EB
};
#define _CODE128_FNC1       (102)
#define _CODE128_FNC3       (96)
#define _CODE128_FNC2       (97)
#define _CODE128_FNC4_A     (101)
#define _CODE128_FNC4_B     (100)
#define _CODE128_SHIFT      (98)
#define _CODE128_CODE_C     (99)
#define _CODE128_CODE_B     (100)
#define _CODE128_CODE_A     (101)
#define _CODE128_START_A    (103)
#define _CODE128_START_B    (104)
#define _CODE128_START_C    (105)
#define _CODE128_STOP       (106)


static int _is_code128_string_true(_code_st * p_barcode)
{
    bar_uint32_t i ;
    bar_uint8_t a = 0 , b = 0 , c = 0;
    for(i = 0 ; i < strlen(p_barcode->p_code->barcode); i++)
    {
        // 判断编码是否属于 code128A
        if(p_barcode->p_code->barcode[i] >= 0 && p_barcode->p_code->barcode[i] <= 0x1F)
        {
            a = 1;
        }
        // 编码在次范围这属于 code128C
        else if((p_barcode->p_code->barcode[i] >= 0x20 && p_barcode->p_code->barcode[i] <= 0x2F) ||
                (p_barcode->p_code->barcode[i] >= 0x3A && p_barcode->p_code->barcode[i] <= 0x5F))
        {
            c = 0;
        }
        // 判断编码是否输入code128B
        else if(p_barcode->p_code->barcode[i] >= 0x60 && p_barcode->p_code->barcode[i] <= 0x7F)
        {
            b = 1;
        }
        // 如果是数字
        else if(_is_number(p_barcode->p_code->barcode[i]))
        {
            ++c;
        }
        else
        {
            // 不是code128的编码范围
            return 0;
        }
    }

    // 如果字符串中同时包含code128A和code128B的编码，这处理为错误
    // 不考虑code128A、code128B和code128C类型组合的问题，只考虑单个code128类型
    if( a && b)
    {
        return 0;
    }
    else if(a)
    {
        // code128A
        p_barcode->p_code->type = GT_FAMILY_BARCODE_TYPE_CODE128A;
    }
    else if(b)
    {
        // code128B
        p_barcode->p_code->type = GT_FAMILY_BARCODE_TYPE_CODE128B;
    }
    else if((GT_FAMILY_BARCODE_TYPE_CODE128C == p_barcode->p_code->type) && (c < p_barcode->len))
    {
        // 默认为 code128A
        p_barcode->p_code->type = GT_FAMILY_BARCODE_TYPE_CODE128A;
    }
    // 其他的无法确定类型，不改变原有类型

    return 1;
}

static bar_uint16_t _get_code128_index(_code_st * p_barcode , bar_uint16_t numb)
{
    bar_uint16_t temp_code = 0 ;

    if(numb >= p_barcode->len)
        return 0;

    temp_code = p_barcode->p_code->barcode[numb];
    if(GT_FAMILY_BARCODE_TYPE_CODE128A == p_barcode->p_code->type)
    {
        if(temp_code >= 0 && temp_code <= 0x1F)
        {
            return temp_code + 64;
        }
        else
        {
            return temp_code-0x20;
        }
    }
    else if(GT_FAMILY_BARCODE_TYPE_CODE128B == p_barcode->p_code->type)
    {
        return temp_code-0x20;
    }
    else
    {

        if(p_barcode->len % 2 && numb == p_barcode->len-1)
        {
            return temp_code-0x20;
        }
        temp_code = _get_string_char(p_barcode->p_code->barcode , numb) * 10;
        temp_code += _get_string_char(p_barcode->p_code->barcode , numb+1);
        return temp_code;
    }
}

static int _code128_data_set(_code_st * p_barcode , _row_st* p_row)
{
    bar_uint16_t code_type = 0 , index = 0;
    bar_uint32_t i = 0 , offset = 1;

    if(GT_FAMILY_BARCODE_TYPE_CODE128C == p_barcode->p_code->type)
        offset = 2;

    for(i = 0 ; i < p_barcode->len; i+=offset)
    {
        if((p_barcode->len % 2) && (i == p_barcode->len -1) && (GT_FAMILY_BARCODE_TYPE_CODE128C == p_barcode->p_code->type))
        {
            code_type = _code128_code_type_a[_CODE128_CODE_A];
            if(!_data_set( p_barcode , p_row , code_type ))
                return 0;
            
            index = _get_code128_index(p_barcode , i);
            code_type = _code128_code_type_a[index];
            if(!_data_set( p_barcode , p_row , code_type ))
                return 0;
            
            return 1;
        }
        index = _get_code128_index(p_barcode , i);
        code_type = _code128_code_type_a[index];

        if(!_data_set( p_barcode , p_row , code_type ))
            return 0;
    }

    return 1;
}

static int _code_128_check_set(_code_st * p_barcode , _row_st* p_row)
{
    bar_uint16_t index = 0 , code_type = 0;
    bar_uint32_t i = 0 , offset = 1 , c = 0;

    if(GT_FAMILY_BARCODE_TYPE_CODE128A == p_barcode->p_code->type)
    {
        c = _CODE128_START_A;
    }
    else if( GT_FAMILY_BARCODE_TYPE_CODE128B == p_barcode->p_code->type)
    {
        c = _CODE128_START_B;
        
    }
    else // if(GT_FAMILY_BARCODE_TYPE_CODE128C == p_barcode->p_code->type)
    {
        c = _CODE128_START_C;
        offset = 2;
    }
        
    for(i = 0 ; i < p_barcode->len; i+=offset)
    {

        if((p_barcode->len % 2) && (i == p_barcode->len -1) && (GT_FAMILY_BARCODE_TYPE_CODE128C == p_barcode->p_code->type))
        {

            c += _CODE128_CODE_A  * ((i/offset)+1) ;

            index = _get_code128_index(p_barcode , i);
            c += index * ((i/offset)+1 + 1);
            
            break;
        }
        index = _get_code128_index(p_barcode , i);
        c += index * ((i/offset)+1);
    }
    p_barcode->p_code->ret_param.check_numb = c % 103;

    // 
    code_type = _code128_code_type_a[p_barcode->p_code->ret_param.check_numb];
    if(!_data_set( p_barcode , p_row , code_type ))
        return 0;
    
    return 1;
}

static int _code128_font_set(_code_st * p_barcode , _row_st* p_row ,bar_uint32_t y)
{
    bar_uint32_t i , w , h ;
    bar_uint8_t temp_buf[16] ;

    w = 5;
    h = 8;

    // 超出显示区域
    if((y + h) > p_barcode->p_code->ret_param.dot_h)
        return 0;

    p_row->pdat = p_barcode->p_code->pdat + (y * p_row->row_byte);

    // 文字居中
    p_row->pos = (p_barcode->p_code->ret_param.dot_w >> 1) - ((p_barcode->len*(w+2)) >> 1) ;

    for(i = 0 ; i < p_barcode->len ; i++)
    {
        p_row->pdat = p_barcode->p_code->pdat + (y * p_row->row_byte);

        if(!_get_font( p_barcode->p_code->barcode[i], temp_buf))
            return 0;
        if(!_font_point_put(p_row , w , h , temp_buf))
            return 0;
        
        p_row->pos += w + 2 ;
    }

    return 1;
}

static int _code128_hri_type_set(_code_st * p_barcode , _row_st* p_row)
{

    // 上
    if( GT_FAMILY_BARCODE_HRI_TYPE_UP == p_barcode->p_code->hri_type || 
        GT_FAMILY_BARCODE_HRI_TYPE_ALL == p_barcode->p_code->hri_type)
    {
        if(!_code128_font_set( p_barcode , p_row , BARCODE_BLANK_SIZE>>1))
            return 0;
    }
    // 下
    if( GT_FAMILY_BARCODE_HRI_TYPE_DOWN == p_barcode->p_code->hri_type ||
        GT_FAMILY_BARCODE_HRI_TYPE_ALL == p_barcode->p_code->hri_type)
    {
        if(!_code128_font_set( p_barcode , p_row , p_barcode->end + (BARCODE_BLANK_SIZE>>1) + p_barcode->itf_bearer_bar ))
            return 0;
    }

    return 1;
}

static int _code128_handler(_code_st * p_barcode)
{
    bar_uint32_t i;
    bar_uint16_t temp_code = 0;
    _row_st* p_row = &row_para;

    if(NULL == p_barcode)
        goto ERR_RET;
    
    _get_bracode_scope(p_barcode);

    memset(p_row , 0 , sizeof(_row_st));
    p_row->row_byte =  p_barcode->p_code->ret_param.dot_w >> 3 ;
    p_row->pos = 0;
    p_row->pdat = p_barcode->p_code->pdat + (p_barcode->start * p_row->row_byte);
    if(NULL == p_row->pdat)
        goto ERR_RET;
    
    // 左空白
    if(!_left_blank_none_set(p_barcode , p_row))
        goto ERR_RET;

    // 起始符
    if(GT_FAMILY_BARCODE_TYPE_CODE128A == p_barcode->p_code->type)
        temp_code = _code128_code_type_a[_CODE128_START_A];
    else if(GT_FAMILY_BARCODE_TYPE_CODE128B == p_barcode->p_code->type)
        temp_code = _code128_code_type_a[_CODE128_START_B];
    else
        temp_code = _code128_code_type_a[_CODE128_START_C];
    if(!_start_code_set(p_barcode , p_row , temp_code))
        goto ERR_RET;

    // 数据
    if(!_code128_data_set(p_barcode , p_row))
        goto ERR_RET;

    // 校验符
    if(!_code_128_check_set(p_barcode , p_row))
        goto ERR_RET;

    // 结束符
    if(!_end_code_set(p_barcode , p_row , _code128_code_type_a[_CODE128_STOP]))
        goto ERR_RET;

    // 右空白
    if(!_right_blank_none_set(p_barcode , p_row))
        goto ERR_RET;
    
    // 第二行数据
    if(p_barcode->p_code->ret_param.is_two_lines_data)
    {
        for(i = p_barcode->start + 1 ; i < p_barcode->p_code->ret_param.dot_h ; i++)
        {
            memmove(p_barcode->p_code->pdat + (i * p_row->row_byte) , p_row->pdat , p_row->row_byte);
        }
        return 1;
    }

    // 绘制范围类的数据
    for(i = p_barcode->start ; i < p_barcode->end;i++)
    {
        memmove(p_barcode->p_code->pdat + (i * p_row->row_byte) , p_row->pdat ,  p_row->row_byte);
    }

    // 文字
    if(!_code128_hri_type_set(p_barcode , p_row))
        goto ERR_RET;

    return 1;
ERR_RET:
    p_barcode->p_code->ret_param.dot_w = 0;
    p_barcode->p_code->ret_param.dot_h = 0;
    return 0;
}

static int _get_code128(_code_st * p_barcode)
{
    bar_uint32_t temp = 0 , offset = 0;
    if(NULL == p_barcode)
        goto ERR_RET;
    if( 0 == (p_barcode->len = strlen(p_barcode->p_code->barcode)))
        goto ERR_RET;
    // 判断输入编码是否属于 a b c ； 并且类型是否正确和对应，不对应这强制类型
    if(!_is_code128_string_true(p_barcode))
        goto ERR_RET;

    if(GT_FAMILY_BARCODE_TYPE_CODE128C == p_barcode->p_code->type)
    {
        offset = 1;
    }

    p_barcode->one_code_size = 11;

    p_barcode->left_blank_none = 0;
    p_barcode->start_code = 11;
    p_barcode->left_data_code = offset ? ((p_barcode->len >> 1) + (p_barcode->len%2)*2) * p_barcode->one_code_size : p_barcode->len * p_barcode->one_code_size;
    p_barcode->separator = 0;
    p_barcode->right_data_code = 0;
    p_barcode->check_code = 11;
    p_barcode->end_code = 13;
    p_barcode->right_blank_none = 0;

    temp = p_barcode->left_blank_none + p_barcode->start_code + p_barcode->left_data_code + p_barcode->check_code + p_barcode->end_code + p_barcode->right_blank_none;

    p_barcode->p_code->mode_w = p_barcode->p_code->mode_w ? p_barcode->p_code->mode_w : 2 ;
    
    // 计算显示宽高
    p_barcode->p_code->ret_param.dot_w =  ((p_barcode->p_code->mode_w * temp + 7 ) >> 3 ) << 3;
    p_barcode->p_code->ret_param.dot_h =  p_barcode->p_code->mode_h;
    // 限制高度 未达到特定高度传输两行数据
    if(p_barcode->p_code->mode_h < BARCODE_MIN_H)
    {
        p_barcode->p_code->ret_param.dot_h = 2;
        p_barcode->p_code->ret_param.is_two_lines_data = 1;
    }

    // 需要的字节数超出buf
    if((( p_barcode->p_code->ret_param.dot_w *  p_barcode->p_code->ret_param.dot_h) >> 3) >  p_barcode->p_code->pdat_size)
        return 0;

    p_barcode->left_blank_none = p_barcode->left_blank_none*p_barcode->p_code->mode_w;
    p_barcode->right_blank_none = p_barcode->right_blank_none*p_barcode->p_code->mode_w;
    temp = temp*p_barcode->p_code->mode_w;
    if( p_barcode->p_code->ret_param.dot_w > temp)
    {
        temp =  p_barcode->p_code->ret_param.dot_w - temp;
        p_barcode->left_blank_none += (temp - ((temp >> 1)));
        p_barcode->right_blank_none += (temp >> 1);
    }

    memset(p_barcode->p_code->pdat , 0 , p_barcode->p_code->pdat_size);

    // return _code128_handler(p_barcode);
    return 1;


ERR_RET:
    p_barcode->p_code->ret_param.dot_w = 0;
    p_barcode->p_code->ret_param.dot_h = 0;
    return 0;
}
#endif

/* code93  ---------------------------------------------------------------*/
#if CODE93_BARCODE_EN
static const bar_uint32_t _code93_logic_type[47] =  {
    0x114,0x148,0x144,0x142,0x128,0x124,0x122,0x150,0x112,0x10A,    //0-9
    0x1A8,0x1A4,0x1A2,0x194,0x192,0x18A,0x168,0x164,0x162,0x134,    //A-J
    0x11A,0x158,0x14C,0x146,0x12C,0x116,0x1B4,0x1B2,0x1AC,0x1A6,    //K-T
    0x196,0x19A,0x16C,0x166,0x136,0x13A,    //U-Z
    0x12E,0x1D4,0x1D2,0x1CA,0x16E,0x176,    //- . 空格 $ / + %
    0x1AE,0x126,0x1DA,0x1D6,0x132};   //扩展



/**
 * @brief code93 数据符设置
 * 
 * @param p_barcode 
 * @param p_row 
 * @return int 
 */
static int _code93_data_code_set(_code_st * p_barcode , _row_st* p_row)
{
    bar_uint32_t i=0 , tmp = 0, code_type = 0;
    char c ;

    tmp = p_barcode->left_data_code / p_barcode->one_code_size;

    for(i = 0 ; i < tmp ; i++)
    {
        c = _get_code93_or_39_string_index(p_barcode->p_code->barcode ,i);

        code_type = _code93_logic_type[c];
        if(!_data_set( p_barcode , p_row , code_type ))
            return 0;
    }

    return 1;
}

static int _code93_check_code_set(_code_st * p_barcode , _row_st* p_row)
{
    bar_uint32_t i ,check_C1=0,check_K1=0;
	unsigned char A=0,K=0;
    bar_uint32_t check_C=0;
    bar_uint32_t check_K=0;
    
    A = p_barcode->len > 20 ? p_barcode->len - 20 : p_barcode->len;
    K = (p_barcode->len + 1) > 15 ? p_barcode->len + 1 - 15 : p_barcode->len+1;

    // 计算check_C
    for(i = 0 ; i < p_barcode->len ; i++)
    {
        check_C1 += A * _get_code93_or_39_string_index(p_barcode->p_code->barcode , i);

        if( --A == 0 )
            A = 20;
    }
    check_C = check_C1 % 47;

    // 计算check_K
    for(i = 0 ; i < p_barcode->len ; i++)
    {
        check_K1 += K * _get_code93_or_39_string_index(p_barcode->p_code->barcode , i);

        if( --K == 0 )
            K = 15;
    }
    check_K1 += check_C;
    check_K = check_K1 % 47;

    p_barcode->p_code->ret_param.check_numb = check_C;
    p_barcode->p_code->ret_param.check_numb2 = check_K;

    if(!_data_set(p_barcode , p_row , _code93_logic_type[check_C]))
        return 0;
    if(!_data_set(p_barcode , p_row , _code93_logic_type[check_K]))
        return 0;
    
    return 1;
    
}
/**
 * @brief ean8 文字设置
 * 
 * @param p_barcode 
 * @param p_row 
 * @param y 
 * @return int 
 */
static int _code93_font_set(_code_st * p_barcode , _row_st* p_row ,bar_uint32_t y)
{
    bar_uint32_t i , w , h ;
    bar_uint8_t temp_buf[16];

    w = 5;
    h = 8;

    // 超出显示区域
    if((y + h) > p_barcode->p_code->ret_param.dot_h)
        return 0;

    // 文字居中
    p_row->pos = (p_barcode->p_code->ret_param.dot_w >> 1) - ((p_barcode->len*(w+2)) >> 1) ;
    // 左数据
    for(i = 0; i < p_barcode->len; i++)
    {
        p_row->pdat = p_barcode->p_code->pdat + (y * p_row->row_byte);

        if(!_get_font(p_barcode->p_code->barcode[i] , temp_buf))
            return 0; 
        
        if(!_font_point_put(p_row , w , h , temp_buf))
            return 0;
        
        p_row->pos += w + 2 ;
    }
		
    return 1;
}
/**
 * @brief ean8 文字位置设置
 * 
 * @param p_barcode 
 * @param p_row 
 * @return int 
 */
static int _code93_hri_type_set(_code_st * p_barcode , _row_st* p_row)
{
    // 上
    if( GT_FAMILY_BARCODE_HRI_TYPE_UP == p_barcode->p_code->hri_type || 
        GT_FAMILY_BARCODE_HRI_TYPE_ALL == p_barcode->p_code->hri_type)
    {
        p_row->pdat = p_barcode->p_code->pdat + ((p_barcode->start-BARCODE_BLANK_SIZE) * p_row->row_byte);


        if(!_code93_font_set( p_barcode , p_row , BARCODE_BLANK_SIZE>>1))
            return 0;
    }
    // 下
    if( GT_FAMILY_BARCODE_HRI_TYPE_DOWN == p_barcode->p_code->hri_type ||
        GT_FAMILY_BARCODE_HRI_TYPE_ALL == p_barcode->p_code->hri_type)
    {
        p_row->pdat = p_barcode->p_code->pdat + (p_barcode->end * p_row->row_byte);

        if(!_code93_font_set( p_barcode , p_row , p_barcode->end+(BARCODE_BLANK_SIZE>>1)))
            return 0;
    }

    return 1;
}
/**
 * @brief code93 条码处理
 * 
 * @param p_barcode 
 * @return int 
 */
static int _code93_handler(_code_st * p_barcode)
{
    bar_uint32_t i;
    _row_st* p_row = &row_para;

    if(NULL == p_barcode)
        goto ERR_RET;
    
    _get_bracode_scope(p_barcode);

    memset(p_row , 0 , sizeof(_row_st));
    p_row->row_byte =  p_barcode->p_code->ret_param.dot_w >> 3 ;     //11byte
    p_row->pos = 0;
    p_row->pdat = p_barcode->p_code->pdat+(p_barcode->start*p_row->row_byte);
    if(NULL == p_row->pdat)
        goto ERR_RET;

     // 左空白
    if(!_left_blank_none_set(p_barcode , p_row))
        goto ERR_RET;

    // 起始符
    if(!_start_code_set(p_barcode , p_row , 0x015e))
        goto ERR_RET;

    // 数据
    if(!_code93_data_code_set(p_barcode , p_row))
        goto ERR_RET;

    // 校验符
    if(!_code93_check_code_set(p_barcode , p_row))
        goto ERR_RET;
	
    // 结束符
    if(!_end_code_set(p_barcode , p_row , 0x2BD))  //0x015E
        goto ERR_RET;

    // 右空白
    if(!_right_blank_none_set(p_barcode , p_row))
        goto ERR_RET;

    if(p_barcode->p_code->ret_param.is_two_lines_data)
    { 
        for(i = p_barcode->start + 1 ; i < p_barcode->p_code->ret_param.dot_h ; i++)
        {
            memmove(p_barcode->p_code->pdat + (i * p_row->row_byte) , p_row->pdat , p_row->row_byte);
        }
        return 1;
    }
    // 绘制范围类的数据
    for(i = p_barcode->start ; i < p_barcode->end;i++)
    {
        memmove(p_barcode->p_code->pdat + (i * p_row->row_byte) , p_row->pdat ,  p_row->row_byte);
    }

    // 文字
    if(!_code93_hri_type_set(p_barcode , p_row))
        goto ERR_RET;

    return 1;
ERR_RET:
    p_barcode->p_code->ret_param.dot_w = 0;
    p_barcode->p_code->ret_param.dot_h = 0;
    return 0;
}

/**
 * @brief 得到 ean93条码
 * 
 * @param p_barcode 
 * @return int 
 */
static int _get_code93(_code_st * p_barcode)
{
    bar_uint32_t temp = 0;
    if(NULL == p_barcode)
        goto ERR_RET;
    if( 0 == (p_barcode->len = strlen(p_barcode->p_code->barcode)))
        goto ERR_RET;
    if( !_is_code93_or_39_string(p_barcode->p_code->barcode))
            goto ERR_RET;
    
    p_barcode->one_code_size = 9;

    p_barcode->left_blank_none = 6;
    p_barcode->start_code = 9;
    p_barcode->left_data_code = p_barcode->one_code_size*p_barcode->len;
    p_barcode->check_code = 18;
    p_barcode->end_code = 10;
    p_barcode->right_blank_none = 5;

    temp = p_barcode->left_blank_none + p_barcode->start_code + p_barcode->left_data_code + p_barcode->check_code + p_barcode->end_code + p_barcode->right_blank_none;

    p_barcode->p_code->mode_w = p_barcode->p_code->mode_w ? p_barcode->p_code->mode_w : 2 ;
    
    // 计算显示宽高
    p_barcode->p_code->ret_param.dot_w =  ((p_barcode->p_code->mode_w * temp + 7) >> 3) << 3;
    p_barcode->p_code->ret_param.dot_h = p_barcode->p_code->mode_h;         //100bit

    if(p_barcode->p_code->mode_h < BARCODE_MIN_H)
    {
        p_barcode->p_code->ret_param.dot_h = 2;
        p_barcode->p_code->ret_param.is_two_lines_data = 1;
    }

    // 需要的字节数超出buf
    if((( p_barcode->p_code->ret_param.dot_w *  p_barcode->p_code->ret_param.dot_h) >> 3) >  p_barcode->p_code->pdat_size)
        return 0;

    p_barcode->left_blank_none = p_barcode->left_blank_none*p_barcode->p_code->mode_w;   //bit
    p_barcode->right_blank_none = p_barcode->right_blank_none*p_barcode->p_code->mode_w;  //bit
    temp = temp*p_barcode->p_code->mode_w;               //总宽度bit
    if( p_barcode->p_code->ret_param.dot_w > temp)
    {
        temp =  p_barcode->p_code->ret_param.dot_w - temp;
        p_barcode->left_blank_none += (temp - ((temp >> 1)));      //10bit
        p_barcode->right_blank_none += (temp >> 1);								//10bit
    }

    memset(p_barcode->p_code->pdat , 0 , p_barcode->p_code->pdat_size );

    // return _code93_handler(p_barcode);
    return 1;


ERR_RET:
    p_barcode->p_code->ret_param.dot_w = 0;
    p_barcode->p_code->ret_param.dot_h = 0;
    return 0;
}

#endif 

/* code39  ---------------------------------------------------------------*/
#if CODE39_BARCODE_EN
static const bar_uint16_t _code39_code_type_a[43] = {
    0x0034,0x0121,0x0061,0x0160,0x0031,0x0130,0x0070,0x0025,0x0124,0x0064,
    0x0109,0x0049,0x0148,0x0019,0x0118,0x0058,0x000D,0x010C,0x004C,0x001C,
    0x0103,0x0043,0x0142,0x0013,0x0112,0x0052,0x0007,0x0106,0x0046,0x0016,
    0x0181,0x00C1,0x01C0,0x0091,0x0190,0x00D0,0x0085,0x0184,0x00C4,0x00A8,
    0x00A2,0x008A,0x002A,
};
#define _CODE39_START   0x0094
#define _CODE39_END     0x0094

static int _code39_data_code_set(_code_st * p_barcode , _row_st* p_row)
{
    bar_uint32_t i ; 
    bar_uint16_t c1 , code_type= 0;

    for(i = 0 ; i < p_barcode->len  ; i++)
    {
        
        c1 = _get_code93_or_39_string_index(p_barcode->p_code->barcode , i);
        code_type = _code39_code_type_a[c1];

        if(!_codxx_data_set(p_barcode , p_row , code_type , 9))
            return 0;
        
        // 字符间隔
        if(!_interval_code_set(p_barcode , p_row))
            return 0;
    }

    return 1;
}

static int _code39_font_set(_code_st * p_barcode , _row_st* p_row ,bar_uint32_t y)
{
    bar_uint32_t i , w , h ;
    bar_uint8_t temp_buf[16] ;

    w = 5;
    h = 8;

    // 超出显示区域
    if((y + h) > p_barcode->p_code->ret_param.dot_h)
        return 0;

    p_row->pdat = p_barcode->p_code->pdat + (y * p_row->row_byte);

    // 文字居中
    p_row->pos = (p_barcode->p_code->ret_param.dot_w >> 1) - ((p_barcode->len*(w+2)) >> 1) ;

    for(i = 0 ; i < p_barcode->len  ; i++)
    {
        p_row->pdat = p_barcode->p_code->pdat + (y * p_row->row_byte);

        if(!_get_font( p_barcode->p_code->barcode[i], temp_buf))
            return 0;
        if(!_font_point_put(p_row , w , h , temp_buf))
            return 0;
        
        p_row->pos += w + 2 ;
    }

    return 1;
}

static int _code39_hri_type_set(_code_st * p_barcode , _row_st* p_row)
{
    // 上
    if( GT_FAMILY_BARCODE_HRI_TYPE_UP == p_barcode->p_code->hri_type || 
        GT_FAMILY_BARCODE_HRI_TYPE_ALL == p_barcode->p_code->hri_type)
    {
        if(!_code39_font_set( p_barcode , p_row , BARCODE_BLANK_SIZE>>1))
            return 0;
    }
    // 下
    if( GT_FAMILY_BARCODE_HRI_TYPE_DOWN == p_barcode->p_code->hri_type ||
        GT_FAMILY_BARCODE_HRI_TYPE_ALL == p_barcode->p_code->hri_type)
    {
        if(!_code39_font_set( p_barcode , p_row , p_barcode->end + (BARCODE_BLANK_SIZE>>1) + p_barcode->itf_bearer_bar ))
            return 0;
    }

    return 1;
}

static int _code39_handler(_code_st * p_barcode)
{
    bar_uint32_t i;
    _row_st* p_row = &row_para;

    if(NULL == p_barcode)
        goto ERR_RET;
    
    _get_bracode_scope(p_barcode);

    memset(p_row , 0 , sizeof(_row_st));
    p_row->row_byte =  p_barcode->p_code->ret_param.dot_w >> 3 ;
    p_row->pos = 0;
    p_row->pdat = p_barcode->p_code->pdat + (p_barcode->start * p_row->row_byte);
    if(NULL == p_row->pdat)
        goto ERR_RET;
    
    // 左空白区
    if(!_left_blank_none_set(p_barcode , p_row))
        goto ERR_RET;
    
    // 起始符
    if(!_codxx_data_set(p_barcode , p_row , _CODE39_START , 9))
        return 0;
    // 字符间隔
    if(!_interval_code_set(p_barcode , p_row))
        return 0;

    // 数据符
    if(!_code39_data_code_set(p_barcode , p_row))
        goto ERR_RET;

    // 结束符
    if(!_codxx_data_set(p_barcode , p_row , _CODE39_END , 9))
        return 0;

    // 右空白区
    if(!_right_blank_none_set(p_barcode , p_row))
        goto ERR_RET;

    // 
    if(p_barcode->p_code->ret_param.is_two_lines_data)
    {
        for(i = p_barcode->start + 1 ; i < p_barcode->p_code->ret_param.dot_h ; i++)
        {
            memmove(p_barcode->p_code->pdat + (i * p_row->row_byte) , p_row->pdat , p_row->row_byte);
        }
        return 1;
    }
    
    // 绘制范围类的数据
    for(i = p_barcode->start+1; i < p_barcode->end ; i++)
    {
        memmove(p_barcode->p_code->pdat + (i * p_row->row_byte) , p_barcode->p_code->pdat + (p_barcode->start*p_row->row_byte) ,  p_row->row_byte);
    }

    // 文字
    if(!_code39_hri_type_set(p_barcode , p_row))
        goto ERR_RET;
    return 1;

ERR_RET:
    p_barcode->p_code->ret_param.dot_w = 0;
    p_barcode->p_code->ret_param.dot_h = 0;
    return 0;
}

static int _get_code39(_code_st* p_barcode)
{
    bar_uint32_t temp = 0 ;
    bar_uint16_t wide_count = 0 , narrow_count = 0;

    if(NULL == p_barcode)
        goto ERR_RET;
    if( 0 == (p_barcode->len = strlen(p_barcode->p_code->barcode)))
        goto ERR_RET;
    if(!_is_code93_or_39_string(p_barcode->p_code->barcode))
        goto ERR_RET;

    // 超宽 强制为 2
    if(p_barcode->p_code->mode_w > 6)
        p_barcode->p_code->mode_w = 2;
    
    p_barcode->p_code->mode_w = p_barcode->p_code->mode_w ? p_barcode->p_code->mode_w : 2 ;
    
    // 模块宽度 双模块使用
    p_barcode->narrow_module = p_barcode->p_code->mode_w;
    p_barcode->wide_module = _wide_module_size[p_barcode->p_code->mode_w];

    wide_count = 3 * p_barcode->len;
    narrow_count = 6 * p_barcode->len;

    p_barcode->interval_code = 1;

    p_barcode->left_blank_none = 6 * p_barcode->narrow_module;
    p_barcode->start_code = 3 * p_barcode->wide_module + 6 * p_barcode->narrow_module;
    p_barcode->left_data_code =  wide_count * p_barcode->wide_module + narrow_count * p_barcode->narrow_module;
    p_barcode->separator = 0;
    p_barcode->right_data_code = 0;
    p_barcode->check_code = 0;
    p_barcode->end_code = 3 * p_barcode->wide_module + 6 * p_barcode->narrow_module;
    p_barcode->right_blank_none = 5 * p_barcode->narrow_module;

    temp =  p_barcode->left_blank_none + p_barcode->start_code + p_barcode->left_data_code + p_barcode->end_code + p_barcode->right_blank_none + (p_barcode->len+1) * p_barcode->narrow_module * p_barcode->interval_code;
    // 计算显示宽高
    p_barcode->p_code->ret_param.dot_w =  ((temp + 7 ) >> 3 ) << 3;
    p_barcode->p_code->ret_param.dot_h =  p_barcode->p_code->mode_h;

    // 限制高度 未达到特定高度传输两行数据
    if(p_barcode->p_code->mode_h < BARCODE_MIN_H)
    {
        p_barcode->p_code->ret_param.dot_h = 2;
        p_barcode->p_code->ret_param.is_two_lines_data = 1;
    }

    // 需要的字节数超出buf
    if((( p_barcode->p_code->ret_param.dot_w *  p_barcode->p_code->ret_param.dot_h) >> 3) >  p_barcode->p_code->pdat_size)
        return 0;

    // 计算空白区域宽度
    if( p_barcode->p_code->ret_param.dot_w > temp)
    {
        temp =  p_barcode->p_code->ret_param.dot_w - temp;
        p_barcode->left_blank_none += (temp - ((temp >> 1)));
        p_barcode->right_blank_none += (temp >> 1);
    }

    // 清空 buf
    memset(p_barcode->p_code->pdat , 0 , p_barcode->p_code->pdat_size);

    // return _code39_handler(p_barcode);
    return 1;


ERR_RET:
    p_barcode->p_code->ret_param.dot_w = 0;
    p_barcode->p_code->ret_param.dot_h = 0;
    return 0;
}
#endif
/* global functions / API interface -------------------------------------*/
/** 
 * @brief 得到条码
 * 
 * @param p_code 
 * @return int 
 */
int gt_barcode_get(barcode_st * p_code)
{
    bar_uint8_t ret = 0;
    if(NULL == p_code)
        goto ERR_RET;
    
    // 清空返回值 防止客户输入出现错误
    memset(&p_code->ret_param , 0 , sizeof(barcoed_ret_param_st));

    // 清空 _p_barcode
    memset(_p_barcode , 0 , sizeof(_code_st));
    
    _p_barcode->p_code = p_code; 
    
    if(NULL == _p_barcode->p_code->barcode)
        goto ERR_RET;

    if(NULL == _p_barcode->p_code->pdat)
        goto ERR_RET;

    switch (_p_barcode->p_code->type)
    {

#if EAN8_BARCODE_EN
    case GT_FAMILY_BARCODE_TYPE_EAN_8:
        ret = _get_ean8(_p_barcode);
        if(ret)
        {
            ret = _ean8_handler(_p_barcode);
        }
        break;
#endif

#if EAN13_BARCODE_EN
    case GT_FAMILY_BARCODE_TYPE_EAN_13:
        ret = _get_ean13(_p_barcode);
        if(ret)
        {
            ret = _ean13_handler(_p_barcode);
        }
        break;
#endif

#if UPC_A_BARCODE_EN
    case GT_FAMILY_BARCODE_TYPE_UPC_A:
        ret = _get_upc_a(_p_barcode);
        if(ret)
        {
            ret = _upc_a_handler(_p_barcode);
        }
        break;
#endif

#if UPC_E_BARCODE_EN
    case GT_FAMILY_BARCODE_TYPE_UPC_E:
        ret = _get_upc_e(_p_barcode);
        if(ret)
        {
            ret = _upce_handler(_p_barcode);
        }
        break;
#endif

#if ITF_BARCODE_EN
    case GT_FAMILY_BARCODE_TYPE_ITF:
        ret = _get_itf(_p_barcode);
        if(ret)
        {
            ret = _itf_handler(_p_barcode);
        }
        break;
#endif

#if CODABAR_BARCODE_EN
    case GT_FAMILY_BARCODE_TYPE_CODABAR:
        ret = _get_codabar(_p_barcode);
        if(ret)
        {
            ret = _codabar_handler(_p_barcode);
        }
        break;
#endif

#if CODE128_BARCODE_EN
    case GT_FAMILY_BARCODE_TYPE_CODE128A:
    case GT_FAMILY_BARCODE_TYPE_CODE128B:
    case GT_FAMILY_BARCODE_TYPE_CODE128C:
        ret = _get_code128(_p_barcode);
        if(ret)
        {
            ret = _code128_handler(_p_barcode);
        }
        break;
#endif

#if CODE93_BARCODE_EN
    case GT_FAMILY_BARCODE_TYPE_CODE93:
        ret = _get_code93(_p_barcode);
        if(ret)
        {
            ret = _code93_handler(_p_barcode);
        }
        break;
#endif

#if CODE39_BARCODE_EN
    case GT_FAMILY_BARCODE_TYPE_CODE39:
        ret = _get_code39(_p_barcode);
        if(ret)
        {
            ret = _code39_handler(_p_barcode);
        }
        break;
#endif
    default:
        return 0;
    }
    
    return ret;

ERR_RET:

    return 0;
}

int gt_get_barcode_w_and_h(barcode_st * p_code)
{
    bar_uint8_t ret = 0;
    if(NULL == p_code)
        goto ERR_RET;
    
    // 清空返回值 防止客户输入出现错误
    memset(&p_code->ret_param , 0 , sizeof(barcoed_ret_param_st));

    // 清空 _p_barcode
    memset(_p_barcode , 0 , sizeof(_code_st));
    
    _p_barcode->p_code = p_code; 
    
    if(NULL == _p_barcode->p_code->barcode)
        goto ERR_RET;

    if(NULL == _p_barcode->p_code->pdat)
        goto ERR_RET;

    switch (_p_barcode->p_code->type)
    {

#if EAN8_BARCODE_EN
    case GT_FAMILY_BARCODE_TYPE_EAN_8:
        ret = _get_ean8(_p_barcode);
        break;
#endif

#if EAN13_BARCODE_EN
    case GT_FAMILY_BARCODE_TYPE_EAN_13:
        ret = _get_ean13(_p_barcode);
        break;
#endif

#if UPC_A_BARCODE_EN
    case GT_FAMILY_BARCODE_TYPE_UPC_A:
        ret = _get_upc_a(_p_barcode);
        break;
#endif

#if UPC_E_BARCODE_EN
    case GT_FAMILY_BARCODE_TYPE_UPC_E:
        ret = _get_upc_e(_p_barcode);
        break;
#endif

#if ITF_BARCODE_EN
    case GT_FAMILY_BARCODE_TYPE_ITF:
        ret = _get_itf(_p_barcode);
        break;
#endif

#if CODABAR_BARCODE_EN
    case GT_FAMILY_BARCODE_TYPE_CODABAR:
        ret = _get_codabar(_p_barcode);
        break;
#endif

#if CODE128_BARCODE_EN
    case GT_FAMILY_BARCODE_TYPE_CODE128A:
    case GT_FAMILY_BARCODE_TYPE_CODE128B:
    case GT_FAMILY_BARCODE_TYPE_CODE128C:
        ret = _get_code128(_p_barcode);
        break;
#endif

#if CODE93_BARCODE_EN
    case GT_FAMILY_BARCODE_TYPE_CODE93:
        ret = _get_code93(_p_barcode);
        break;
#endif

#if CODE39_BARCODE_EN
    case GT_FAMILY_BARCODE_TYPE_CODE39:
        ret = _get_code39(_p_barcode);
        break;
#endif
    default:
        return 0;
    }

    return ret;

ERR_RET:

    return 0;

}


/* end of file ----------------------------------------------------------*/


