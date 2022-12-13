/**
 * @file barcode.h
 * @author Yang (your@email.com)
 * @brief 
 * @version 0.1
 * @date 2022-06-16 13:44:03
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_BARCODE_SRC_H_
#define _GT_BARCODE_SRC_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/



/* define ---------------------------------------------------------------*/
#define EAN13_BARCODE_EN        1
#define EAN8_BARCODE_EN         1
#define UPC_A_BARCODE_EN        1
#define UPC_E_BARCODE_EN        1
#define ITF_BARCODE_EN          1
#define CODABAR_BARCODE_EN      1
#define CODE128_BARCODE_EN      1
#define CODE93_BARCODE_EN		1
#define CODE39_BARCODE_EN       1
/* typedef --------------------------------------------------------------*/
typedef unsigned char   bar_uint8_t;
typedef unsigned short  bar_uint16_t;
typedef unsigned int    bar_uint32_t;

typedef enum
{
    GT_FAMILY_BARCODE_HRI_TYPE_NONE  = 0,    // 不显示
    GT_FAMILY_BARCODE_HRI_TYPE_DOWN ,        // 下方
    GT_FAMILY_BARCODE_HRI_TYPE_UP ,          // 上方
    GT_FAMILY_BARCODE_HRI_TYPE_ALL,          // 上下方
}hri_type_te;

typedef enum
{
    GT_FAMILY_BARCODE_TYPE_EAN_13 = 0,
    GT_FAMILY_BARCODE_TYPE_EAN_8,
    GT_FAMILY_BARCODE_TYPE_UPC_A,
    GT_FAMILY_BARCODE_TYPE_UPC_E,
    GT_FAMILY_BARCODE_TYPE_ITF,
    GT_FAMILY_BARCODE_TYPE_CODABAR,
    GT_FAMILY_BARCODE_TYPE_CODE128A,
    GT_FAMILY_BARCODE_TYPE_CODE128B,
    GT_FAMILY_BARCODE_TYPE_CODE128C,
	GT_FAMILY_BARCODE_TYPE_CODE93,
    GT_FAMILY_BARCODE_TYPE_CODE39,

}barcode_type_te;

typedef struct 
{
    bar_uint16_t dot_w;
    bar_uint16_t dot_h;
    bar_uint8_t check_numb;       // 校验码1
	bar_uint8_t check_numb2;
    bar_uint8_t is_two_lines_data ;   // 1 两行数据  0 正常高度数据
}barcoed_ret_param_st;

typedef struct
{
    char* barcode;
    bar_uint8_t mode_w;       // 模块宽度 (1 - 6)     0 为默认为 2
    bar_uint8_t mode_h;       // 模块高度 (1 - 255)   小于32输出两行数据
    bar_uint8_t type ;        // 条码类型  @ barcode_type_te
    bar_uint8_t hri_type;     // hir 字符  @ hri_type_e
    bar_uint8_t upc_e_sys;    // ucp-e条码系统字符    0 or 1
    
    bar_uint8_t *pdat;        // 条码显示数据存储区
    bar_uint32_t pdat_size;     // pdat 大小

    // 返回值
    barcoed_ret_param_st ret_param;
}barcode_st;

/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
int gt_barcode_get(barcode_st * p_code);

int gt_barcode_get_w_and_h(barcode_st * p_code);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_BARCODE_H_

/* end of file ----------------------------------------------------------*/


