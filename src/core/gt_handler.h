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



/* define ---------------------------------------------------------------*/

/**
 * @brief 主版本号
 */
#define GT_HMI_ENGINE_MAJOR_VERSION 1
/**
 * @brief 次版本号
 */
#define GT_HMI_ENGINE_MINOR_VERSION 3
/**
 * @brief 修订号
 */
#define GT_HMI_ENGINE_PATCH_VERSION 0

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

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_HANDLER_H_
