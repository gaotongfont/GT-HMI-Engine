/**
 * @file gt_example_textarea_file.c
 * @author Feyoung
 * @brief Textarea file-mode demo with on-the-fly test data generation
 * @version 0.2
 * @date 2026-07-15
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_textarea.h"
#include "../../src/widgets/gt_label.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES
#if GT_TEXTAREA_FILE_MODE

#define DEMO_FILE_PATH      "gt_textarea_demo_data.txt"
#define DEMO_LINE_COUNT     3000
#define DEMO_SECTION_LINES  500
#define INFO_BUF_SIZE       128

/* private typedef ------------------------------------------------------*/

typedef struct {
    gt_obj_st *textarea;
    gt_obj_st *info_label;
    bool       word_wrap;
} _demo_ctx_t;

/* static variables -----------------------------------------------------*/
static _demo_ctx_t _ctx;

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/

/* --- custom I/O callbacks (wrap standard C file I/O) --- */

static void * _demo_fs_open(const char *path, const char *mode)
{
    return (void *)fopen(path, mode);
}

static int _demo_fs_read(void *handle, void *buf, unsigned int size)
{
    return (int)fread(buf, 1, size, (FILE *)handle);
}

static int _demo_fs_write(void *handle, const void *buf, unsigned int size)
{
    return (int)fwrite(buf, 1, size, (FILE *)handle);
}

static long _demo_fs_seek(void *handle, long offset, int whence)
{
    return fseek((FILE *)handle, offset, whence) == 0 ? 0 : -1;
}

static long _demo_fs_tell(void *handle)
{
    return ftell((FILE *)handle);
}

static int _demo_fs_close(void *handle)
{
    return fclose((FILE *)handle);
}

static const gt_textarea_file_io_t _demo_io = {
    .io_open  = _demo_fs_open,
    .io_read  = _demo_fs_read,
    .io_write = _demo_fs_write,
    .io_seek  = _demo_fs_seek,
    .io_tell  = _demo_fs_tell,
    .io_close = _demo_fs_close,
};

/* --- test data generation --- */

/**
 * @brief Generate a multi-MB test text file with mixed content.
 *
 * Produces ~3000 lines (200-300 KB) of UTF-8 text with:
 *  - Line numbers
 *  - English prose
 *  - Chinese characters
 *  - Section headers every DEMO_SECTION_LINES lines
 *  - Simulated log entries, sensor readings, and diagnostic messages
 */
static void _generate_test_file(void)
{
    FILE *fp = fopen(DEMO_FILE_PATH, "w");
    if (NULL == fp) return;

    fprintf(fp, "=== GT-HMI-Engine Textarea File Mode Demo ===\n");
    fprintf(fp, "This file is auto-generated for demonstration purposes.\n");
    fprintf(fp, "Total lines: %d | Encoding: UTF-8\n", DEMO_LINE_COUNT);
    fprintf(fp, "Scroll down to browse through the content.\n");
    fprintf(fp, "==============================================\n\n");

    const char *english_texts[] = {
        "The quick brown fox jumps over the lazy dog.",
        "Pack my box with five dozen liquor jugs.",
        "How vexingly quick daft zebras jump!",
        "The five boxing wizards jump quickly.",
        "Sphinx of black quartz, judge my vow.",
        "Waltz, bad nymph, for quick jigs vex.",
        "Glib jocks quiz nymph to vex dwarf.",
        "Jackdaws love my big sphinx of quartz.",
        "Pack my red box with five dozen quality jugs.",
        "The jay, pig, fox, zebra, and my wolves quack!",
    };

    const char *chinese_texts[] = {
        "\344\272\272\345\267\245\346\231\272\350\203\275\346\255\243\345\234\250\346\224\271\345\217\230\344\270\226\347\225\214",
        "\345\265\214\345\205\245\345\274\217\347\263\273\347\273\237\350\256\276\350\256\241\344\270\216\345\274\200\345\217\221",
        "\345\256\236\346\227\266\346\225\260\346\215\256\351\207\207\351\233\206\344\270\216\345\210\206\346\236\220",
        "\344\274\240\346\204\237\345\231\250\347\275\221\347\273\234\347\233\221\346\265\213\347\263\273\347\273\237",
        "\347\224\250\346\210\267\347\225\214\351\235\242\344\272\244\344\272\222\350\256\276\350\256\241",
        "\345\244\232\345\252\222\344\275\223\346\230\276\347\244\272\344\270\216\346\216\247\345\210\266",
        "\347\275\221\347\273\234\351\200\232\344\277\241\345\215\217\350\256\256\346\240\210",
        "\345\255\230\345\202\250\346\226\207\344\273\266\347\263\273\347\273\237\347\256\241\347\220\206",
        "\350\256\276\345\244\207\351\251\261\345\212\250\345\261\202\344\274\230\345\214\226",
        "\345\233\276\345\275\242\346\270\262\346\237\223\345\274\225\346\223\216\346\200\247\350\203\275\350\260\203\344\274\230",
    };

    const char *log_levels[] = { "INFO", "WARN", "ERROR", "DEBUG", "TRACE" };
    const char *modules[]    = { "SENSOR", "NET", "FS", "GUI", "PWR", "CTRL" };

    for (int i = 1; i <= DEMO_LINE_COUNT; i++) {
        /* Section header every DEMO_SECTION_LINES lines */
        if ((i % DEMO_SECTION_LINES) == 1) {
            int sec = i / DEMO_SECTION_LINES + 1;
            fprintf(fp, "\n==========================================\n");
            fprintf(fp, "  Section %d -- Lines %d to %d\n", sec, i, i + DEMO_SECTION_LINES - 1);
            fprintf(fp, "==========================================\n\n");
        }

        /* Vary line format by section */
        int sec = (i - 1) / DEMO_SECTION_LINES;
        switch (sec % 6) {
        case 0: /* English prose + Chinese */
            fprintf(fp, "[%04d] %s | %s\n",
                    i,
                    english_texts[i % 10],
                    chinese_texts[(i * 3) % 10]);
            break;
        case 1: /* Simulated log */
            fprintf(fp, "[%04d] %-5s [%s] %s -- %s\n",
                    i,
                    log_levels[i % 5],
                    modules[i % 6],
                    english_texts[(i * 7) % 10],
                    chinese_texts[(i * 2) % 10]);
            break;
        case 2: /* Sensor readings */
            fprintf(fp, "[%04d] Temp: %5.1f C | Hum: %3d %% | Press: %4d hPa | Bat: %2d %% | Signal: %s\n",
                    i,
                    25.0 + (i % 150) * 0.1,
                    40 + (i % 41),
                    990 + (i % 40),
                    50 + (i % 51),
                    (i % 3 == 0) ? "Strong" : (i % 3 == 1) ? "Medium" : "Weak");
            break;
        case 3: /* Network diagnostics */
            fprintf(fp, "[%04d] %-5s [%-6s] TX: %4u bytes | RX: %4u bytes | RTT: %3d ms | Status: %s | %s\n",
                    i,
                    log_levels[(i * 3) % 5],
                    modules[(i * 2) % 6],
                    100 + (unsigned int)(i * 37 % 900),
                    200 + (unsigned int)(i * 53 % 800),
                    1 + (i % 200),
                    (i % 5 == 0) ? "TIMEOUT" : "OK",
                    chinese_texts[i % 10]);
            break;
        case 4: /* System events */
            fprintf(fp, "[%04d] [%s] Event %04d: task_%s %s at tick=%u | %s\n",
                    i,
                    log_levels[i % 5],
                    i * 7,
                    modules[i % 6],
                    (i % 3 == 0) ? "started" : (i % 3 == 1) ? "completed" : "suspended",
                    10000 + (unsigned int)(i * 317),
                    chinese_texts[(i * 4) % 10]);
            break;
        case 5: /* Data table rows */
            fprintf(fp, "[%04d] | %-8s | %4d | %6.2f | %4d.%03d | %-10s | %s\n",
                    i,
                    modules[i % 6],
                    i * 10,
                    (i % 100) * 1.5,
                    i % 256, i % 1000,
                    log_levels[i % 5],
                    chinese_texts[(i * 5) % 10]);
            break;
        }
    }

    /* File footer */
    fprintf(fp, "\n==========================================\n");
    fprintf(fp, "  End of demo file -- %d lines generated\n", DEMO_LINE_COUNT);
    fprintf(fp, "  Encoding: UTF-8 | Chunk buffer: 4KB\n");
    fprintf(fp, "  Scroll back to review content.\n");
    fprintf(fp, "==========================================\n");

    fclose(fp);
}

/* --- info label update --- */

static void _update_info_label(void)
{
    char buf[INFO_BUF_SIZE];
    gt_textarea_mode_t mode = gt_textarea_get_mode(_ctx.textarea);

    if (mode == GT_TEXTAREA_MODE_MEMORY) {
        gt_label_set_text(_ctx.info_label, "Mode: Memory (file not loaded)");
        return;
    }

    float pct = gt_textarea_get_scroll_percent(_ctx.textarea);
    uint32_t total_h = gt_textarea_get_total_height(_ctx.textarea);
    uint32_t offset  = gt_textarea_get_display_offset(_ctx.textarea);

    snprintf(buf, sizeof(buf),
             "Mode: File I/O | Wrap: %s | Scroll: %.1f%% | Pos: %u / %u px",
             _ctx.word_wrap ? "ON" : "OFF",
             pct, offset, total_h);
    gt_label_set_text(_ctx.info_label, buf);
}

/* --- button callbacks --- */

static void _btn_toggle_wrap_cb(gt_event_st *e)
{
    (void)e;
    _ctx.word_wrap = !_ctx.word_wrap;
    gt_textarea_set_word_wrap(_ctx.textarea, _ctx.word_wrap);
    _update_info_label();
    gt_event_send(_ctx.textarea, GT_EVENT_TYPE_DRAW_START, NULL);
}

static void _btn_jump_top_cb(gt_event_st *e)
{
    (void)e;
    gt_textarea_display_by_precent(_ctx.textarea, 0.0f);
    _update_info_label();
}

static void _btn_jump_bottom_cb(gt_event_st *e)
{
    (void)e;
    gt_textarea_display_by_precent(_ctx.textarea, 100.0f);
    _update_info_label();
}

/* --- scroll end callback (update info on each scroll stop) --- */

static void _on_scroll_end_cb(gt_event_st *e)
{
    (void)e;
    _update_info_label();
}
#endif  //GT_TEXTAREA_FILE_MODE

/* global functions / API interface -------------------------------------*/

/**
 * @brief File-mode textarea demo with auto-generated test data.
 *
 * Generates a ~3000-line test file (~200-300 KB UTF-8 text),
 * opens it via custom I/O callbacks (standard C file ops),
 * and displays it in a file-mode textarea with 4KB chunk buffer.
 *
 * Interactive controls:
 *  - [Toggle Wrap]  Switch word-wrap on/off
 *  - [Top]          Jump to file beginning
 *  - [Bottom]       Jump to file end
 *  - Info bar       Shows file size, lines, encoding, wrap state, scroll %
 */
gt_obj_st * gt_examples_textarea_file(void)
{
    gt_obj_st *screen = gt_obj_create(NULL);

#if GT_TEXTAREA_FILE_MODE
    /* --- generate test data file --- */
    _generate_test_file();

    /* --- title label --- */
    gt_obj_st *title = gt_label_create(screen);
    gt_obj_set_pos(title, 10, 5);
    gt_obj_set_size(title, 500, 25);
    gt_label_set_text(title, "Textarea File Mode Demo -- Large Text File Viewer");

    /* --- file-mode textarea --- */
    gt_obj_st *ta = gt_textarea_create(screen);
    gt_obj_set_pos(ta, 10, 35);
    gt_obj_set_size(ta, 780, 365);
    gt_textarea_set_font_size(ta, 16);
    gt_textarea_set_font_color(ta, gt_color_black());
    gt_textarea_set_bg_color(ta, gt_color_hex(0xF8F8F8));
    gt_textarea_set_border_width(ta, 1);
    gt_textarea_set_border_color(ta, gt_color_hex(0xAAAAAA));
    gt_textarea_set_radius(ta, 4);
    gt_textarea_set_space(ta, 0, 3);
    gt_textarea_set_padding(ta, 4);

    /* Open file via custom I/O callbacks (standard C fopen/fread/...) */
    gt_textarea_set_file_buf_size(ta, 4096);
    int ret = gt_textarea_set_file_with_io(ta, DEMO_FILE_PATH, &_demo_io);
    if (0 != ret) {
        /* fallback: show error in memory mode */
        gt_textarea_set_text(ta, "ERROR: Failed to open demo file.\n"
                                  "Check that %s exists in the working directory.",
                                  DEMO_FILE_PATH);
    }

    /* listen for scroll-end to update info */
    gt_obj_add_event_cb(ta, _on_scroll_end_cb, GT_EVENT_TYPE_INPUT_SCROLL_END, NULL);

    /* --- info label --- */
    _ctx.info_label = gt_label_create(screen);
    gt_obj_set_pos(_ctx.info_label, 10, 408);
    gt_obj_set_size(_ctx.info_label, 780, 22);
    gt_label_set_text(_ctx.info_label, "Initializing...");

    /* --- control buttons --- */
    gt_obj_st *btn_wrap = gt_btn_create(screen);
    gt_obj_set_pos(btn_wrap, 10, 440);
    gt_obj_set_size(btn_wrap, 150, 30);
    gt_btn_set_text(btn_wrap, "Toggle Wrap");
    gt_obj_add_event_cb(btn_wrap, _btn_toggle_wrap_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    gt_obj_st *btn_top = gt_btn_create(screen);
    gt_obj_set_pos(btn_top, 170, 440);
    gt_obj_set_size(btn_top, 100, 30);
    gt_btn_set_text(btn_top, "Top");
    gt_obj_add_event_cb(btn_top, _btn_jump_top_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    gt_obj_st *btn_bottom = gt_btn_create(screen);
    gt_obj_set_pos(btn_bottom, 280, 440);
    gt_obj_set_size(btn_bottom, 100, 30);
    gt_btn_set_text(btn_bottom, "Bottom");
    gt_obj_add_event_cb(btn_bottom, _btn_jump_bottom_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    /* --- init context --- */
    _ctx.textarea  = ta;
    _ctx.word_wrap = true;
    _update_info_label();
#endif //GT_TEXTAREA_FILE_MODE

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif  /* GT_USE_EXAMPLES */
/* end ------------------------------------------------------------------*/
