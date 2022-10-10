/**
 * @file gt_port_vf.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-06-22 09:58:01
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_port_vf.h"
#include "../gt.h"
#include "../src/others/gt_log.h"


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/
static const gt_vfs_st virt_file_sys[] ={
	/*name, w, h, addr, size*/
    {"1.png",400,157,0,0},
    {"2.png",400,160,125600,0},
    {"3.png",400,299,253600,0},
    // {NULL,0,0,0,0},

    // {"1.BMP",31,13,0,0},
    // {"10.BMP",31,13,806,0},
    // {"11.BMP",31,13,1612,0},
    // {"12.BMP",31,13,2418,0},
    // {"13.BMP",31,13,3224,0},
    // {"14.BMP",31,13,4030,0},
    // {"15.BMP",31,13,4836,0},
    // {"16.BMP",31,13,5642,0},
    // {"17.BMP",31,13,6448,0},
    // {"18.BMP",31,13,7254,0},
    // {"19.BMP",31,13,8060,0},
    // {"2.BMP",31,13,8866,0},
    // {"20.BMP",31,13,9672,0},
    // {"21.BMP",31,13,10478,0},
    // {"3.BMP",31,13,11284,0},
    // {"4.BMP",31,13,12090,0},
    // {"5.BMP",31,13,12896,0},
    // {"6.BMP",31,13,13702,0},
    // {"7.BMP",31,13,14508,0},
    // {"8.BMP",31,13,15314,0},
    // {"9.BMP",31,13,16120,0},
    // {"22.BMP",31,13,16926,0},

    {"baoyu.png",80,80,0,0},
    {"cai4.png",80,80,12800,0},
    {"cai6.png",80,80,25600,0},
    {"chaomian.png",80,80,38400,0},
    {"chashao.png",80,80,51200,0},
    {"duojiaoyutou.png",80,80,64000,0},
    {"gongbaojiding.png",80,80,76800,0},
    {"kourou.png",80,80,89600,0},
    {"liangcai.png",80,80,102400,0},
    {"muer.png",80,80,115200,0},
    {"QRCode.png",330,330,128000,0},
    {"shaoya.png",80,80,345800,0},
    {"tianpin.png",80,80,358600,0},
    {"xiaochaorou.png",80,80,371400,0},
    {NULL,0,0,0,0},
};
static gt_fs_drv_st drv_vf;

/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/


/* global functions / API interface -------------------------------------*/

#if GT_USE_MODE_FLASH
/**
 * @brief [flash] write or read data from flash
 *
 * @param data_write the data to be written
 * @param len_write the length of the data to be written
 * @param data_read the data to be read from flash
 * @param len_read the length need to read from flash
 *
 * @return uint32_t The number of bytes data read back preferentially,
 *      otherwise the number of bytes written successfully.
 */
extern uint32_t spi_wr(uint8_t const * const data_write, uint32_t len_write, uint8_t * data_read, uint32_t len_read);

#endif
void gt_port_vf_init(void)
{
    gt_vf_init(virt_file_sys);
#if GT_USE_MODE_FLASH
	drv_vf.rw_cb = spi_wr;
#else
	drv_vf.rw_cb = NULL;
#endif
    gt_vf_drv_register(&drv_vf);
}



/* end ------------------------------------------------------------------*/
