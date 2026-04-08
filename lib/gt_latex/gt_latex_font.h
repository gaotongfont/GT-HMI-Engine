/**
 * @file gt_latex_font.h
 * @brief GT-Latex 字体系统 / Font system
 *        管理字体级别配置，通过回调获取字形数据，与具体字库完全解耦
 *        Manages font level config, fetches glyph via callback, decoupled from font data
 */

#ifndef _GT_LATEX_FONT_H_
#define _GT_LATEX_FONT_H_

#include "gt_latex_config.h"

#if GT_LATEX_ENABLE

#include <stdint.h>

/* ----------------------------------------------------------------
 * 字体模式枚举 / Font mode enum
 * ---------------------------------------------------------------- */
typedef enum {
    GT_LATEX_FONT_MODE_3 = 3,   /* 3级模式：基础、一级脚本、二级脚本 / 3-level mode */
    GT_LATEX_FONT_MODE_5 = 5,   /* 5级模式：大型运算符、基础、一级脚本、二级脚本、小标注 / 5-level mode */
} gt_latex_font_mode_t;

/* ----------------------------------------------------------------
 * 字体级别配置 / Font level config
 * ---------------------------------------------------------------- */
typedef struct {
    uint16_t size;          /* 该级字号（像素高度）/ Font size in pixels */
    uint16_t font_type;     /* 字体类型，由用户定义含义，透传给回调 / Font type, user-defined */
} gt_latex_font_level_t;

/* ----------------------------------------------------------------
 * 字形尺寸信息（布局用）/ Glyph metrics (for layout)
 * 只含宽高和数学轴，不含 bitmap / Width, height and math axis only, no bitmap
 * ---------------------------------------------------------------- */
typedef struct {
    uint8_t  font_w;    /* 文字实际宽度（像素）/ Actual glyph width in pixels */
    uint16_t height;    /* 字形高度（像素），等于字号 / Glyph height in pixels */
    uint16_t axis;      /* 数学轴距顶部距离（像素）/ Math axis distance from top */
} gt_latex_glyph_metrics_t;

/* ----------------------------------------------------------------
 * 外部字体尺寸回调类型 / External font metrics callback type
 * 仅用于布局阶段获取宽高，不涉及 bitmap / Used in layout to get size only
 * @param code      字符编码（Unicode）/ Character code (Unicode)
 * @param level_cfg 当前级别配置 / Current level config
 * @param metrics   输出宽高信息 / Output metrics
 * @param user_data 用户数据指针 / User data pointer
 * @return          0 成功，负数失败 / 0 on success, negative on failure
 * ---------------------------------------------------------------- */
typedef int (*gt_latex_font_get_cb)(uint32_t code,
                                    const gt_latex_font_level_t *level_cfg,
                                    gt_latex_glyph_metrics_t    *metrics,
                                    void                        *user_data);

/* ----------------------------------------------------------------
 * 接口声明 / API declarations
 * ---------------------------------------------------------------- */

/**
 * @brief 初始化字体模块，加载默认配置 / Init font module with default config
 */
void gt_latex_font_init(void);

/**
 * @brief 设置字体模式 / Set font mode
 * @param mode GT_LATEX_FONT_MODE_3 或 GT_LATEX_FONT_MODE_5
 */
void gt_latex_font_set_mode(gt_latex_font_mode_t mode);

/**
 * @brief 获取当前字体模式 / Get current font mode
 * @return 当前模式 / Current mode
 */
gt_latex_font_mode_t gt_latex_font_get_mode(void);

/**
 * @brief 设置某一级的字体配置 / Set font level config
 * @param level     级别索引（0 起）/ Level index (0-based)
 * @param size      字号（像素）/ Font size in pixels
 * @param font_type 字体类型（用户自定义）/ Font type (user-defined)
 */
void gt_latex_font_set_level(uint8_t level, uint16_t size, uint16_t font_type);

/**
 * @brief 获取某一级的字体配置 / Get font level config
 * @param level 级别索引 / Level index
 * @return 指向配置的指针，level 越界返回 NULL / Pointer to config, NULL if out of range
 */
const gt_latex_font_level_t *gt_latex_font_get_level(uint8_t level);

/**
 * @brief 注册外部字体获取回调 / Register external font fetch callback
 * @param cb        回调函数指针 / Callback function pointer
 * @param user_data 用户数据指针，回调时会透传 / User data pointer, passed to callback
 */
void gt_latex_font_register(gt_latex_font_get_cb cb, void *user_data);

/**
 * @brief 单独设置字体回调函数（保留当前 user_data）/ Set font callback only (preserve current user_data)
 * @param cb 回调函数指针 / Callback function pointer
 */
void gt_latex_font_set_callback(gt_latex_font_get_cb cb);

/**
 * @brief 单独设置全局 user_data（保留当前回调）/ Set global user_data only (preserve current callback)
 * @param user_data 用户数据指针 / User data pointer
 */
void gt_latex_font_set_user_data(void *user_data);

/**
 * @brief 获取指定级别的字形尺寸（布局用）/ Get glyph metrics at given level (for layout)
 * @param level   级别索引 / Level index
 * @param code    字符编码 / Character code
 * @param metrics 输出宽高信息 / Output metrics
 * @return        0 成功，负数失败 / 0 on success, negative on failure
 */
int gt_latex_font_get_metrics(uint8_t level, uint32_t code,
                               gt_latex_glyph_metrics_t *metrics);

/**
 * @brief 设置临时字体模式（优先级高于默认模式）/ Set temporary font mode (higher priority than default)
 * @param mode 临时模式，GT_LATEX_FONT_MODE_3 或 GT_LATEX_FONT_MODE_5
 * @note 设置后，gt_latex_font_get_mode() 将返回此临时值
 *       调用 gt_latex_font_clear_temp() 可清除临时设置
 */
void gt_latex_font_set_temp_mode(gt_latex_font_mode_t mode);

/**
 * @brief 设置临时字体级别配置（优先级高于默认配置）/ Set temporary font level config (higher priority than default)
 * @param level     级别索引（0 起）/ Level index (0-based)
 * @param size      字号（像素）/ Font size in pixels
 * @param font_type 字体类型（用户自定义）/ Font type (user-defined)
 * @note 设置后，gt_latex_font_get_level() 将返回此临时值
 *       调用 gt_latex_font_clear_temp() 可清除临时设置
 */
void gt_latex_font_set_temp_level(uint8_t level, uint16_t size, uint16_t font_type);

/**
 * @brief 清除所有临时字体设置，恢复为默认配置 / Clear all temporary font settings, restore defaults
 * @note 清除后，gt_latex_font_get_mode() 和 gt_latex_font_get_level()
 *       将返回通过 gt_latex_font_set_mode() 和 gt_latex_font_set_level() 设置的默认值
 */
void gt_latex_font_clear_temp(void);

/**
 * @brief 设置临时 user_data（优先级高于全局 user_data）/ Set temporary user_data (higher priority than global)
 * @param user_data 用户数据指针 / User data pointer
 * @note 设置后，字体回调将接收此临时 user_data
 *       调用 gt_latex_font_clear_temp() 可清除临时设置
 */
void gt_latex_font_set_temp_user_data(void *user_data);

/**
 * @brief 单独清除临时 user_data（不影响其他临时设置）/ Clear temporary user_data only (preserve other temp settings)
 * @note 清除后，将使用全局 user_data
 */
void gt_latex_font_clear_temp_user_data(void);

/**
 * @brief 获取当前 user_data / Get current user_data
 *
 * @return void* 用户数据指针 / User data pointer
 */
const void* gt_latex_font_get_user_data(void);
#endif /* GT_LATEX_ENABLE */

#endif /* _GT_LATEX_FONT_H_ */
