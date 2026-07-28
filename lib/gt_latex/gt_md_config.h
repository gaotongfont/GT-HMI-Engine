/**
 * @file gt_md_config.h
 * @brief GT-MD 编译期配置 / Compile-time configuration for Markdown renderer
 */

#ifndef _GT_MD_CONFIG_H_
    #define _GT_MD_CONFIG_H_

/* ----------------------------------------------------------------
 * 顶层总开关 / Top-level enable switch
 * 设为 0 可关闭整个 GT-MD 库 / Set to 0 to disable the whole library
 * ---------------------------------------------------------------- */
#ifndef GT_MD_ENABLE
    #define GT_MD_ENABLE    1
#endif

#if GT_MD_ENABLE

#include "gt_latex_config.h"

/* ----------------------------------------------------------------
 * 功能裁剪开关 / Feature enable switches
 * ---------------------------------------------------------------- */

#ifndef GT_MD_CFG_ENABLE_HEADING
    #define GT_MD_CFG_ENABLE_HEADING    1   /* 标题 H1-H6 / Headings H1-H6 */
#endif

#ifndef GT_MD_CFG_ENABLE_LIST
    #define GT_MD_CFG_ENABLE_LIST       1   /* 有序/无序列表 / Ordered and unordered lists */
#endif

#ifndef GT_MD_CFG_ENABLE_CODE_BLOCK
    #define GT_MD_CFG_ENABLE_CODE_BLOCK 1   /* 代码块 / Fenced code blocks */
#endif

#ifndef GT_MD_CFG_ENABLE_BLOCKQUOTE
    #define GT_MD_CFG_ENABLE_BLOCKQUOTE 1   /* 引用块 / Block quotes */
#endif

#ifndef GT_MD_CFG_ENABLE_HR
    #define GT_MD_CFG_ENABLE_HR         1   /* 分隔线 --- / Horizontal rules */
#endif

#ifndef GT_MD_CFG_ENABLE_BOLD
    #define GT_MD_CFG_ENABLE_BOLD       1   /* 粗体 **text** / Bold */
#endif

#ifndef GT_MD_CFG_ENABLE_ITALIC
    #define GT_MD_CFG_ENABLE_ITALIC     1   /* 斜体 *text* / Italic */
#endif

#ifndef GT_MD_CFG_ENABLE_CODE_SPAN
    #define GT_MD_CFG_ENABLE_CODE_SPAN  1   /* 行内代码 `code` / Inline code span */
#endif

#ifndef GT_MD_CFG_ENABLE_STRIKETHROUGH
    #define GT_MD_CFG_ENABLE_STRIKETHROUGH 1 /* 删除线 ~~text~~ / Strikethrough */
#endif

#ifndef GT_MD_CFG_ENABLE_LATEX
    #define GT_MD_CFG_ENABLE_LATEX      1   /* LaTeX 公式 $...$ $$...$$ / LaTeX formulas (requires gt_latex) */
#endif

/* ----------------------------------------------------------------
 * Markdown 内部 LaTeX 渲染模式 / Internal LaTeX render mode for Markdown
 * ---------------------------------------------------------------- */
typedef enum {
    GT_MD_LATEX_MODE_DIRECT   = 0, /* render 阶段直接调用 gt_latex_draw() */
    GT_MD_LATEX_MODE_PREPARED = 1  /* layout 阶段准备 cache，render 阶段只调用 gt_latex_render() */
} gt_md_latex_mode_t;

/* 是否固定 LaTeX mode（1=固定为编译期默认值，0=允许运行时切换）
 * Whether to fix LaTeX mode at compile time (1=fixed, 0=runtime switchable) */
#ifndef GT_MD_CFG_LATEX_MODE_FIXED
    #define GT_MD_CFG_LATEX_MODE_FIXED 0
#endif

/* 默认 LaTeX mode / Default LaTeX mode */
#ifndef GT_MD_CFG_LATEX_MODE_DEFAULT
    #define GT_MD_CFG_LATEX_MODE_DEFAULT GT_MD_LATEX_MODE_DIRECT
#endif

/* ----------------------------------------------------------------
 * 内存参数 / Memory parameters
 * ---------------------------------------------------------------- */

/* 子节点指针数组初始容量（倍增扩容）
 * Initial capacity of children pointer array (doubles on expansion) */
#ifndef GT_MD_CFG_CHILD_INIT_CAP
    #define GT_MD_CFG_CHILD_INIT_CAP   4
#endif

/* 最大嵌套深度（列表/引用块嵌套）
 * Maximum nesting depth for lists and blockquotes */
#ifndef GT_MD_CFG_MAX_NEST_DEPTH
    #define GT_MD_CFG_MAX_NEST_DEPTH   8
#endif

/* 行缓冲最大长度（栈分配，用于块级扫描）
 * Maximum line length for block scanner line buffer (stack allocated) */
#ifndef GT_MD_CFG_MAX_LINE_LEN
    #define GT_MD_CFG_MAX_LINE_LEN     256
#endif

/* ----------------------------------------------------------------
 * 间距配置（单位：像素）/ Spacing configuration (pixels)
 * 0 表示使用内部默认比例 / 0 = use internal default ratio
 * ---------------------------------------------------------------- */

/* 段落间额外间距（0 = 字号×0.5）
 * Extra spacing between paragraphs (0 = font_size × 0.5) */
#ifndef GT_MD_CFG_PARAGRAPH_SPACING
    #define GT_MD_CFG_PARAGRAPH_SPACING 2
#endif

/* 标题前后额外间距（0 = 字号×0.3）
 * Extra spacing before/after headings (0 = font_size × 0.3) */
#ifndef GT_MD_CFG_HEADING_SPACING
    #define GT_MD_CFG_HEADING_SPACING   2
#endif

/* 列表项间距（0 = 字号×0.2）
 * Spacing between list items (0 = font_size × 0.2) */
#ifndef GT_MD_CFG_LIST_ITEM_SPACING
    #define GT_MD_CFG_LIST_ITEM_SPACING 2
#endif

/* 引用块竖线线宽倍率（基于内部基准线宽）/ Blockquote border thickness scale */
#ifndef GT_MD_CFG_BLOCKQUOTE_LINE_THICKNESS_SCALE
    #define GT_MD_CFG_BLOCKQUOTE_LINE_THICKNESS_SCALE 2
#endif

/* 引用块竖线右移量：按“半个线宽”为单位 / Blockquote border right offset in half-line-width units */
#ifndef GT_MD_CFG_BLOCKQUOTE_LINE_OFFSET_HALF_WIDTHS
    #define GT_MD_CFG_BLOCKQUOTE_LINE_OFFSET_HALF_WIDTHS 2
#endif

/* 引用块竖线上下内缩像素 / Blockquote border vertical inset in pixels */
#ifndef GT_MD_CFG_BLOCKQUOTE_LINE_VERTICAL_INSET
    #define GT_MD_CFG_BLOCKQUOTE_LINE_VERTICAL_INSET 4
#endif

/* 列表缩进像素（每层）/ List indent per nesting level (pixels) */
#ifndef GT_MD_CFG_LIST_INDENT
    #define GT_MD_CFG_LIST_INDENT       8
#endif

/* 列表层级识别的空格数（Lexer 用于解析）/ Spaces per list level for parsing */
#ifndef GT_MD_CFG_LIST_LEVEL_SPACES
    #define GT_MD_CFG_LIST_LEVEL_SPACES  4  /* 4 空格 = 1 层 / 4 spaces = 1 level */
#endif

/* 列表渲染缩进的空格宽度数（Layout/Render 用于计算像素缩进）/ Space widths per level for rendering indent */
#ifndef GT_MD_CFG_LIST_RENDER_SPACES
    #define GT_MD_CFG_LIST_RENDER_SPACES  2  /* 每层缩进 = 2 个空格宽度 / Indent per level = 2 space widths */
#endif

/* 无序列表各层级的 bullet 字符 / Bullet characters for each unordered list level */
#ifndef GT_MD_CFG_LIST_BULLET_L0
    #define GT_MD_CFG_LIST_BULLET_L0  0x2022  /* • BULLET (实心圆点) */
#endif
#ifndef GT_MD_CFG_LIST_BULLET_L1
    #define GT_MD_CFG_LIST_BULLET_L1  0x25E6  /* ◦ WHITE BULLET (空心小圆点) */
#endif
#ifndef GT_MD_CFG_LIST_BULLET_L2
    #define GT_MD_CFG_LIST_BULLET_L2  0x25AA  /* ▪ BLACK SMALL SQUARE (小方块) */
#endif
#ifndef GT_MD_CFG_LIST_BULLET_L3
    #define GT_MD_CFG_LIST_BULLET_L3  0x2023  /* ‣ TRIANGULAR BULLET (三角) */
#endif

/* 行间距（文本换行时的额外间距）/ Line spacing (extra spacing between wrapped lines) */
#ifndef GT_MD_CFG_LINE_SPACING
    #define GT_MD_CFG_LINE_SPACING      2
#endif

/* ----------------------------------------------------------------
 * 行内 LaTeX 基线对齐模式 / Inline LaTeX axis alignment mode
 * ---------------------------------------------------------------- */
typedef enum {
    GT_MD_AXIS_MODE_AUTO   = 0,  /* 自动：axis 偏离 h/2 太远时使用 h/2，否则用 axis / Auto: use h/2 if axis deviates too far, else axis */
    GT_MD_AXIS_MODE_AXIS   = 1,  /* 使用 metrics.axis / Use metrics.axis directly */
    GT_MD_AXIS_MODE_CENTER = 2   /* 居中：使用 h/2 / Center: always use h/2 */
} gt_md_axis_mode_t;

/* 默认 axis 模式 / Default axis alignment mode */
#ifndef GT_MD_CFG_DEFAULT_AXIS_MODE
    #define GT_MD_CFG_DEFAULT_AXIS_MODE  GT_MD_AXIS_MODE_AUTO
#endif

/* AUTO 模式下的偏差阈值（axis 与 h/2 的最大差值比例）
 * Deviation threshold in AUTO mode (max ratio of |axis - h/2| to h) */
#ifndef GT_MD_CFG_AXIS_AUTO_THRESHOLD
    #define GT_MD_CFG_AXIS_AUTO_THRESHOLD  0.15f  /* 15% */
#endif

/* ----------------------------------------------------------------
 * 错误码 / Error codes
 * 与 gt_latex 错误码数值区间错开 / Non-overlapping with gt_latex error codes
 * ---------------------------------------------------------------- */
#define GT_MD_OK              0     /* 成功 / Success */
#define GT_MD_ERR_PARAM      -1     /* 参数错误 / Invalid parameter (shared with gt_latex) */
#define GT_MD_ERR_NOMEM      -2     /* 内存不足 / Out of memory (shared with gt_latex) */
#define GT_MD_ERR_SYNTAX     -3     /* MD 语法错误（通常降级处理）/ MD syntax error (usually degraded) */
#define GT_MD_ERR_NO_CB      -6     /* 未注册必要回调 / Required callback not registered (shared with gt_latex) */
#define GT_MD_ERR_NEST_DEEP  -10    /* 嵌套深度超过 GT_MD_CFG_MAX_NEST_DEPTH / Nesting too deep */
#define GT_MD_ERR_LINE_LEN   -11    /* 单行超过 GT_MD_CFG_MAX_LINE_LEN / Line too long */
#define GT_MD_ERR_EOF        1      /* 输入结束（非错误，用于迭代器）/ End of input (not an error, for iterators) */

/* ----------------------------------------------------------------
 * 布局盒子（前置声明，避免循环依赖）
 * Layout box (forward-declared here to break circular dependency)
 * gt_md_layout.h 直接使用本定义，无需再 typedef
 * ---------------------------------------------------------------- */
#include <stdint.h>
typedef struct {
    int16_t  x;     /* 相对父节点 x 偏移 / x offset from parent */
    int16_t  y;     /* 相对父节点 y 偏移 / y offset from parent */
    uint16_t w;     /* 宽度 / Width */
    uint16_t h;     /* 高度 / Height */
    uint16_t axis;  /* 数学轴（顶部到基线距离）/ Math axis (top-to-baseline) */
    uint16_t pad;   /* 对齐填充 / Alignment padding */
} gt_md_lbox_t;

/* ----------------------------------------------------------------
 * 调试与断言 / Debug and assert
 * ---------------------------------------------------------------- */
#ifndef GT_MD_CFG_ASSERT_ENABLE
    #define GT_MD_CFG_ASSERT_ENABLE 0   /* 断言开关，裸机建议关闭 / Assert, disable on bare-metal */
#endif

#ifndef GT_MD_CFG_DEBUG_ENABLE
    #define GT_MD_CFG_DEBUG_ENABLE  0   /* 调试输出开关 / Debug output */
#endif

#endif /* GT_MD_ENABLE */

#endif /* _GT_MD_CONFIG_H_ */
