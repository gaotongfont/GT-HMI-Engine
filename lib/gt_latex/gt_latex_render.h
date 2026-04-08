/**
 * @file gt_latex_render.h
 * @brief GT-Latex 渲染输出 / Render engine
 *        遍历布局树，通过外部回调将字形和图元绘制到画布
 *        Traverses layout tree and draws glyphs/primitives via external callbacks
 */

#ifndef _GT_LATEX_RENDER_H_
#define _GT_LATEX_RENDER_H_

#include "gt_latex_config.h"

#if GT_LATEX_ENABLE

#include "gt_latex_layout.h"
#include "gt_latex_font.h"
#include <stdint.h>

/* ----------------------------------------------------------------
 * 外部绘制回调类型 / External draw callback types
 * 由用户实现，适配具体 GUI 系统 / Implemented by user to adapt GUI system
 * ---------------------------------------------------------------- */

/**
 * 绘制字形 / Draw glyph
 * 外部负责根据 code + level_cfg 取 bitmap 并绘制，渲染层不介入字库
 * External side fetches bitmap from font and draws; render layer stays out of font data
 * @param x         字形左上角 x / Glyph top-left x
 * @param y         字形左上角 y / Glyph top-left y
 * @param code      字符编码（Unicode）/ Character code (Unicode)
 * @param level_cfg 字体级别配置（含 size / font_type）/ Font level config
 * @param color     前景色 / Foreground color
 * @param user_data 用户数据 / User data
 */
typedef void (*gt_latex_draw_glyph_cb)(int16_t x, int16_t y,
                                       uint32_t code,
                                       const gt_latex_font_level_t *level_cfg,
                                       uint32_t color, void *user_data);

/**
 * 绘制水平线（用于分数线）/ Draw horizontal line (for fraction bar)
 * @param x         起始 x / Start x
 * @param y         y 坐标 / Y coordinate
 * @param w         线段宽度（像素）/ Line width in pixels
 * @param thickness 线条粗细（像素）/ Line thickness in pixels
 * @param color     颜色 / Color
 * @param user_data 用户数据 / User data
 */
typedef void (*gt_latex_draw_hline_cb)(int16_t x, int16_t y,
                                       uint16_t w, uint8_t thickness,
                                       uint32_t color, void *user_data);

/**
 * 绘制斜线段（用于根号勾）/ Draw line segment (for sqrt tick)
 * @param x0        起点 x / Start x
 * @param y0        起点 y / Start y
 * @param x1        终点 x / End x
 * @param y1        终点 y / End y
 * @param thickness 线条粗细（像素）/ Line thickness in pixels
 * @param color     颜色 / Color
 * @param user_data 用户数据 / User data
 */
typedef void (*gt_latex_draw_line_cb)(int16_t x0, int16_t y0,
                                      int16_t x1, int16_t y1,
                                      uint8_t thickness,
                                      uint32_t color, void *user_data);

/**
 * 绘制圆弧（用于括号）/ Draw circular arc (for delimiters)
 * 角度以度为单位，0° = 右（3点钟方向），顺时针增大
 * Angles in degrees, 0° = right (3 o'clock), clockwise
 * @param cx        圆心 x / Center x
 * @param cy        圆心 y / Center y
 * @param r         半径（像素）/ Radius in pixels
 * @param start_deg 起始角度（0~360）/ Start angle in degrees
 * @param end_deg   终止角度（0~360）/ End angle in degrees
 * @param thickness 线条粗细（像素）/ Line thickness in pixels
 * @param color     颜色 / Color
 * @param user_data 用户数据 / User data
 */
typedef void (*gt_latex_draw_arc_cb)(int16_t cx, int16_t cy,
                                     uint16_t r,
                                     uint16_t start_deg, uint16_t end_deg,
                                     uint8_t thickness,
                                     uint32_t color, void *user_data);

/* ----------------------------------------------------------------
 * 画布描述 / Canvas descriptor
 * ---------------------------------------------------------------- */
typedef struct {
    int16_t  x;             /* 公式渲染起始 x 坐标 / Formula start x */
    int16_t  y;             /* 公式渲染起始 y 坐标 / Formula start y */
    uint32_t fg_color;      /* 前景色（字形、线条）/ Foreground color */
    uint32_t bg_color;      /* 背景色（可选）/ Background color */
    void    *user_data;     /* 透传给绘制回调的用户数据 / User data passed to callbacks */
} gt_latex_canvas_t;

/* ----------------------------------------------------------------
 * 接口声明 / API declarations
 * ---------------------------------------------------------------- */

/**
 * @brief 注册外部绘制回调 / Register external draw callbacks
 * @param glyph_cb  字形位图绘制回调 / Glyph bitmap draw callback
 * @param hline_cb  水平线绘制回调 / Horizontal line draw callback
 * @param line_cb   斜线段绘制回调 / Line segment draw callback
 * @param arc_cb    圆弧绘制回调（可为 NULL，NULL 时括号退化为折线）/ Arc draw callback (NULL = fallback to polyline)
 */
void gt_latex_render_register(gt_latex_draw_glyph_cb glyph_cb,
                               gt_latex_draw_hline_cb hline_cb,
                               gt_latex_draw_line_cb  line_cb,
                               gt_latex_draw_arc_cb   arc_cb);

/**
 * @brief 渲染布局树到画布 / Render layout tree to canvas
 * @param layout 布局结果 / Layout result
 * @param canvas 画布描述 / Canvas descriptor
 * @return       0 成功，负数失败 / 0 on success, negative on failure
 */
int gt_latex_render_draw(const gt_latex_layout_t *layout,
                         const gt_latex_canvas_t *canvas);

#endif /* GT_LATEX_ENABLE */

#endif /* _GT_LATEX_RENDER_H_ */
