/**
 * @file gt_tex.c
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2025-06-16 15:45:58
 * @copyright Copyright (c) 2014-2025, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_tex.h"

#if GT_USE_LIB_TEX_PRINTF
#include "texprintf.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static prototypes ----------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
char * gt_tex_string(const char *str)
{
    uint32_t len = str ? strlen(str) : 0;
    char *s = gt_mem_malloc(len + 1);
    if(!s) {
        GT_LOGE(GT_LOG_TAG_GUI,"malloc failed!!! len: %d", len + 1);
        return NULL;
    }
    gt_memset_0(s, len + 1);
    gt_memcpy(s, str, len);
    s[len] = '\0';

    char* ret = texstring(s);
    gt_mem_free(s);
    return ret;
}

char * gt_tex_string_by_len(const char *str, int len)
{
    char *s = gt_mem_malloc(len + 1);
    if(!s) {
        GT_LOGE(GT_LOG_TAG_GUI,"malloc failed!!! len: %d", len + 1);
        return NULL;
    }
    gt_memcpy(s, str, len);
    s[len] = '\0';

    char* ret = texstring(s);
    gt_mem_free(s);
    return ret;
}

void gt_tex_string_free(char *ptr)
{
    _TEX_FREE(ptr);
}


#endif /* GT_USE_LIB_TEX_PRINTF  */
/* end of file ----------------------------------------------------------*/

