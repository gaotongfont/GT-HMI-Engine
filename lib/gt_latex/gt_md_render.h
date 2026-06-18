/**
 * @file gt_md_render.h
 * @brief GT-MD 渲染器 / Markdown renderer
 *        遍历 AST + 布局结果，通过回调绘制像素
 *        Traverses AST + layout result and draws pixels via callbacks
 */

#ifndef _GT_MD_RENDER_H_
    #define _GT_MD_RENDER_H_

    #include "gt_md_config.h"

    #if GT_MD_ENABLE

    #include <stdint.h>
    #include "gt_md_layout.h"
    #include "gt_latex_render.h"  /* 使用 gt_latex_canvas_t / Use gt_latex_canvas_t */

    /* ----------------------------------------------------------------
     * 接口声明 / API declarations
     * ---------------------------------------------------------------- */

    /**
     * @brief 渲染整个 MD 布局结果到画布
     *        Render the complete MD layout result to the canvas
     *
     * @param layout    布局上下文（含 AST 根节点和尺寸）/ Layout context
     * @param canvas    目标画布（含偏移和回调）/ Target canvas
     * @return          0 成功，负数失败 / 0 on success, negative on failure
     */
    int gt_md_render(const gt_md_layout_t *layout,
                     const gt_latex_canvas_t *canvas);

    /**
     * @brief 从布局结果读取总尺寸
     *        Read total rendered size from layout
     *
     * @param layout    布局上下文 / Layout context
     * @param w         输出宽度 / Output width
     * @param h         输出高度 / Output height
     * @return          0 成功 / 0 on success
     */
    int gt_md_render_size(const gt_md_layout_t *layout,
                          uint16_t *w, uint16_t *h);

    #endif /* GT_MD_ENABLE */

#endif /* _GT_MD_RENDER_H_ */
