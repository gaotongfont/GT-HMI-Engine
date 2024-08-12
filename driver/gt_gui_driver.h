#ifndef _GT_GUI_DRIVER_H_
#define _GT_GUI_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
/* 外部函数声明 */
extern unsigned long r_dat_bat(unsigned long address, unsigned long DataLen, unsigned char* pBuff);
/*------------------------------------------------------------------  数据地址  -----------------------------------------------------------------*/

typedef enum {
	FONT_OPTION_MAX_LEN,
}font_option_et;

typedef struct gt_font_func_s
{
    font_option_et option;
    union
    {
        int (*read_dot_font)(unsigned long , unsigned int , unsigned char*);
        int (*read_vec_font)(unsigned char* , unsigned int ,unsigned long ,unsigned int , unsigned char , unsigned char);
    }func;
    unsigned char type;
}gt_font_func_st;

/**
* @brief  读取中文的点阵数据
* @param  fontcode：		中文编码
* @param  font_option：	字体选择
* @param  pbuffer：		存储读取出的点阵数据
* @retval 读取的数据个数
*/
int GT_GetChinese(unsigned long fontcode, unsigned int font_option, unsigned char* pbuffer);

/**
* @brief  读取英文的点阵数据
* @param  fontcode：		英文编码
* @param  font_option：	字体选择
* @param  pbuffer：		存储读取出的点阵数据
* @retval 读取的数据个数
*/
int GT_GetASCII(unsigned long fontcode, unsigned int font_option, unsigned char* pbuffer);

/**
* @brief  读取日文的点阵数据
* @param  fontcode：		日文编码
* @param  font_option：	字体选择
* @param  pbuffer：		存储读取出的点阵数据
* @retval 读取的数据个数
*/
int GT_GetJapanese(unsigned long fontcode, unsigned int font_option, unsigned char* pbuffer);

/**
* @brief  读取韩文的点阵数据
* @param  fontcode：		韩文编码
* @param  font_option：	字体选择
* @param  pbuffer：		存储读取出的点阵数据
* @retval 读取的数据个数
*/
int GT_GetKorean(unsigned long fontcode, unsigned int font_option, unsigned char* pbuffer);

/**
* @brief  读取拉丁文的点阵数据
* @param  fontcode：		拉丁文编码
* @param  font_option：	字体选择
* @param  pbuffer：		存储读取出的点阵数据
* @retval 读取的数据个数
*/
int GT_GetLatin(unsigned long fontcode, unsigned int font_option, unsigned char* pbuffer);

/**
* @brief  读取西里尔文的点阵数据
* @param  fontcode：		西里尔文编码
* @param  font_option：	字体选择
* @param  pbuffer：		存储读取出的点阵数据
* @retval 读取的数据个数
*/
int GT_GetCyrillic(unsigned long fontcode, unsigned int font_option, unsigned char* pbuffer);

/**
* @brief  读取希腊文的点阵数据
* @param  fontcode：		希腊文编码
* @param  font_option：	字体选择
* @param  pbuffer：		存储读取出的点阵数据
* @retval 读取的数据个数
*/
int GT_GetGreek(unsigned long fontcode, unsigned int font_option, unsigned char* pbuffer);

/**
* @brief  读取希伯来文的点阵数据
* @param  fontcode：		希伯来文编码
* @param  font_option：	字体选择
* @param  pbuffer：		存储读取出的点阵数据
* @retval 读取的数据个数
*/
int GT_GetHebrew(unsigned long fontcode, unsigned int font_option, unsigned char* pbuffer);

/**
* @brief  读取阿拉伯文的点阵数据
* @param  fontcode：		阿拉伯文编码
* @param  font_option：	字体选择
* @param  pbuffer：		存储读取出的点阵数据
* @retval 读取的数据个数
*/
int GT_GetArabic(unsigned long fontcode, unsigned int font_option, unsigned char* pbuffer);

/**
* @brief  读取泰文的点阵数据
* @param  fontcode：		泰文编码
* @param  font_option：	字体选择
* @param  pbuffer：		存储读取出的点阵数据
* @retval 读取的数据个数
*/
int GT_GetThai(unsigned long fontcode, unsigned int font_option, unsigned char* pbuffer);

/**
* @brief  读取印地文的点阵数据
* @param  fontcode：		印地文编码
* @param  font_option：	字体选择
* @param  pbuffer：		存储读取出的点阵数据
* @retval 读取的数据个数
*/
int GT_GetHindi(unsigned long fontcode, unsigned int font_option, unsigned char* pbuffer);


/******************* 两种调用模式配置 *******************/
/**
 * 方式一 VEC_ST_MODE : 通过使用声明VECFONT_ST结构体变量, 配置结构体信息,
 *   获取点阵数据到zk_buffer[]数组中.
 * 方式二 VEC_PARM_MODE : 通过指定参数进行调用, 获取点阵数据到pBits[]数组中.
 * ps: 两种方式可同时配置使用, 择一使用亦可.
*/
#define VEC_ST_MODE
#define VEC_PARM_MODE

/********************* 分割线 *********************/

#ifdef VEC_ST_MODE

    #define ZK_BUFFER_LEN   4608    //可修改大小, 约等于 字号*字号/8.

    typedef struct vecFont
    {
        unsigned long fontCode;		//字符编码中文:GB18030, ASCII/外文: unicode
        unsigned char type;			//字体	@矢量公用部分
        unsigned char size;			//文字大小
        unsigned char thick;		//文字粗细
        unsigned char zkBuffer[ZK_BUFFER_LEN];	//数据存储
    }VECFONT_ST;

    unsigned int get_font_st(VECFONT_ST * font_st);
#endif

#ifdef VEC_PARM_MODE
	/*
	 *函数名：	get_font()
	 *功能：		矢量文字读取函数
	 *参数：pBits		数据存储
	 *		sty			文字字体选择  @矢量公用部分
	 *		fontCode	字符编码中文:GB18030, ASCII/外文: unicode
	 *		width		文字宽度
	 *		height		文字高度
	 *		thick		文字粗细
	 *返回值：文字显示宽度
	**/
    unsigned int get_font(unsigned char *pBits,unsigned char sty,unsigned long fontCode,unsigned char width,unsigned char height, unsigned char thick);
#endif
/********************* 矢量区域结束 *********************/

/*
 *函数名：	get_Font_Gray()
 *功能		灰度矢量文字读取函数
 *参数：pBits		数据存储
 *		sty			文字字体选择  @矢量公用部分
 *		fontCode	字符编码中文:GB18030, ASCII/外文: unicode
 *		fontSize	文字大小
 *		thick		文字粗细
 *返回值：re_buff[0] 字符的显示宽度 , re_buff[1] 字符的灰度阶级[1阶/2阶/3阶/4阶]
**/
unsigned int* get_Font_Gray(unsigned char *pBits,unsigned char sty,unsigned long fontCode,unsigned char fontSize, unsigned char thick);

/*----------------------------------------------------------------------------------------
 * 灰度数据转换函数 2阶灰度/4阶灰度
 * 说明 : 将点阵数据转换为灰度数据 [eg:32点阵数据转2阶灰度数据则转为16点阵灰度数据]
 * 参数 ：
 *   data灰度数据;  x,y=显示起始坐标 ; w 宽度, h 高度,grade 灰度阶级[1阶/2阶/4阶]
 *------------------------------------------------------------------------------------------*/
void Gray_Process(unsigned char *OutPutData ,int width,int High,unsigned char Grade);

/**
  * @brief  点阵字体宽度获取函数
  * @param  p        ：字体点阵数据
  * @param  zfwidth        ：字体的宽
  * @param  zfhigh        ：字体的长
  * @retval 字体宽度值
  */
unsigned int get_width_func_vec(unsigned char *p,unsigned int zfwidth,unsigned int zfhigh);

/**
 * @brief 得到矢量字库文字数据
 *
 * @param pBits 数据存储
 * @param font_option 字体
 * @param fontcode 文字编码
 * @param fontsize 文字大小
 * @param fontgray 文字灰度
 * @param thick    粗细
 * @return int 读取的数据个数
 */
int GT_GetVec(unsigned char *pBits,unsigned int font_option ,unsigned long fontcode,unsigned int fontsize , unsigned char fontgray, unsigned char thick);


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
    uint8_t * data ;

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
