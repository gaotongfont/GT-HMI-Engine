/**
 * @file gt_latex_lexer.h
 * @brief GT-Latex 词法分析器 / Lexer
 *        将 LaTeX 公式字符串解析为 Token 流
 *        Tokenizes a LaTeX formula string into a token stream
 */

#ifndef _GT_LATEX_LEXER_H_
#define _GT_LATEX_LEXER_H_

#include "gt_latex_config.h"

#if GT_LATEX_ENABLE

#include <stdint.h>

/* ----------------------------------------------------------------
 * Token 类型枚举 / Token type enum
 * ---------------------------------------------------------------- */
typedef enum {
    GT_LATEX_TOKEN_UNKNOWN  = 0,    /* 未知 / Unknown */
    GT_LATEX_TOKEN_CMD,             /* LaTeX 命令，如 \alpha \frac / LaTeX command */
    GT_LATEX_TOKEN_CHAR,            /* 单个字母或数字 / Single letter or digit */
    GT_LATEX_TOKEN_NUMBER,          /* 多位数字 / Multi-digit number */
    GT_LATEX_TOKEN_SUPERSCRIPT,     /* 上标符号 ^ / Superscript */
    GT_LATEX_TOKEN_SUBSCRIPT,       /* 下标符号 _ / Subscript */
    GT_LATEX_TOKEN_LBRACE,          /* 左花括号 { / Left brace */
    GT_LATEX_TOKEN_RBRACE,          /* 右花括号 } / Right brace */
    GT_LATEX_TOKEN_PLUS,            /* + */
    GT_LATEX_TOKEN_MINUS,           /* - */
    GT_LATEX_TOKEN_TIMES,           /* * */
    GT_LATEX_TOKEN_SLASH,           /* / */
    GT_LATEX_TOKEN_EQUAL,           /* = */
    GT_LATEX_TOKEN_LPAREN,          /* 左圆括号 ( / Left parenthesis */
    GT_LATEX_TOKEN_RPAREN,          /* 右圆括号 ) / Right parenthesis */
    GT_LATEX_TOKEN_LBRACKET,        /* 左方括号 [ / Left bracket */
    GT_LATEX_TOKEN_RBRACKET,        /* 右方括号 ] / Right bracket */
    GT_LATEX_TOKEN_COMMA,           /* 逗号 , / Comma */
    GT_LATEX_TOKEN_DOT,             /* 小数点 . / Dot */
    GT_LATEX_TOKEN_AMPERSAND,       /* & 列分隔符（矩阵）/ Column separator (matrix) */
    GT_LATEX_TOKEN_NEWLINE,         /* \\ 行分隔符（矩阵）/ Row separator (matrix) */
    GT_LATEX_TOKEN_EOF,             /* 字符串结束 / End of input */
} gt_latex_token_type_t;

/* ----------------------------------------------------------------
 * Token 结构体 / Token struct
 * ---------------------------------------------------------------- */
typedef struct {
    gt_latex_token_type_t type;     /* Token 类型 / Token type */
    const char           *start;   /* 指向原始字符串中的起始位置 / Pointer to start in source */
    uint16_t              len;      /* Token 长度（字节）/ Token length in bytes */
    uint32_t              code;     /* 预解码的 Unicode code point（CHAR token 使用）/ Pre-decoded Unicode (CHAR token) */
} gt_latex_token_t;

/* 流式模式环形缓冲深度（2槽：1当前 + 1预取，满足1-token前瞻）
 * Stream mode ring buffer depth (2 slots: current + prefetch, enough for 1-token lookahead) */
#define GT_LATEX_STREAM_BUF_SIZE  2

/* ----------------------------------------------------------------
 * 词法分析器上下文 / Lexer context
 * ---------------------------------------------------------------- */
typedef struct {
    const char        *src;         /* 输入字符串 / Input string */
    uint16_t           pos;         /* 当前解析位置 / Current parse position */
    uint16_t           src_len;     /* 输入字符串长度 / Input string length */
    gt_latex_token_t  *tokens;      /* Token 数组（动态分配，流式模式为 NULL）/ Token array (NULL in stream mode) */
    uint16_t           token_count; /* 已解析的 Token 数量 / Number of tokens parsed */
    uint16_t           token_cap;   /* Token 数组容量 / Token array capacity */
    /* ---- 流式模式专用字段 / Streaming mode fields ---- */
    uint8_t            stream_mode; /* 0=批量模式，1=流式模式 / 0=batch, 1=stream */
    gt_latex_token_t   sbuf[GT_LATEX_STREAM_BUF_SIZE]; /* 固定环形缓冲 / Fixed ring buffer */
    uint8_t            sbuf_head;   /* 下一个待消费的槽位 / Next slot to consume */
    uint8_t            sbuf_fill;   /* 缓冲中有效 token 数 / Valid tokens in buffer */
    uint8_t            sbuf_eof;    /* 1=源已扫描完毕 / 1=source exhausted */
} gt_latex_lexer_t;

/* ----------------------------------------------------------------
 * 接口声明 / API declarations
 * ---------------------------------------------------------------- */

/**
 * @brief 初始化词法分析器 / Initialize lexer
 * @param lexer    词法分析器上下文 / Lexer context
 * @param src      输入的 LaTeX 公式字符串 / Input LaTeX formula string
 * @param src_len  公式字符串长度（字节）/ Formula string length in bytes
 * @return         0 成功，负数失败 / 0 on success, negative on failure
 */
int gt_latex_lexer_init(gt_latex_lexer_t *lexer, const char *src, uint16_t src_len);

/**
 * @brief 执行词法分析，生成完整 Token 流 / Run lexer, produce full token stream
 * @param lexer 词法分析器上下文 / Lexer context
 * @return      0 成功，负数失败 / 0 on success, negative on failure
 */
int gt_latex_lexer_tokenize(gt_latex_lexer_t *lexer);

/**
 * @brief 释放词法分析器占用的资源 / Free lexer resources
 * @param lexer 词法分析器上下文 / Lexer context
 */
void gt_latex_lexer_deinit(gt_latex_lexer_t *lexer);

/**
 * @brief 获取指定索引的 Token / Get token at index
 * @param lexer 词法分析器上下文 / Lexer context
 * @param index Token 索引 / Token index
 * @return      指向 Token 的指针，越界返回 NULL / Pointer to token, NULL if out of range
 */
const gt_latex_token_t *gt_latex_lexer_get(const gt_latex_lexer_t *lexer, uint16_t index);

/**
 * @brief 流式模式初始化（零堆内存，固定 40B 环形缓冲）
 *        Initialize lexer in streaming mode (zero heap, fixed 40B ring buffer)
 * @param lexer    词法分析器上下文 / Lexer context
 * @param src      输入的 LaTeX 公式字符串 / Input LaTeX formula string
 * @param src_len  公式字符串长度（字节）/ Formula string length in bytes
 * @return         0 成功，负数失败 / 0 on success, negative on failure
 */
int gt_latex_lexer_init_stream(gt_latex_lexer_t *lexer, const char *src, uint16_t src_len);

#endif /* GT_LATEX_ENABLE */

#endif /* _GT_LATEX_LEXER_H_ */
