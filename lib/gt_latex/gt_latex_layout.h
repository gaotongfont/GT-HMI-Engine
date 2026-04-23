/**
 * @file gt_latex_layout.h
 * @brief GT-Latex 排版布局 / Layout engine
 *        递归遍历 AST，计算每个节点的坐标、宽高、数学轴
 *        Recursively traverses AST to compute position, size and math axis of each node
 */

#ifndef _GT_LATEX_LAYOUT_H_
#define _GT_LATEX_LAYOUT_H_

#include "gt_latex_config.h"

#if GT_LATEX_ENABLE

#include "gt_latex_parser.h"
#include "gt_latex_font.h"
#include <stdint.h>

/* ----------------------------------------------------------------
 * 布局盒子 / Layout box
 * 描述一个节点在画布上的几何信息 / Geometry info of a node on canvas
 * 坐标系：左上角为原点，x 向右，y 向下 / Origin at top-left, x right, y down
 * ---------------------------------------------------------------- */
typedef struct {
    int16_t  x;         /* 相对父节点的 x 偏移 / X offset relative to parent */
    int16_t  y;         /* 相对父节点的 y 偏移 / Y offset relative to parent */
    uint16_t w;         /* 宽度（像素）/ Width in pixels */
    uint16_t h;         /* 高度（像素）/ Height in pixels */
    uint16_t axis;      /* 数学轴距顶部的距离（对齐参考线）/ Distance from top to math axis (alignment reference) */
} gt_latex_box_t;

/* ----------------------------------------------------------------
 * 首行偏移量 / First line offset
 * ---------------------------------------------------------------- */
typedef struct {
    uint16_t x;         /* x 偏移（像素）/ X offset in pixels */
    uint16_t y;         /* y 偏移（像素）/ Y offset in pixels */
} gt_latex_off_t;

/* ----------------------------------------------------------------
 * 布局节点 / Layout node
 * 与 AST 节点一一对应，存储布局结果 / One-to-one with AST node, stores layout result
 * ---------------------------------------------------------------- */
struct gt_latex_layout_node_s;

typedef struct gt_latex_layout_node_s {
    const gt_latex_node_base_t         *ast_node;       /* 对应的 AST 节点 / Corresponding AST node */
    gt_latex_box_t                      box;            /* 几何信息 / Geometry */
    uint8_t                             font_level;     /* 使用的字体级别 / Font level used */
    uint8_t                             line_thickness; /* 线条粗细（像素）/ Line thickness in pixels */
    struct gt_latex_layout_node_s     **children;       /* 子布局节点（动态数组）/ Child layout nodes (dynamic) */
    uint8_t                             child_count;    /* 子节点数量 / Child count */
    uint8_t                             child_cap;      /* 子节点容量 / Child capacity */
} gt_latex_layout_node_t;

/* ----------------------------------------------------------------
 * 布局上下文 / Layout context
 * ---------------------------------------------------------------- */
typedef struct {
    gt_latex_layout_node_t *root;        /* 布局树根节点 / Layout tree root */
    uint16_t                total_w;     /* 整体宽度 / Total width */
    uint16_t                total_h;     /* 整体高度 / Total height */
    gt_latex_off_t          line_off;    /* 首行偏移 / First line offset */
} gt_latex_layout_t;

/* ----------------------------------------------------------------
 * 接口声明 / API declarations
 * ---------------------------------------------------------------- */

/**
 * @brief 设置字符间距 / Set letter spacing
 * @param s 间距像素数 / Spacing in pixels
 */
void gt_latex_layout_set_letter_spacing(uint8_t s);

/**
 * @brief 获取当前字符间距设置 / Get current letter spacing setting
 * @return 用户设置值（默认 2）/ User-set value (default 2)
 */
uint8_t gt_latex_layout_get_letter_spacing(void);

/**
 * @brief 对 AST 执行布局计算，生成布局树 / Compute layout from AST
 * @param ast       AST 根节点 / AST root node
 * @param layout    输出布局结果 / Output layout result
 * @param level     起始字体级别（通常为基础级 1 或 0）/ Starting font level
 * @param max_width 最大宽度限制（像素），0 表示不限制 / Max width in pixels, 0 = unlimited
 * @return          0 成功，负数失败 / 0 on success, negative on failure
 */
int gt_latex_layout_calc(const gt_latex_node_base_t *ast,
                         gt_latex_layout_t          *layout,
                         uint8_t                     level,
                         uint16_t                    max_width,
                         gt_latex_off_t              line_off);

/**
 * @brief 释放布局树占用的资源 / Free layout tree resources
 * @param layout 布局结果 / Layout result
 */
void gt_latex_layout_deinit(gt_latex_layout_t *layout);

/**
 * @brief 获取整体包围盒尺寸 / Get total bounding box size
 * @param layout 布局结果 / Layout result
 * @param w      输出宽度 / Output width
 * @param h      输出高度 / Output height
 */
void gt_latex_layout_get_size(const gt_latex_layout_t *layout,
                              uint16_t *w, uint16_t *h);

/**
 * @brief 获取公式数学轴（基线到顶部距离）/ Get math axis (distance from top to baseline)
 * @param layout 布局结果 / Layout result
 * @return       axis 值，layout 或 root 为 NULL 时返回 0
 */
uint16_t gt_latex_layout_get_axis(const gt_latex_layout_t *layout);

/**
 * @brief 创建布局节点（动态分配）/ Allocate a layout node
 * @param ast_node 对应的 AST 节点 / Corresponding AST node
 * @return         布局节点指针，失败返回 NULL / Layout node pointer, NULL on failure
 */
gt_latex_layout_node_t *gt_latex_layout_node_create(const gt_latex_node_base_t *ast_node);

/**
 * @brief 递归释放布局节点 / Recursively free layout node
 * @param node 布局节点 / Layout node
 */
void gt_latex_layout_node_free(gt_latex_layout_node_t *node);

#endif /* GT_LATEX_ENABLE */

#endif /* _GT_LATEX_LAYOUT_H_ */
