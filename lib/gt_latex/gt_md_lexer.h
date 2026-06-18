/**
 * @file gt_md_lexer.h
 * @brief GT-MD 块级扫描器 / Block-level lexer
 *        将 Markdown 文本扫描为块级 Token 流（不处理行内元素）
 *        Scans Markdown text into block-level token stream (inline elements not processed here)
 */

#ifndef _GT_MD_LEXER_H_
    #define _GT_MD_LEXER_H_

    #include "gt_md_config.h"

    #if GT_MD_ENABLE

    #include <stdint.h>

    /* ----------------------------------------------------------------
     * 块级 Token 类型枚举 / Block token type enum
     * ---------------------------------------------------------------- */
    typedef enum {
        GT_MD_BLK_HEADING     = 0,  /* # ## ### 标题 / Heading H1-H6 */
        GT_MD_BLK_PARAGRAPH,        /* 普通段落 / Paragraph */
        GT_MD_BLK_LIST_ITEM,        /* - / * / 1. 列表项 / List item */
        GT_MD_BLK_CODE_BLOCK,       /* ```...``` 代码块 / Fenced code block */
        GT_MD_BLK_BLOCKQUOTE,       /* > 引用块 / Block quote */
        GT_MD_BLK_HR,               /* --- / *** 分隔线 / Horizontal rule */
        GT_MD_BLK_LATEX_BLOCK,      /* $$...$$ 块级公式 / Block-level LaTeX formula */
        GT_MD_BLK_BLANK,            /* 空行 / Blank line */
    } gt_md_block_type_t;

    /* ----------------------------------------------------------------
     * 块级 Token 结构体 / Block token struct
     * ---------------------------------------------------------------- */
    typedef struct {
        gt_md_block_type_t  type;       /* 块类型 / Block type */
        const char         *start;      /* 指向原始文本（无需复制）/ Pointer to source text (no copy) */
        uint16_t            len;        /* 内容字节长度（不含前缀标记）/ Content length in bytes */
        uint8_t             level;      /* 标题级别 1-6 / 列表缩进级别 / Heading level 1-6 or list indent */
        uint8_t             ordered;    /* 1=有序列表 0=无序列表 / 1=ordered list, 0=unordered */
        uint16_t            num;        /* 有序列表起始序号 / Ordered list start number */
    } gt_md_block_token_t;

    /* ----------------------------------------------------------------
     * 块级扫描器上下文 / Block lexer context
     * ---------------------------------------------------------------- */
    typedef struct {
        const char *src;        /* 输入文本指针 / Input text pointer */
        uint16_t    src_len;    /* 输入文本长度 / Input text length */
        uint16_t    pos;        /* 当前扫描位置 / Current scan position */
    } gt_md_lexer_t;

    /* ----------------------------------------------------------------
     * 接口声明 / API declarations
     * ---------------------------------------------------------------- */

    /**
     * @brief 初始化块级扫描器 / Initialize block lexer
     * @param lexer    扫描器上下文 / Lexer context
     * @param src      输入的 Markdown 文本 / Input Markdown text
     * @param src_len  文本长度（字节）/ Text length in bytes
     */
    void gt_md_lexer_init(gt_md_lexer_t *lexer, const char *src, uint16_t src_len);

    /**
     * @brief 获取下一个块级 Token（迭代器模式）
     *        Get next block-level token (iterator mode)
     * @note  采用单步迭代模式，无动态分配，行缓冲栈上分配
     *        Single-step iterator, no dynamic allocation, line buffer on stack
     * @param lexer 扫描器上下文 / Lexer context
     * @param token 输出的 Token / Output token
     * @return      1=成功取到 Token，0=已到达末尾 / 1=token available, 0=end of input
     */
    int gt_md_lexer_next(gt_md_lexer_t *lexer, gt_md_block_token_t *token);

    /**
     * @brief 查看下一个块级 Token 但不消费（前瞻）
     *        Peek at next block-level token without consuming it
     * @param lexer 扫描器上下文 / Lexer context
     * @param token 输出的 Token / Output token
     * @return      1=成功取到 Token，0=已到达末尾 / 1=token available, 0=end of input
     */
    int gt_md_lexer_peek(gt_md_lexer_t *lexer, gt_md_block_token_t *token);

    /**
     * @brief 检查扫描器是否已到达末尾 / Check if lexer reached end of input
     * @param lexer 扫描器上下文 / Lexer context
     * @return      1=已结束 0=还有内容 / 1=end, 0=more content
     */
    int gt_md_lexer_is_end(const gt_md_lexer_t *lexer);

    /* ----------------------------------------------------------------
     * 列表子迭代器（流式模式低内存方案）
     * List item sub-iterator (low-memory streaming)
     * ---------------------------------------------------------------- */
    typedef struct {
        gt_md_lexer_t *parent;  /* 父扫描器 / Parent lexer */
        uint8_t        ordered; /* 是否有序列表 / Is ordered list */
        uint16_t       num;     /* 当前列表项编号 / Current item number */
        uint8_t        indent;  /* 缩进层级 / Indent level */
    } gt_md_list_iter_t;

    /**
     * @brief 初始化列表子迭代器 / Initialize list item sub-iterator
     * @param it      列表迭代器 / List iterator
     * @param parent  父扫描器（已定位到列表第一项）/ Parent lexer (positioned at first item)
     * @param token   第一个列表项 Token / First list item token
     */
    void gt_md_list_iter_init(gt_md_list_iter_t *it, gt_md_lexer_t *parent,
                               const gt_md_block_token_t *token);

    /**
     * @brief 获取下一个列表项 Token / Get next list item token
     * @param it    列表迭代器 / List iterator
     * @param token 输出的 Token / Output token
     * @return      1=成功 0=列表结束 / 1=success, 0=end of list
     */
    int gt_md_list_iter_next(gt_md_list_iter_t *it, gt_md_block_token_t *token);

    #endif /* GT_MD_ENABLE */

#endif /* _GT_MD_LEXER_H_ */
