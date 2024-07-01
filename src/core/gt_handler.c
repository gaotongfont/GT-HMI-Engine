/**
 * @file gt_handler.c
 * @author yongg
 * @brief GUI task management interface
 * @version 0.1
 * @date 2022-05-11 15:03:23
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

 /* include --------------------------------------------------------------*/
 #include "./gt_handler.h"
#include "stdbool.h"
#include "../gt_conf.h"
#include "./gt_mem.h"
#include "./gt_timer.h"
#include "../../driver/gt_port_disp.h"
#include "../../driver/gt_port_indev.h"
#include "../../driver/gt_port_vf.h"
#include "../../driver/gt_port_src.h"
#include "../others/gt_anim.h"
#include "../others/gt_log.h"
#include "../others/gt_gc.h"
#include "gt_event.h"
#include "../extra/gt_extra.h"
#include "../core/gt_img_decoder.h"

#if _GT_USE_TEST
#include "../../test/gt_test_rand_widget.h"
#endif

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/

static GT_ATTRIBUTE_LARGE_RAM_ARRAY bool _gt_init_done_flag = false;


/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
#if GT_BOOTING_INFO_MSG
static void gt_print_info_start(void) {
	GT_LOG_A(GT_LOG_TAG_BOOT, "------------------- GT-HMI-Engine Start -------------------");

    GT_LOG_A(GT_LOG_TAG_BOOT, "Date: %s, Time: %s", __DATE__, __TIME__);

    GT_LOG_A(GT_LOG_TAG_BOOT, "Engine version: v%d.%d.%d",
                GT_HMI_ENGINE_MAJOR_VERSION, GT_HMI_ENGINE_MINOR_VERSION, GT_HMI_ENGINE_PATCH_VERSION);

    GT_LOG_A(GT_LOG_TAG_LCD, "Width: %d, Height: %d", GT_SCREEN_WIDTH, GT_SCREEN_HEIGHT);

    GT_LOG_A(GT_LOG_TAG_LCD, "Color depth: %d, swap: [%s]", GT_COLOR_DEPTH, GT_COLOR_16_SWAP ? "Yes" : "No");

    GT_LOG_A(GT_LOG_TAG_LCD, "Refresh max area: %d", _GT_REFR_AREA_MAX);

    GT_LOG_A(GT_LOG_TAG_LCD, "Flush style: %d", GT_REFRESH_STYLE);

    GT_LOG_A(GT_LOG_TAG_TIM, "Indev: %d, Event: %d, Anim: %d, Refr: %d",
                GT_TASK_PERIOD_TIME_INDEV, GT_TASK_PERIOD_TIME_EVENT, GT_TASK_PERIOD_TIME_ANIM, GT_TASK_PERIOD_TIME_REFR);

    GT_LOG_A(GT_LOG_TAG_FS, "Enabled src array: [%s], prefix name: \"%c\"", GT_USE_MODE_SRC ? "Yes" : "No", GT_FS_LABEL_ARRAY);

    GT_LOG_A(GT_LOG_TAG_FS, "Enabled flash or sd: [%s], prefix name: \"%c\"", GT_USE_MODE_FLASH ? "Yes" : "No", GT_FS_LABEL_FLASH);

    GT_LOG_A(GT_LOG_TAG_FS, "Flash begin addr: 0x%x, size: 0x%x", GT_VF_FLASH_START, GT_VF_FLASH_SIZE);

    GT_LOG_A(GT_LOG_TAG_MEM, "Using memory management: [%s]", GT_MEM_CUSTOM ? "Yes" : "No");
}

static void gt_print_info_end(void) {
    GT_LOG_A(GT_LOG_TAG_FS, "Using file header: [%s]", GT_USE_FILE_HEADER ? "Yes" : "No");

#if GT_USE_FILE_HEADER
    _gt_file_header_ctl_st * fh_ctl = &_GT_GC_GET_ROOT(file_header_ctl);
    GT_LOG_A(GT_LOG_TAG_FS, "  file header count: %d", fh_ctl->count);
#endif

    GT_LOG_A(GT_LOG_TAG_MEM, "Using full img buffer: [%s]", GT_USE_EXTRA_FULL_IMG_BUFFER ? "Yes" : "No");

#if GT_USE_EXTRA_FULL_IMG_BUFFER
    _gt_gc_full_img_buffer_st * img_buffer = _gt_gc_get_full_img_buffer();
    GT_LOG_A(GT_LOG_TAG_MEM, "  Buffer addr: 0x%x size: %d", img_buffer->buffer, img_buffer->len);
#endif

	GT_LOG_A(GT_LOG_TAG_BOOT, "------------------- GT-HMI-Engine End ---------------------\r\n");
}
#endif

static void _gt_init_set_flag(bool flag) {
    _gt_init_done_flag = flag;
}

static bool _gt_init_is_done(void) {
    return _gt_init_done_flag;
}

static void _gt_port_init(void) {
#ifndef _GT_PORT_SIMULATOR_ENVS
    gt_port_disp_init();
    gt_port_indev_init();
#endif
    gt_port_vf_init();
    gt_port_src_init();
}

/* global functions / API interface -------------------------------------*/

void gt_init(void)
{
    if (_gt_init_is_done()) {
        return ;
    }

#if GT_BOOTING_INFO_MSG
    gt_print_info_start();
#endif

    gt_mem_init();

    _gt_timer_core_init();

    _gt_anim_core_init();

    _gt_img_decoder_init();

    _gt_extra_init();

    _gt_init_set_flag(true);

    _gt_port_init();

#if GT_BOOTING_INFO_MSG
    gt_print_info_end();
#endif
}

void gt_task_handler(void)
{
    if (!_gt_init_is_done()) {
        return ;
    }
    _gt_timer_handler();

#if _GT_TEST_RAND_WIDGET_EVENT
    _gt_test_rand_widget();
#endif
}



/* end ------------------------------------------------------------------*/
