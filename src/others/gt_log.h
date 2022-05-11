/**
 * @file gt_log.h
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-05-11 15:12:39
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_LOG_H_
#define _GT_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/



/* define ---------------------------------------------------------------*/
#define GT_LOG_INFO(info)       do{printf("[%s][%s][%d]\n",__FILE__,__func__)}while(0)
#define GT_LOG_WARN(warn)       do{}while(0)
#define GT_LOG_ERROR(err)       do{}while(0)


/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_LOG_H_
