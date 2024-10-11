/**
 * @file gt_bin_convert.h
 * @author Feyoung
 * @brief bin hex buffer convert to target object struct or print to hex buffer
 * @version 0.1
 * @date 2024-09-23 11:48:12
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_BIN_CONVERT_H_
#define _GT_BIN_CONVERT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../../gt_conf.h"

#if GT_USE_BIN_CONVERT

#include "../../others/gt_types.h"
#include "stddef.h"



/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
/**
 * @brief buffer data parse to object(target)
 * @param tar object
 * @param buffer hex data
 * @param len hex data byte length
 * @param
 */
typedef gt_res_t ( * parse_cb_t)(void * const, uint8_t const * const , uint32_t);

/**
 * @brief object(target) print(output) to buffer data
 * @param tar object
 * @param res_buffer get hex data
 * @param len hex data byte length
 */
typedef gt_res_t ( * print_cb_t)(void const * const, uint8_t * const , uint32_t);

/**
 * @brief bin item struct
 */
typedef struct gt_bin_item_s {
    uint16_t len;           /** current callback handler Number of bytes occupied */
    parse_cb_t parse_cb;    /** callback: hex buffer data parse to object */
    print_cb_t print_cb;    /** callback: object print to hex buffer */
}gt_bin_item_st;

/**
 * @brief bin analysis struct
 */
typedef struct gt_bin_convert_s {
    void * tar;         /** pointer or strcut object */
    uint8_t * buffer;   /** hex bin data buffer */
    uint32_t byte_len;  /** buffer byte length */
}gt_bin_convert_st;

typedef struct gt_bin_res_s {
    gt_res_t res;
    uint16_t index;    /** The index of the current bin item get error or inv */
}gt_bin_res_st;

/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief Get the total byte length of the bin item list
 *
 * @param list such as:
 *              gt_bin_item_st format[] = {
 *                  // { len, parse_call_back_func, print_call_back_func },
 *                  { 0x2, _param_0_parse_cb, _param_0_print_cb },
 *                  { 0x8, _param_1_parse_cb, _param_1_print_cb },
 *                  { 0x0, (parse_cb_t)NULL, (print_cb_t)NULL }  // [Warn] End of list, len must be set to 0.
 *               };
 * @return uint32_t total byte length
 */
uint32_t gt_bin_get_total_byte_length(gt_bin_item_st const * const list);

/**
 * @brief According to the list of callback functions, the data in the bin
 *      is parsed and the parse_cb() is called.
 *      The parsed data is stored in the target object.
 *
 * @param bin The hex data to be parsed
 * @param list such as:
 *              gt_bin_item_st format[] = {
 *                  // { len, parse_call_back_func, print_call_back_func },
 *                  { 0x2, _param_0_parse_cb, _param_0_print_cb },
 *                  { 0x8, _param_1_parse_cb, _param_1_print_cb },
 *                  { 0x0, (parse_cb_t)NULL, (print_cb_t)NULL }  // [Warn] End of list, len must be set to 0.
 *               };
 * @return gt_res_t GT_RES_OK or GT_RES_FAIL; index of the current bin item get error or inv
 */
gt_bin_res_st gt_bin_convert_parse(gt_bin_convert_st const * const bin, gt_bin_item_st const * const list);

/**
 * @brief According to the list of callback functions, the tar object is printed
 *      to the buffer data, which all print_cb() will be called.
 *
 * @param bin The tar object will be printed to the buffer data
 * @param list such as:
 *              gt_bin_item_st format[] = {
 *                  // { len, parse_call_back_func, print_call_back_func },
 *                  { 0x2, _param_0_parse_cb, _param_0_print_cb },
 *                  { 0x8, _param_1_parse_cb, _param_1_print_cb },
 *                  { 0x0, (parse_cb_t)NULL, (print_cb_t)NULL }  // [Warn] End of list, len must be set to 0.
 *               };
 * @return gt_res_t GT_RES_OK or GT_RES_FAIL; index of the current bin item get error or inv
 */
gt_bin_res_st gt_bin_convert_print(gt_bin_convert_st const * const bin, gt_bin_item_st const * const list);


/* ------------------- convert api ------------------- */

/**
 *  @brief Convert the hex buffer to a uint64_t value by len
 *
 * @param buffer The hex buffer to be converted
 * @return uint32_t The converted value
 */
uint64_t  gt_convert_parse_by_len(uint8_t const * const buffer, uint8_t len);

/**
 * @brief Convert the value to a hex buffer
 *
 * @param res result buffer to store the converted value
 * @param val value to be converted
 * @return uint16_t byte length
 */
uint16_t gt_convert_print_by_len(uint8_t * const res, uint64_t val, uint8_t len);

/**
 *  @brief Convert the hex buffer to a uint16_t value
 *
 * @param buffer The hex buffer to be converted
 * @return uint16_t The converted value
 */
uint16_t gt_convert_parse_u16(uint8_t const * const buffer);

/**
 * @brief Convert the value to a hex buffer
 *
 * @param res result buffer to store the converted value
 * @param val value to be converted
 * @return uint16_t byte length
 */
uint16_t gt_convert_print_u16(uint8_t * const res, uint16_t val);

/**
 *  @brief Convert the hex buffer to a uint32_t value
 *
 * @param buffer The hex buffer to be converted
 * @return uint32_t The converted value
 */
uint32_t gt_convert_parse_u32(uint8_t const * const buffer);

/**
 * @brief Convert the value to a hex buffer
 *
 * @param res result buffer to store the converted value
 * @param val value to be converted
 * @return uint16_t byte length
 */
uint16_t gt_convert_print_u32(uint8_t * const res, uint32_t val);

/**
 * @brief Convert the hex buffer to a float value
 *
 * @param buffer The hex buffer to be converted
 * @return float The converted value
 */
float gt_convert_parse_float(uint8_t const * const buffer);

/**
 * @brief Convert the hex buffer to a double value
 *
 * @param buffer The hex buffer to be converted
 * @return double The converted value
 */
double gt_convert_parse_double(uint8_t const * const buffer);

/**
 * @brief Convert the hex buffer to a int16_t value
 *
 * @param buffer The hex buffer to be converted
 * @return int16_t The converted value
 */
int16_t gt_convert_parse_i16(uint8_t const * const buffer);

/**
 * @brief Convert the hex buffer to a int32_t value
 *
 * @param buffer The hex buffer to be converted
 * @return int32_t The converted value
 */
int32_t gt_convert_parse_i32(uint8_t const * const buffer);

/**
 * @brief Convert the hex buffer to a int64_t value
 *
 * @param buffer The hex buffer to be converted
 * @return int64_t The converted value
 */
int64_t gt_convert_parse_i64(uint8_t const * const buffer);



#endif   /** GT_USE_BIN_CONVERT */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_BIN_CONVERT_H_
