/**
 * @file gt_latex_symbol.h
 * @brief GT-Latex 符号映射表 / Symbol mapping table
 *        LaTeX 命令字符串 → Unicode 编码
 *        Maps LaTeX command strings to Unicode code points
 */

#ifndef _GT_LATEX_SYMBOL_H_
#define _GT_LATEX_SYMBOL_H_

#include "gt_latex_config.h"

#if GT_LATEX_ENABLE

#include <stdint.h>

/* ----------------------------------------------------------------
 * 符号类别枚举 / Symbol category enum
 * ---------------------------------------------------------------- */
typedef enum {
    GT_LATEX_SYM_CAT_UNKNOWN    = 0,    /* 未知 / Unknown */
    GT_LATEX_SYM_CAT_GREEK,             /* 希腊字母 / Greek letters */
    GT_LATEX_SYM_CAT_OPERATOR,          /* 数学运算符 / Math operators */
    GT_LATEX_SYM_CAT_RELATION,          /* 关系符号 / Relation symbols */
    GT_LATEX_SYM_CAT_ARROW,             /* 箭头符号 / Arrow symbols */
    GT_LATEX_SYM_CAT_LARGE_OP,          /* 大型运算符 / Large operators */
} gt_latex_sym_cat_t;

/* ----------------------------------------------------------------
 * 符号映射条目 / Symbol map entry
 * ---------------------------------------------------------------- */
typedef struct {
    const char        *cmd;     /* LaTeX 命令名（不含 \）/ Command name (without \) */
    uint32_t           code;    /* Unicode 编码 / Unicode code point */
    gt_latex_sym_cat_t cat;     /* 符号类别 / Symbol category */
} gt_latex_sym_entry_t;

/* ----------------------------------------------------------------
 * 接口声明 / API declarations
 * ---------------------------------------------------------------- */

/**
 * @brief 根据命令名查找 Unicode 编码 / Look up Unicode code by command name
 * @param cmd  命令名字符串（不含 \，如 "alpha"）/ Command name without backslash
 * @param len  命令名长度 / Command name length
 * @param code 输出 Unicode 编码 / Output Unicode code point
 * @return     0 成功，-1 未找到 / 0 on success, -1 if not found
 */
int gt_latex_symbol_lookup(const char *cmd, uint16_t len, uint32_t *code);

/**
 * @brief 根据命令名查找符号类别 / Look up symbol category by command name
 * @param cmd 命令名字符串（不含 \）/ Command name without backslash
 * @param len 命令名长度 / Command name length
 * @return    符号类别，未找到返回 GT_LATEX_SYM_CAT_UNKNOWN
 */
gt_latex_sym_cat_t gt_latex_symbol_get_cat(const char *cmd, uint16_t len);

/**
 * @brief 获取符号映射表指针及条目数量 / Get symbol table pointer and entry count
 * @param count 输出条目数量 / Output entry count
 * @return      映射表指针 / Pointer to symbol table
 */
const gt_latex_sym_entry_t *gt_latex_symbol_table(uint16_t *count);

#endif /* GT_LATEX_ENABLE */

#endif /* _GT_LATEX_SYMBOL_H_ */
