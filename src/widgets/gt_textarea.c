/**
 * @file gt_textarea.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-07-11 15:03:35
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_textarea.h"

#if GT_CFG_ENABLE_TEXTAREA
#include "../core/gt_mem.h"
#include "../core/gt_indev.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../others/gt_assert.h"
#include "../core/gt_disp.h"

#if GT_TEXTAREA_FILE_MODE
#include "../core/gt_fs.h"
#endif

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_TEXTAREA
#define MY_CLASS    &gt_textarea_class

#define _FONT_AREA_REDUCE_SIZE      2
#define _FILE_PAGE_CACHE_COUNT      8

/* private typedef ------------------------------------------------------*/

typedef struct _textarea_reg_s {
    uint8_t hold_on : 1;                    /** finger hold on content for scroll */
    uint8_t click_valid : 1;                /** click status: scroll will set to false, only click event valid */
    uint8_t click_single_cn : 1;            /** click single chinese character */
    uint8_t pull_to_refresh : 1;            /** pull to refresh */
    uint8_t load_more : 1;                  /** load more */
    uint8_t always_scroll_to_newest : 1;    /** always scroll to newest string */
    uint8_t reserved : 2;
}_textarea_reg_st;

#if GT_TEXTAREA_FILE_MODE
typedef struct {
    uint32_t start;
    uint32_t end;
    uint32_t page_no;
    uint16_t line_count;
    uint8_t  start_first_line;
    uint8_t  end_first_line;
} _gt_textarea_page_item_t;

/** @brief File-mode runtime context */
typedef struct {
    uint8_t      io_backend;             /**< 0=VFS, 1=custom I/O */
    union {
        gt_fs_fp_st *fp;
        struct {
            gt_textarea_file_io_t io;
            void *handle;
        } custom;
    } io;
    char          file_path[128];

    uint8_t      *read_buf;
    uint16_t      buf_size;
    uint16_t      read_len;
    uint32_t      buf_file_offset;

    uint32_t      scroll_byte_offset;
    uint32_t      file_total_bytes;
    uint32_t      page_end_offset;
    uint32_t      current_page;
    uint32_t      layout_key;

    _gt_textarea_page_item_t page_cache[_FILE_PAGE_CACHE_COUNT];
    _gt_textarea_page_item_t page_index[GT_TEXTAREA_FILE_IDX_COUNT];
    uint8_t      page_cache_count;
    uint8_t      page_cache_write;
    uint8_t      page_index_count;
    uint8_t      page_index_write;
    uint8_t      current_first_line;
    uint8_t      page_end_first_line;
    uint16_t     current_line_count;

    gt_textarea_enc_t encoding;

    uint32_t     estimated_total_lines;
    int32_t      last_scroll_y;           /**< scroll.y from previous handle_scroll, for delta computation */
    int16_t      last_touch_y;
    int32_t      pixel_offset;            /**< sub-line pixel remainder for smooth scrolling */
    int32_t      gesture_px;

    uint8_t      dirty : 1;
    uint8_t      editable : 1;
    uint8_t      word_wrap : 1;
    uint8_t      touch_active : 1;
    uint8_t      _reserved : 4;

    uint8_t      pending_overlap;
} _gt_textarea_file_ctx_t;
#endif  /* GT_TEXTAREA_FILE_MODE */

#if GT_TEXTAREA_SUB_STRING_SCROLL_CACHE
typedef struct {
    gt_size_t idx;
    gt_area_st offset_abs;
}_scroll_cache_item_st;

typedef struct {
    _scroll_cache_item_st * list;
    uint16_t item_count;
}_sub_string_scroll_cache_st;
#endif

#if GT_TEXTAREA_LINE_CACHE
typedef struct {
    uint16_t byte_offset;   /** byte offset from contents[seg_idx].text start */
    uint16_t seg_idx;       /** which contents[] segment this line belongs to */
    gt_size_t y;            /** absolute y of this line (logical_area.y based) */
}_line_cache_item_st;

typedef struct {
    _line_cache_item_st * list;
    uint16_t count;
    uint16_t capacity;
}_line_cache_st;
#endif  /** GT_TEXTAREA_LINE_CACHE */

typedef struct _gt_textarea_s {
    gt_obj_st base;
    gt_textarea_param_st * contents;
    uint16_t cnt_contents;
    uint32_t max_height;            /** update when current display max height */
    gt_anim_st * scroll_to_newest_anim;     /** when add newest string */

    gt_color_t color_border;
    gt_color_t color_font;
    uint8_t border_width;

    gt_font_info_st font_info;

    uint16_t indent;
    uint8_t bg_opa;     /* @ref gt_color.h */
    uint8_t space_x;
    uint8_t space_y;
    uint8_t font_align;     //@ref gt_align_et
    uint8_t padding;

    uint8_t start_x_off;
    uint8_t start_y_off;

#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
    /** detail @ref gt_font.h */
    gt_font_touch_word_st select_word;
#endif

#if GT_TEXTAREA_SUB_STRING_SCROLL_CACHE
    _sub_string_scroll_cache_st scroll_cache;
#endif

#if GT_TEXTAREA_LINE_CACHE
    _line_cache_st line_cache;
#endif

    _textarea_reg_st reg;

#if GT_TEXTAREA_FILE_MODE
    gt_textarea_mode_t       mode;
    _gt_textarea_file_ctx_t *file_ctx;
    uint16_t                 file_buf_size_cfg;
    bool                     file_word_wrap;
    bool                     file_editable;
#endif
}_gt_textarea_st;


/* static variables -----------------------------------------------------*/
static void _textarea_init_cb(gt_obj_st * obj);
static void _textarea_deinit_cb(gt_obj_st * obj);
static void _textarea_event_cb(struct gt_obj_s * obj, gt_event_st * e);
GT_ATTRIBUTE_RAM_TEXT static void _create_auto_scoll_to(gt_obj_st * textarea);

#if GT_TEXTAREA_FILE_MODE
static void _draw_content_file(gt_obj_st *obj, gt_point_st *touch_point_p, bool is_only_calc_height);
static void _textarea_file_handle_scroll(gt_obj_st *obj);
#endif

static GT_ATTRIBUTE_RAM_DATA const gt_obj_class_st gt_textarea_class = {
    ._init_cb      = _textarea_init_cb,
    ._deinit_cb    = _textarea_deinit_cb,
    ._event_cb     = _textarea_event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_textarea_st)
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
static inline void _set_touch_word(_gt_textarea_st * style, char * word, uint16_t len) {
    style->select_word.word_p = word;
    style->select_word.len = len;
}
#endif

static inline bool _is_hold_on_contents_for_scroll(_gt_textarea_st * style);

#if GT_TEXTAREA_FILE_MODE

#define _BUF_RELOAD_THRESHOLD(ctx)  ((ctx)->buf_size >> 3)
#define _ENC_DETECT_SAMPLE_SIZE 256
#define _AVG_BYTES_PER_LINE    80

#define _FILE_PAGE_INDEX_STEP  8

/* file-mode I/O dispatchers ---------------------------------------------*/
static inline int _file_read(_gt_textarea_file_ctx_t *ctx, void *buf, uint32_t size) {
    if (ctx->io_backend == 0) {
        uint32_t ret_len = 0;
        return (gt_fs_read(ctx->io.fp, (uint8_t *)buf, size, &ret_len) == GT_FS_RES_OK)
               ? (int)ret_len : -1;
    } else {
        if (NULL == ctx->io.custom.io.io_read) return -1;
        return ctx->io.custom.io.io_read(ctx->io.custom.handle, buf, (unsigned int)size);
    }
}

static inline int _file_seek(_gt_textarea_file_ctx_t *ctx, uint32_t offset, int whence) {
    if (ctx->io_backend == 0) {
        return (gt_fs_seek(ctx->io.fp, offset, (gt_fs_whence_et)whence) == GT_FS_RES_OK)
               ? 0 : -1;
    } else {
        if (NULL == ctx->io.custom.io.io_seek) return -1;
        return (ctx->io.custom.io.io_seek(ctx->io.custom.handle, (long)offset, whence) >= 0)
               ? 0 : -1;
    }
}

static inline int _file_write(_gt_textarea_file_ctx_t *ctx, const void *buf, uint32_t size) {
    if (ctx->io_backend == 0) {
        uint32_t ret_len = 0;
        return (gt_fs_write(ctx->io.fp, (uint8_t *)buf, size, &ret_len) == GT_FS_RES_OK)
               ? (int)ret_len : -1;
    } else {
        if (NULL == ctx->io.custom.io.io_write) return -1;
        return ctx->io.custom.io.io_write(ctx->io.custom.handle, buf, (unsigned int)size);
    }
}

static inline uint32_t _file_tell(_gt_textarea_file_ctx_t *ctx) {
    if (ctx->io_backend == 0) {
        uint32_t pos = 0;
        gt_fs_tell(ctx->io.fp, &pos);
        return pos;
    } else {
        if (ctx->io.custom.io.io_tell) {
            long pos = ctx->io.custom.io.io_tell(ctx->io.custom.handle);
            return (pos >= 0) ? (uint32_t)pos : 0;
        }
        if (ctx->io.custom.io.io_seek) {
            long pos = ctx->io.custom.io.io_seek(ctx->io.custom.handle, 0, 1);
            return (pos >= 0) ? (uint32_t)pos : 0;
        }
        return 0;
    }
}

static inline void _file_close(_gt_textarea_file_ctx_t *ctx) {
    if (ctx->io_backend == 0) {
        if (ctx->io.fp) { gt_fs_close(ctx->io.fp); ctx->io.fp = NULL; }
    } else {
        if (ctx->io.custom.io.io_close && ctx->io.custom.handle) {
            ctx->io.custom.io.io_close(ctx->io.custom.handle);
            ctx->io.custom.handle = NULL;
        }
    }
}

/* encoding detection ----------------------------------------------------*/
static inline bool _is_utf8_cont(uint8_t b) {
    return (b & 0xC0) == 0x80;
}

static inline uint8_t _utf8_seq_len(uint8_t lead) {
    if ((lead & 0x80) == 0x00) return 1;
    if ((lead & 0xE0) == 0xC0) return 2;
    if ((lead & 0xF0) == 0xE0) return 3;
    if ((lead & 0xF8) == 0xF0) return 4;
    return 0;
}

static gt_textarea_enc_t _detect_encoding(const uint8_t *data, uint32_t len) {
    if (NULL == data || len < 2) return GT_TEXTAREA_ENC_UTF8;

    /* BOM */
    if (len >= 3 && data[0] == 0xEF && data[1] == 0xBB && data[2] == 0xBF)
        return GT_TEXTAREA_ENC_UTF8;
    if (len >= 2 && data[0] == 0xFF && data[1] == 0xFE)
        return GT_TEXTAREA_ENC_UTF8;

    uint32_t sample = len > _ENC_DETECT_SAMPLE_SIZE ? _ENC_DETECT_SAMPLE_SIZE : len;
    uint32_t ascii_n = 0, utf8_n = 0;
    bool valid_utf8 = true;

    for (uint32_t i = 0; i < sample;) {
        uint8_t lead = data[i];
        if (lead < 0x80) {
            if (lead == 0) valid_utf8 = false;
            ascii_n++;
            i++;
            continue;
        }

        uint8_t seq_len = _utf8_seq_len(lead);
        if (seq_len < 2) {
            valid_utf8 = false;
            break;
        }
        if (i + seq_len > sample) break;
        for (uint8_t j = 1; j < seq_len; ++j) {
            if (!_is_utf8_cont(data[i + j])) {
                valid_utf8 = false;
                break;
            }
        }
        if (!valid_utf8) break;
        utf8_n++;
        i += seq_len;
    }

    if (valid_utf8 && utf8_n > 0) return GT_TEXTAREA_ENC_UTF8;
    if (ascii_n == sample) return GT_TEXTAREA_ENC_ASCII;

    uint32_t gbk_n = 0;
    for (uint32_t i = 0; i + 1 < sample;) {
        uint8_t lead = data[i];
        uint8_t trail = data[i + 1];
        if (lead < 0x80) {
            i++;
        } else if (lead >= 0x81 && lead <= 0xFE
                   && ((trail >= 0x40 && trail <= 0x7E)
                       || (trail >= 0x80 && trail <= 0xFE))) {
            gbk_n++;
            i += 2;
        } else {
            i++;
        }
    }
    if (gbk_n > 0) return GT_TEXTAREA_ENC_GBK;
    return GT_TEXTAREA_ENC_UTF8;
}

/* multi-byte boundary ---------------------------------------------------*/
static uint8_t _calc_overlap(_gt_textarea_file_ctx_t *ctx) {
    if (NULL == ctx->read_buf || ctx->read_len == 0) return 0;

    uint16_t check_start = (ctx->read_len > 4) ? (ctx->read_len - 4) : 0;

    switch (ctx->encoding) {
    case GT_TEXTAREA_ENC_UTF8:
        for (uint16_t i = ctx->read_len; i > check_start; i--) {
            if (i == 0) break;
            uint8_t b  = ctx->read_buf[i - 1];
            uint8_t sl = _utf8_seq_len(b);
            if (sl >= 2) {
                uint8_t present = (uint8_t)(ctx->read_len - (i - 1));
                return (present < sl) ? present : 0;
            }
            if (!_is_utf8_cont(b) && b < 0x80) return 0;
        }
        return 0;
    case GT_TEXTAREA_ENC_GBK: {
        uint8_t last = ctx->read_buf[ctx->read_len - 1];
        return (last >= 0x81 && last <= 0xFE) ? 1 : 0;
    }
    default:
        return 0;
    }
}

/* buffer management -----------------------------------------------------*/
static bool _ensure_buffer(_gt_textarea_file_ctx_t *ctx, uint32_t target) {
    if (NULL == ctx->read_buf) return false;
    uint32_t buf_end = ctx->buf_file_offset + ctx->read_len;
    uint32_t reload_at = (ctx->read_len > _BUF_RELOAD_THRESHOLD(ctx))
                       ? (buf_end - _BUF_RELOAD_THRESHOLD(ctx)) : buf_end;
    if (target >= ctx->buf_file_offset && target < reload_at)
        return true;

    uint32_t aligned = target & ~((uint32_t)ctx->buf_size - 1);
    if (aligned >= ctx->file_total_bytes && ctx->file_total_bytes > ctx->buf_size)
        aligned = ctx->file_total_bytes - ctx->buf_size;

    if (_file_seek(ctx, aligned, 0) != 0) return false;
    int rd = _file_read(ctx, ctx->read_buf, ctx->buf_size);
    if (rd <= 0) return false;
    ctx->buf_file_offset = aligned;
    ctx->read_len = (uint16_t)rd;
    ctx->pending_overlap = _calc_overlap(ctx);
    return true;
}

/* file context lifecycle ------------------------------------------------*/
static _gt_textarea_file_ctx_t *
_file_ctx_init(const char *path, uint16_t buf_size, const gt_textarea_file_io_t *io) {
    _gt_textarea_file_ctx_t *ctx = (_gt_textarea_file_ctx_t *)
        gt_mem_malloc(sizeof(_gt_textarea_file_ctx_t));
    if (NULL == ctx) return NULL;
    gt_memset(ctx, 0, sizeof(_gt_textarea_file_ctx_t));

    ctx->buf_size = buf_size;
    ctx->read_buf = (uint8_t *)gt_mem_malloc(buf_size);
    if (NULL == ctx->read_buf) { gt_mem_free(ctx); return NULL; }
    gt_memset(ctx->read_buf, 0, buf_size);

    uint16_t plen = (uint16_t)strlen(path);
    if (plen >= sizeof(ctx->file_path)) plen = sizeof(ctx->file_path) - 1;
    gt_memcpy(ctx->file_path, path, plen);
    ctx->file_path[plen] = '\0';

    if (io) {
        ctx->io_backend = 1;
        ctx->io.custom.io = *io;
        ctx->io.custom.handle = io->io_open(path, "r");
        if (NULL == ctx->io.custom.handle) goto fail;
    } else {
        ctx->io_backend = 0;
        ctx->io.fp = gt_fs_open(path, GT_FS_MODE_RD);
        if (NULL == ctx->io.fp) goto fail;
    }

    _file_seek(ctx, 0, 2);  /* SEEK_END */
    ctx->file_total_bytes = _file_tell(ctx);
    _file_seek(ctx, 0, 0);  /* SEEK_SET */

    /* initial read */
    {
        int rd = _file_read(ctx, ctx->read_buf, ctx->buf_size);
        if (rd <= 0 && ctx->file_total_bytes > 0) goto fail_close;
        ctx->read_len = (rd > 0) ? (uint16_t)rd : 0;
    }
    ctx->buf_file_offset = 0;
    ctx->scroll_byte_offset = 0;

    /* encoding */
    {
        uint32_t sl = ctx->read_len < _ENC_DETECT_SAMPLE_SIZE
                    ? ctx->read_len : _ENC_DETECT_SAMPLE_SIZE;
        ctx->encoding = _detect_encoding(ctx->read_buf, sl);
        ctx->pending_overlap = _calc_overlap(ctx);
    }

    /* initial line estimate */
    ctx->estimated_total_lines = ctx->file_total_bytes / _AVG_BYTES_PER_LINE;
    if (ctx->estimated_total_lines < 1) ctx->estimated_total_lines = 1;

    return ctx;

fail_close:
    _file_close(ctx);
fail:
    gt_mem_free(ctx->read_buf);
    gt_mem_free(ctx);
    return NULL;
}

static void _file_ctx_deinit(_gt_textarea_file_ctx_t *ctx) {
    if (NULL == ctx) return;
    _file_close(ctx);
    if (ctx->read_buf) { gt_mem_free(ctx->read_buf); ctx->read_buf = NULL; }
    gt_mem_free(ctx);
}

static uint32_t _file_layout_key(gt_obj_st *obj, _gt_textarea_st *ta) {
    uint32_t key = ((uint32_t)obj->area.w << 16) ^ (uint32_t)obj->area.h;
    const uint8_t *font_bytes = (const uint8_t *)&ta->font_info;
    for (uint32_t i = 0; i < sizeof(ta->font_info); ++i) {
        key = key * 33U + font_bytes[i];
    }
    key = key * 33U + ta->space_x;
    key = key * 33U + ta->space_y;
    key = key * 33U + ta->padding;
    key = key * 33U + ta->border_width;
    key = key * 33U + ta->indent;
    key = key * 33U + ta->font_align;
    key = key * 33U + ta->file_word_wrap;
    return key ? key : 1;
}

static void _file_reset_pages(_gt_textarea_file_ctx_t *ctx) {
    ctx->scroll_byte_offset = 0;
    ctx->page_end_offset = 0;
    ctx->current_page = 0;
    ctx->current_line_count = 0;
    ctx->current_first_line = 1;
    ctx->page_end_first_line = 1;
    ctx->page_cache_count = 0;
    ctx->page_cache_write = 0;
    ctx->page_index_count = 0;
    ctx->page_index_write = 0;
    ctx->gesture_px = 0;
    ctx->pixel_offset = 0;
}

static void _file_check_layout(gt_obj_st *obj, _gt_textarea_st *ta,
                               _gt_textarea_file_ctx_t *ctx) {
    uint32_t key = _file_layout_key(obj, ta);
    if (ctx->layout_key == key) return;
    ctx->layout_key = key;
    _file_reset_pages(ctx);
}

static void _file_set_font_encoding(gt_font_st *font,
                                    const _gt_textarea_file_ctx_t *ctx) {
    font->info.encoding = (GT_TEXTAREA_ENC_GBK == ctx->encoding)
                        ? GT_ENCODING_GB : GT_ENCODING_UTF8;
}

static uint16_t _file_line_height(const _gt_textarea_st *ta) {
    uint8_t style_space_y = 0;
    if (ta->font_info.style.reg.underline
        || ta->font_info.style.reg.strikethrough) {
        style_space_y = (ta->font_info.size >> 4)
                      ? (ta->font_info.size >> 4) : 1;
    }
    return ta->font_info.size + ta->space_y + (style_space_y << 1);
}

static bool _file_scan_window(_gt_textarea_file_ctx_t *ctx, uint32_t offset,
                              const uint8_t **data, uint32_t *len) {
    uint32_t safe_end;
    if (offset >= ctx->file_total_bytes) return false;
    if (!_ensure_buffer(ctx, offset)) return false;

    safe_end = ctx->buf_file_offset + ctx->read_len;
    if (safe_end < ctx->file_total_bytes && safe_end > ctx->pending_overlap) {
        safe_end -= ctx->pending_overlap;
    }
    if (offset >= safe_end
        || (safe_end - offset < _BUF_RELOAD_THRESHOLD(ctx)
            && safe_end < ctx->file_total_bytes)) {
        if (_file_seek(ctx, offset, 0) != 0) return false;
        {
            int rd = _file_read(ctx, ctx->read_buf, ctx->buf_size);
            if (rd <= 0) return false;
            ctx->buf_file_offset = offset;
            ctx->read_len = (uint16_t)rd;
            ctx->pending_overlap = _calc_overlap(ctx);
        }
        safe_end = ctx->buf_file_offset + ctx->read_len;
        if (safe_end < ctx->file_total_bytes && safe_end > ctx->pending_overlap) {
            safe_end -= ctx->pending_overlap;
        }
    }
    if (offset >= safe_end) return false;
    *data = ctx->read_buf + (offset - ctx->buf_file_offset);
    *len = safe_end - offset;
    return true;
}

static bool _file_scan_page(gt_obj_st *obj, _gt_textarea_st *ta,
                            uint32_t start, bool start_first_line,
                            uint32_t page_no, _gt_textarea_page_item_t *page) {
    _gt_textarea_file_ctx_t *ctx = ta->file_ctx;
    gt_area_st padding_font = gt_area_reduce(obj->area, ta->padding);
    gt_area_st area_font = gt_area_reduce(padding_font,
                                          ta->border_width + _FONT_AREA_REDUCE_SIZE);
    uint16_t line_height = _file_line_height(ta);
    uint32_t lines_per_page;
    uint32_t offset = start;
    bool first_line = start_first_line;
    gt_font_st font = { .info = ta->font_info, .res = NULL };

    if (NULL == page || line_height == 0 || area_font.w <= 0 || area_font.h <= 0) {
        return false;
    }
    _file_set_font_encoding(&font, ctx);
    lines_per_page = (uint32_t)area_font.h / line_height;
    if (lines_per_page == 0) lines_per_page = 1;

    uint16_t line_count = 0;
    for (uint32_t line = 0; line < lines_per_page && offset < ctx->file_total_bytes; ++line) {
        const uint8_t *data;
        uint32_t len;
        uint32_t width = first_line
                       ? gt_font_get_indent_width(&font.info, ta->indent) : 0;
        int32_t step;
        bool newline = false;

        if (!_file_scan_window(ctx, offset, &data, &len)) return false;
        font.utf8 = (char *)data;
        font.len = len;
        step = gt_font_split_line_str(&font, (uint32_t)area_font.w, ta->space_x,
                                      &width, NULL, NULL, NULL, NULL, first_line);
        if (step < 0) return false;
        if ((uint32_t)step < len && data[step] == '\n') newline = true;
        if (step == 0 && !newline) {
            uint32_t unicode = 0;
            uint32_t char_len = gt_font_one_char_code_len_get((uint8_t *)data,
                                                               &unicode,
                                                               font.info.encoding);
            step = (int32_t)(char_len ? char_len : 1);
        }
        offset += (uint32_t)step + (newline ? 1U : 0U);
        first_line = newline;
        line_count++;
    }

    page->start = start;
    page->end = offset;
    page->page_no = page_no;
    page->line_count = line_count;
    page->start_first_line = start_first_line ? 1 : 0;
    page->end_first_line = first_line ? 1 : 0;
    return offset > start || start >= ctx->file_total_bytes;
}

static void _file_page_remember(_gt_textarea_file_ctx_t *ctx,
                                const _gt_textarea_page_item_t *page,
                                bool force_index) {
    uint8_t slot;
    for (uint8_t i = 0; i < ctx->page_cache_count; ++i) {
        if (ctx->page_cache[i].start == page->start) {
            ctx->page_cache[i] = *page;
            goto index_page;
        }
    }
    slot = ctx->page_cache_write++ % _FILE_PAGE_CACHE_COUNT;
    ctx->page_cache[slot] = *page;
    if (ctx->page_cache_count < _FILE_PAGE_CACHE_COUNT) ctx->page_cache_count++;

index_page:
    if (!force_index && (page->page_no % _FILE_PAGE_INDEX_STEP) != 0) return;
    for (uint8_t i = 0; i < ctx->page_index_count; ++i) {
        if (ctx->page_index[i].start == page->start) {
            ctx->page_index[i] = *page;
            return;
        }
    }
    slot = ctx->page_index_write++ % GT_TEXTAREA_FILE_IDX_COUNT;
    if (ctx->page_index_count < GT_TEXTAREA_FILE_IDX_COUNT) ctx->page_index_count++;
    ctx->page_index[slot] = *page;
}

static bool _file_page_find_end(_gt_textarea_file_ctx_t *ctx, uint32_t end,
                                _gt_textarea_page_item_t *page) {
    for (uint8_t i = 0; i < ctx->page_cache_count; ++i) {
        if (ctx->page_cache[i].end == end) { *page = ctx->page_cache[i]; return true; }
    }
    for (uint8_t i = 0; i < ctx->page_index_count; ++i) {
        if (ctx->page_index[i].end == end) { *page = ctx->page_index[i]; return true; }
    }
    return false;
}

static bool _file_page_find_start(_gt_textarea_file_ctx_t *ctx, uint32_t start,
                                  _gt_textarea_page_item_t *page) {
    for (uint8_t i = 0; i < ctx->page_cache_count; ++i) {
        if (ctx->page_cache[i].start == start) { *page = ctx->page_cache[i]; return true; }
    }
    for (uint8_t i = 0; i < ctx->page_index_count; ++i) {
        if (ctx->page_index[i].start == start) { *page = ctx->page_index[i]; return true; }
    }
    return false;
}

static bool _file_page_checkpoint(_gt_textarea_file_ctx_t *ctx, uint32_t target,
                                  _gt_textarea_page_item_t *page) {
    bool found = false;
    for (uint8_t i = 0; i < ctx->page_index_count; ++i) {
        if (ctx->page_index[i].start <= target
            && (!found || ctx->page_index[i].start > page->start)) {
            *page = ctx->page_index[i];
            found = true;
        }
    }
    return found;
}

static void _file_page_apply(_gt_textarea_file_ctx_t *ctx,
                             const _gt_textarea_page_item_t *page) {
    ctx->scroll_byte_offset = page->start;
    ctx->page_end_offset = page->end;
    ctx->current_page = page->page_no;
    ctx->current_line_count = page->line_count;
    ctx->current_first_line = page->start_first_line;
    ctx->page_end_first_line = page->end_first_line;
    ctx->pixel_offset = 0;
}

static bool _file_page_ensure_current(gt_obj_st *obj, _gt_textarea_st *ta) {
    _gt_textarea_file_ctx_t *ctx = ta->file_ctx;
    _gt_textarea_page_item_t page;
    _file_check_layout(obj, ta, ctx);
    if (ctx->page_end_offset > ctx->scroll_byte_offset
        || ctx->scroll_byte_offset >= ctx->file_total_bytes) return true;
    if (!_file_scan_page(obj, ta, ctx->scroll_byte_offset,
                         ctx->current_first_line, ctx->current_page, &page)) return false;
    _file_page_remember(ctx, &page, ctx->current_page == 0);
    _file_page_apply(ctx, &page);
    return true;
}

static uint16_t _file_page_pixel_span(gt_obj_st *obj, _gt_textarea_st *ta) {
    gt_area_st padding_font = gt_area_reduce(obj->area, ta->padding);
    gt_area_st area_font = gt_area_reduce(padding_font,
                                          ta->border_width + _FONT_AREA_REDUCE_SIZE);
    uint16_t line_height = _file_line_height(ta);
    uint16_t line_count;
    if (line_height == 0 || area_font.h <= 0) return 1;
    line_count = (uint16_t)area_font.h / line_height;
    return line_count ? (uint16_t)(line_count * line_height) : line_height;
}

static int32_t _file_page_max_pixel_offset(gt_obj_st *obj, _gt_textarea_st *ta) {
    _gt_textarea_file_ctx_t *ctx = ta->file_ctx;
    if (ctx->page_end_offset < ctx->file_total_bytes) {
        return _file_page_pixel_span(obj, ta) - 1;
    }
    {
        gt_area_st padding_font = gt_area_reduce(obj->area, ta->padding);
        gt_area_st area_font = gt_area_reduce(padding_font,
                                              ta->border_width + _FONT_AREA_REDUCE_SIZE);
        int32_t content_h = (int32_t)ctx->current_line_count * _file_line_height(ta);
        int32_t max_offset = content_h - area_font.h;
        return max_offset > 0 ? max_offset : 0;
    }
}

static bool _file_page_get_next(gt_obj_st *obj, _gt_textarea_st *ta,
                                _gt_textarea_page_item_t *page) {
    _gt_textarea_file_ctx_t *ctx = ta->file_ctx;
    if (!_file_page_ensure_current(obj, ta)
        || ctx->page_end_offset >= ctx->file_total_bytes) return false;
    if (_file_page_find_start(ctx, ctx->page_end_offset, page)) return true;
    if (!_file_scan_page(obj, ta, ctx->page_end_offset,
                         ctx->page_end_first_line, ctx->current_page + 1, page)) return false;
    _file_page_remember(ctx, page, false);
    return true;
}

static bool _file_page_next(gt_obj_st *obj, _gt_textarea_st *ta) {
    _gt_textarea_file_ctx_t *ctx = ta->file_ctx;
    _gt_textarea_page_item_t page;
    if (!_file_page_get_next(obj, ta, &page)) return false;
    _file_page_apply(ctx, &page);
    return true;
}

static bool _file_page_prev(gt_obj_st *obj, _gt_textarea_st *ta) {
    _gt_textarea_file_ctx_t *ctx = ta->file_ctx;
    _gt_textarea_page_item_t scan = {0}, previous = {0};
    uint32_t target = ctx->scroll_byte_offset;
    bool have_previous = false;
    if (target == 0) return false;
    if (_file_page_find_end(ctx, target, &previous)) {
        _file_page_apply(ctx, &previous);
        return true;
    }
    if (!_file_page_checkpoint(ctx, target - 1, &scan)) {
        scan.start = 0;
        scan.page_no = 0;
        scan.start_first_line = 1;
    }
    while (scan.start < target) {
        _gt_textarea_page_item_t next;
        if (!_file_scan_page(obj, ta, scan.start, scan.start_first_line,
                             scan.page_no, &next)) break;
        _file_page_remember(ctx, &next, false);
        if (next.end >= target) {
            if (next.end == target) previous = next, have_previous = true;
            break;
        }
        scan.start = next.end;
        scan.page_no = next.page_no + 1;
        scan.start_first_line = next.end_first_line;
    }
    if (!have_previous) return false;
    _file_page_apply(ctx, &previous);
    return true;
}

static bool _file_page_seek_byte(gt_obj_st *obj, _gt_textarea_st *ta,
                                 uint32_t target) {
    _gt_textarea_file_ctx_t *ctx = ta->file_ctx;
    _gt_textarea_page_item_t scan = {0}, page;
    bool have_checkpoint = _file_page_checkpoint(ctx, target, &scan);
    if (!have_checkpoint) {
        scan.start = 0;
        scan.page_no = 0;
        scan.start_first_line = 1;
    }
    while (scan.start < target) {
        if (!_file_scan_page(obj, ta, scan.start, scan.start_first_line,
                             scan.page_no, &page)) return false;
        _file_page_remember(ctx, &page, false);
        if (target < page.end || page.end >= ctx->file_total_bytes) {
            _file_page_apply(ctx, &page);
            return true;
        }
        scan.start = page.end;
        scan.page_no = page.page_no + 1;
        scan.start_first_line = page.end_first_line;
    }
    if (scan.start <= target) {
        if (!_file_scan_page(obj, ta, scan.start, scan.start_first_line,
                             scan.page_no, &page)) return false;
        _file_page_remember(ctx, &page, true);
        _file_page_apply(ctx, &page);
        return true;
    }
    return false;
}

/* file-mode rendering ---------------------------------------------------*/
static void _file_draw_page(gt_obj_st *obj, _gt_textarea_st *ta,
                            const _gt_textarea_page_item_t *page,
                            int32_t y_offset, bool is_only_calc_height) {
    _gt_textarea_file_ctx_t *ctx = ta->file_ctx;
    gt_font_st font = { .info = ta->font_info, .res = NULL };
    gt_attr_font_st font_attr = {
        .font       = &font,
        .space_x    = ta->space_x,
        .space_y    = ta->space_y,
        .align      = ta->font_align,
        .indent     = ta->indent,
        .opa        = is_only_calc_height ? GT_OPA_0 : obj->opa,
        .font_color = ta->color_font,
        .reg.immediately_return = false,
    };
    _gt_draw_font_res_st font_res = {0};
    gt_area_st padding_font = gt_area_reduce(obj->area, ta->padding);
    gt_area_st area_font = gt_area_reduce(padding_font,
                                          ta->border_width + _FONT_AREA_REDUCE_SIZE);
    uint32_t offset = page->start;

    _file_set_font_encoding(&font, ctx);
    font_attr.logical_area = area_font;
    font_attr.logical_area.y += y_offset;
    font_attr.logical_area.h = 0xffff;
    if (!page->start_first_line) {
        font_attr.start_x = area_font.x;
        font_attr.start_y = font_attr.logical_area.y;
        font_attr.reg.enabled_start = true;
    }

    while (offset < page->end) {
        const uint8_t *data;
        uint32_t len;
        if (!_file_scan_window(ctx, offset, &data, &len)) break;
        if (len > page->end - offset) len = page->end - offset;
        font.utf8 = (char *)data;
        font.len = len;
        font_res = draw_text(obj->draw_ctx, &font_attr, &area_font);
        if (GT_RES_INV == font_res.res) break;
        font_attr.start_x = font_res.abs_pos.x;
        font_attr.start_y = font_res.abs_pos.y;
        font_attr.reg.enabled_start = true;
        offset += len;
    }
}

static void _draw_content_file(gt_obj_st *obj, gt_point_st *touch_point_p,
                               bool is_only_calc_height) {
    _gt_textarea_st *ta = (_gt_textarea_st *)obj;
    _gt_textarea_file_ctx_t *ctx = ta->file_ctx;
    _gt_textarea_page_item_t current;
    uint16_t page_span;
    if (NULL == ctx || NULL == ctx->read_buf) return;
    if (0 == obj->area.w || 0 == obj->area.h) return;
    (void)touch_point_p;
    if (!_file_page_ensure_current(obj, ta)) return;

    current.start = ctx->scroll_byte_offset;
    current.end = ctx->page_end_offset;
    current.page_no = ctx->current_page;
    current.line_count = ctx->current_line_count;
    current.start_first_line = ctx->current_first_line;
    current.end_first_line = ctx->page_end_first_line;
    page_span = _file_page_pixel_span(obj, ta);
    _file_draw_page(obj, ta, &current, -ctx->pixel_offset, is_only_calc_height);

    if (ctx->pixel_offset > 0) {
        _gt_textarea_page_item_t next;
        if (_file_page_get_next(obj, ta, &next)) {
            _file_draw_page(obj, ta, &next,
                            (int32_t)page_span - ctx->pixel_offset,
                            is_only_calc_height);
        }
    }
    if (!is_only_calc_height) {
        uint16_t line_height = _file_line_height(ta);
        ta->max_height = ctx->estimated_total_lines * (line_height ? line_height : 1);
    }
}

/* file-mode scroll handling ---------------------------------------------*/
static void _textarea_file_handle_scroll(gt_obj_st *obj) {
    _gt_textarea_st *ta = (_gt_textarea_st *)obj;
    _gt_textarea_file_ctx_t *ctx = ta->file_ctx;
    if (NULL == ctx) return;

    if (ctx->file_total_bytes == 0) {
        obj->process_attr.scroll.y = 0;
        _file_reset_pages(ctx);
        gt_disp_invalid_area(obj);
        return;
    }

    gt_point_st point = gt_indev_get_point();
    if (!ctx->touch_active) {
        ctx->last_touch_y = point.y;
        ctx->touch_active = 1;
        obj->process_attr.scroll.y = 0;
        return;
    }
    int32_t delta_px = (int32_t)point.y - (int32_t)ctx->last_touch_y;
    ctx->last_touch_y = point.y;

    if (0 == delta_px) {
        obj->process_attr.scroll.y = 0;
        ctx->last_scroll_y = 0;
        return;
    }

    ctx->gesture_px -= delta_px;
    ctx->pixel_offset -= delta_px;
    {
        int32_t page_span = _file_page_pixel_span(obj, ta);
        while (ctx->pixel_offset >= page_span) {
            int32_t remainder = ctx->pixel_offset - page_span;
            if (!_file_page_next(obj, ta)) {
                ctx->pixel_offset = 0;
                break;
            }
            ctx->pixel_offset = remainder;
        }
        while (ctx->pixel_offset < 0) {
            int32_t remainder = ctx->pixel_offset + page_span;
            if (!_file_page_prev(obj, ta)) {
                ctx->pixel_offset = 0;
                break;
            }
            ctx->pixel_offset = remainder;
        }
        if (ctx->page_end_offset >= ctx->file_total_bytes) {
            int32_t max_offset = _file_page_max_pixel_offset(obj, ta);
            if (ctx->pixel_offset > max_offset) ctx->pixel_offset = max_offset;
        }
    }
    obj->process_attr.scroll.y = 0;
    ctx->last_scroll_y = 0;
    gt_disp_invalid_area(obj);
}

#endif  /* GT_TEXTAREA_FILE_MODE */

#if GT_TEXTAREA_SUB_STRING_SCROLL_CACHE
static void _free_reset_scroll_cache(_gt_textarea_st * style) {
    if (style->scroll_cache.list) {
        gt_mem_free(style->scroll_cache.list);
        style->scroll_cache.list = NULL;
    }
    style->scroll_cache.item_count = 0;
}

static void _reset_scroll_list_offset(_gt_textarea_st * style, uint16_t count) {
    if (NULL == style->scroll_cache.list) {
        return;
    }
    gt_memset(style->scroll_cache.list, 0, count);
}

static gt_res_t _resize_scroll_cache(_gt_textarea_st * style, uint16_t cnt) {
    if (0 == cnt) {
        _free_reset_scroll_cache(style);
        return GT_RES_OK;
    }
    uint16_t count = cnt * sizeof(_scroll_cache_item_st);
    if (style->scroll_cache.item_count == count) {
        _reset_scroll_list_offset(style, count);
        return GT_RES_OK;
    }
    if (NULL == style->scroll_cache.list) {
        style->scroll_cache.list = gt_mem_malloc(count);
    } else {
        style->scroll_cache.list = gt_mem_realloc(style->scroll_cache.list, count);
    }
    if (NULL == style->scroll_cache.list) {
        style->scroll_cache.item_count = 0;
        return GT_RES_FAIL;
    }
    style->scroll_cache.item_count = cnt;
    _reset_scroll_list_offset(style, count);
    return GT_RES_OK;
}

static _scroll_cache_item_st * _scroll_cache_get_remark_item_by(_gt_textarea_st * style, gt_size_t offset) {
    if (NULL == style->scroll_cache.list) {
        return NULL;
    }
    uint8_t is_cur_half_display = 0;
    offset = gt_abs(offset);

    /** [Warn] except the begin one */
    for (gt_size_t i = 0, cnt = style->scroll_cache.item_count; i < cnt; ++i) {
        if (style->scroll_cache.list[i].offset_abs.y + style->scroll_cache.list[i].offset_abs.h > offset) {
            return &style->scroll_cache.list[i];
        }
    }
    return NULL;
}

static void _scroll_cache_remark_item(_gt_textarea_st * style, gt_size_t idx, gt_area_st * item_area) {
    if (NULL == style->scroll_cache.list) {
        return;
    }
    if (idx >= style->scroll_cache.item_count) {
        return;
    }
    style->scroll_cache.list[idx].idx = idx;
    style->scroll_cache.list[idx].offset_abs = *item_area;
}
#endif  /** GT_TEXTAREA_SUB_STRING_SCROLL_CACHE */

#if GT_TEXTAREA_LINE_CACHE
static void _line_cache_free(_gt_textarea_st * style) {
    if (style->line_cache.list) {
        gt_mem_free(style->line_cache.list);
        style->line_cache.list = NULL;
    }
    style->line_cache.count = 0;
    style->line_cache.capacity = 0;
}

static gt_res_t _line_cache_push(_gt_textarea_st * style,
                                  uint16_t seg_idx, uint16_t byte_offset, gt_size_t y) {
    if (style->line_cache.count >= style->line_cache.capacity) {
        uint16_t new_cap = style->line_cache.capacity ? style->line_cache.capacity * 2 : 16;
        _line_cache_item_st * p = (_line_cache_item_st *)gt_mem_realloc(
            style->line_cache.list, new_cap * sizeof(_line_cache_item_st));
        if (NULL == p) { return GT_RES_FAIL; }
        style->line_cache.list = p;
        style->line_cache.capacity = new_cap;
    }
    _line_cache_item_st * item = &style->line_cache.list[style->line_cache.count++];
    item->seg_idx    = seg_idx;
    item->byte_offset = byte_offset;
    item->y          = y;
    return GT_RES_OK;
}

/**
 * Binary search: find the last checkpoint whose y_offset <= target_offset.
 * target_offset = gt_abs(scroll_y) — pixels scrolled past the top.
 * Returns NULL if none found.
 */
static const _line_cache_item_st * _line_cache_find(_line_cache_st * lc, gt_size_t target_offset) {
    if (NULL == lc->list || 0 == lc->count) { return NULL; }
    if (lc->list[0].y > target_offset) { return NULL; }
    int32_t lo = 0, hi = (int32_t)lc->count - 1, mid;
    while (lo < hi) {
        mid = lo + ((hi - lo + 1) >> 1);
        if (lc->list[mid].y <= target_offset) { lo = mid; }
        else { hi = mid - 1; }
    }
    return &lc->list[lo];
}
#endif  /** GT_TEXTAREA_LINE_CACHE */

#if GT_TEXTAREA_LINE_CACHE
/**
 * Pre-scan all segments and record a checkpoint every GT_TEXTAREA_LINE_CACHE_STEP lines.
 * Each checkpoint stores (seg_idx, byte_offset, y) so that during scrolling we can
 * jump font.utf8 directly to the right byte and set start_y, skipping all
 * gt_font_split_line_str calls for lines above the viewport.
 *
 * Called once after a full (non-scroll) render completes.
 */
static void _line_cache_build(gt_obj_st * obj) {
    _gt_textarea_st * style = (_gt_textarea_st *)obj;
    if (NULL == style->contents || 0 == style->cnt_contents) { return; }

    _line_cache_free(style);

    gt_area_st padding_font = gt_area_reduce(obj->area, style->padding);
    gt_area_st area_font    = gt_area_reduce(padding_font, style->border_width + _FONT_AREA_REDUCE_SIZE);
    uint32_t max_w = (uint32_t)area_font.w;
    uint8_t  space_x = style->space_x;
    uint16_t line_height = style->font_info.size + style->space_y;
    if (0 == line_height) { return; }
    uint32_t indent_w = (uint32_t)gt_font_get_indent_width(&style->font_info, style->indent);

    /** cur_y_offset: pixel offset from area_font.y (0 = first line top) */
    gt_size_t cur_y_offset = 0;
    uint16_t  line_counter = 0;
    /**
     * Carry-over state between segments: draw_text continues the next
     * segment on the same line as the previous segment's last line
     * (unless that line ended with '\n').  We track the remaining
     * width so _line_cache_build mirrors draw_text exactly.
     */
    uint32_t carry_disp_w = max_w - indent_w;   /** first line has indent */
    bool     carry_first  = true;    /** is_first_line for next segment */

    for (uint16_t si = 0; si < style->cnt_contents; ++si) {
        if (NULL == style->contents[si].text) { continue; }

        gt_font_st tmp_font = {
            .info = style->font_info,
            .utf8 = style->contents[si].text,
            .len  = style->contents[si].len,
            .res  = NULL,
        };
#if GT_TEXTAREA_CUSTOM_FONT_STYLE
        tmp_font.info = style->contents[si].font_info;
#endif
        uint32_t seg_len = (uint32_t)style->contents[si].len;
        uint32_t byte_idx = 0;
        bool is_first_line = carry_first;
        uint32_t disp_w = carry_disp_w;

        while (byte_idx < seg_len) {
            if (0 == (line_counter % GT_TEXTAREA_LINE_CACHE_STEP) &&
                (0 == line_counter || disp_w == max_w)) {
                /** Record checkpoints at the start of a full line.
                 *  Always record the very first checkpoint (line_counter==0)
                 *  so _line_cache_find has a base entry. */
                _line_cache_push(style, si, (uint16_t)byte_idx, cur_y_offset);
            }

            gt_font_st scan_font = tmp_font;
            scan_font.utf8 = &style->contents[si].text[byte_idx];
            scan_font.len  = seg_len - byte_idx;

            uint32_t line_w = max_w - disp_w;
            int32_t step = gt_font_split_line_str(&scan_font, max_w, space_x,
                                                   &line_w, NULL, NULL, NULL, NULL,
                                                   is_first_line);
            if (step < 0) { break; }

            if (byte_idx + (uint32_t)step < seg_len &&
                '\n' == style->contents[si].text[byte_idx + (uint32_t)step]) {
                byte_idx += (uint32_t)step + 1;
                cur_y_offset += line_height;
                ++line_counter;
                is_first_line = true;
                disp_w = max_w - indent_w;
                continue;
            }

            if (step == 0) {
                if (disp_w < max_w) {
                    /**
                     * Remaining width on a continuation line is too narrow
                     * for even one character.  draw_text wraps to a new line
                     * without consuming any bytes (goto _disp_font path).
                     */
                    cur_y_offset += line_height;
                    ++line_counter;
                    is_first_line = false;
                    disp_w = max_w;
                    continue;
                }
                step = 1;
            }
            byte_idx += (uint32_t)step;
            disp_w -= line_w;

            if (byte_idx < seg_len) {
                /** mid-segment wrap: advance to next line */
                cur_y_offset += line_height;
                ++line_counter;
                is_first_line = false;
                disp_w = max_w;
            }
            /** else: last piece of segment — do NOT advance y.
             *  draw_text does not advance area_font.y here either;
             *  the next segment will continue on this same line. */
        }
        /** Propagate line state to next segment */
        carry_disp_w = disp_w;
        carry_first  = is_first_line;
        /**
         * If segment ended with '\n', draw_text advances y in the
         * else-if branch of next_line_lb.  We already did that above.
         * carry_disp_w = max_w and carry_first = true from the \n path.
         */
    }
}

/**
 * Apply a line-cache hit: adjust font.utf8/len and font_attr start position
 * so draw_text begins at the cached line instead of the segment start.
 * Returns the segment index to start the outer loop from.
 */
static uint16_t _line_cache_apply(gt_obj_st * obj,
                                   gt_font_st * font,
                                   gt_attr_font_st * font_attr,
                                   gt_size_t scroll_y) {
    _gt_textarea_st * style = (_gt_textarea_st *)obj;
    /**
     * scroll_y is negative when scrolled down (e.g. -200 means 200px scrolled).
     * target_offset = pixels scrolled past the top = gt_abs(scroll_y).
     * Checkpoints store y as offset from area_font.y (0 = first line).
     */
    gt_size_t target_offset = (gt_size_t)gt_abs(scroll_y);

    const _line_cache_item_st * hit = _line_cache_find(&style->line_cache, target_offset);
    if (NULL == hit) { return 0; }

    uint16_t si = hit->seg_idx;
    if (NULL == style->contents[si].text) { return 0; }

    uint16_t off = hit->byte_offset;
    if (off >= style->contents[si].len) { return si + 1; }

    font->utf8 = style->contents[si].text + off;
    font->len  = style->contents[si].len  - off;
#if GT_TEXTAREA_CUSTOM_FONT_STYLE
    font->info = style->contents[si].font_info;
#endif

    font_attr->start_x = font_attr->logical_area.x;
    /**
     * hit->y is the line's offset from area_font.y.
     * logical_area.y = area_font.y + scroll_y  (scroll_y < 0)
     * So the screen y of this cached line = area_font.y - target_offset + hit->y
     *                                     = logical_area.y + hit->y
     */
    font_attr->start_y = font_attr->logical_area.y + (gt_size_t)hit->y;
    font_attr->reg.enabled_start = true;

    return si;
}
#endif  /** GT_TEXTAREA_LINE_CACHE */

static inline void _set_hold_on_contents_for_scroll(_gt_textarea_st * style, bool hold_on) {
    style->reg.hold_on = hold_on;
#if GT_TEXTAREA_LINE_CACHE
    /** When content changes (hold_on=false), invalidate stale cache immediately
     *  so _line_cache_apply never jumps to a checkpoint from old content. */
    if (false == hold_on) {
        _line_cache_free(style);
    }
#endif
}

static inline bool _is_hold_on_contents_for_scroll(_gt_textarea_st * style) {
    return style->reg.hold_on;
}

static GT_ATTRIBUTE_RAM_TEXT void
_draw_content_or_get_words(gt_obj_st * obj, gt_point_st * touch_point_p, bool is_only_calc_height) {
    _gt_textarea_st * style = (_gt_textarea_st * )obj;
#if GT_TEXTAREA_FILE_MODE
    if (style->mode != GT_TEXTAREA_MODE_MEMORY) {
        _draw_content_file(obj, touch_point_p, is_only_calc_height);
        return;
    }
#endif
    uint16_t idx = 0;
    gt_font_st font = {
        .info       = style->font_info,
        .res        = NULL,
    };
    if (NULL == style->contents) {
        return;
    }
    if (0 == obj->area.w || 0 == obj->area.h) {
        return;
    }

    gt_attr_font_st font_attr = {
        .font       = &font,
        .space_x    = style->space_x,
        .space_y    = style->space_y,
        .align      = style->font_align,
        .indent     = style->indent,
        .opa        = is_only_calc_height ? GT_OPA_0 : obj->opa,
        .reg.immediately_return = _is_hold_on_contents_for_scroll(style),
#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
        .reg.touch_point = touch_point_p ? true : false,
        .reg.single_cn = style->reg.click_single_cn,
        .touch_point = touch_point_p,
#endif
    };
#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
    if (touch_point_p) {
        _set_touch_word(style, NULL, 0);
    }
#endif

    gt_size_t scroll_y = obj->process_attr.scroll.y;
    _gt_draw_font_res_st font_res = {0};
    gt_area_st padding_font = gt_area_reduce(obj->area, style->padding);
    gt_area_st area_font = gt_area_reduce(padding_font, style->border_width + _FONT_AREA_REDUCE_SIZE);
    font_attr.logical_area = area_font;
    font_attr.logical_area.y += scroll_y;
    font_attr.logical_area.h = 0xffff;

    uint16_t height = font_attr.font->info.size + (font_attr.space_y << 1);

#if GT_TEXTAREA_SUB_STRING_SCROLL_CACHE
    gt_area_st cur_offset = {0};
    if (_is_hold_on_contents_for_scroll(style)) {
        _scroll_cache_item_st * remark_item_p = _scroll_cache_get_remark_item_by(style, scroll_y);
        if (remark_item_p) {
            font_attr.reg.enabled_start = true;
            idx = remark_item_p->idx;
            font_attr.start_x = remark_item_p->offset_abs.x + font_attr.logical_area.x;
            font_attr.start_y = remark_item_p->offset_abs.y + font_attr.logical_area.y;
        }
    }
#endif

#if GT_TEXTAREA_LINE_CACHE
    bool _lc_is_scroll = _is_hold_on_contents_for_scroll(style);
    bool _lc_seg_applied = false;
    char * _lc_utf8_start = NULL;
    uint16_t _lc_utf8_len = 0;
    if (_lc_is_scroll && scroll_y < 0 && style->line_cache.count > 0) {
        idx = _line_cache_apply(obj, &font, &font_attr, scroll_y);
        /* save the adjusted pointer so the loop doesn't overwrite it */
        _lc_utf8_start = font.utf8;
        _lc_utf8_len   = font.len;
        _lc_seg_applied = (_lc_utf8_start != NULL);
    }
#endif

    if(0 != style->start_x_off || 0 != style->start_y_off) {
        font_attr.start_x = style->start_x_off + area_font.x + gt_font_get_indent_width(&font.info, style->indent);
        font_attr.start_y = style->start_y_off + area_font.y;
        font_attr.reg.enabled_start = true;
    }

    while (idx < style->cnt_contents) {
        font.utf8               = style->contents[idx].text;
        if (NULL == font.utf8) {
            ++idx;
            continue;
        }
        font.len                = style->contents[idx].len;
        font.info.style.all = style->contents[idx].mask_style;
#if GT_TEXTAREA_LINE_CACHE
        /** First iteration after a cache hit: use the offset pointer, not segment start */
        if (_lc_seg_applied) {
            font.utf8 = _lc_utf8_start;
            font.len  = _lc_utf8_len;
            _lc_seg_applied = false;
        }
#endif

#if GT_TEXTAREA_CUSTOM_FONT_STYLE
        font_attr.font_color    = style->contents[idx].font_info.palette;
        font.info               = style->contents[idx].font_info;
#else
        font_attr.font_color    = style->contents[idx].color;
#endif
        font_res = draw_text(obj->draw_ctx, &font_attr, &area_font);
#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
        if (touch_point_p && font_res.touch_word.word_p) {
            _set_touch_word(style, font_res.touch_word.word_p, font_res.touch_word.len);
            break;
        }
#endif
        if (_is_hold_on_contents_for_scroll(style) && GT_RES_INV == font_res.res) {
            break;
        }
#if GT_TEXTAREA_SUB_STRING_SCROLL_CACHE
        if (false == _is_hold_on_contents_for_scroll(style)) {
            if (idx) {
                cur_offset.x = font_attr.start_x - font_attr.logical_area.x;
                cur_offset.y = font_attr.start_y - font_attr.logical_area.y;
            } else {
                cur_offset.x = 0;
                cur_offset.y = 0;
            }
            cur_offset.w = font_res.size.x;
            cur_offset.h = font_res.size.y;
            _scroll_cache_remark_item(style, idx, &cur_offset);
        }
#endif
        font_attr.start_x       = font_res.abs_pos.x;
        font_attr.start_y       = font_res.abs_pos.y;
        font_attr.reg.enabled_start = true;
        ++idx;
    }

    if (touch_point_p) {
        /** Get touch word handler, do not calc all text height to scroll */
        return ;
    }
    height += font_res.abs_pos.y + font.info.size - font_attr.logical_area.y;
    if (height > style->max_height) {
        style->max_height = height;
        if (style->reg.always_scroll_to_newest) {
            _create_auto_scoll_to(obj);
        }
    }
#if GT_TEXTAREA_LINE_CACHE
    /** Build line cache after a full render (not during scroll, not touch-word query) */
    if (false == _lc_is_scroll) {
        _line_cache_build(obj);
        _set_hold_on_contents_for_scroll(style, true);
    }
#endif
}

/**
 * @brief obj init textarea widget call back
 *
 * @param obj
 */
static GT_ATTRIBUTE_RAM_TEXT void
_textarea_init_cb(gt_obj_st * obj) {
    _gt_textarea_st * style = (_gt_textarea_st * )obj;

    /** draw border */
    if (obj->show_bg) {
        gt_attr_rect_st rect_attr;
        gt_graph_init_rect_attr(&rect_attr);
        rect_attr.radius = obj->radius;
        rect_attr.border_width = style->border_width;
        rect_attr.reg.is_fill = 1;
        rect_attr.bg_color = obj->bgcolor;
        rect_attr.border_color = style->color_border;
        rect_attr.bg_opa = style->bg_opa;

        draw_bg(obj->draw_ctx, &rect_attr, &obj->area);
    }

    _draw_content_or_get_words(obj, NULL, false);

    // focus
    draw_focus(obj);
}

static GT_ATTRIBUTE_RAM_TEXT inline void
_reset_scroll_value(gt_obj_st * obj) {
    obj->process_attr.scroll.y = 0;
    gt_obj_set_scroll_prev(obj, gt_obj_get_scroll_prev_x(obj), 0);
#if GT_TEXTAREA_FILE_MODE
    {
        _gt_textarea_st *style = (_gt_textarea_st *)obj;
        if (style->file_ctx) {
            _file_reset_pages(style->file_ctx);
            style->file_ctx->last_scroll_y = 0;
        }
    }
#endif
}

static GT_ATTRIBUTE_RAM_TEXT void
_free_contents(gt_obj_st * obj) {
    _gt_textarea_st * style_p = (_gt_textarea_st * )obj;
    if (style_p->contents) {
        _set_hold_on_contents_for_scroll(style_p, false);
#if GT_TEXTAREA_SUB_STRING_SCROLL_CACHE
        _free_reset_scroll_cache(style_p);
#endif

        for (gt_size_t i = style_p->cnt_contents - 1; i >= 0; i--) {
            if (NULL == style_p->contents[i].text) {
                continue;
            }
            gt_mem_free(style_p->contents[i].text);
            style_p->contents[i].text = NULL;
            style_p->contents[i].len = 0;
        }

        gt_mem_free(style_p->contents);
        style_p->contents = NULL;
        style_p->max_height = 0;
        style_p->cnt_contents = 0;
        _reset_scroll_value(obj);
    }

    if (style_p->scroll_to_newest_anim) {
        gt_anim_stop(style_p->scroll_to_newest_anim);
        gt_anim_del_safe_by(obj);
        style_p->scroll_to_newest_anim = NULL;
    }
}

/**
 * @brief obj deinit call back
 *
 * @param obj
 */
static GT_ATTRIBUTE_RAM_TEXT void
_textarea_deinit_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "start deinit_cb");
    if (NULL == obj) {
        return ;
    }
    _free_contents(obj);
#if GT_TEXTAREA_FILE_MODE
    {
        _gt_textarea_st *style = (_gt_textarea_st *)obj;
        if (style->file_ctx) {
            _file_ctx_deinit(style->file_ctx);
            style->file_ctx = NULL;
        }
    }
#endif
}

static GT_ATTRIBUTE_RAM_TEXT void
_ready_pull_to_refresh_cb(struct gt_anim_s * anim) {
    gt_obj_st * obj = (gt_obj_st *)anim->tar;
    _gt_textarea_st * style = (_gt_textarea_st * )obj;

    style->reg.pull_to_refresh = false;
    gt_event_send(obj, GT_EVENT_TYPE_PULL_TO_REFRESH, NULL);
}

static GT_ATTRIBUTE_RAM_TEXT void
_pull_wait_to_refresh(struct gt_obj_s * obj) {
    gt_anim_st anim;
    gt_anim_init(&anim);
    gt_anim_set_target(&anim, obj);
    gt_anim_set_time(&anim, 0);
    gt_anim_set_time_delay_start(&anim, 200);
    gt_anim_set_ready_cb(&anim, _ready_pull_to_refresh_cb);
    gt_anim_start(&anim);
}

static GT_ATTRIBUTE_RAM_TEXT void
_ready_load_more_cb(struct gt_anim_s * anim) {
    gt_obj_st * obj = (gt_obj_st *)anim->tar;
    _gt_textarea_st * style = (_gt_textarea_st * )obj;

    style->reg.load_more = false;
    gt_event_send(obj, GT_EVENT_TYPE_LOAD_MORE, NULL);
}

static GT_ATTRIBUTE_RAM_TEXT void
_load_more(struct gt_obj_s * obj) {
    gt_anim_st anim;
    gt_anim_init(&anim);
    gt_anim_set_target(&anim, obj);
    gt_anim_set_time(&anim, 0);
    gt_anim_set_time_delay_start(&anim, 200);
    gt_anim_set_ready_cb(&anim, _ready_load_more_cb);
    gt_anim_start(&anim);
}

GT_ATTRIBUTE_RAM_TEXT static inline bool
_is_scroll_to_bottom(gt_obj_st *obj) {
    _gt_textarea_st *style = (_gt_textarea_st *)obj;
#if GT_TEXTAREA_FILE_MODE
    if (style->mode != GT_TEXTAREA_MODE_MEMORY && style->file_ctx) {
        return style->file_ctx->page_end_offset
               >= style->file_ctx->file_total_bytes;
    }
#endif
    return gt_abs(obj->process_attr.scroll.y) + obj->area.h - style->padding > style->max_height;
}

GT_ATTRIBUTE_RAM_TEXT static void
_anim_scroll_exec_cb(void * tv, int32_t val) {
    gt_obj_st * obj = (gt_obj_st * )tv;
    _gt_textarea_st * style = (_gt_textarea_st * )tv;
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    if (_is_scroll_to_bottom(obj)) {
        gt_anim_stop(style->scroll_to_newest_anim);
        gt_anim_set_repeat_count(style->scroll_to_newest_anim, 0);
        style->scroll_to_newest_anim = NULL;
        return;
    }
    obj->process_attr.scroll.y -= 1;
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

GT_ATTRIBUTE_RAM_TEXT static void
_anim_scroll_ready_cb(struct gt_anim_s * anim) {
    _gt_textarea_st * style = (_gt_textarea_st * )anim->tar;
    if (false == gt_obj_is_type((gt_obj_st * )style, OBJ_TYPE)) {
        return;
    }
    style->scroll_to_newest_anim = NULL;
}

GT_ATTRIBUTE_RAM_TEXT static void
_create_auto_scoll_to(gt_obj_st * textarea) {
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    int32_t scroll_time = 3 * 60 * 1000;    /** predicted value */
    if (style->scroll_to_newest_anim) {
        /** update time */
        gt_anim_set_time(style->scroll_to_newest_anim, scroll_time);
        return;
    }
    gt_anim_st anim;
    gt_anim_init(&anim);
    gt_anim_set_target(&anim, textarea);
    gt_anim_set_time(&anim, scroll_time);
    gt_anim_set_value(&anim, 0, 0xffff);
    gt_anim_set_exec_cb(&anim, _anim_scroll_exec_cb);
    gt_anim_set_ready_cb(&anim, _anim_scroll_ready_cb);
    style->scroll_to_newest_anim = gt_anim_start(&anim);
}

/**
 * @brief obj event handler call back
 *
 * @param obj
 * @param e event
 */
static GT_ATTRIBUTE_RAM_TEXT void
_textarea_event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code_val = gt_event_get_code(e);
    _gt_textarea_st * style = (_gt_textarea_st * )obj;
    switch(code_val) {
        case GT_EVENT_TYPE_DRAW_START: {
            gt_disp_invalid_area(obj);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;
        }
        case GT_EVENT_TYPE_INPUT_PRESSED: {
            style->reg.click_valid = true;
#if GT_TEXTAREA_FILE_MODE
            if (style->mode != GT_TEXTAREA_MODE_MEMORY && style->file_ctx) {
                gt_point_st point = gt_indev_get_point();
                style->file_ctx->last_touch_y = point.y;
                style->file_ctx->touch_active = 1;
                style->file_ctx->gesture_px = 0;
            }
#endif
#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
            _set_touch_word(style, NULL, 0);
#endif
            break;
        }
        case GT_EVENT_TYPE_INPUT_SCROLL: {
            style->reg.click_valid = false;
#if GT_TEXTAREA_FILE_MODE
            if (style->mode != GT_TEXTAREA_MODE_MEMORY) {
                _textarea_file_handle_scroll(obj);
                break;
            }
#endif
            if (style->max_height < obj->area.h) {
                _reset_scroll_value(obj); break;
            }
            style->reg.load_more = false;
            style->reg.pull_to_refresh = false;
            if (obj->process_attr.scroll.y > 0) {
                if (false == style->reg.pull_to_refresh && obj->process_attr.scroll.y > obj->area.h >> 4)
                    style->reg.pull_to_refresh = true;
                _reset_scroll_value(obj);
            } else if (_is_scroll_to_bottom(obj)) {
                if (false == style->reg.load_more &&
                    gt_abs(obj->process_attr.scroll.y) + obj->area.h - (obj->area.h >> 4) > style->max_height)
                    style->reg.load_more = true;
                obj->process_attr.scroll.y = obj->area.h - style->max_height;
            }
#if GT_TEXTAREA_LINE_CACHE
            if (style->line_cache.count > 0) _set_hold_on_contents_for_scroll(style, true);
#endif
            gt_disp_invalid_area(obj);
            break;
        }
        case GT_EVENT_TYPE_INPUT_SCROLL_END: {
#if GT_TEXTAREA_FILE_MODE
            if (style->mode != GT_TEXTAREA_MODE_MEMORY) {
                int32_t gesture_px = style->file_ctx->gesture_px;
                style->file_ctx->gesture_px = 0;
                style->file_ctx->touch_active = 0;
                gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
                break;
            }
#endif
            if (style->reg.pull_to_refresh) {
                _pull_wait_to_refresh(obj);
            } else if (style->reg.load_more) {
                _load_more(obj);
            } else {
#if GT_TEXTAREA_LINE_CACHE
                /** Only set hold_on=true if cache is valid; otherwise keep false
                 *  so the next draw triggers a full render to rebuild cache. */
                if (style->line_cache.count > 0) {
                    _set_hold_on_contents_for_scroll(style, true);
                }
#else
                _set_hold_on_contents_for_scroll(style, true);
#endif
                gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            }
            break;
        }
        case GT_EVENT_TYPE_INPUT_RELEASED: {
            /* click event finish */
            if (false == style->reg.click_valid) {
#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
                _set_touch_word(style, NULL, 0);
#endif
                break;
            }
#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
            gt_point_st tp = gt_indev_get_point();
            _draw_content_or_get_words(obj, &tp, false);
#endif
            // gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;
        }
        default:
            break;
    }
}

/* global functions / API interface -------------------------------------*/

gt_obj_st * gt_textarea_create(gt_obj_st * parent) {
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    if (NULL == obj) {
        return obj;
    }
    obj->fixed = false;
    gt_obj_set_scroll_dir(obj, GT_SCROLL_VERTICAL);
    obj->radius = 4;
    obj->area.w = 100;
    obj->area.h = 100;
    obj->show_bg = true;

    _gt_textarea_st * style = (_gt_textarea_st * )obj;
    gt_font_info_init(&style->font_info);

    obj->bgcolor             = gt_color_white();
    style->color_font        = gt_color_black();
    style->bg_opa            = GT_OPA_100;
    style->font_align        = GT_ALIGN_NONE;
    style->space_x           = 0;
    style->space_y           = 0;
    style->color_border      = gt_color_hex(0xc7c7c7);
    style->border_width      = 2;

    _set_hold_on_contents_for_scroll(style, false);
#if GT_TEXTAREA_FILE_MODE
    style->mode = GT_TEXTAREA_MODE_MEMORY;
    style->file_ctx = NULL;
    style->file_buf_size_cfg = GT_TEXTAREA_FILE_BUF_SIZE;
    style->file_word_wrap = true;
    style->file_editable = false;
#endif
    return obj;
}

gt_res_t gt_textarea_set_always_scroll_to_newest(gt_obj_st * textarea, bool always_scroll_to_newest)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return GT_RES_FAIL;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->reg.always_scroll_to_newest = always_scroll_to_newest ? true : false;
    return GT_RES_OK;
}

void gt_textarea_set_text(gt_obj_st * textarea, const char * fmt, ...)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }

    char buffer[8] = {0};
    va_list args;
    va_list args2;
    va_start(args, fmt);
    va_copy(args2, args);
    uint16_t size = (NULL == fmt) ? 0 : (vsnprintf(buffer, sizeof(buffer), fmt, args) + 1);
    va_end(args);
    if (!size) {
        goto free_lb;
    }
    char * text = gt_mem_malloc(size);
    if (NULL == text) {
        goto free_lb;
    }
    gt_memset(text, 0, size);
    va_start(args2, fmt);
    vsnprintf(text, size, fmt, args2);
    va_end(args2);

    gt_textarea_set_text_by_len(textarea, (const char* )text, strlen(text));

    gt_mem_free(text);
    return ;
free_lb:
    va_end(args2);
}

void gt_textarea_set_text_by_len(gt_obj_st * textarea, const char * text, uint32_t len)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
#if GT_TEXTAREA_FILE_MODE
    if (style->mode != GT_TEXTAREA_MODE_MEMORY) return;
#endif
    uint8_t style_mask = GT_FONT_STYLE_NONE;
    gt_color_t color = style->color_font;

    if (style->cnt_contents) {
#if GT_TEXTAREA_CUSTOM_FONT_STYLE
        color = style->contents[style->cnt_contents - 1].font_info.palette;
#else
        color = style->contents[style->cnt_contents - 1].color;
#endif
        _free_contents(textarea);
        style->cnt_contents = 0;
    }
    if (NULL == text) {
        return;
    }
    style->contents  = gt_mem_malloc( sizeof(gt_textarea_param_st) );
    GT_CHECK_BACK(style->contents);
    style->contents[0].len  = len;
    style->contents[0].text = gt_mem_malloc(style->contents[0].len + 1);
    if (NULL == style->contents[0].text) {
        GT_CHECK_PRINT(style->contents[0].text);
        gt_mem_free(style->contents);
        style->contents = NULL;
        style->cnt_contents = 0;
        style->max_height = 0;
        return;
    }
    gt_memcpy(style->contents[0].text, text, len);
    style->contents[0].text[len] = '\0';
    style->contents[0].mask_style = style_mask;
#if GT_TEXTAREA_CUSTOM_FONT_STYLE
    style->contents[0].font_info.palette = color;
#else
    style->contents[0].color = color;
#endif
    style->cnt_contents = 1;
    style->max_height = 0;

#if GT_TEXTAREA_CUSTOM_FONT_STYLE
    style->contents[0].font_info = style->font_info;
    style->contents[0].font_info.palette = color;
    gt_font_info_update_font_thick(&style->contents[0].font_info);
#endif

#if GT_TEXTAREA_SUB_STRING_SCROLL_CACHE
    _resize_scroll_cache(style, style->cnt_contents);
#endif
    _set_hold_on_contents_for_scroll(style, false);

    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_textarea_hide_background(gt_obj_st * textarea, bool hide)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    gt_obj_show_bg(textarea, hide ? false : true);
}

char * gt_textarea_get_text(gt_obj_st * textarea,uint16_t idx)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return NULL;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
#if GT_TEXTAREA_FILE_MODE
    if (style->mode != GT_TEXTAREA_MODE_MEMORY) return NULL;
#endif
    return style->contents[idx].text;
}

void gt_textarea_clear_all_str(gt_obj_st * textarea) {
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) return;
#if GT_TEXTAREA_FILE_MODE
    _gt_textarea_st *style = (_gt_textarea_st *)textarea;
    if (style->mode != GT_TEXTAREA_MODE_MEMORY) {
        gt_textarea_close_file(textarea);
        return;
    }
#endif
    _free_contents(textarea);
    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_textarea_add_str(gt_obj_st * textarea, char * str, gt_font_style_et style_mask, gt_color_t color)
{
    gt_textarea_add_str_by_len(textarea, str, strlen(str), style_mask, color);
}

void gt_textarea_add_str_by_len(gt_obj_st * textarea, char * str, uint32_t len, gt_font_style_et style_mask, gt_color_t color)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    if (NULL == str || 0 == len) { return; }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
#if GT_TEXTAREA_FILE_MODE
    if (style->mode != GT_TEXTAREA_MODE_MEMORY) return;
#endif
    uint16_t idx = style->cnt_contents;

    if (idx) {
        style->contents = gt_mem_realloc(style->contents, (style->cnt_contents + 1) * sizeof(gt_textarea_param_st));
    } else {
        style->contents = gt_mem_malloc(sizeof(gt_textarea_param_st));
    }
    if (NULL == style->contents) {
        GT_CHECK_PRINT(style->contents);
        style->cnt_contents = 0;
        return;
    }
    style->contents[idx].len  = len;
    style->contents[idx].text = gt_mem_malloc(style->contents[idx].len + 1);
    if (NULL == style->contents[idx].text) {
        /** Reset to before */
        style->contents = gt_mem_realloc(style->contents, (style->cnt_contents) * sizeof(gt_textarea_param_st));
        return;
    }
    gt_memcpy(style->contents[idx].text, str, style->contents[idx].len);
    style->contents[idx].text[style->contents[idx].len] = '\0';
    style->contents[idx].mask_style = style_mask;

#if GT_TEXTAREA_CUSTOM_FONT_STYLE
    style->contents[idx].font_info = style->font_info;
    style->contents[idx].font_info.palette = color;
    gt_font_info_update_font_thick(&style->contents[idx].font_info);
#else
    style->contents[idx].color = color;
#endif

    ++style->cnt_contents;
#if GT_TEXTAREA_SUB_STRING_SCROLL_CACHE
    _resize_scroll_cache(style, style->cnt_contents);
#endif
    _set_hold_on_contents_for_scroll(style, false);
    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
}


void gt_textarea_append_str(gt_obj_st * textarea, char * str, gt_font_style_et style, gt_color_t color)
{
    gt_textarea_append_str_by_len(textarea, str, strlen(str), style, color);
}

void gt_textarea_append_str_by_len(gt_obj_st * textarea, char * str, uint32_t len, gt_font_style_et style_mask, gt_color_t color)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    if (NULL == str || 0 == len) { return; }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
#if GT_TEXTAREA_FILE_MODE
    if (style->mode != GT_TEXTAREA_MODE_MEMORY) return;
#endif
    if(style->cnt_contents == 0) {
        gt_textarea_add_str_by_len(textarea, str, len, style_mask, color);
        return;
    }
    int32_t idx = style->cnt_contents - 1;
    if(idx < 0) {
        return;
    }

    char* new_str = gt_mem_realloc(style->contents[idx].text, style->contents[idx].len + len + 1);
    if (NULL == new_str) {
        return;
    }
    gt_memcpy(new_str + style->contents[idx].len, str, len);
    new_str[style->contents[idx].len + len] = '\0';
    style->contents[idx].len += len;
    style->contents[idx].text = new_str;

#if GT_TEXTAREA_SUB_STRING_SCROLL_CACHE
    _resize_scroll_cache(style, style->cnt_contents);
#endif
    _set_hold_on_contents_for_scroll(style, false);
    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
}

uint16_t gt_textarea_get_str_count(gt_obj_st * textarea)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return 0;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
#if GT_TEXTAREA_FILE_MODE
    if (style->mode != GT_TEXTAREA_MODE_MEMORY) return 0;
#endif
    return style->cnt_contents;
}

#if GT_TEXTAREA_CUSTOM_FONT_STYLE
void gt_textarea_add_str_by_param(gt_obj_st * textarea, gt_textarea_param_st * param)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    if (NULL == param) {
        return;
    }
    if (NULL == param->text || 0 == param->len) {
        return;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    uint16_t idx = style->cnt_contents;

    if (idx) {
        style->contents = gt_mem_realloc(style->contents, (style->cnt_contents + 1) * sizeof(gt_textarea_param_st));
    } else {
        style->contents = gt_mem_malloc(sizeof(gt_textarea_param_st));
    }
    if (NULL == style->contents) {
        GT_CHECK_PRINT(style->contents);
        style->cnt_contents = 0;
        return;
    }

    uint8_t new_line_len = param->new_line ? 1 : 0;

    style->contents[idx].len = param->len + new_line_len;
    style->contents[idx].text = gt_mem_malloc(style->contents[idx].len + 1);
    if (NULL == style->contents[idx].text) {
        /** Reset to before */
        style->contents = gt_mem_realloc(style->contents, (style->cnt_contents) * sizeof(gt_textarea_param_st));
        return;
    }
    gt_memcpy(style->contents[idx].text, param->text, param->len);
    if (param->new_line) {
        style->contents[idx].text[param->len] = '\n';
    }
    style->contents[idx].text[style->contents[idx].len] = '\0';
    style->contents[idx].mask_style = param->mask_style;
    style->contents[idx].font_info = param->font_info;
    gt_font_info_update_font_thick(&style->contents[idx].font_info);
    ++style->cnt_contents;
#if GT_TEXTAREA_SUB_STRING_SCROLL_CACHE
    _resize_scroll_cache(style, style->cnt_contents);
#endif
    _set_hold_on_contents_for_scroll(style, false);
}
#endif

void gt_textarea_set_space(gt_obj_st * textarea, uint8_t space_x, uint8_t space_y)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->space_x = space_x;
    style->space_y = space_y;

    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_textarea_set_font_size(gt_obj_st * textarea, gt_font_size_t size)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->font_info.size = size;
    gt_font_info_update_font_thick(&style->font_info);
    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
}
void gt_textarea_set_font_gray(gt_obj_st * textarea, uint8_t gray)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->font_info.gray = gray;
}

void gt_textarea_set_indent(gt_obj_st * textarea, uint16_t indent)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->indent = indent;
    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_textarea_set_font_align(gt_obj_st * textarea, gt_align_et align)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;

    // NOTE : textarea only support left align
    align = GT_ALIGN_LEFT;

    style->font_align = align;
}

void gt_textarea_set_font_color(gt_obj_st * textarea, gt_color_t color)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    uint16_t idx = 0;
    style->color_font = color;
    while (idx < style->cnt_contents) {
#if GT_TEXTAREA_CUSTOM_FONT_STYLE
        style->contents[idx].font_info.palette = color;
#else
        style->contents[idx].color = color;
#endif
        idx ++;
    }
    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_textarea_set_bg_color(gt_obj_st * textarea, gt_color_t color)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    textarea->bgcolor = color;
    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_textarea_set_bg_opa(gt_obj_st * textarea, uint8_t opa)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->bg_opa = opa;
    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_textarea_set_border_width(gt_obj_st * textarea, uint8_t width)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->border_width = width;
    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_textarea_set_border_color(gt_obj_st * textarea, gt_color_t color)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->color_border = color;
    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_textarea_set_radius(gt_obj_st * textarea, gt_radius_t radius)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    textarea->radius = radius;
    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
}
#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
void gt_textarea_set_font_family_cn(gt_obj_st * textarea, gt_family_t family)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->font_info.style_cn = family;
}

void gt_textarea_set_font_family_en(gt_obj_st * textarea, gt_family_t family)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->font_info.style_en = family;
}
void gt_textarea_set_font_family_fl(gt_obj_st * textarea, gt_family_t family)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->font_info.style_fl = family;
}
void gt_textarea_set_font_family_numb(gt_obj_st * textarea, gt_family_t family)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->font_info.style_numb = family;
}
#else
void gt_textarea_set_font_family(gt_obj_st * textarea, gt_family_t family)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    gt_font_set_family(&style->font_info, family);
}
void gt_textarea_set_font_cjk(gt_obj_st* textarea, gt_font_cjk_et cjk)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->font_info.cjk = cjk;
}

#endif
void gt_textarea_set_font_thick_en(gt_obj_st * textarea, uint8_t thick)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->font_info.thick_en = thick;
    gt_font_info_update_font_thick(&style->font_info);
}

void gt_textarea_set_font_thick_cn(gt_obj_st * textarea, uint8_t thick)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->font_info.thick_cn = thick;
    gt_font_info_update_font_thick(&style->font_info);
}

void gt_textarea_set_font_encoding(gt_obj_st * textarea, gt_encoding_et encoding)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->font_info.encoding = encoding;
}

void gt_textarea_set_font_info(gt_obj_st * textarea, gt_font_info_st * font_info)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    if (NULL == font_info) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    gt_memcpy(&style->font_info, font_info, sizeof(gt_font_info_st));
}

gt_font_info_st * gt_textarea_get_font_info(gt_obj_st * textarea)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return 0;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    return &style->font_info;
}

uint32_t gt_textarea_get_total_height(gt_obj_st * textarea)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return 0;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    _draw_content_or_get_words(textarea, NULL, true);
    return style->max_height;
}

uint32_t gt_textarea_get_display_offset(gt_obj_st * textarea)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return 0;
    }
    return gt_abs(textarea->process_attr.scroll.y);
}

float gt_textarea_get_scroll_percent(gt_obj_st * textarea)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return 0.0;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
#if GT_TEXTAREA_FILE_MODE
    /* File mode: byte offset determines position, not pixel scroll.
       Avoids int16_t scroll.y overflow when max_height > 32767. */
    if (style->mode != GT_TEXTAREA_MODE_MEMORY && style->file_ctx
        && style->file_ctx->file_total_bytes > 0) {
        uint32_t byte_offset = style->file_ctx->scroll_byte_offset;
        if (style->file_ctx->page_end_offset >= style->file_ctx->file_total_bytes) {
            int32_t max_offset = _file_page_max_pixel_offset(textarea, style);
            if (max_offset <= 0) return 100.0f;
            if (style->file_ctx->pixel_offset > 0) {
                uint32_t page_bytes = style->file_ctx->page_end_offset
                                    - style->file_ctx->scroll_byte_offset;
                byte_offset += (uint32_t)((uint64_t)page_bytes
                             * (uint32_t)style->file_ctx->pixel_offset
                             / (uint32_t)max_offset);
            }
        } else if (style->file_ctx->pixel_offset > 0) {
            uint16_t page_span = _file_page_pixel_span(textarea, style);
            uint32_t page_bytes = style->file_ctx->page_end_offset
                                - style->file_ctx->scroll_byte_offset;
            if (page_span > 0) {
                byte_offset += (uint32_t)((uint64_t)page_bytes
                             * (uint32_t)style->file_ctx->pixel_offset / page_span);
            }
        }
        return 100.0f * (float)byte_offset
             / (float)style->file_ctx->file_total_bytes;
    }
#endif
    int32_t scroll_height = (int32_t)style->max_height - (int32_t)gt_obj_get_h(textarea);
    if (0 == style->max_height) {
        return 0.0;
    }
    if (scroll_height <= 0) {
        return 0.0;
    }
    return (float)(100.0 * gt_abs(textarea->process_attr.scroll.y) / scroll_height);
}

#if GT_TEXTAREA_FILE_MODE
uint32_t gt_textarea_get_file_total_bytes(gt_obj_st * textarea)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return 0;
    }
    _gt_textarea_st * style = (_gt_textarea_st *)textarea;
    if (style->mode == GT_TEXTAREA_MODE_MEMORY || NULL == style->file_ctx) {
        return 0;
    }
    return style->file_ctx->file_total_bytes;
}
#endif

gt_res_t gt_textarea_display_by_precent(gt_obj_st * textarea, float percent)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return GT_RES_FAIL;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    if (percent < 0.0)          { return GT_RES_FAIL; }
    if (percent > 100.0)        { percent = 100.0; }

#if GT_TEXTAREA_FILE_MODE
    if (style->mode != GT_TEXTAREA_MODE_MEMORY && style->file_ctx
        && style->file_ctx->file_total_bytes > 0) {
        _gt_textarea_file_ctx_t *ctx = style->file_ctx;
        uint32_t target_byte = (uint32_t)((double)ctx->file_total_bytes
                                          * (double)percent / 100.0);
        if (target_byte >= ctx->file_total_bytes && ctx->file_total_bytes > 0) {
            target_byte = ctx->file_total_bytes - 1;
        }
        _file_check_layout(textarea, style, ctx);
        if (!_file_page_seek_byte(textarea, style, target_byte)) {
            GT_LOGD(GT_LOG_TAG_GUI,
                    "textarea file seek failed: percent[%d], target[%u]\n",
                    (int32_t)percent, target_byte);
            return GT_RES_FAIL;
        }
        textarea->process_attr.scroll.y = 0;
        ctx->last_scroll_y = 0;
        ctx->pixel_offset = percent >= 100.0
                          ? _file_page_max_pixel_offset(textarea, style) : 0;
        _ensure_buffer(ctx, ctx->scroll_byte_offset);
        gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
        return GT_RES_OK;
    }
#endif

    if (0 == style->max_height) { return GT_RES_FAIL; }

    int32_t scroll_height = (int32_t)style->max_height - (int32_t)gt_obj_get_h(textarea);
    if (scroll_height <= 0) {
        textarea->process_attr.scroll.y = 0;
    } else {
        textarea->process_attr.scroll.y = -(int32_t)(scroll_height * percent / 100.0);
    }

    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
    return GT_RES_OK;
}

#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
void gt_textarea_set_touch_single_chinese_word(gt_obj_st * textarea, bool is_single_cn)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->reg.click_single_cn = is_single_cn ? 1 : 0;
}

gt_font_touch_word_st gt_textarea_get_touch_word(gt_obj_st * textarea)
{
    gt_font_touch_word_st ret = {0};
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ret;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    return style->select_word;
}
#endif

void gt_textarea_set_padding(gt_obj_st * textarea, uint8_t padding)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->padding = padding;
    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
}

gt_split_line_ret_st gt_textarea_get_font_split_line_numb(gt_obj_st* obj, gt_font_split_line_st* sp_line)
{
    gt_split_line_ret_st s_line_ret = {0};

    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return s_line_ret;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )obj;
    uint16_t idx = 0, max_w = 0;

    gt_font_split_line_st sp_line_tmp;
    if (sp_line) {
        sp_line_tmp.indent = sp_line->indent;
        sp_line_tmp.max_w = sp_line->max_w;
        sp_line_tmp.space = sp_line->space;
        sp_line_tmp.start_w = sp_line->start_w;
    }else{
        sp_line_tmp.indent = style->indent;
        sp_line_tmp.max_w = obj->area.w;
        sp_line_tmp.start_w = obj->area.w;
        sp_line_tmp.space = style->space_x;
    }
    gt_split_line_ret_st s_line_tmp_ret = {0};
    s_line_tmp_ret.disp_w = sp_line_tmp.start_w;

    while (idx < style->cnt_contents) {

        sp_line_tmp.text = style->contents[idx].text;
        sp_line_tmp.len = style->contents[idx].len;
        sp_line_tmp.indent = idx == 0 ? sp_line_tmp.indent : 0;
        if(0 == sp_line_tmp.len) {
            ++idx;
            continue;
        }

        s_line_tmp_ret = gt_font_split_line_numb(&style->font_info, &sp_line_tmp);
        sp_line_tmp.start_w = s_line_tmp_ret.disp_w;

        if(s_line_tmp_ret.line_numb > 1) {
            s_line_ret.line_numb += s_line_tmp_ret.line_numb - 1;
            s_line_ret.max_w = GT_MIN(s_line_tmp_ret.max_w, sp_line_tmp.max_w);
        }
        else if(s_line_tmp_ret.disp_w < style->font_info.size && idx != style->cnt_contents -1){
            s_line_ret.line_numb += 1;
            s_line_ret.max_w = GT_MIN(s_line_tmp_ret.max_w, sp_line_tmp.max_w);
            sp_line_tmp.start_w = sp_line_tmp.max_w;
        }
        else if(0x0A == sp_line_tmp.text[sp_line_tmp.len-1]) {
            s_line_ret.line_numb += (idx == style->cnt_contents-1) ? s_line_tmp_ret.line_numb - 1 : s_line_tmp_ret.line_numb;
            s_line_ret.max_w = GT_MIN(s_line_tmp_ret.max_w, sp_line_tmp.max_w);
            sp_line_tmp.start_w = sp_line_tmp.max_w;
        }

        ++idx;
    }

    s_line_ret.max_w = GT_MAX(s_line_ret.max_w, s_line_tmp_ret.max_w);
    s_line_ret.line_numb += 1;

    return s_line_ret;
}

#if 0
void gt_textarea_append_text(gt_obj_st * obj, const char * str)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )obj;

#if 01
    if(style->cnt_contents > 0) {
        char * text = style->contents[0].text;
        gt_textarea_set_text(obj, "%s%s", text, str);
    }else{
        gt_textarea_set_text(obj, "%s", str);
    }
#else
    gt_textarea_add_str(obj, (char* )str, GT_FONT_STYLE_NONE, style->color_font);
#endif
}
#endif

void gt_textarea_set_start_point_offset(gt_obj_st * textarea, uint8_t x_off, uint8_t y_off)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->start_x_off = x_off;
    style->start_y_off = y_off;
}

#if GT_TEXTAREA_FILE_MODE

gt_obj_st *gt_textarea_create_with_file(gt_obj_st *parent, const char *file_path) {
    gt_obj_st *obj = gt_textarea_create(parent);
    if (NULL == obj) return NULL;
    if (0 != gt_textarea_set_file(obj, file_path)) return obj;
    return obj;
}

int gt_textarea_set_file(gt_obj_st *textarea, const char *file_path) {
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) return -1;
    if (NULL == file_path) return -1;
    _gt_textarea_st *style = (_gt_textarea_st *)textarea;

    if (style->file_ctx) { _file_ctx_deinit(style->file_ctx); style->file_ctx = NULL; }
    _free_contents(textarea);

    style->file_ctx = _file_ctx_init(file_path, style->file_buf_size_cfg, NULL);
    if (NULL == style->file_ctx) { style->mode = GT_TEXTAREA_MODE_MEMORY; return -1; }
    style->file_ctx->word_wrap = style->file_word_wrap;
    style->file_ctx->editable  = style->file_editable;
    style->mode      = GT_TEXTAREA_MODE_FILE_VFS;
    style->max_height = 0;
    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
    return 0;
}

int gt_textarea_set_file_with_io(gt_obj_st *textarea, const char *file_path,
                                  const gt_textarea_file_io_t *io) {
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) return -1;
    if (NULL == file_path || NULL == io) return -1;
    if (NULL == io->io_open || NULL == io->io_read || NULL == io->io_seek || NULL == io->io_close)
        return -1;
    _gt_textarea_st *style = (_gt_textarea_st *)textarea;

    if (style->file_ctx) { _file_ctx_deinit(style->file_ctx); style->file_ctx = NULL; }
    _free_contents(textarea);

    style->file_ctx = _file_ctx_init(file_path, style->file_buf_size_cfg, io);
    if (NULL == style->file_ctx) { style->mode = GT_TEXTAREA_MODE_MEMORY; return -1; }
    style->file_ctx->word_wrap = style->file_word_wrap;
    style->file_ctx->editable  = style->file_editable;
    style->mode      = GT_TEXTAREA_MODE_FILE_IO;
    style->max_height = 0;
    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
    return 0;
}

int gt_textarea_close_file(gt_obj_st *textarea) {
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) return -1;
    _gt_textarea_st *style = (_gt_textarea_st *)textarea;
    if (NULL == style->file_ctx) return 0;
    _file_ctx_deinit(style->file_ctx);
    style->file_ctx = NULL;
    style->mode = GT_TEXTAREA_MODE_MEMORY;
    style->max_height = 0;
    _reset_scroll_value(textarea);
    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
    return 0;
}

int gt_textarea_save_to_file(gt_obj_st *textarea) {
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) return -1;
    _gt_textarea_st *style = (_gt_textarea_st *)textarea;
    if (NULL == style->file_ctx || !style->file_ctx->dirty) return 0;
    if (style->file_ctx->io_backend == 1
        && NULL == style->file_ctx->io.custom.io.io_write) return -1;

    _file_seek(style->file_ctx, 0, 0);
    uint32_t write_sz = (style->file_ctx->buf_size < style->file_ctx->file_total_bytes)
                        ? style->file_ctx->buf_size : style->file_ctx->file_total_bytes;
    int wr = _file_write(style->file_ctx, style->file_ctx->read_buf, write_sz);
    if (wr < 0) return -1;
    style->file_ctx->dirty = false;
    return 0;
}

gt_textarea_mode_t gt_textarea_get_mode(gt_obj_st *textarea) {
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) return GT_TEXTAREA_MODE_MEMORY;
    return ((_gt_textarea_st *)textarea)->mode;
}

void gt_textarea_set_word_wrap(gt_obj_st *textarea, bool enable) {
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) return;
    _gt_textarea_st *style = (_gt_textarea_st *)textarea;
    style->file_word_wrap = enable;
    if (style->file_ctx) style->file_ctx->word_wrap = enable;
}

void gt_textarea_set_file_editable(gt_obj_st *textarea, bool editable) {
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) return;
    _gt_textarea_st *style = (_gt_textarea_st *)textarea;
    style->file_editable = editable;
    if (style->file_ctx) style->file_ctx->editable = editable;
}

int gt_textarea_set_file_buf_size(gt_obj_st *textarea, uint16_t size) {
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) return -1;
    _gt_textarea_st *style = (_gt_textarea_st *)textarea;
    if (style->file_ctx != NULL) return -1;          /* already in file mode */
    if (size < 512 || size > 8192) return -1;         /* out of range */
    if ((size & (size - 1)) != 0) return -1;           /* not power of 2 */
    style->file_buf_size_cfg = size;
    return 0;
}

#endif  /* GT_TEXTAREA_FILE_MODE */

#endif  /** GT_CFG_ENABLE_TEXTAREA */
/* end ------------------------------------------------------------------*/
