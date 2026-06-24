/**
 * @file gt_md_layout.h
 * @brief GT-MD 布局引擎 / Markdown layout engine
 *        计算每个 MD 节点的几何位置和尺寸
 *        Computes geometry (position and size) for each MD AST node
 */

#ifndef _GT_MD_LAYOUT_H_
    #define _GT_MD_LAYOUT_H_

    #include "gt_md_config.h"

    #if GT_MD_ENABLE

    #include <stdint.h>
    #include "gt_md_parser.h"
    #include "gt_latex_layout.h"
    #include "gt_latex_font.h"

    /* ----------------------------------------------------------------
     * 间距配置（可运行时覆盖）/ Spacing config (runtime override supported)
     * ---------------------------------------------------------------- */
    typedef struct {
        uint16_t paragraph_sp;      /* 段落间额外间距 / Paragraph spacing */
        uint16_t heading_sp;        /* 标题前后额外间距 / Heading spacing */
        uint16_t list_item_sp;      /* 列表项间距 / List item spacing */
        uint16_t list_indent;       /* 列表缩进每层（像素）/ List indent per level */
        uint16_t line_spacing;      /* 行间距（文本换行时）/ Line spacing (for wrapped lines) */
        gt_md_axis_mode_t axis_mode; /* 行内 LaTeX 基线对齐模式 / Inline LaTeX axis alignment mode */
    } gt_md_spacing_t;

    /* ----------------------------------------------------------------
     * 布局上下文 / Layout context
     * ---------------------------------------------------------------- */
    typedef struct {
        gt_md_node_container_t *root;       /* AST 根节点 / AST root */
        uint16_t                total_w;    /* 渲染总宽度 / Total render width */
        uint16_t                total_h;    /* 渲染总高度 / Total render height */
        uint16_t                max_width;  /* 最大宽度约束 / Max width constraint */
        gt_md_spacing_t         spacing;    /* 间距配置 / Spacing config */
    } gt_md_layout_t;

    /* ----------------------------------------------------------------
     * 接口声明 / API declarations
     * ---------------------------------------------------------------- */

    /**
     * @brief 初始化布局上下文 / Initialize layout context
     * @param layout    布局上下文 / Layout context
     * @param root      AST 根节点 / AST root node
     * @param max_width 最大宽度（像素）/ Max width in pixels
     * @param spacing   间距配置（NULL 使用默认值）/ Spacing config (NULL = defaults)
     * @return          0 成功，负数失败 / 0 on success, negative on failure
     */
    int gt_md_layout_init(gt_md_layout_t *layout,
                          gt_md_node_container_t *root,
                          uint16_t max_width,
                          const gt_md_spacing_t *spacing);

    /**
     * @brief 对整棵 AST 执行布局计算 / Run layout on full AST tree
     * @param layout 布局上下文（需先调用 gt_md_layout_init）/ Layout context
     * @return       0 成功，负数失败 / 0 on success, negative on failure
     */
    int gt_md_layout_run(gt_md_layout_t *layout);

    /**
     * @brief 释放布局中分配的资源（LaTeX 子树等）
     *        Free resources allocated during layout (LaTeX sub-trees etc.)
     * @param layout 布局上下文 / Layout context
     */
    void gt_md_layout_free(gt_md_layout_t *layout);

    #endif /* GT_MD_ENABLE */

#endif /* _GT_MD_LAYOUT_H_ */
