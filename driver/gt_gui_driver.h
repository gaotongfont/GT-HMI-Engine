#ifdef GT_GUI_DRIVER_EXT_PATH
    #include GT_GUI_DRIVER_EXT_PATH
#else

#ifndef _GT_GUI_DRIVER_H_
#define _GT_GUI_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"

//字库初始化
int GT_Font_Init(void);

typedef enum {
	FONT_OPTION_MAX_LEN,
}font_option_et;

typedef struct gt_font_func_s
{
    uint32_t option;
    union
    {
        int (*read_dot_font)(unsigned long , unsigned int , unsigned char*);
        int (*read_vec_font)(unsigned char* , unsigned int ,unsigned long ,unsigned int , unsigned char , unsigned char);
        int (*read_img_font)(unsigned long , unsigned int , unsigned char*, unsigned char*, unsigned int);
    }func;
    unsigned char type;
}gt_font_func_st;


/**
 * @brief  Unicode转GBK
 * @param  fontcode	Unicode编码
 * @retval int  转换后编码
 */
unsigned int UnicodeToGBK(unsigned int fontcode);
/**
 * @brief  Unicode转JIS0208
 * @param  fontcode	Unicode编码
 * @retval int  转换后编码
 */
unsigned int UnicodeToJIS0208(unsigned int fontcode);
/**
 * @brief  Unicode转KSC5601
 * @param  fontcode	Unicode编码
 * @retval int  转换后编码
 */
unsigned int UnicodeToKSC5601(unsigned int fontcode);
/**
 * @brief  GBK转Unicode
 * @param  fontcode	GBK编码
 * @retval int  转换后编码
 */
unsigned int GBKToUnicode(unsigned int fontcode);
/**
 * @brief  BIG5转GBK
 * @param  fontcode	BIG5编码
 * @retval int  转换后编码
 */
unsigned int BIG5ToGBK(unsigned int fontcode);
/**
 * @brief  ShiftJis转JIS0208
 * @param  fontcode	ShiftJis编码
 * @retval int  转换后编码
 */
unsigned int Shift_JIS_To_JIS0208(unsigned int fontcode);

/* macros ---------------------------------------------------------------*/

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
int GT_Get_Font_Width(unsigned long fontcode, unsigned int font_option, unsigned int size , unsigned int gray , unsigned char* pBits);

/**
* @brief 得到文字的高度
*
* @param fontcode
* @param font_option 字体选择
* @param size 文字大小（矢量使用
* @param gray 文字灰度（矢量使用）
* @return int 宽度
*/
int GT_Get_Font_Height(unsigned long fontcode, unsigned int font_option, unsigned int size , unsigned int gray);

/**
* @brief 检查字体是否是矢量字体
*
* @param font_option 字体选择
* @return true 是矢量
* @return false 不是矢量
*/
char GT_Check_Is_Vec(unsigned int font_option);

/**
 * @brief 得到文字字体读取函数
 *
 * @param style 字体类型
 * @return struct gt_font_func_s*  @gt_font_func_s
 */
struct gt_font_func_s* GT_Get_Font_Func_S(unsigned int style);
unsigned char GT_Get_Font_Type(unsigned int style);
unsigned char GT_Get_Chinese_Charset_Type(void);
void GT_Set_Chinese_Charset_Type(unsigned char charset);

typedef struct font_convertor_s
{
    uint16_t *fontcode ;
    uint16_t fontsize ;
    uint16_t font_option ;

    uint8_t fontgray;
    uint8_t thick;
    uint8_t * data_p;

    bool is_rev ; // false is normal, true is reverse
    bool is_vec ;

    uint32_t code_len ;
    uint32_t data_len ; // Minimum three-character data storage length

    int (*read_dot_font)(unsigned long , unsigned int , unsigned char*);
    int (*read_vec_font)(unsigned char* , unsigned int ,unsigned long ,unsigned int , unsigned char , unsigned char);
}font_convertor_st;

int GT_Font_Code_Transform(font_convertor_st *convert);
int GT_Get_Font_Convertor_Data(font_convertor_st *convert , uint32_t pos);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif


/*------------------------------------------------------------------ end of file -----------------------------------------------------------------*/

#endif /* GT_GUI_DRIVER_EXT_PATH */
