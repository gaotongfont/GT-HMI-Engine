/**
 * @file tex_config.h
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2025-06-16 14:43:04
 * @copyright Copyright (c) 2014-2025, Company Genitop. Co., Ltd.
 */
#ifndef _TEX_CONFIG_H_
#define _TEX_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../../gt_conf.h"
#include "../../core/gt_mem.h"

#if GT_USE_LIB_TEX_PRINTF
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
/* define ---------------------------------------------------------------*/

#if 01
    #define _TEX_MALLOC(size)  			gt_mem_malloc(size)
    #define _TEX_REALLOC(ptr, size)  	gt_mem_realloc(ptr, size)
    #define _TEX_CALLOC(num, size)  	gt_mem_calloc(num, size)
    #define _TEX_FREE(ptr)      		gt_mem_free(ptr)
    #define _TEX_FPRINTF(...)  		    GT_LOGE(__VA_ARGS__)  /* GT_LOGE(__VA_ARGS__) */
#else
    #define _TEX_MALLOC(size)  			malloc(size)
    #define _TEX_REALLOC(ptr, size)  	realloc(ptr, size)
    #define _TEX_CALLOC(num, size)  	calloc(num, size)
    #define _TEX_FREE(ptr)      		free(ptr)
    #define _TEX_FPRINTF(...)  		    fprintf(__VA_ARGS__)
#endif

/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/


#endif //!GT_USE_LIB_TEX_PRINTF
//
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_TEX_CONFIG_H_

/* end of file ----------------------------------------------------------*/


