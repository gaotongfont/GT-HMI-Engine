/**
 * @file gt_latex_parser.h
 * @brief GT-Latex 语法解析器 / Parser
 *        将 Token 流解析为 AST 节点树
 *        Parses token stream into an AST node tree
 */

#ifndef _GT_LATEX_PARSER_H_
#define _GT_LATEX_PARSER_H_

#include "gt_latex_config.h"

#if GT_LATEX_ENABLE

#include "gt_latex_lexer.h"
#include <stdint.h>

/* ----------------------------------------------------------------
 * AST 节点类型枚举 / AST node type enum
 * ---------------------------------------------------------------- */
typedef enum {
    GT_LATEX_NODE_UNKNOWN   = 0,
    GT_LATEX_NODE_SEQ,              /* 序列（多个子节点并排）/ Sequence */
    GT_LATEX_NODE_CHAR,             /* 单个字符 / Single character */
    GT_LATEX_NODE_NUMBER,           /* 数字 / Number */
    GT_LATEX_NODE_SYMBOL,           /* LaTeX 符号命令（\alpha 等）/ Symbol command */
    GT_LATEX_NODE_SUPERSCRIPT,      /* 上标 a^{...} / Superscript */
    GT_LATEX_NODE_SUBSCRIPT,        /* 下标 a_{...} / Subscript */
    GT_LATEX_NODE_SUPSUB,           /* 同时含上下标 / Both sup and sub */
    GT_LATEX_NODE_GROUP,            /* 分组 {...} / Group */
    GT_LATEX_NODE_FRAC,             /* 分数 \frac{}{} / Fraction */
    GT_LATEX_NODE_SQRT,             /* 根号 \sqrt{} / Square root */
    GT_LATEX_NODE_SUM,              /* 求和 \sum / Summation */
    GT_LATEX_NODE_INT,              /* 积分 \int / Integral */
    GT_LATEX_NODE_PROD,             /* 连乘 \prod / Product */
    GT_LATEX_NODE_OVERLINE,         /* 上划线 \overline{} / Overline */
    GT_LATEX_NODE_UNDERLINE,        /* 下划线 \underline{} / Underline */
    GT_LATEX_NODE_VEC,              /* 向量箭头 \vec{} / Vector arrow */
    GT_LATEX_NODE_HAT,              /* 帽子 \hat{} / Hat accent */
    GT_LATEX_NODE_BAR,              /* 短横线 \bar{} / Bar accent */
    GT_LATEX_NODE_DOT,              /* 单点 \dot{} / Dot accent */
    GT_LATEX_NODE_DDOT,             /* 双点 \ddot{} / Double dot accent */
    GT_LATEX_NODE_TILDE,            /* 波浪号 \tilde{} / Tilde accent */
    GT_LATEX_NODE_CHECK,            /* 倒帽 \check{} / Check accent */
    GT_LATEX_NODE_GRAVE,            /* 重音 \grave{} / Grave accent */
    GT_LATEX_NODE_ACUTE,            /* 锐音 \acute{} / Acute accent */
    GT_LATEX_NODE_BREVE,            /* 短音 \breve{} / Breve accent */
    GT_LATEX_NODE_NOT,              /* 否定斜线 \not{} / Negation slash */
    GT_LATEX_NODE_WIDETILDE,        /* 宽波浪 \widetilde{} / Wide tilde */
    GT_LATEX_NODE_WIDEHAT,          /* 宽帽 \widehat{} / Wide hat */
    GT_LATEX_NODE_OVERLEFTARROW,    /* 左箭头上方 \overleftarrow{} / Overleft arrow */
    GT_LATEX_NODE_OVERRIGHTARROW,   /* 右箭头上方 \overrightarrow{} / Overright arrow */
    GT_LATEX_NODE_OVERLEFTRIGHTARROW, /* 双向箭头上方 \overleftrightarrow{} / Overleftrightarrow */
    GT_LATEX_NODE_OVERBRACE,        /* 上花括号 \overbrace{} / Overbrace */
    GT_LATEX_NODE_UNDERBRACE,       /* 下花括号 \underbrace{} / Underbrace */
    GT_LATEX_NODE_OVERSET,          /* 上方堆叠 \overset{a}{X} / Overset */
    GT_LATEX_NODE_UNDERSET,         /* 下方堆叠 \underset{a}{X} / Underset */
    GT_LATEX_NODE_XARROW,           /* 可扩展箭头 \xleftarrow / \xrightarrow / Extensible arrow */
    GT_LATEX_NODE_DELIM,            /* 自适应括号 \left...\right / Adaptive delimiter pair */
    GT_LATEX_NODE_MATHFN,           /* 数学函数名 \sin \cos 等 / Math function name */
    GT_LATEX_NODE_LIMOP,            /* 极限运算符 \lim \limsup \liminf / Limit operator */
    GT_LATEX_NODE_MATRIX,           /* 矩阵 \begin{matrix}...\end{matrix} / Matrix */
    GT_LATEX_NODE_CASES,            /* 分段函数 \begin{cases}...\end{cases} / Cases */
    GT_LATEX_NODE_ALIGN,            /* 多行对齐 \begin{align*}...\end{align*} / Align */
} gt_latex_node_type_t;

/* ----------------------------------------------------------------
 * 公共头部（所有节点结构体首字段）/ Common header for all node types
 * ---------------------------------------------------------------- */
typedef struct {
    gt_latex_node_type_t type;
} gt_latex_node_base_t;

/* ----------------------------------------------------------------
 * 各类型独立节点结构体 / Per-type node structs
 * ---------------------------------------------------------------- */

/* 字符节点（CHAR / NUMBER）/ Character or number node */
typedef struct {
    gt_latex_node_base_t base;
    uint32_t             code;      /* 字符编码（ASCII/Unicode）/ Character code */
} gt_latex_node_char_t;

/* 符号节点（\alpha 等）/ Symbol node */
typedef struct {
    gt_latex_node_base_t base;
    const char          *cmd;       /* 命令名起始（含 \）/ Command start (includes \) */
    uint16_t             cmd_len;   /* 命令名长度 / Command length */
} gt_latex_node_symbol_t;

/* 分组节点 {...} / Group node */
typedef struct {
    gt_latex_node_base_t  base;
    gt_latex_node_base_t *inner;    /* 组内内容 / Inner content */
} gt_latex_node_group_t;

/* 序列节点 / Sequence node */
typedef struct {
    gt_latex_node_base_t   base;
    gt_latex_node_base_t **items;   /* 动态数组 / Dynamic array */
    uint8_t                count;   /* 元素数量 / Item count */
    uint8_t                cap;     /* 当前容量 / Current capacity */
} gt_latex_node_seq_t;

/* 上标节点 / Superscript node */
typedef struct {
    gt_latex_node_base_t  base;
    gt_latex_node_base_t *base_node; /* 底数 / Base */
    gt_latex_node_base_t *sup;       /* 上标内容 / Superscript content */
} gt_latex_node_superscript_t;

/* 下标节点 / Subscript node */
typedef struct {
    gt_latex_node_base_t  base;
    gt_latex_node_base_t *base_node;
    gt_latex_node_base_t *sub;       /* 下标内容 / Subscript content */
} gt_latex_node_subscript_t;

/* 上下标节点 / Sup+sub node */
typedef struct {
    gt_latex_node_base_t  base;
    gt_latex_node_base_t *base_node;
    gt_latex_node_base_t *sup;
    gt_latex_node_base_t *sub;
} gt_latex_node_supsub_t;

/* 分数节点 / Fraction node */
typedef struct {
    gt_latex_node_base_t  base;
    gt_latex_node_base_t *numer;    /* 分子 / Numerator */
    gt_latex_node_base_t *denom;    /* 分母 / Denominator */
    uint8_t               is_tfrac; /* 1 = \tfrac（强制文本字号）/ text-style frac */
    uint8_t               is_binom; /* 1 = \binom（无分数线，二项式系数）/ binomial coeff */
} gt_latex_node_frac_t;

/* 根号节点 / Square root node */
typedef struct {
    gt_latex_node_base_t  base;
    gt_latex_node_base_t *index;    /* 根指数（可 NULL，NULL 表示平方根）/ Root index, NULL = square root */
    gt_latex_node_base_t *inner;    /* 被开方内容 / Radicand */
} gt_latex_node_sqrt_t;

/* 大型运算符节点（\sum / \int / \prod 共用）/ Large operator node */
typedef struct {
    gt_latex_node_base_t  base;
    const char           *cmd;      /* 命令名起始（含 \）/ Command start */
    uint16_t              cmd_len;
    gt_latex_node_base_t *lower;    /* 下限（可 NULL）/ Lower limit */
    gt_latex_node_base_t *upper;    /* 上限（可 NULL）/ Upper limit */
} gt_latex_node_large_op_t;

/* 上下划线节点（\overline / \underline 共用）/ Accent node (overline / underline) */
typedef struct {
    gt_latex_node_base_t  base;
    gt_latex_node_base_t *inner;    /* 被修饰内容 / Decorated content */
} gt_latex_node_accent_t;

/* 自适应括号节点 \left...\right / Adaptive delimiter node */
typedef struct {
    gt_latex_node_base_t  base;
    uint32_t              open;     /* 左括号字符编码，0 = 无 / Left delimiter code, 0 = none */
    uint32_t              close;    /* 右括号字符编码，0 = 无 / Right delimiter code, 0 = none */
    gt_latex_node_base_t *inner;    /* 括号内内容 / Inner content */
} gt_latex_node_delim_t;

/* 数学函数名节点 \sin \cos 等 / Math function name node */
typedef struct {
    gt_latex_node_base_t  base;
    const char           *name;     /* 函数名（不含 \），指向源字符串 / Name without backslash */
    uint8_t               name_len; /* 函数名长度 / Name length */
} gt_latex_node_mathfn_t;

/* 极限运算符节点 \lim \limsup \liminf / Limit operator node */
typedef struct {
    gt_latex_node_base_t  base;
    const char           *name;     /* 函数名（不含 \），指向源字符串 / Name without backslash */
    uint8_t               name_len; /* 函数名长度 / Name length */
    gt_latex_node_base_t *sub;      /* 下标（可 NULL），居中于函数名正下方 / Subscript, centered below name */
} gt_latex_node_limop_t;

/* 上/下方堆叠节点 \overset{a}{X} \underset{a}{X} / Overset / Underset node */
typedef struct {
    gt_latex_node_base_t  base;
    gt_latex_node_base_t *over;      /* 上方内容（小字号）/ Over content (smaller font) */
    gt_latex_node_base_t *base_node; /* 主体内容 / Base content */
} gt_latex_node_overset_t;

/* 可扩展箭头节点 \xleftarrow[sub]{sup} \xrightarrow[sub]{sup} / Extensible arrow node */
typedef struct {
    gt_latex_node_base_t  base;
    uint8_t               is_left;  /* 1 = 左箭头，0 = 右箭头 / 1=left, 0=right */
    gt_latex_node_base_t *sup;      /* 上方内容（可 NULL）/ Above content */
    gt_latex_node_base_t *sub;      /* 下方内容（可 NULL）/ Below content */
} gt_latex_node_xarrow_t;

/* 矩阵节点 \begin{matrix}...\end{matrix} / Matrix node */
typedef struct {
    gt_latex_node_base_t   base;
    uint8_t                rows;         /* 行数 / Number of rows */
    uint8_t                cols;         /* 列数 / Number of columns */
    gt_latex_node_base_t **cells;        /* 单元格数组（rows * cols）/ Cell array */
    uint8_t                bracket_type; /* 括号类型：0=无 1=() 2=[] 3={} 4=| 5=|| 6=仅左{} / Bracket type */
    uint8_t                is_small;     /* 是否为 smallmatrix / Is smallmatrix */
    uint8_t               *row_spacings; /* 每行后额外间距（像素），NULL=全默认 / Extra spacing after each row (px), NULL=default */
} gt_latex_node_matrix_t;

/* cases 节点（复用 matrix 结构）/ Cases node (reuses matrix structure) */
typedef gt_latex_node_matrix_t gt_latex_node_cases_t;

/* align* 节点（复用 matrix 结构）/ Align node (reuses matrix structure) */
typedef gt_latex_node_matrix_t gt_latex_node_align_t;

/* ----------------------------------------------------------------
 * 解析器上下文 / Parser context
 * ---------------------------------------------------------------- */
typedef struct {
    const gt_latex_lexer_t *lexer;
    uint16_t                pos;
    gt_latex_node_base_t   *root;
} gt_latex_parser_t;

/* ----------------------------------------------------------------
 * 接口声明 / API declarations
 * ---------------------------------------------------------------- */

int gt_latex_parser_init(gt_latex_parser_t *parser, const gt_latex_lexer_t *lexer);
int gt_latex_parser_parse(gt_latex_parser_t *parser);
void gt_latex_parser_deinit(gt_latex_parser_t *parser);

/**
 * @brief 递归释放节点及其所有子节点 / Recursively free node and all children
 */
void gt_latex_node_free(gt_latex_node_base_t *node);

#endif /* GT_LATEX_ENABLE */

#endif /* _GT_LATEX_PARSER_H_ */
