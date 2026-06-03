/**
 * @file gt_port_disp.h
 * @author yongg
 * @brief The Interface of the display screen
 * @version 0.1
 * @date 2022-06-22 14:41:38
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifdef GT_PORT_DISP_EXT_PATH
    #include GT_PORT_DISP_EXT_PATH
#else

#ifndef _GT_PORT_DISP_H_
#define _GT_PORT_DISP_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/



/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief display buffer init
 */
void gt_port_disp_init(void);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_PORT_DISP_H_

#endif /* GT_PORT_DISP_EXT_PATH */
