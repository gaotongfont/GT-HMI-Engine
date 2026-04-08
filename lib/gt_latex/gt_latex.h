/**
 * @file gt_latex.h
 * @brief GT-Latex 对外统一接口 / Unified public API
 *        串联 lexer → parser → layout → render 全流程
 *        Chains lexer → parser → layout → render pipeline
 */

#ifndef _GT_LATEX_H_
#define _GT_LATEX_H_

#include "gt_latex_config.h"

#if GT_LATEX_ENABLE

#include "gt_latex_font.h"
#include "gt_latex_lexer.h"
#include "gt_latex_parser.h"
#include "gt_latex_layout.h"
#include "gt_latex_render.h"

/* ----------------------------------------------------------------
 * 错误码 / Error codes
 * ---------------------------------------------------------------- */
#define GT_LATEX_OK             0   /* 成功 / Success */
#define GT_LATEX_ERR_PARAM     -1   /* 参数错误 / Invalid parameter */
#define GT_LATEX_ERR_NOMEM     -2   /* 内存不足 / Out of memory */
#define GT_LATEX_ERR_SYNTAX    -3   /* 语法错误 / Syntax error */
#define GT_LATEX_ERR_SYMBOL    -4   /* 未知符号 / Unknown symbol */
#define GT_LATEX_ERR_FONT      -5   /* 字体获取失败 / Font fetch failed */
#define GT_LATEX_ERR_NO_CB     -6   /* 未注册回调 / Callback not registered */

/* ----------------------------------------------------------------
 * 接口声明 / API declarations
 * ---------------------------------------------------------------- */

/**
 * @brief 初始化库，加载默认字体配置 / Initialize library with default font config
 */
void gt_latex_init(void);

/**
 * @brief 获取公式渲染所需的宽高（不执行渲染）/ Get bounding box without rendering
 * @param formula LaTeX 公式字符串 / LaTeX formula string
 * @param w       输出宽度 / Output width
 * @param h       输出高度 / Output height
 * @return        GT_LATEX_OK 成功，负数失败 / GT_LATEX_OK on success
 */
int gt_latex_get_size(const char *formula, uint16_t *w, uint16_t *h);

/**
 * @brief 一体化接口：解析 + 布局 + 渲染 / All-in-one: parse + layout + render
 * @param formula LaTeX 公式字符串 / LaTeX formula string
 * @param canvas  画布描述 / Canvas descriptor
 * @return        GT_LATEX_OK 成功，负数失败 / GT_LATEX_OK on success
 */
int gt_latex_draw(const char *formula, const gt_latex_canvas_t *canvas);

/**
 * @brief 分步接口：仅解析，生成 AST / Step API: parse only, produce AST
 * @param formula LaTeX 公式字符串 / LaTeX formula string
 * @param parser  输出解析器上下文（含 AST）/ Output parser context (with AST)
 * @param lexer   输出词法分析器上下文 / Output lexer context
 * @return        GT_LATEX_OK 成功，负数失败 / GT_LATEX_OK on success
 * @note          使用完毕后需调用 gt_latex_parse_free() 释放资源
 *                Call gt_latex_parse_free() to release resources when done
 */
int gt_latex_parse(const char *formula,
                   gt_latex_parser_t *parser,
                   gt_latex_lexer_t  *lexer);

/**
 * @brief 释放 gt_latex_parse() 分配的资源 / Free resources from gt_latex_parse()
 * @param parser 解析器上下文 / Parser context
 * @param lexer  词法分析器上下文 / Lexer context
 */
void gt_latex_parse_free(gt_latex_parser_t *parser, gt_latex_lexer_t *lexer);

/**
 * @brief 分步接口：对已解析的 AST 执行布局 / Step API: layout from existing AST
 * @param parser 已完成解析的上下文 / Completed parser context
 * @param layout 输出布局结果 / Output layout result
 * @return       GT_LATEX_OK 成功，负数失败 / GT_LATEX_OK on success
 * @note         使用完毕后需调用 gt_latex_layout_deinit() 释放资源
 */
int gt_latex_layout(const gt_latex_parser_t *parser,
                    gt_latex_layout_t       *layout);

/**
 * @brief 分步接口：对已完成布局执行渲染 / Step API: render from existing layout
 * @param layout 已完成布局的结果 / Completed layout result
 * @param canvas 画布描述 / Canvas descriptor
 * @return       GT_LATEX_OK 成功，负数失败 / GT_LATEX_OK on success
 */
int gt_latex_render(const gt_latex_layout_t *layout,
                    const gt_latex_canvas_t *canvas);

#endif /* GT_LATEX_ENABLE */

#endif /* _GT_LATEX_H_ */
