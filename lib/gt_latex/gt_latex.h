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
#include "gt_latex_mem.h"

#include "gt_md.h"
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
 * 度量信息 / Metrics
 * ---------------------------------------------------------------- */

/**
 * @brief LaTeX 公式度量信息 / LaTeX formula metrics
 */
typedef struct {
    uint16_t w;     /* 宽度（像素）/ Width in pixels */
    uint16_t h;     /* 高度（像素）/ Height in pixels */
    uint16_t axis;  /* 数学轴距顶部距离（像素）/ Math axis offset from top */
} gt_latex_metrics_t;

/* ----------------------------------------------------------------
 * 接口声明 / API declarations
 * ---------------------------------------------------------------- */

/**
 * @brief 初始化库，加载默认字体配置 / Initialize library with default font config
 */
void gt_latex_init(void);

/**
 * @brief 获取公式渲染所需的度量信息（不执行渲染）/ Get metrics without rendering
 * @param formula    LaTeX 公式字符串 / LaTeX formula string
 * @param formula_len 公式长度 / Formula length
 * @param metrics    输出度量信息（宽度、高度、axis）/ Output metrics (w, h, axis)
 * @param max_width  最大宽度限制（像素），0 表示不限制 / Max width in pixels, 0 = unlimited
 * @param line_off   首行偏移 / Line offset
 * @return           GT_LATEX_OK 成功，负数失败 / GT_LATEX_OK on success
 */
int gt_latex_get_size(const char *formula, uint16_t formula_len,
                      gt_latex_metrics_t *metrics, uint16_t max_width,
                      gt_latex_off_t line_off);

/**
 * @brief 一体化接口：解析 + 布局 + 渲染 / All-in-one: parse + layout + render
 * @param formula   LaTeX 公式字符串 / LaTeX formula string
 * @param canvas    画布描述 / Canvas descriptor
 * @param max_width 最大宽度限制（像素），0 表示不限制 / Max width in pixels, 0 = unlimited
 * @return          GT_LATEX_OK 成功，负数失败 / GT_LATEX_OK on success
 */
int gt_latex_draw(const char *formula, uint16_t formula_len,
                  const gt_latex_canvas_t *canvas, uint16_t max_width,
                  gt_latex_off_t line_off);

/**
 * @brief 分步接口：仅解析，生成 AST / Step API: parse only, produce AST
 * @param formula LaTeX 公式字符串 / LaTeX formula string
 * @param parser  输出解析器上下文（含 AST）/ Output parser context (with AST)
 * @param lexer   输出词法分析器上下文 / Output lexer context
 * @return        GT_LATEX_OK 成功，负数失败 / GT_LATEX_OK on success
 * @note          使用完毕后需调用 gt_latex_parse_free() 释放资源
 *                Call gt_latex_parse_free() to release resources when done
 */
int gt_latex_parse(const char *formula, uint16_t formula_len,
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
 * @param parser    已完成解析的上下文 / Completed parser context
 * @param layout    输出布局结果 / Output layout result
 * @param max_width 最大宽度限制（像素），0 表示不限制 / Max width in pixels, 0 = unlimited
 * @return          GT_LATEX_OK 成功，负数失败 / GT_LATEX_OK on success
 * @note            使用完毕后需调用 gt_latex_layout_deinit() 释放资源
 */
int gt_latex_layout(const gt_latex_parser_t *parser,
                    gt_latex_layout_t       *layout,
                    uint16_t                 max_width,
                    gt_latex_off_t           line_off);

/**
 * @brief 分步接口：对已完成布局执行渲染 / Step API: render from existing layout
 * @param layout 已完成布局的结果 / Completed layout result
 * @param canvas 画布描述 / Canvas descriptor
 * @return       GT_LATEX_OK 成功，负数失败 / GT_LATEX_OK on success
 */
int gt_latex_render(const gt_latex_layout_t *layout,
                    const gt_latex_canvas_t *canvas);

/**
 * @brief 流式模式获取尺寸（token 内存固定 40B，适合大文本）
 *        Get size in streaming mode (fixed 40B token memory, for large text)
 * @param formula    LaTeX 公式字符串 / LaTeX formula string
 * @param formula_len 公式长度 / Formula length
 * @param metrics    输出度量信息（宽度、高度、axis）/ Output metrics (w, h, axis)
 * @param max_width  最大宽度限制（像素），0 表示不限制 / Max width in pixels, 0 = unlimited
 * @param line_off   首行偏移 / Line offset
 * @return           GT_LATEX_OK 成功，负数失败 / GT_LATEX_OK on success
 */
int gt_latex_get_size_stream(const char *formula, uint16_t formula_len,
                              gt_latex_metrics_t *metrics, uint16_t max_width,
                              gt_latex_off_t line_off);

/**
 * @brief 流式模式渲染（token 内存固定 40B，适合大文本）
 *        Draw in streaming mode (fixed 40B token memory, for large text)
 */
int gt_latex_draw_stream(const char *formula, uint16_t formula_len,
                          const gt_latex_canvas_t *canvas, uint16_t max_width,
                          gt_latex_off_t line_off);

/* ----------------------------------------------------------------
 * 逐行迭代接口 / Line-by-line iterator API
 * 将多行文本（\n 分隔）逐行解析+渲染，峰值内存仅为单行复杂度
 * ---------------------------------------------------------------- */

/**
 * @brief 逐行迭代器上下文 / Line iterator context
 */
typedef struct {
    const char     *text;        /* 原始文本指针 / Raw text pointer */
    uint16_t        text_len;    /* 总长度 / Total length */
    uint16_t        pos;         /* 当前扫描位置 / Current scan position */
    uint16_t        max_width;   /* 最大宽度限制 / Max width */
    gt_latex_off_t  line_off;    /* 行间偏移 / Line offset */
    int             use_stream;  /* 0=批量parse 1=流式parse / 0=batch 1=stream */
    int16_t         cur_y;       /* 已累积的 y 高度（渲染时使用）/ Accumulated y */
} gt_latex_line_iter_t;

/**
 * @brief 初始化逐行迭代器 / Initialize line iterator
 * @param use_stream 0=批量parse模式（兼容性好），1=流式parse模式（内存更低）
 */
void gt_latex_line_iter_init(gt_latex_line_iter_t *it,
                             const char *text, uint16_t text_len,
                             uint16_t max_width, gt_latex_off_t line_off,
                             int use_stream);

/**
 * @brief 获取下一行的渲染尺寸 / Get size of next line
 * @param it      迭代器 / Iterator
 * @param metrics 输出度量信息（宽度、高度、axis）/ Output metrics (w, h, axis)
 * @return GT_LATEX_OK 有数据，GT_LATEX_ERR_PARAM 已到末尾，其他负数为错误
 */
int gt_latex_line_iter_next_size(gt_latex_line_iter_t *it,
                                 gt_latex_metrics_t *metrics);

/**
 * @brief 渲染下一行到画布 / Render next line to canvas
 *        canvas->y 为起始基准，迭代器内部自动累加行高
 * @return GT_LATEX_OK 有数据并渲染成功，GT_LATEX_ERR_PARAM 已到末尾
 */
int gt_latex_line_iter_next_draw(gt_latex_line_iter_t *it,
                                 const gt_latex_canvas_t *canvas);

/**
 * @brief 多行文本逐行渲染便利函数（批量parse模式）
 *        Line-by-line render helper (batch parse mode)
 */
int gt_latex_draw_lines(const char *text, uint16_t text_len,
                        const gt_latex_canvas_t *canvas,
                        uint16_t max_width, gt_latex_off_t line_off);

/**
 * @brief 多行文本逐行渲染便利函数（流式parse模式，内存更低）
 *        Line-by-line render helper (stream parse mode, lower memory)
 */
int gt_latex_draw_lines_stream(const char *text, uint16_t text_len,
                               const gt_latex_canvas_t *canvas,
                               uint16_t max_width, gt_latex_off_t line_off);

#endif /* GT_LATEX_ENABLE */

#endif /* _GT_LATEX_H_ */
