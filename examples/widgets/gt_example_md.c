/**
 * @file gt_example_md.c
 * @author Feyoung
 * @brief Markdown widget example, used to reproduce the character-overlap
 *      rendering issue (字符重叠显示) reported in gtmd-render-report.html:
 *      - 问题②: 表格中文长列宽度虚高 -> 文字重叠/无网格
 *      - 问题⑤: 行内样式混排 + 自动折行 -> 文本重复渲染
 *      - M-7: 嵌套列表状态污染 -> 父列表续项编号/标记被子列表污染
 *        （列表用例置于屏幕最前，打开即见；各节括号内为预期结果）
 *      - 嵌套子列表按层级缩进（每级 2 空格），与父列表区分层级关系
 * @version 0.2
 * @date 2026-11-06
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_markdown.h"
#include <string.h>

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/* 复现源：
 *  - M-7 嵌套列表状态污染（放在最前，打开屏幕即见；预期结果见各节注释）
 *  - 表格中文长列 + 行内样式混排长段落（问题②/⑤） */
static const char _md_source[] =
    "# 渲染问题复现\n"
    "\n"
    "## M-7 嵌套列表状态污染\n"
    "\n"
    "**有序嵌套有序**（父列表续项应为 2. 3.；修复前被污染为 4. 5.）：\n"
    "\n"
    "1. 甲项\n"
    "   1. 子一\n"
    "   2. 子二\n"
    "   3. 子三\n"
    "2. 乙项\n"
    "3. 丙项\n"
    "\n"
    "**有序嵌套无序**（退出嵌套后父项应恢复数字标记；修复前变成圆点）：\n"
    "\n"
    "1. 甲项\n"
    "   - 子一\n"
    "   - 子二\n"
    "   - 子三\n"
    "2. 乙项\n"
    "3. 丙项\n"
    "\n"
    "**无序嵌套无序**（父/子均应显示圆点；修复前父项被编号）：\n"
    "\n"
    "- 甲项\n"
    "  - 子一\n"
    "  - 子二\n"
    "- 乙项\n"
    "- 丙项\n"
    "\n"
    "**有序列表起始编号**（应从 3 开始依次递增）：\n"
    "\n"
    "3. 起始三\n"
    "4. 起始四\n"
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
