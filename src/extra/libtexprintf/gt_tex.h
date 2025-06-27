/**
 * @file gt_tex.h
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2025-06-16 15:46:07
 * @copyright Copyright (c) 2014-2025, Company Genitop. Co., Ltd.
 */
#ifndef _GT_TEX_H_
#define _GT_TEX_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../../gt_conf.h"

#if GT_USE_LIB_TEX_PRINTF
/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief Convert the string to LaTeX format
 *
 * @param str The string to be converted
 * @return char* The converted string
 */
char * gt_tex_string(const char *str);

/**
 * @brief Convert the string to LaTeX format with a specified length
 *
 * @param str The string to be converted
 * @param len The length of the string
 * @return char* The converted string
 */
char * gt_tex_string_by_len(const char *str, int len);


/**
 * @brief Free the memory allocated by gt_tex_string
 *
 * @param ptr The string to be freed
 */
void gt_tex_string_free(char *ptr);


//
#endif /*GT_USE_LIB_TEX_PRINTF*/
//
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_TEX_H_

/* end of file ----------------------------------------------------------*/


