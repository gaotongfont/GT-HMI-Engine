/**
 * @file gt_example_font_selftest.c
 * @author Feyoung
 * @brief Font family/archive availability matrix self-test (task P1-7)
 * @version 0.2
 * @date 2026-10-15
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 *
 * Two probe levels:
 *  1) driver-level (archive truth): call GT_GetChinese() directly with a
 *     GT_FAMILY_* option id. Charset is UTF8, so the driver expects a
 *     UNICODE code point and converts internally (UnicodeToGBK).
 *  2) engine-level (real render path): gt_font_get_dot() with
 *     info.family = family id (family_dot_16 / family_gray_* / family_vec).
 *     The family table maps (family, language) -> GT_FAMILY_* option.
 *     NOTE: passing a raw option id as info.family is WRONG (it is used as
 *     a table index); direct archive addressing via GT_FONT_FAMILY_ONE_TYPE
 *     is deliberately NOT used here, vector archives go through family_vec.
 *
 * Console output: [FONT_TC] lines.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"

#if GT_USE_EXAMPLES

#include "../../src/font/gt_font.h"
#include "../../driver/gt_gui_driver.h"     /* GT_FAMILY_* option ids, GT_GetChinese */
#include "../../driver/gt_font_config.h"    /* family_* ids */
#include "../../src/widgets/gt_label.h"
#include <stdio.h>
#include <string.h>

/* private define -------------------------------------------------------*/

/* Unicode code point of the CJK probe character (charset is UTF8: the
 * driver converts Unicode -> GBK internally, so pass Unicode here) */
#define _TC_PROBE_CN        (0x4E2Du)   /* '中' U+4E2D */
#define _TC_PROBE_ASCII     (0x0041u)   /* 'A'  */

/* original baked base address of GT_FAMILY_GBK_16_ST in _gt_font_addr_list */
#define _TC_GBK16_ST_ORIG_ADDR  (0x00397568UL)

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/
static int _pass = 0, _total = 0;
static uint8_t _res[4096];   /* large: vec/gray fetch may write dot data
                              * beyond the reported len (width tables etc.) */

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/
static void _tc_report(const char * name, bool ok, const char * detail)
{
    ++_total;
    if (ok) { ++_pass; }
    printf("[FONT_TC] %-44s %s | %s\n", name, ok ? "PASS" : "FAIL", detail);
    fflush(stdout);
}

/**
 * @brief Probe the ENGINE path: resolve (family, language) via the family
 *        table and fetch the glyph through gt_font_get_dot().
 * @return true if len > 0 and glyph data contains ink
 */
static bool _probe_family(int16_t family, uint8_t gray, uint16_t size, uint32_t uni,
                          uint32_t * out_len)
{
    gt_font_st f = {0};
    f.utf8 = (char *)"";
    f.res = _res;
    f.len = 0;
    f.info.family = (uint16_t)family;
    f.info.size = size;
    f.info.gray = gray;
    f.info.encoding = GT_ENCODING_UTF8;
    memset(_res, 0, sizeof(_res));

    _gt_font_dot_ret_st r = gt_font_get_dot(&f, uni);
    if (out_len) { *out_len = r.len; }
    if (0 == r.len) { return false; }
    for (uint32_t i = 0; i < r.len && i < sizeof(_res); ++i) {
        if (_res[i]) { return true; }
    }
    return false;
}

/** engine-level family case: name / family / gray / pt */
static void _tc_family(const char * name, int16_t family, uint8_t gray, uint16_t size)
{
    char buf[96];
    uint32_t len = 0;
    bool ok = _probe_family(family, gray, size, _TC_PROBE_CN, &len);
    uint16_t opt = gt_font_family_get_option((uint16_t)family, FONT_LAN_CN, 0);
    snprintf(buf, sizeof(buf), "fam=%d->opt=%u gray=%u %upt '中' len=%lu",
        family, opt, gray, size, (unsigned long)len);
    _tc_report(name, ok, buf);
}

/** driver-level archive case: direct GT_GetChinese(option) with Unicode */
static void _tc_archive(const char * name, uint16_t option)
{
    char buf[96];
    uint8_t local[512] = {0};
    int len = GT_GetChinese(_TC_PROBE_CN, option, local);
    bool ink = false;
    for (int i = 0; i < len && i < (int)sizeof(local); ++i) {
        if (local[i]) { ink = true; break; }
    }
    snprintf(buf, sizeof(buf), "opt=%u uni->len=%d", option, len);
    _tc_report(name, ink, buf);
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief font family/archive availability self-test page
 */
gt_obj_st * gt_examples_font_selftest(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);
    gt_screen_set_bgcolor(screen, gt_color_hex(0xffffff));

    gt_obj_st * title = gt_label_create(screen);
    gt_label_set_text(title, "font family self-test");
    gt_obj_set_pos(title, 10, 5);
    gt_obj_set_size(title, 400, 30);

    /* ---------- engine level: family table -> dot matrix 1bit -------- */
    _tc_family("TC1 family_dot_16 CN",    family_dot_16,  1, 16);
    _tc_family("TC2 family_dot_24 CN",    family_dot_24,  1, 24);
    _tc_family("TC3 family_dot_32 CN",    family_dot_32,  1, 32);

    /* ---------- engine level: family table -> gray 4bit -------------- */
    _tc_family("TC4 family_gray_16 CN",   family_gray_16, 4, 16);
    _tc_family("TC5 family_gray_20 CN",   family_gray_20, 4, 20);
    _tc_family("TC6 family_gray_24 CN",   family_gray_24, 4, 24);
    _tc_family("TC7 family_gray_32 CN",   family_gray_32, 4, 32);

    /* ---------- engine level: family table -> vector ------------------ */
    {
        uint32_t len16 = 0, len40 = 0;
        bool ok16 = _probe_family(family_vec, 4, 16, _TC_PROBE_CN, &len16);
        {
            char buf[64];
            snprintf(buf, sizeof(buf), "16pt len=%lu", (unsigned long)len16);
            _tc_report("TC8 family_vec CN ink @16pt", ok16, buf);
        }
        bool ok40 = _probe_family(family_vec, 4, 40, _TC_PROBE_CN, &len40);
        (void)ok40;
        {
            char buf[64];
            bool diff = (len16 != len40);
            snprintf(buf, sizeof(buf), "len16=%lu len40=%lu shall differ",
                (unsigned long)len16, (unsigned long)len40);
            _tc_report("TC9 family_vec scales with size", diff, buf);
        }
        {
            uint32_t len = 0;
            char buf[64];
            bool ok = _probe_family(family_vec, 4, 16, _TC_PROBE_ASCII, &len);
            snprintf(buf, sizeof(buf), "'A' len=%lu", (unsigned long)len);
            _tc_report("TC10 family_vec ascii ink", ok, buf);
        }
    }

    /* ---------- driver level: archive truth (Unicode in) -------------- */
    _tc_archive("TC11 drv GB2312_16_ST",      GT_FAMILY_GB2312_16_ST);
    _tc_archive("TC12 drv GBK_32_ST",         GT_FAMILY_GBK_32_ST);
    _tc_archive("TC13 drv GB18030_24_ST",     GT_FAMILY_GB18030_24_ST);
    _tc_archive("TC14 drv GBK_16_ST_4bit",    GT_FAMILY_GBK_16_ST_4bit);
    _tc_archive("TC15 drv GBK_32_ST_4bit",    GT_FAMILY_GBK_32_ST_4bit);

    /* ---------- gt_set_font_base_addr injection honored --------------- */
    {
        char buf[96];
        uint8_t local[512] = {0};

        /* sanity: fetch works with the baked address */
        int len = GT_GetChinese(_TC_PROBE_CN, GT_FAMILY_GBK_16_ST, local);
        _tc_report("TC16 drv GBK_16_ST @baked addr", len > 0,
            len > 0 ? "baseline" : "baseline broken");

        /* inject an out-of-range address: fetch must fail */
        gt_set_font_base_addr(GT_FAMILY_GBK_16_ST, 0xE0000000UL);
        memset(local, 0, sizeof(local));
        len = GT_GetChinese(_TC_PROBE_CN, GT_FAMILY_GBK_16_ST, local);
        bool ink = false;
        for (int i = 0; i < len && i < (int)sizeof(local); ++i) {
            if (local[i]) { ink = true; break; }
        }
        snprintf(buf, sizeof(buf), "len=%d, shall be 0/blank", len);
        _tc_report("TC17 injected bad addr breaks fetch", !ink, buf);

        /* restore: fetch works again */
        gt_set_font_base_addr(GT_FAMILY_GBK_16_ST, _TC_GBK16_ST_ORIG_ADDR);
        memset(local, 0, sizeof(local));
        len = GT_GetChinese(_TC_PROBE_CN, GT_FAMILY_GBK_16_ST, local);
        _tc_report("TC18 restored addr works again", len > 0,
            len > 0 ? "injection honored" : "fetch still broken");
    }

    printf("[FONT_TC] ===== %d/%d passed =====\n", _pass, _total);
    fflush(stdout);

    gt_obj_st * result = gt_label_create(screen);
    char rtxt[48];
    snprintf(rtxt, sizeof(rtxt), "self-test: %d/%d passed", _pass, _total);
    gt_label_set_text(result, rtxt);
    gt_obj_set_pos(result, 420, 5);
    gt_obj_set_size(result, 300, 30);

    /* rendered samples for eyeballing, addressed by family id */
    const struct { int16_t fam; uint8_t gray; uint16_t pt; const char * txt; } vis[] = {
        { family_dot_16,   1, 16, "dot16 ABC"    },
        { family_gray_24,  4, 24, "gray24 ABC"   },
        { family_vec,      4, 16, "vec16 ABC"    },
        { family_vec,      4, 40, "vec40 ABC"    },
    };
    for (unsigned i = 0; i < sizeof(vis) / sizeof(vis[0]); ++i) {
        gt_obj_st * lb = gt_label_create(screen);
        gt_label_set_font_family(lb, (uint16_t)vis[i].fam);
        gt_label_set_font_size(lb, vis[i].pt);
        gt_label_set_font_gray(lb, vis[i].gray);
        gt_label_set_font_color(lb, gt_color_black());
        gt_label_set_text(lb, vis[i].txt);
        gt_obj_set_pos(lb, 30, 60 + (gt_size_t)i * 50);
        gt_obj_set_size(lb, 500, 46);
    }

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
