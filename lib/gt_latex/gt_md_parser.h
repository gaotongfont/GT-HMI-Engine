/**
 * @file gt_md_parser.h
 * @brief GT-MD 语法解析器 / Markdown AST builder
 *        两阶段解析：块级 AST 构建 + 行内元素解析
 *        Two-phase parser: block-level AST + inline element parsing
 */

#ifndef _GT_MD_PARSER_H_
    #define _GT_MD_PARSER_H_

    #include "gt_md_config.h"

    #if GT_MD_ENABLE

    #include <stdint.h>
    #include "gt_md_lexer.h"

    /* ----------------------------------------------------------------
     * AST 节点类型枚举 / AST node type enum
     * ---------------------------------------------------------------- */
    typedef enum {
        /* 块级节点 / Block nodes */
        GT_MD_NODE_DOCUMENT = 0,    /* 根节点 / Root document node */
        GT_MD_NODE_HEADING,         /* 标题 H1-H6 / Heading */
        GT_MD_NODE_PARAGRAPH,       /* 段落 / Paragraph */
        GT_MD_NODE_LIST,            /* 列表容器 / List container */
        GT_MD_NODE_LIST_ITEM,       /* 列表项 / List item */
        GT_MD_NODE_CODE_BLOCK,      /* 代码块 / Fenced code block */
        GT_MD_NODE_BLOCKQUOTE,      /* 引用块 / Block quote */
        GT_MD_NODE_HR,              /* 分隔线 / Horizontal rule */
        GT_MD_NODE_LATEX_BLOCK,     /* 块级公式 $$...$$ / Block-level LaTeX */

        /* 行内节点 / Inline nodes */
        GT_MD_NODE_TEXT,            /* 普通文字 / Plain text */
        GT_MD_NODE_BOLD,            /* 粗体容器 **...** / Bold container */
        GT_MD_NODE_ITALIC,          /* 斜体容器 *...* / Italic container */
        GT_MD_NODE_CODE_SPAN,       /* 行内代码 `...` / Inline code span */
        GT_MD_NODE_LATEX_INLINE,    /* 行内公式 $...$ / Inline LaTeX formula */
        GT_MD_NODE_SOFTBREAK,       /* 软换行 / Soft line break */
        GT_MD_NODE_HARDBREAK,       /* 硬换行（两空格+换行）/ Hard line break */
    } gt_md_node_type_t;

    /* ----------------------------------------------------------------
     * 节点基类（所有节点首字段）/ Node base (first field of all nodes)
     * ---------------------------------------------------------------- */
    typedef struct {
        gt_md_node_type_t  type;    /* 节点类型 / Node type          4B */
        uint8_t            flags;   /* 粗体/斜体等标志位 / Style flags 1B */
        uint8_t            level;   /* 标题级别 1-6 / 列表层级 / Heading level or list depth 1B */
        uint8_t            ordered; /* 1=有序列表 / 1=ordered list    1B */
        uint8_t            pad;     /* 对齐填充 / Alignment padding   1B */
        /* 布局阶段填充 / Filled during layout phase */
        gt_md_lbox_t       lbox;   /* 几何位置与尺寸 / Geometry (position & size) */
    } gt_md_node_base_t;

    /* 标志位 / Flags */
    #define GT_MD_FLAG_BOLD    0x01 /* 粗体 / Bold */
    #define GT_MD_FLAG_ITALIC  0x02 /* 斜体 / Italic */

    /* ----------------------------------------------------------------
     * 文本行信息（多行文本布局）/ Text line info (multi-line text layout)
     * ---------------------------------------------------------------- */
    typedef struct {
        uint16_t start_offset;  /* 在原始文本中的起始偏移（字节）/ Start offset in source text (bytes) */
        uint16_t len;           /* 该行的字节长度 / Byte length of this line */
        int16_t  x;             /* 该行的 x 坐标（相对父容器）/ X coordinate relative to parent */
        int16_t  y;             /* 该行的 y 坐标（相对父容器）/ Y coordinate relative to parent */
        uint16_t w;             /* 该行的宽度 / Width of this line */
        uint16_t h;             /* 该行的高度 / Height of this line */
    } gt_md_text_line_t;        /* 12B total */

    /* ----------------------------------------------------------------
     * 文字节点（叶节点）/ Text node (leaf)
     * ---------------------------------------------------------------- */
    typedef struct {
        gt_md_node_base_t  base;       /* 8B */
        const char        *start;      /* 指向原始文本（零拷贝）/ Pointer to source text (zero-copy) 8B */
        uint16_t           len;        /* 字节长度 / Byte length  2B */
        uint16_t           line_count; /* 多行布局时的行数（0表示未布局/单行）/ Line count for multi-line (0=not laid out/single line) 2B */
        gt_md_text_line_t *lines;      /* 多行信息数组（layout阶段分配）/ Line info array (allocated in layout) 8B */
    } gt_md_node_text_t;               /* 28B total */

    /* ----------------------------------------------------------------
     * 容器节点（含子节点数组）/ Container node (with children array)
     * ---------------------------------------------------------------- */
    typedef struct {
        gt_md_node_base_t   base;       /* 8B */
        gt_md_node_base_t **children;   /* 子节点指针数组（动态分配）/ Child pointer array 8B */
        uint16_t            count;      /* 子节点数量 / Child count     2B */
        uint16_t            cap;        /* 数组容量 / Array capacity    2B */
        uint16_t            list_num;   /* 有序列表起始编号 / Ordered list start number 2B */
        uint16_t            pad;        /* 对齐填充 / Padding           2B */
        char               *owned_src;  /* 自持文本缓冲，仅部分容器使用 / Owned source buffer for rebuilt sub-docs */
        uint16_t            owned_len;  /* 自持文本长度 / Owned source length */
        uint16_t            owned_pad;  /* 对齐填充 / Padding */
    } gt_md_node_container_t;           /* 24B total */

    /* ----------------------------------------------------------------
     * LaTeX 节点（行内/块级公式）/ LaTeX node (inline or block formula)
     * ---------------------------------------------------------------- */
    typedef struct {
        gt_md_node_base_t  base;        /* 8B */
        const char        *formula;    /* LaTeX 字符串指针（零拷贝）/ Formula pointer (zero-copy) 8B */
        uint16_t           formula_len;/* 公式字节长度 / Formula length  2B */
        uint16_t           pad;        /* 对齐填充 / Padding             2B */
    } gt_md_node_latex_t;               /* 20B total */

    /* ----------------------------------------------------------------
     * 代码块/代码段节点 / Code block or code span node
     * ---------------------------------------------------------------- */
    typedef struct {
        gt_md_node_base_t  base;    /* 8B */
        const char        *code;   /* 代码内容指针（零拷贝）/ Code pointer (zero-copy) 8B */
        uint16_t           len;    /* 代码字节长度 / Code length  2B */
        uint16_t           pad;    /* 对齐填充 / Padding          2B */
    } gt_md_node_code_t;            /* 20B total */

    /* ----------------------------------------------------------------
     * 解析器上下文 / Parser context
     * ---------------------------------------------------------------- */
    typedef struct {
        gt_md_lexer_t              lexer_buf; /* 内嵌词法器（init 时初始化）/ Embedded lexer */
        gt_md_lexer_t             *lexer;     /* 指向 lexer_buf / Points to lexer_buf */
        gt_md_node_container_t    *root;      /* 根节点（document）/ Root node */
        /* 行内解析栈（固定大小，栈上分配）/ Inline parse stack (fixed, on stack) */
        uint8_t                    depth;     /* 当前嵌套深度 / Current nesting depth */
    } gt_md_parser_t;

    /* ----------------------------------------------------------------
     * 接口声明 / API declarations
     * ---------------------------------------------------------------- */

    /**
     * @brief 初始化解析器 / Initialize parser
     * @param parser    解析器上下文 / Parser context
     * @param text      原始 Markdown 文本 / Raw Markdown text
     * @param text_len  文本字节长度 / Text byte length
     * @return          0 成功，负数失败 / 0 on success, negative on failure
     */
    int gt_md_parser_init(gt_md_parser_t *parser,
                          const char *text, uint16_t text_len);

    /**
     * @brief 执行完整解析，构建 AST 树 / Run full parse, build AST
     * @param parser 解析器上下文 / Parser context
     * @return       根节点指针，NULL 表示失败 / Root node pointer, NULL on failure
     */
    gt_md_node_container_t *gt_md_parser_parse(gt_md_parser_t *parser);

    /**
     * @brief 释放 AST 树所有节点内存 / Free all AST node memory
     * @param node 根节点或任意子树根 / Root or subtree root
     */
    void gt_md_node_free(gt_md_node_base_t *node);

    /**
     * @brief 释放解析器上下文（不释放 AST，需先调用 gt_md_node_free）
     *        Free parser context (AST not freed; call gt_md_node_free first)
     * @param parser 解析器上下文 / Parser context
     */
    void gt_md_parser_deinit(gt_md_parser_t *parser);

    /**
     * @brief 行内解析（内部使用，供 parser.c 调用）
     *        Parse inline elements in text span (internal, called by parser.c)
     * @param parser  解析器上下文 / Parser context
     * @param parent  目标容器节点 / Target container node
     * @param text    文本指针 / Text pointer
     * @param len     文本长度 / Text length
     * @return        0 成功，负数失败 / 0 on success, negative on failure
     */
    int gt_md_parse_inline(gt_md_parser_t *parser,
                           gt_md_node_container_t *parent,
                           const char *text, uint16_t len,
                           uint8_t parent_flags);

    #endif /* GT_MD_ENABLE */

#endif /* _GT_MD_PARSER_H_ */
