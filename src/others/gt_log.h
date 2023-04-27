/**
 * @file gt_log.h
 * @author Feyoung
 * @brief The gui log information
 * @version 0.1
 * @date 2022-04-25 10:43:59
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_LOG_H_
#define _GT_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/

#include "stdio.h"

/* define ---------------------------------------------------------------*/

/**
 * @brief 打开日志功能
 */
#define GT_USE_LOG

#ifdef GT_USE_LOG

/**
 * @brief 映射底层打印接口
 */
#define _GT_PRINT          printf

//日志/调试信息
#define GT_LOG_LEVEL_VERBOSE    1   //冗余信息, 全打印
#define GT_LOG_LEVEL_DEBUG      2
#define GT_LOG_LEVEL_INFO       3
#define GT_LOG_LEVEL_WARNING    4
#define GT_LOG_LEVEL_ERROR      5
#define GT_LOG_LEVEL_CRASH      6


//字库相关能显示的调试信息等级
#define GT_LOG_LEVEL_DEFALUT    5//GT_LOG_LEVEL_DEBUG

/* TAG ------------------------------------------------------------------*/

#define GT_LOG_TAG_CHECK        "CHECK"
#define GT_LOG_TAG_MEM          "MEM"
#define GT_LOG_TAG_FLASH        "FLASH"
#define GT_LOG_TAG_LIST         "LIST"
#define GT_LOG_TAG_LCD          "LCD"
#define GT_LOG_TAG_TP           "TP"
#define GT_LOG_TAG_TIM          "TIM"
#define GT_LOG_TAG_UI           "UI"
#define GT_LOG_TAG_PSRAM        "PSRAM"
#define GT_LOG_TAG_SRAM         "SRAM"
#define GT_LOG_TAG_DATA         "DATA"
#define GT_LOG_TAG_SPI          "SPI"
#define GT_LOG_TAG_FONT         "FONT"
#define GT_LOG_TAG_EXTI         "EXTI"
#define GT_LOG_TAG_QUEUE        "QUEUE"
#define GT_LOG_TAG_FAULT        "FAULT"
#define GT_LOG_TAG_BOOT         "BOOT"
#define GT_LOG_TAG_EXTRA        "EXTRA"
#define GT_LOG_TAG_GUI          "GUI"
#define GT_LOG_TAG_ASSERT       "ASSERT"

#define GT_LOG_TAG_TEST         "TEST"


/* global functions / API interface -------------------------------------*/

//冗余信息
#if defined(GT_USE_LOG) && (GT_LOG_LEVEL_DEFALUT <= GT_LOG_LEVEL_VERBOSE)
    #define GT_LOGV(_tag, _fmt, _arg...)                                                        \
        do {                                                                                    \
            _GT_PRINT("[GT_VERB] [%s] [%s:%s():%d] " _fmt "\n", _tag, __FILE__, __func__, __LINE__, ##_arg); \
        }while(0)
#else
    #define GT_LOGV(_tag, _fmt, _arg...)    ((void)0)
#endif

//调试信息
#if defined(GT_USE_LOG) && (GT_LOG_LEVEL_DEFALUT <= GT_LOG_LEVEL_DEBUG)
    #define GT_LOGD(_tag, _fmt, _arg...)                                                        \
        do {                                                                                    \
            _GT_PRINT("[GT_DEBG] [%s] [%s:%s():%d] " _fmt "\n", _tag, __FILE__, __func__, __LINE__, ##_arg); \
        }while(0)
#else
    #define GT_LOGD(_tag, _fmt, _arg...)    ((void)0)
#endif

//日志信息
#if defined(GT_USE_LOG) && (GT_LOG_LEVEL_DEFALUT <= GT_LOG_LEVEL_INFO)
    #define GT_LOGI(_tag, _fmt, _arg...)                                                        \
        do {                                                                                    \
            _GT_PRINT("[GT_INFO] [%s] [%s:%s():%d] " _fmt "\n", _tag, __FILE__, __func__, __LINE__, ##_arg); \
        }while(0)
#else
    #define GT_LOGI(_tag, _fmt, _arg...)    ((void)0)
#endif

//警告信息
#if defined(GT_USE_LOG) && (GT_LOG_LEVEL_DEFALUT <= GT_LOG_LEVEL_WARNING)
    #define GT_LOGW(_tag, _fmt, _arg...)                                                        \
        do {                                                                                    \
            _GT_PRINT("[GT_WARN] [%s] [%s:%s():%d] " _fmt "\n", _tag, __FILE__, __func__, __LINE__, ##_arg); \
        }while(0)
#else
    #define GT_LOGW(_tag, _fmt, _arg...)    ((void)0)
#endif

//错误信息
#if defined(GT_USE_LOG) && (GT_LOG_LEVEL_DEFALUT <= GT_LOG_LEVEL_ERROR)
    #define GT_LOGE(_tag, _fmt, _arg...)                                                        \
        do {                                                                                    \
            _GT_PRINT("[GT_ERRO] [%s] [%s:%s():%d] " _fmt "\n", _tag, __FILE__, __func__, __LINE__, ##_arg); \
        }while(0)
#else
    #define GT_LOGE(_tag, _fmt, _arg...)    ((void)0)
#endif

//崩溃捕获, 生成断定
#define GT_CATCH_FAULT()                                                                        \
    do {                                                                                        \
        GT_LOGE(GT_LOG_TAG_FAULT, "catching fault!!!");                                         \
        while(1);                                                                               \
    }while(0)

#else
#define GT_CATCH_FAULT()  ((void)0)
#endif  //!GT_USE_LOG

//崩溃信息
#if defined(GT_USE_LOG) && (GT_LOG_LEVEL_DEFALUT <= GT_LOG_LEVEL_CRASH)
    #define GT_LOGC(_tag, _fmt, _arg...)                                                        \
        do {                                                                                    \
            _GT_PRINT("[GT_CRAS] [%s] [%s:%s():%d] " _fmt "\n", _tag, __FILE__, __func__, __LINE__, ##_arg); \
            GT_CATCH_FAULT();                                                                   \
        }while(0)
#else
    #define GT_LOGC(_tag, _fmt, _arg...)    ((void)0)
#endif

/* value check ------------------------------------------------------------*/

/**
 * @brief 变量检查, 并跳出函数.
 * @param _value 检查对象
 */
#define GT_CHECK_BACK(_value)                                                               \
    do {                                                                                    \
        if (!_value) {                                                                      \
            GT_LOGE(GT_LOG_TAG_CHECK, "[%s] value check failed!", #_value);                 \
            return ;                                                                        \
        }                                                                                   \
    }while(0)

/**
 * @brief 变量检查, 并跳出函数.
 * @param _value 检查对象
 * @param _ret_code 检查失败, 返回的错误码
 */
#define GT_CHECK(_value, _ret_code)                                                         \
    do {                                                                                    \
        if (!_value) {                                                                      \
            GT_LOGE(GT_LOG_TAG_CHECK, "[%s] value check failed!", #_value);                 \
            return _ret_code;                                                               \
        }                                                                                   \
    }while(0)

/**
 * @brief 变量检查, 并跳到处理标签处.
 * @param _value 检查对象
 * @param _label 跳转到的标签
 */
#define GT_CHECK_GOTO(_value, _label)                                                       \
    do {                                                                                    \
        if (!_value) {                                                                      \
            GT_LOGE(GT_LOG_TAG_CHECK, "[%s] value check failed!", #_value);                 \
            goto _label;                                                                    \
        }                                                                                   \
    }while(0)

#define GT_LOG_COLOR_ARR(_arr, _start, _len)      	\
	do{																								\
		int i=_start;																		\
		printf("\n");																		\
		while(i<_len+_start){														\
			printf("%s[%d]=%04x  ",#_arr,i,_arr[i].full);								\
			i++;																					\
		}																								\
		printf("\n");																		\
	}while(0);


#define GT_LOG_ARR(_arr, _start, _len)  \
    do{																								\
		int i=_start;																		\
		printf("\n");																		\
		while(i<_len+_start){														\
			printf("%s[%d]=%02x  ",#_arr,i,_arr[i]);								\
			i++;																					\
		}																								\
		printf("\n");																		\
	}while(0);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_LOG_H_
