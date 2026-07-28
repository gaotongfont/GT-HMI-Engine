/**
 * @file gt_md.h
 * @brief GT-MD 顶层统一接口 / Top-level unified interface for GT-MD
 *        Markdown + LaTeX 混合渲染库，面向嵌入式系统
 *        Markdown + LaTeX mixed rendering library for embedded systems
 *
 * 典型用法 / Typical usage:
 * @code
 *   // 注册字体回调（复用 gt_latex 的注册方式）
 *   gt_latex_font_register(...);
 *
 *   // 构造画布
 *   gt_latex_canvas_t canvas = {
 *       .x = 0, .y = 0,
 *       .draw_glyph = my_draw_glyph,
 *       .draw_hline = my_draw_hline,
 *       .draw_line  = my_draw_line,
 *       .draw_arc   = my_draw_arc,
 *       .user_data  = NULL,
 *       .font_flags = 0,
 *   };
 *
 *   // 获取渲染尺寸
 *   uint16_t w, h;
 *   gt_md_get_size(text, len, &w, &h, max_width);
 *
 *   // 渲染
 *   gt_md_draw(text, len, &canvas, max_width);
 * @endcode
 */

#ifndef _GT_MD_H_
    #define _GT_MD_H_

    #include "gt_md_config.h"

    #if GT_MD_ENABLE

    #include <stdint.h>
    #include "gt_md_render.h"   /* 包含 canvas / render 相关类型 */
    #include "gt_md_layout.h"   /* 包含 spacing 配置 */

    /* ----------------------------------------------------------------
     * 全局间距配置（运行时可覆盖）
     * Global spacing config (can be overridden at runtime)
     * ---------------------------------------------------------------- */

    /**
     * @brief 设置全局间距配置 / Set global spacing config
     * @param spacing 间距配置指针 / Spacing config pointer
     *                NULL 时恢复默认值 / NULL restores defaults
     */
    void gt_md_set_spacing(const gt_md_spacing_t *spacing);

    /**
     * @brief 获取当前全局间距配置 / Get current global spacing config
     * @param spacing 输出间距配置 / Output spacing config
     */
    void gt_md_get_spacing(gt_md_spacing_t *spacing);

    /* ----------------------------------------------------------------
     * 全局 LaTeX 模式配置（运行时可覆盖，除非被编译期固定）
     * Global LaTeX mode config (runtime override unless compile-time fixed)
     * ---------------------------------------------------------------- */

    /**
     * @brief 设置 GT-MD 内部 LaTeX 渲染模式 / Set internal LaTeX render mode for GT-MD
     * @param mode 目标模式 / Target mode
     *
     * @note 若 GT_MD_CFG_LATEX_MODE_FIXED == 1，则该接口不改变实际模式
     */
    void gt_md_set_latex_mode(gt_md_latex_mode_t mode);

    /**
     * @brief 获取当前 GT-MD 内部 LaTeX 渲染模式 / Get current internal LaTeX render mode
     * @return 当前模式 / Current mode
     */
    gt_md_latex_mode_t gt_md_get_latex_mode(void);

    /* ----------------------------------------------------------------
     * 一体化接口（推荐）/ All-in-one API (recommended)
     * ---------------------------------------------------------------- */

    /**
     * @brief 计算 Markdown 文本渲染尺寸（不绘制）
     *        Compute render size of Markdown text (no drawing)
     *
     * @param text      Markdown 文本指针 / Markdown text pointer
     * @param text_len  文本字节长度 / Text byte length
     * @param w         输出宽度（像素）/ Output width in pixels
     * @param h         输出高度（像素）/ Output height in pixels
     * @param max_width 最大宽度约束（0 = 不限）/ Max width (0 = unlimited)
     * @return          0 成功，负数失败（见错误码）/ 0 on success
     */
    int gt_md_get_size(const char *text, uint16_t text_len,
                       uint16_t *w, uint16_t *h,
                       uint16_t max_width);

    /**
     * @brief 渲染 Markdown 文本到画布
     *        Render Markdown text to canvas
     *
     * @param text      Markdown 文本指针 / Markdown text pointer
     * @param text_len  文本字节长度 / Text byte length
     * @param canvas    目标画布 / Target canvas
     * @param max_width 最大宽度约束（0 = 不限）/ Max width (0 = unlimited)
     * @return          0 成功，负数失败 / 0 on success
     */
    int gt_md_draw(const char *text, uint16_t text_len,
                   const gt_latex_canvas_t *canvas,
                   uint16_t max_width);

    /* ----------------------------------------------------------------
     * 流式接口（低内存，逐块处理）
     * Streaming API (low-memory, block-by-block processing)
     * ---------------------------------------------------------------- */

    /**
     * @brief 流式获取渲染尺寸 / Streaming get size
     *        逐块处理，峰值内存仅为单块复杂度
     *        Block-by-block; peak memory is single-block complexity
     */
    int gt_md_get_size_stream(const char *text, uint16_t text_len,
                              uint16_t *w, uint16_t *h,
                              uint16_t max_width);

    /**
     * @brief 流式渲染 / Streaming draw
     *        逐块处理，峰值内存仅为单块复杂度
     *        Block-by-block; peak memory is single-block complexity
     */
    int gt_md_draw_stream(const char *text, uint16_t text_len,
                          const gt_latex_canvas_t *canvas,
                          uint16_t max_width);

    /* ----------------------------------------------------------------
     * 分步接口（需缓存布局时使用）
     * Step-by-step API (when layout caching is needed)
     * ---------------------------------------------------------------- */

    /**
     * @brief 解析 Markdown 文本为 AST
     *        Parse Markdown text into AST
     *
     * @param text      输入文本 / Input text
     * @param text_len  字节长度 / Byte length
     * @param root      输出 AST 根节点（需调用 gt_md_node_free 释放）
     *                  Output AST root (must be freed with gt_md_node_free)
     * @return          0 成功 / 0 on success
     */
    int gt_md_parse(const char *text, uint16_t text_len,
                    gt_md_node_container_t **root);

    /**
     * @brief 对 AST 执行布局计算
     *        Run layout computation on AST
     *
     * @param root      AST 根节点 / AST root
     * @param layout    布局上下文（输出）/ Layout context (output)
     * @param max_width 最大宽度约束 / Max width constraint
     * @return          0 成功 / 0 on success
     */
    int gt_md_layout(gt_md_node_container_t *root,
                     gt_md_layout_t *layout,
                     uint16_t max_width);

    /**
     * @brief 解析 + 布局（一步完成）
     *        Parse + layout in one step
     *
     * @param text      Markdown 文本指针 / Markdown text pointer
     * @param text_len  文本字节长度 / Text byte length
     * @param max_width 最大宽度约束 / Max width constraint
     * @param parser    输出 parser 上下文（需调用 gt_md_free_all 释放）
     *                  Output parser context (must be freed with gt_md_free_all)
     * @param root      输出 AST 根节点（需调用 gt_md_free_all 释放）
     *                  Output AST root (must be freed with gt_md_free_all)
     * @param layout    输出布局上下文（需调用 gt_md_free_all 释放）
     *                  Output layout context (must be freed with gt_md_free_all)
     * @return          0 成功，负数失败 / 0 on success, negative on error
     *
     * @note 使用示例 / Usage example:
     * @code
     *   gt_md_parser_t parser;
     *   gt_md_node_container_t *root;
     *   gt_md_layout_t layout;
     *
     *   // 一步完成解析和布局
     *   int ret = gt_md_parse_and_layout(text, len, max_width,
     *                                     &parser, &root, &layout);
     *   if (ret == GT_MD_OK) {
     *       // 可以多次使用 layout 进行渲染或其他操作
     *       gt_md_render(&layout, &canvas1);
     *       gt_md_render(&layout, &canvas2);
     *
     *       // 完成后统一释放资源
     *       gt_md_free_all(&parser, root, &layout);
     *   }
     * @endcode
     */
    int gt_md_parse_and_layout(const char *text, uint16_t text_len,
                               uint16_t max_width,
                               gt_md_parser_t *parser,
                               gt_md_node_container_t **root,
                               gt_md_layout_t *layout);

    /**
     * @brief 释放 parse_and_layout 分配的所有资源
     *        Free all resources allocated by parse_and_layout
     *
     * @param parser  parser 上下文指针 / Parser context pointer
     * @param root    AST 根节点 / AST root node
     * @param layout  布局上下文指针 / Layout context pointer
     *
     * @note 与 gt_md_parse_and_layout 配套使用
     *       Use with gt_md_parse_and_layout
     */
    void gt_md_free_all(gt_md_parser_t *parser,
                        gt_md_node_container_t *root,
                        gt_md_layout_t *layout);

    /**
     * @brief 将布局结果渲染到画布
     *        Render layout result to canvas
     *
     * @param layout    布局上下文（含 AST 根节点和尺寸信息）
     *                  Layout context (contains AST root and size info)
     * @param canvas    目标画布 / Target canvas
     * @return          0 成功 / 0 on success
     */
    int gt_md_render(const gt_md_layout_t *layout,
                     const gt_latex_canvas_t *canvas);

    /* ----------------------------------------------------------------
     * 逐块迭代器（极低内存模式，块级流式处理）
     * Block iterator (minimal memory mode, block-level streaming)
     * ---------------------------------------------------------------- */

    /**
     * @brief 逐块迭代器状态 / Block-level iterator state
     *        每次调用 next_size / next_draw 处理一个 Markdown 块
     *        Each next_size / next_draw call processes one Markdown block
     */
    typedef struct {
        const char    *text;        /* 原始文本指针 / Raw text pointer */
        uint16_t       text_len;    /* 总字节长度 / Total byte length */
        uint16_t       pos;         /* 当前扫描位置（字节偏移）/ Current scan position */
        uint16_t       max_width;   /* 最大宽度约束 / Max width constraint */
        int16_t        cur_y;       /* 已累积的 y 高度 / Accumulated y height */
    } gt_md_line_iter_t;

    /**
     * @brief 初始化逐块迭代器 / Initialize block iterator
     *
     * @param it        迭代器指针 / Iterator pointer
     * @param text      Markdown 文本 / Markdown text
     * @param text_len  字节长度 / Byte length
     * @param max_width 最大宽度约束 / Max width constraint
     */
    void gt_md_line_iter_init(gt_md_line_iter_t *it,
                              const char *text, uint16_t text_len,
                              uint16_t max_width);

    /**
     * @brief 获取下一块的渲染尺寸 / Get render size of next block
     *
     * @param it  迭代器 / Iterator
     * @param w   输出宽度 / Output width
     * @param h   输出高度 / Output height
     * @return    GT_MD_OK 成功，1 已结束，负数失败
     *            GT_MD_OK on success, 1 at end-of-input, negative on error
     */
    int gt_md_line_iter_next_size(gt_md_line_iter_t *it,
                                  uint16_t *w, uint16_t *h);

    /**
     * @brief 渲染下一块到画布 / Render next block to canvas
     *
     * @param it      迭代器 / Iterator
     * @param canvas  目标画布（y 偏移由迭代器自动累加）
     *                Target canvas (y offset accumulated by iterator)
     * @return        GT_MD_OK 成功，1 已结束，负数失败
     *                GT_MD_OK on success, 1 at end-of-input, negative on error
     */
    int gt_md_line_iter_next_draw(gt_md_line_iter_t *it,
                                  const gt_latex_canvas_t *canvas);

    /**
     * @brief 便利函数：完整文本渲染（等同 gt_md_draw）
     *        Convenience: render full text (same as gt_md_draw)
     */
    int gt_md_draw_all(const char *text, uint16_t text_len,
                       const gt_latex_canvas_t *canvas,
                       uint16_t max_width);

    #endif /* GT_MD_ENABLE */

#endif /* _GT_MD_H_ */
