/**
 * @file gt_serial_cfg.h
 * @author Feyoung
 * @brief Serial configuration
 * @version 0.1
 * @date 2024-09-20 16:06:38
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_SERIAL_CFG_H_
#define _GT_SERIAL_CFG_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../../gt_conf.h"
#include "../../others/gt_types.h"

#if GT_USE_SERIAL


/* define ---------------------------------------------------------------*/
#ifndef GT_USE_SERIAL_CFG
    /**
     * @brief Serial configuration
     *      [default: 0]
     */
    #define GT_USE_SERIAL_CFG       01
#endif

#endif  /** GT_USE_SERIAL */


#if GT_USE_SERIAL_CFG && GT_USE_BIN_CONVERT

/* typedef --------------------------------------------------------------*/

typedef struct {
    uint8_t crc_checksum : 1;
    uint8_t beep : 1;                       /** 0: beep; 1: music */
    uint8_t load_22 : 1;
    uint8_t touch_auto_upload : 1;
    uint8_t touch_beep : 1;
    uint8_t touch_back_light_sleep : 1;
    uint8_t disp_direct : 2;
}gt_serial_cfg_param_0_st;

typedef struct {
    uint8_t pwm_output : 1;
    uint8_t nand_flash_expand : 1;
    uint8_t nand_flash_format_clean_once : 1;       /** Reset to 0 after clean */
    uint8_t nand_flash_size : 1;                    /** 0: 1G bits; 1: 4G bits */
    uint8_t nand_flash_expand_format_clean : 1;
    uint8_t ad_resolution : 2;                      /** 0: 12bit; 1: 16bit */
    uint8_t reserved : 1;
    uint8_t sd_download_os_run : 1;
}gt_serial_cfg_param_1_st;

typedef struct {
    gt_serial_cfg_param_0_st param_0;
    gt_serial_cfg_param_1_st param_1;
    uint8_t wae_id;
    uint8_t icl_id;
    uint8_t touch_report_rate;          /** Range 0x01-0xff; [default 0x28, rate: 400Hz/val] */
    uint32_t uart_baud_rate;            /** rate = 3225600 / val; 115200bps = 0x001c, max = 0x3FF */
}gt_serial_cfg_system_config_st;

typedef struct {
    uint8_t brightness;                 /** 0x00-0x64: 1% */
    uint8_t brightness_sleep;           /** 0x00-0x64: 1% */
    uint16_t wake_up_time;              /** 0x0001-0xffff: 10ms */
}gt_serial_cfg_backlight_sleep_st;

typedef struct {
    uint16_t disp_enabled;              /** 0x5aa5 valid */
    uint8_t pll_clk_phs;                /** 0x00: falling edge; 0x01: rising edge */
    uint8_t pll_clk_div;
    uint8_t hor_w;
    uint8_t hor_s;
    uint16_t hor_d;                     /** resolution radio */
    uint8_t hor_e;
    uint8_t ver_w;
    uint8_t ver_s;
    uint16_t ver_d;                     /** resolution radio */
    uint8_t ver_e;
    uint8_t tcon_sel;                   /** 0x00: no need set TCON */
}gt_serial_cfg_display_st;

/**
 *  7-4b type: 0x0*
 *  3b: 电阻屏校准, 在SD下载时启用
 *  2-0b: [电容屏有效]
 *  2b: hor direct 0: 0 - X_max; 1: X_max - 0;
 *  1b: ver direct 0: 0 - Y_max; 1: Y_max - 0;
 *  0b: switch x/y: 0: xy; 1: yx.
 */
typedef struct {
    uint8_t type : 4;
    uint8_t resistance_calibration : 1;
    uint8_t hor_dir : 1;
    uint8_t ver_dir : 1;
    uint8_t switch_hor_ver : 1;
}gt_serial_cfg_touch_mode_st;

typedef struct {
    uint8_t reserved;                   /** 0x00 */
    uint8_t enabled;                    /** 0x5A: valid */
    gt_serial_cfg_touch_mode_st mode;
    /**
     * default: 0x14
     */
    uint8_t sense;
    uint8_t freq;       /** 0x01-0x14 fixed; 0x00 flex */
}gt_serial_cfg_touch_st;

typedef struct {
    uint8_t set_en;     /** 0x5A */
    uint8_t enabled;
    uint8_t div;
}gt_serial_cfg_clk_output_st;

typedef struct {
    uint8_t set_en;             /** 0x5a valid */
    uint8_t freq_div1;          /** 825753.6 / (freq_div1 * freq_div1) KHz */
    uint16_t freq_div2;         /** default: div1 = 0x6e, div2 = 0x0bb8, equal to 2.5 KHz */
    uint16_t freq_duty;         /** high level%: freq_duty / freq_div2; default: 0x00f0 = 8% */
    uint8_t time;               /** default: 0x0A; unit: 10ms */
}gt_serial_cfg_beep_st;

typedef struct {
    uint8_t set_en;         /** 0x5a valid */
    uint8_t file_id;        /** default: 0x16 */
}gt_serial_cfg_init_file_st;

typedef struct {
    uint8_t set_en;         /** 0x5a valid */
    uint8_t adj_set;        /** 0x00 */
}gt_serial_cfg_sys_clk_st;

typedef struct {
    uint8_t set_en;         /** 0x5a valid */
    uint8_t icl_id;         /** 0x00: cannot replace */
}gt_serial_cfg_pic_replace_st;

typedef struct {
    uint8_t period : 1;
    uint8_t uart1_crc : 1;
    uint8_t reserved : 6;   /** 0x00 */
}_expand_param_st;

typedef struct {
    uint8_t set_en_start;   /** 0x5a Start an extended parameter configuration */
    uint8_t set_en;         /** 0x5a valid */
    _expand_param_st param_1;
    uint8_t reserved[9];
}gt_serial_cfg_expand_param_st;

typedef struct {
    uint16_t set_en;        /** 0x5a, 0xa5 valid, save it in flash on the screen */
    uint8_t name_len;       /** folder name length range: 0x00-0x08 */
    uint8_t folder_name[8]; /** folder name */
    uint8_t reserved[37];   /** 0x00 */
}gt_serial_cfg_sd_st;

typedef struct {
    uint8_t set_en;         /** 0x5a valid */
    uint8_t delay;          /** default: 0x0a, unit: 10ms */
}gt_serial_cfg_backlight_delay_st;

typedef struct {
    uint32_t addr_or_id;    /** expand flash flash address or area id, maybe 4k algin */
    uint32_t var_addr;
    uint32_t crc32_checksum;

    uint8_t handler_type;   /** @ref _write_expand_128mbit_flash() 0xAA */
    uint8_t is_running;     /** 0x00: done; 0x01: running */
    uint16_t font_id;
    uint16_t count_block;   /** the count of block to store */
    uint16_t delay_to_next; /** delay to next command */
}gt_serial_cfg_expand_flash_st;

typedef struct gt_serial_cfg_s {
    uint8_t header[5];
    gt_serial_cfg_system_config_st   system_config;      /** system config */
    gt_serial_cfg_backlight_sleep_st backlight_sleep;    /** backlight or sleep */
    gt_serial_cfg_display_st         display;            /** display */
    gt_serial_cfg_touch_st           tp;                 /** touch */
    gt_serial_cfg_clk_output_st      clk;                /** clk output */
    gt_serial_cfg_beep_st            beep;
    gt_serial_cfg_init_file_st       init_file;
    gt_serial_cfg_sys_clk_st         sys_clk;
    gt_serial_cfg_pic_replace_st     pic_replace;        /** picture replace */
    gt_serial_cfg_expand_param_st    expand_param;
    gt_serial_cfg_sd_st              sd;                 /** SD download folder setting */
    gt_serial_cfg_backlight_delay_st backlight_delay;    /** backlight delay time */
    uint8_t reserved[14];

    /* ------------------- expand param ------------------- */

    gt_serial_cfg_expand_flash_st   expand_flash_hd;
}gt_serial_cfg_st;


/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief Read serial configuration from file
 */
void gt_serial_cfg_init(void);

/**
 * @brief Get serial configuration object
 *
 * @return gt_serial_cfg_st * const
 */
gt_serial_cfg_st * const gt_serial_cfg_get_root(void);



#endif  /** GT_USE_SERIAL_CFG && GT_USE_BIN_CONVERT */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_SERIAL_CFG_H_
