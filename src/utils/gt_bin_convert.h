/**
 * @file gt_bin_convert.h
 * @author Feyoung
 * @brief
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

#endif   /** GT_USE_BIN_CONVERT */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_BIN_CONVERT_H_
