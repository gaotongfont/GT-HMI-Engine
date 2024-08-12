#include "gt_gui_driver.h"
#include <string.h>

typedef enum _gt_font_charset_s {
    GT_CHARSET_UTF8 = 0,
    GT_CHARSET_GB ,
}gt_font_charset_st;
static gt_font_charset_st gt_font_chinese_encoding = GT_CHARSET_UTF8;
void GT_Set_Chinese_Charset_Type(unsigned char charset)
{
    gt_font_chinese_encoding = charset;
}
unsigned char GT_Get_Chinese_Charset_Type(void)
{
    return gt_font_chinese_encoding;
}

unsigned int GBKToUnicode(unsigned int fontcode)
{
    return 0;
}
unsigned int UnicodeToGBK(unsigned int fontcode)
{
    return 0;
}


const struct gt_font_func_s gt_get_font_func[] = {
    {.option = 0 , .func.read_dot_font = NULL , .type = 1},
};
/*------------------------------------------------------------------ Separation -----------------------------------------------------------------*/
/* global functions / API interface -------------------------------------*/

/**
* @brief 得到文字的宽度
*
* @param fontcode 文字编码
* @param font_option 字体选择
* @param size 文字大小（矢量使用）
* @param gray 文字灰度（矢量使用）
* @param pBits 存储数据（矢量使用）
* @return int 宽度
*/
int GT_Get_Font_Width(unsigned long fontcode, unsigned int font_option, unsigned int size , unsigned int gray , unsigned char* pBits)
{
    return 0;
}
/**
* @brief 得到文字的高度
*
* @param fontcode 文字编码
* @param font_option 字体选择
* @param size 文字大小（矢量使用
* @param gray 文字灰度（矢量使用）
* @return int 宽度
*/
int GT_Get_Font_Height(unsigned long fontcode, unsigned int font_option, unsigned int size , unsigned int gray)
{
    return 0;
}
/**
* @brief 检查字体是否是矢量字体
*
* @param font_option 字体选择
* @return true 是矢量
* @return false 不是矢量
*/
char GT_Check_Is_Vec(unsigned int font_option)
{
    return 0;
}
/**
 * @brief 得到文字字体读取函数
 *
 * @param style 字体类型
 * @return struct gt_font_func_s*  @gt_font_func_s
 */
struct gt_font_func_s* GT_Get_Font_Func_S(unsigned int style)
{
    return NULL;
}
unsigned char GT_Get_Font_Type(unsigned int style)
{
    return 0;
}

int GT_Font_Code_Transform(font_convertor_st* convert)
{
    unsigned int len = 0;

    if (NULL == convert)
    {
        return len;
    }

    len = convert->code_len;
    switch (convert->font_option)
    {

        default:
            break;
    }

    return len;
}
int GT_Get_Font_Convertor_Data(font_convertor_st* convert, uint32_t pos)
{
    unsigned int len = 0;

    if (NULL == convert)
    {
        return len;
    }

    len = convert->code_len;
    switch (convert->font_option)
    {

        default:
            break;
    }

    return len;
}
/*------------------------------------------------------------------ end of file -----------------------------------------------------------------*/
