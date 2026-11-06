/**
 * @file gt_example_md_selftest.c
 * @author Feyoung
 * @brief Markdown widget self-test: M-1 code block, M-2 table column lines,
 *        M-3 full-width character advance (markdown-issues-2026-10-18.docx)
 * @version 0.1
 * @date 2026-10-19
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 *
 * M-1: fenced code block must produce its own block and preserve content.
 * M-2: table column separators must be crisp vertical lines (visual check).
 * M-3: CJK characters must advance by full em width, even if the font returns
 *      an ink-bounds width smaller than the em (vector fallback contract).
 */

/* include --------------------------------------------------------------*/

#include "../gt_examples.h"

#if GT_USE_EXAMPLES

#include "../../src/widgets/gt_markdown.h"
#include "../../src/widgets/gt_textarea.h"
#include "../../src/widgets/gt_label.h"
#include "../../src/widgets/gt_obj.h"
#include "../../src/font/gt_font.h"
#include <stdio.h>
#include <string.h>

/* M-7 source: nested list state pollution. The parent list MUST have items
 * AFTER the nested child list — the old v15-nest case ended with the nesting,
 * which is exactly why the pollution was never caught. */
static const char _md_m7_ol_ol[] =
    "1. 甲项\n"
    "   1. 子一\n"
    "   2. 子二\n"
    "   3. 子三\n"
    "2. 乙项\n"
    "3. 丙项\n";

/* OL nesting UL: parent must keep its ordered marker and counter */
static const char _md_m7_ol_ul[] =
    "1. 甲项\n"
    "   - 子一\n"
    "   - 子二\n"
    "   - 子三\n"
    "2. 乙项\n";

/* UL nesting UL: parent must keep the bullet marker (no numbers) */
static const char _md_m7_ul_ul[] =
    "- 甲项\n"
    "  - 子一\n"
    "  - 子二\n"
    "- 乙项\n";

/* private define -------------------------------------------------------*/

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/
static int _pass = 0, _total = 0;
static uint8_t _font_res[8192];

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* M-1 source: paragraph / fenced code / paragraph, as reported */
static const char _md_m1[] =
    "# M-1 code block\n"
    "\n"
    "代码块之前的一段\n"
    "\n"
    "```c\n"
    "int main(void) {\n"
    "    printf(\"hello\");\n"
    "    return 0;\n"
    "}\n"
    "```\n"
    "\n"
    "代码块之后的一段**加粗测试**\n";

/* M-2 source: the exact table from the report */
static const char _md_m2[] =
    "| 参数 | 数值 | 说明 |\n"
    "| --- | --- | --- |\n"
    "| 1200 | 25 | 短中文 |\n"
    "| 3300 | 18 | 中文较长内容测试 |\n"
    "| 25 | 1200 | 数字列 |\n";

/* M-3 source: full-width sequence characters (report text) */
static const char _md_m3[] =
    "序数字符宽度测试\n"
    "\n"
    "一二三四五六七八九十壹贰叁肆伍陆柒捌玖拾佰仟万亿兆"
    "甲乙丙丁戊己庚辛壬癸子丑寅卯辰巳午未申酉戌亥文字内容\n";

/* static functions -----------------------------------------------------*/
static void _tc_report(const char * name, bool ok, const char * detail)
{
    ++_total;
    if (ok) { ++_pass; }
    printf("[MD_TC] %-46s %s | %s\n", name, ok ? "PASS" : "FAIL", detail);
    fflush(stdout);
}

/** @brief Search all textarea children of the markdown block for needle */
static bool _child_text_contains(gt_obj_st * md, int idx, const char * needle)
{
    if (idx >= 0) {
        /* explicit child index */
        if (idx >= md->cnt_child) {
            return false;
        }
        gt_obj_st * only = md->child[idx];
        if (false == gt_obj_is_type(only, GT_TYPE_TEXTAREA)) {
            return false;
        }
        uint16_t n = gt_textarea_get_str_count(only);
        for (uint16_t i = 0; i < n; ++i) {
            const char * text = gt_textarea_get_text(only, i);
            if (text && strstr(text, needle)) {
                return true;
            }
        }
        return false;
    }
    for (int c = 0; c < md->cnt_child; ++c) {
        gt_obj_st * child = md->child[c];
        if (false == gt_obj_is_type(child, GT_TYPE_TEXTAREA)) {
            continue;
        }
        uint16_t cnt = gt_textarea_get_str_count(child);
        for (uint16_t i = 0; i < cnt; ++i) {
            const char * text = gt_textarea_get_text(child, i);
            if (text && strstr(text, needle)) {
                return true;
            }
        }
    }
    return false;
}

/** @brief True if some textarea entry contains marker and the very next
 *         entry contains text. List markers (" 2. ", " · ") are appended as
 *         their own entry right before the item text, so a list item's
 *         rendered marker is verified by this adjacency. */
static bool _child_marker_followed_by(gt_obj_st * md, const char * marker,
                                      const char * text)
{
    for (int c = 0; c < md->cnt_child; ++c) {
        gt_obj_st * child = md->child[c];
        if (false == gt_obj_is_type(child, GT_TYPE_TEXTAREA)) {
            continue;
        }
        uint16_t cnt = gt_textarea_get_str_count(child);
        for (uint16_t i = 0; i + 1 < cnt; ++i) {
            const char * m = gt_textarea_get_text(child, i);
            const char * t = gt_textarea_get_text(child, i + 1);
            if (m && t && strstr(m, marker) && strstr(t, text)) {
                return true;
            }
        }
    }
    return false;
}

/* M-3 probe: measured advance of one Unicode code point with markdown body font */
static uint16_t _md_char_width(gt_obj_st * md, uint32_t unicode)
{
    gt_font_info_st * info = gt_markdown_get_font_info_small(md);
    gt_font_st font = {
        .info = *info,
        .res = _font_res,
        .utf8 = NULL,
        .len = 0,
    };
    return gt_font_get_one_word_width(unicode, &font);
}

/** @brief Measure one code point advance for an explicit family/size/gray */
static uint16_t _fam_char_width_gray(gt_obj_st * md, gt_family_t fam, uint16_t size,
                                     uint8_t gray, uint32_t unicode)
{
    gt_font_info_st info = *gt_markdown_get_font_info_small(md);
    gt_font_set_family(&info, fam);
    info.size = size;
    info.gray = gray;
    gt_font_st font = {
        .info = info,
        .res = _font_res,
        .utf8 = NULL,
        .len = 0,
    };
    return gt_font_get_one_word_width(unicode, &font);
}

static uint16_t _fam_char_width(gt_obj_st * md, gt_family_t fam, uint16_t size, uint32_t unicode)
{
    return _fam_char_width_gray(md, fam, size, 1, unicode);
}

/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_examples_md_selftest(void)
{
    gt_obj_st * screen;
    gt_obj_st * md;
    char detail[96];

    screen = gt_obj_create(NULL);

    /* ---------- M-1: code block ---------- */
    md = gt_markdown_create(screen);
    gt_obj_set_pos(md, 0, 0);
    gt_obj_set_size(md, GT_SCREEN_WIDTH, 220);
    gt_markdown_set_text(md, _md_m1, (uint32_t)strlen(_md_m1));

    snprintf(detail, sizeof(detail), "children=%d (expect 4: h/p/code/p)", md->cnt_child);
    _tc_report("M1 code block creates its own block", 4 == md->cnt_child, detail);

    bool has_code = _child_text_contains(md, -1, "printf");
    _tc_report("M1 code content preserved", has_code,
        has_code ? "found printf in code block" : "code text lost");

    bool has_indent = _child_text_contains(md, -1, "    printf");
    _tc_report("M1 code indentation preserved", has_indent,
        has_indent ? "4-space indent kept" : "indent stripped");

    /* code block frame height must include padding, else the last line
       (the closing brace) is drawn outside the light background */
    gt_obj_st * code = (md->cnt_child > 2) ? md->child[2] : NULL;
    bool frame_ok = false;
    if (code && gt_obj_is_type(code, GT_TYPE_TEXTAREA)) {
        uint32_t content_h = gt_textarea_get_total_height(code);
        frame_ok = gt_obj_get_h(code) >= content_h + 16;
        snprintf(detail, sizeof(detail), "frame h=%d, content h=%u",
            gt_obj_get_h(code), content_h);
    } else {
        snprintf(detail, sizeof(detail), "code block child missing");
    }
    _tc_report("M1 code frame encloses all lines", frame_ok, detail);

    /* ---------- M-2: table column separators ---------- */
    gt_obj_st * md2 = gt_markdown_create(screen);
    gt_obj_set_pos(md2, 0, 225);
    gt_obj_set_size(md2, GT_SCREEN_WIDTH, 150);
    gt_markdown_set_text(md2, _md_m2, (uint32_t)strlen(_md_m2));
    snprintf(detail, sizeof(detail), "table h=%d (visual: 2 v-lines)",
        md2->cnt_child ? gt_obj_get_h(md2->child[0]) : 0);
    _tc_report("M2 table built", md2->cnt_child > 0, detail);

    /* ---------- M-3: full-width advance ---------- */
    gt_obj_st * md3 = gt_markdown_create(screen);
    gt_obj_set_pos(md3, 0, 380);
    gt_obj_set_size(md3, GT_SCREEN_WIDTH, 100);
    /** Visual uses the default (dot) family; the vector families are covered
     *  by the width probes below. Vector CJK rendering in the simulator's v2
     *  driver is a separate, pre-existing issue and must not mask M-3. */
    gt_markdown_set_text(md3, _md_m3, (uint32_t)strlen(_md_m3));

    /* control: common CJK, and report's sequence chars */
    static const struct { uint32_t cp; const char * name; } probe[] = {
        { 0x4E2D, "中(control)" },
        { 0x6587, "文(control)" },
        { 0x4E00, "一" },
        { 0x4E8C, "二" },
        { 0x53C1, "叁" },
        { 0x5146, "兆" },
        { 0x7532, "甲" },
        { 0x5B50, "子" },
        { 0x3001, "、(punct)" },
        { 'A',    "A(ascii)" },
    };
    const uint16_t EM = 16;
    uint16_t full = _md_char_width(md, 0x4E2D);
    bool m3_ok = (full >= EM);
    for (uint32_t i = 0; i < sizeof(probe) / sizeof(probe[0]); ++i) {
        uint16_t w = _md_char_width(md, probe[i].cp);
        printf("[MD_TC]   width U+%04X %-12s = %u\n", probe[i].cp, probe[i].name, w);
        if (probe[i].cp != 'A' && w < EM) {
            m3_ok = false;
        }
    }
    snprintf(detail, sizeof(detail), "em=%u, all full-width >= em", full);
    _tc_report("M3 CJK advance is full width", m3_ok, detail);

    /* vector family (the family that uses ink-bounds width, M-3 report env.) */
    uint16_t vec_full = _fam_char_width(md, family_vec, 16, 0x4E2D);
    bool vec_ok = (vec_full >= EM);
    for (uint32_t i = 0; i < sizeof(probe) / sizeof(probe[0]); ++i) {
        uint16_t w = _fam_char_width(md, family_vec, 16, probe[i].cp);
        printf("[MD_TC]   vec width U+%04X %-12s = %u\n", probe[i].cp, probe[i].name, w);
        if (probe[i].cp != 'A' && w < EM) {
            vec_ok = false;
        }
    }
    snprintf(detail, sizeof(detail), "vec em=%u, all full-width >= em", vec_full);
    _tc_report("M3 vector CJK advance is full width", vec_ok, detail);

    /* vector 4bpp: driver computes advance from glyph ink bounds */
    uint16_t vec4_full = _fam_char_width_gray(md, family_vec, 16, 4, 0x4E2D);
    bool vec4_ok = (vec4_full >= EM);
    for (uint32_t i = 0; i < sizeof(probe) / sizeof(probe[0]); ++i) {
        uint16_t w = _fam_char_width_gray(md, family_vec, 16, 4, probe[i].cp);
        printf("[MD_TC]   vec4 width U+%04X %-12s = %u\n", probe[i].cp, probe[i].name, w);
        if (probe[i].cp != 'A' && w < EM) {
            vec4_ok = false;
        }
    }
    snprintf(detail, sizeof(detail), "vec4 em=%u, all full-width >= em", vec4_full);
    _tc_report("M3 vec4 CJK advance is full width", vec4_ok, detail);

    /** Control: 4bpp dot-matrix font. Grayscale alone must not narrow CJK;
     *  only the vector gray>1 ink-bounds path used to be affected. */
    uint16_t dot4_full = _fam_char_width_gray(md, family_gray_16, 16, 4, 0x4E2D);
    uint16_t dot4_punct = _fam_char_width_gray(md, family_gray_16, 16, 4, 0x3001);
    bool dot4_ok = (dot4_full >= EM) && (dot4_punct >= EM);
    snprintf(detail, sizeof(detail), "dot4 em=16, 中=%u 、=%u", dot4_full, dot4_punct);
    _tc_report("M3 4bpp dot CJK advance is full width", dot4_ok, detail);

    /* ---------- M-7: nested list state pollution ---------- */
    {
        gt_obj_st * md7 = gt_markdown_create(screen);
        gt_obj_set_pos(md7, 0, 490);
        gt_obj_set_size(md7, GT_SCREEN_WIDTH, 150);

        /* 7a: OL nest OL — parent items after the nesting must continue
         *     the parent counter (2., 3.), not the child's (4., 5.). */
        gt_markdown_set_text(md7, _md_m7_ol_ol, (uint32_t)strlen(_md_m7_ol_ol));
        bool b2 = _child_marker_followed_by(md7, " 2. ", "乙项");
        _tc_report("M7 OL/OL parent counter restored", b2,
            b2 ? "parent item 2. after nested list" : "parent numbering polluted by child list");
        bool b3 = _child_marker_followed_by(md7, " 3. ", "丙项");
        _tc_report("M7 OL/OL parent counter +1", b3,
            b3 ? "parent item 3. after nested list" : "parent numbering polluted by child list");
        bool bind = _child_marker_followed_by(md7, "   2. ", "子二");
        _tc_report("M7 nested child marker indented", bind,
            bind ? "child marker carries 2-space level indent" : "child marker not indented");

        /* 7b: OL nest UL — parent must restore the ordered marker type too */
        gt_markdown_set_text(md7, _md_m7_ol_ul, (uint32_t)strlen(_md_m7_ol_ul));
        bool t2 = _child_marker_followed_by(md7, " 2. ", "乙项");
        _tc_report("M7 OL/UL parent type restored", t2,
            t2 ? "ordered marker kept after UL nesting" : "marker type polluted by child UL");

        /* 7c: UL nest UL — parent stays unordered (bullet, no numbers) */
        gt_markdown_set_text(md7, _md_m7_ul_ul, (uint32_t)strlen(_md_m7_ul_ul));
        bool ul_ok = _child_marker_followed_by(md7, " · ", "乙项");
        bool no_num = !_child_marker_followed_by(md7, " 3. ", "乙项");
        _tc_report("M7 UL/UL parent bullet kept", ul_ok && no_num,
            ul_ok ? "bullet marker kept after nesting" : "unordered parent got numbered");
    }

    printf("[MD_TC] ===== %d/%d passed =====\n", _pass, _total);
    fflush(stdout);

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
