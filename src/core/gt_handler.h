/**
 * @file gt_handler.h
 * @author yongg
 * @brief GUI task management interface
 * @version 0.1
 * @date 2022-05-11 15:07:33
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_HANDLER_H_
#define _GT_HANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "stdbool.h"
#include "stdint.h"
#include "../gt_conf.h"



/* define ---------------------------------------------------------------*/

#ifndef GT_HMI_ENGINE_MAJOR_VERSION
    /**
     * @brief 主版本号
     */
    #define GT_HMI_ENGINE_MAJOR_VERSION     1
#endif

#ifndef GT_HMI_ENGINE_MINOR_VERSION
    /**
     * @brief 次版本号
     */
    #define GT_HMI_ENGINE_MINOR_VERSION     4
#endif

#ifndef GT_HMI_ENGINE_PATCH_VERSION
    /**
     * @brief 修订号
     */
    #define GT_HMI_ENGINE_PATCH_VERSION     14
#endif

/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief GT-HMI all init
 */
void gt_init(void);

/**
 * @brief GT-HMI task handler function called by task
 */
void gt_task_handler(void);

void gt_task_set_paused(bool paused);
bool gt_task_is_paused(void);

bool gt_task_is_running(void);

bool GT_ATTRIBUTE_RAM_TEXT gt_task_check_idle_timeout(uint32_t timeout);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_HANDLER_H_
