/**
 * @file gt_hal_vf.h
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-06-20 17:58:44
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_HAL_VF_H_
#define _GT_HAL_VF_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "stdint.h"
#include "stdio.h"



/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/

/**********************************************************************
 * @brief virt file system file message
 *********************************************************************/
typedef struct _gt_vfs_s{
    char * name;
    uint16_t w;
    uint16_t h;
    uint32_t addr;
    uint32_t size;
}gt_vfs_st;


/**********************************************************************
 * @brief virt file drv operate result
 *********************************************************************/
typedef enum{
    GT_VF_RES_FAIL = -1,
    GT_VF_RES_OK,
    GT_VF_RES_HW_ERR,
    GT_VF_RES_NOT_EX,
    GT_VF_RES_DEINIT,
    GT_VF_RES_BUSY,
    GT_VF_RES_READY,
    GT_VF_RES_NULL,
}gt_vf_res_e;

typedef enum _gt_vf_type_e{
    GT_VF_TYPE_FLASH,
    GT_VF_TYPE_ARRAY,
}gt_vf_type_et;

/**********************************************************************
 * @brief virt file drv open_cb return type
 *********************************************************************/
typedef struct _gt_vfp_s{
    uint32_t start;
    uint32_t end;
    uint32_t pos;

    uint8_t type;   //file type flash or array,this can be @gt_vf_type_et

    union {
        struct{
            uint32_t w;
            uint32_t h;
            uint32_t type;
        }pic;
        struct{
            uint32_t encoding;
        }txt;
    }msg;
    void * buffer;
}gt_vfp_st;

/*********************************************************************
 * @brief seek set pos type
 *********************************************************************/
typedef enum {
    GT_VF_SEEK_SET = 0x00,      /**< Set the position from absolutely (from the start of file)*/
    GT_VF_SEEK_CUR = 0x01,      /**< Set the position from the current position*/
    GT_VF_SEEK_END = 0x02,      /**< Set the position from the end of the file*/
} gt_vf_whence_e;

/**********************************************************************
 * @brief virt file system drv
 **********************************************************************/
typedef struct _gt_vf_drv_s{
    /* user set: spi read write func */
    void (*rw_cb)(uint8_t * data_write, uint32_t len_write, uint8_t * data_read, uint32_t len_read);

    /* do not change */
    void *(*open_cb)(struct _gt_vf_drv_s * drv, char * name);
    void (*close_cb)(struct _gt_vf_drv_s * drv, void * fp);
    gt_vf_res_e (*read_cb)(struct _gt_vf_drv_s * drv, void * fp, uint8_t * data, uint32_t len);
    gt_vf_res_e (*write_cb)(struct _gt_vf_drv_s * drv, void * fp, uint8_t * data, uint32_t len);
    gt_vf_res_e (*seek_cb)(struct _gt_vf_drv_s * drv, void * fp, uint32_t pos, gt_vf_whence_e whence);
    gt_vf_res_e (*tell_cb)(struct _gt_vf_drv_s * drv, void * fp, uint32_t * pos);
}gt_vf_drv_st;


/***********************************************************************
 * @brief virt file system device
 ***********************************************************************/
typedef struct _gt_vf_s{
    gt_vf_drv_st * drv;
    uint32_t addr_max;
    uint32_t addr_start;
}gt_vf_st;


/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief init virt file system
 *
 * @param vfs gt_vfs_st arr,pic files messages
 */
void gt_vf_init(gt_vfs_st * vfs);

/**
 * @brief register virt file system drv
 *
 * @param drv gt_vf_drv_st device, user need only set _gt_vf_rw
 */
void gt_vf_drv_register(gt_vf_drv_st * drv);

/**
 * @brief get default virt file system dev
 *
 * @return gt_vf_st *
 */
gt_vf_st * gt_vf_get_default(void);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_VIRT_FILE_H_
