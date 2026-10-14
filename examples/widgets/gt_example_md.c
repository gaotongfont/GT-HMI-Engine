/**
 * @file gt_example_md.c
 * @author Feyoung
 * @brief Markdown widget example, used to reproduce the character-overlap
 *      rendering issue (字符重叠显示) reported in gtmd-render-report.html:
 *      - 问题②: 表格中文长列宽度虚高 -> 文字重叠/无网格
 *      - 问题⑤: 行内样式混排 + 自动折行 -> 文本重复渲染
 * @version 0.1
 * @date 2026-10-14
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_markdown.h"
#include <string.h>

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/* 复现源：表格中文长列 + 行内样式混排长段落 */
static const char _md_source[] =
    "# 渲染问题复现\n"
    "\n"
    "以下表格用于复现**问题②**：中文长列（如\"主轴转速\"\"循环周期\"）\n"
    "会导致列宽虚高，从而出现文字重叠或无网格线。\n"
    "\n"
    "| 参数 | 数值 |\n"
    "|------|------|\n"
    "| 主轴转速 | 1200 |\n"
    "| 循环周期 | 46 |\n"
    "| 进给速度 | 800 |\n"
    "| 加工时间 | 12 |\n"
    "\n"
    "以下段落用于复现**问题⑤**：行内样式混排与自动折行，前面这段普通文本接近一行，"
    "**这段加粗文字将把内容推向下一行。**\n"
    "\n"
    "后续段落：用于验证上方混合样式段落的高度计算是否准确，若上方高度被低估，本段将与上方文字重叠。\n";

/* static variables -----------------------------------------------------*/

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/

/* global functions / API interface -------------------------------------*/

/**
 * @brief Create a markdown screen that reproduces the character-overlap issue.
 *
 * @return gt_obj_st* The created screen object.
 */
gt_obj_st * gt_examples_md(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);

    gt_obj_st * markdown = gt_markdown_create(screen);
    gt_obj_set_pos(markdown, 10, 10);
    gt_obj_set_size(markdown, GT_SCREEN_WIDTH - 20, GT_SCREEN_HEIGHT - 50);
    gt_markdown_set_font_color(markdown, gt_color_black());
    /* [Warn] 必须先在 gt_obj_set_size() 之后调用 */
    gt_markdown_set_text(markdown, _md_source, (uint32_t)strlen(_md_source));

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
