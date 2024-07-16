#include "gifdec.h"

#if GT_USE_GIF
#include "../../gt_conf.h"
#include "../../core/gt_mem.h"
#include "../../others/gt_log.h"
#include "../../others/gt_types.h"
#include "../../others/gt_color.h"

#include <string.h>

#define _GIF_HEADER_OFFSET 0
#define _GIF_VERSION_OFFSET 3
#define _GIF_WIDTH_OFFSET 6
#define _GIF_HEIGHT_OFFSET 8
#define _GIF_COLOR_TABLE_SIZE_OFFSET 10
#define _GIF_BACKGROUND_COLOR_OFFSET 11
#define _GIF_ASPACT_RATIO_OFFSET 12

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))

typedef struct Entry {
    uint16_t length;
    uint16_t prefix;
    uint8_t  suffix;
} Entry;

typedef struct Table {
    int bulk;
    int nentries;
    Entry *entries;
} Table;


/* =================== driver begin =================== */
static inline gt_fs_fp_st * _gif_open(const char * path, gt_fs_mode_et mode) {
    return gt_fs_open(path, mode);
}

#if GT_USE_FILE_HEADER
static inline gt_fs_fp_st * _fh_gif_open(gt_file_header_param_st const * const fh, gt_fs_mode_et mode) {
    return gt_fs_fh_open(fh, mode);
}
#endif

static inline int _gif_read(gt_fs_fp_st * fp, uint8_t * buffer, uint32_t len) {
    uint32_t ret_len = 0;
    gt_fs_read(fp, buffer, len, &ret_len);
    return ret_len;
}

static inline int _gif_seek(gt_fs_fp_st * fp, uint32_t offset, gt_fs_whence_et whence) {
    uint32_t ret_len = 0;
    gt_fs_seek(fp, offset, whence);
    if (GT_FS_RES_NULL == gt_fs_tell(fp, &ret_len)) {
        return -1;
    }
    return ret_len;
}

static inline void _gif_close(gt_fs_fp_st * fp) {
    gt_fs_close(fp);
}

static inline uint16_t _get_uint16(uint8_t const * const buffer) {
    return (buffer[1] << 8) | buffer[0];
}

static uint16_t read_num(gt_fs_fp_st * fp) {
    uint8_t bytes[2];

    _gif_read(fp, bytes, 2);
    return bytes[0] + (((uint16_t) bytes[1]) << 8);
}

static gd_GIF * _common_open_gif(gd_GIF * gif) {
    uint8_t fdsz, aspect;
    uint8_t *bgcolor;
    uint8_t buffer[13] = {0};
    uint32_t size = 0;

    _gif_read(gif->fd, buffer, sizeof(buffer));

    /* Header */
    if (memcmp(&buffer[_GIF_HEADER_OFFSET], "GIF", 3)) {
        GT_LOGD("GIF", "invalid signature");
        goto fail;
    }
    if (memcmp(&buffer[_GIF_VERSION_OFFSET], "89a", 3)) {
        GT_LOGD("GIF", "invalid version");
        goto fail;
    }
    /* Width x Height */
    gif->width = _get_uint16(&buffer[_GIF_WIDTH_OFFSET]);
    gif->height = _get_uint16(&buffer[_GIF_HEIGHT_OFFSET]);
    size = gif->width * gif->height;

    /* FDSZ */
    fdsz = buffer[_GIF_COLOR_TABLE_SIZE_OFFSET];
    /* Presence of GCT */
    if (!(fdsz & 0x80)) {
        GT_LOGD("GIF", "no global color table");
        goto fail;
    }
    /* Color Space's Depth */
    gif->depth = ((fdsz >> 4) & 7) + 1;
    /* Ignore Sort Flag. */
    /* GCT Size */
    /* Read GCT */
    gif->gct.size = 1 << ((fdsz & 0x07) + 1);
    /* Background Color Index */
    gif->bgindex = buffer[_GIF_BACKGROUND_COLOR_OFFSET];
    /* Aspect Ratio */
    aspect = buffer[_GIF_ASPACT_RATIO_OFFSET];
    /* Create gd_GIF Structure. */
    _gif_read(gif->fd, gif->gct.colors, 3 * gif->gct.size);
    gif->palette = &gif->gct;

#if GT_COLOR_DEPTH == 32
    gif->frame = gt_mem_malloc(size * 5);
#elif GT_COLOR_DEPTH == 16
    gif->frame = gt_mem_malloc(size * 4);
#elif GT_COLOR_DEPTH == 8 || GT_COLOR_DEPTH == 1
    gif->frame = gt_mem_malloc(size * 3);
#endif
    if (!gif->frame) {
        goto fail;
    }

    gif->canvas = &gif->frame[size];
#if GT_COLOR_DEPTH == 32
    gif->mask = &gif->frame[size * 4];
#elif GT_COLOR_DEPTH == 16
    gif->mask = &gif->frame[size * 3];
#elif GT_COLOR_DEPTH == 8 || GT_COLOR_DEPTH == 1
    gif->mask = &gif->frame[size * 2];
#endif
    if (gif->bgindex) {
        memset(gif->frame, gif->bgindex, size);
    }
    bgcolor = &gif->palette->colors[gif->bgindex * 3];
    if (bgcolor[0] || bgcolor[1] || bgcolor [2]) {
        gt_color_t color;
        for (uint32_t i = 0, cnt = 0; i < size; i++) {
#if 01
#if GT_COLOR_DEPTH == 32
            cnt = i * 3;
            gif->canvas[cnt]     = bgcolor[2];
            gif->canvas[cnt + 1] = bgcolor[1];
            gif->canvas[cnt + 2] = bgcolor[0];
            gif->mask[i]         = GT_OPA_100;
#elif GT_COLOR_DEPTH == 16
            cnt = i << 1;
            color = gt_color_make(bgcolor[0], bgcolor[1], bgcolor[2]);
            gif->canvas[cnt]     = color.full & 0xff;
            gif->canvas[cnt + 1] = color.full >> 8;
            gif->mask[i]         = GT_OPA_100;
#elif GT_COLOR_DEPTH == 8
            color = gt_color_make(bgcolor[0], bgcolor[1], bgcolor[2]);
            gif->canvas[i] = color.full;
            gif->mask[i]   = GT_OPA_100;
#elif GT_COLOR_DEPTH == 1
            color = gt_color_make(bgcolor[0], bgcolor[1], bgcolor[2]);
            gif->canvas[i] = color.full > 128 ? 1 : 0;
            gif->mask[i]   = GT_OPA_100;
#endif

#else
            memcpy(&gif->canvas[i*3], bgcolor, 3);
#endif
        }
    }
    gif->anim_start = _gif_seek(gif->fd, 0, GT_FS_SEEK_CUR);

    return gif;
fail:
    _gif_close(gif->fd);
    gif->fd = NULL;
    return NULL;
}

/* =================== driver end =================== */

gd_GIF *
gd_open_gif(const char *fname)
{
    gd_GIF * gif = gt_mem_malloc(sizeof(gd_GIF));
    if (!gif) return NULL;
    gt_memset(gif, 0, sizeof(gd_GIF));

    gif->fd = _gif_open(fname, GT_FS_MODE_RD);
    if (!gif->fd) {
        goto free_gif;
    }
    return _common_open_gif(gif);
free_gif:
    gt_mem_free(gif);
    return NULL;
}

#if GT_USE_FILE_HEADER
gd_GIF *
gd_fh_open_gif(gt_file_header_param_st const * const file_header)
{
    gd_GIF * gif = gt_mem_malloc(sizeof(gd_GIF));
    if (!gif) return NULL;
    gt_memset(gif, 0, sizeof(gd_GIF));

    gif->fd = _fh_gif_open(file_header, GT_FS_MODE_RD);
    if (!gif->fd) {
        goto free_gif;
    }
    return _common_open_gif(gif);
free_gif:
    gt_mem_free(gif);
    return NULL;
}
#endif

static void
discard_sub_blocks(gd_GIF *gif)
{
    uint8_t size;

    do {
        _gif_read(gif->fd, &size, 1);
        _gif_seek(gif->fd, size, GT_FS_SEEK_CUR);
    } while (size);
}

static void
read_plain_text_ext(gd_GIF *gif)
{
    if (gif->plain_text) {
        uint8_t buffer[12] = {0};
        uint16_t tx, ty, tw, th;
        uint8_t cw, ch, fg, bg;
        uint32_t sub_block;
        _gif_seek(gif->fd, 1, GT_FS_SEEK_CUR); /* block size = 12 */
        _gif_read(gif->fd, buffer, sizeof(buffer));
        tx = _get_uint16(&buffer[0]);
        ty = _get_uint16(&buffer[2]);
        tw = _get_uint16(&buffer[4]);
        th = _get_uint16(&buffer[6]);
        cw = buffer[8];
        ch = buffer[9];
        fg = buffer[10];
        bg = buffer[11];
        sub_block = _gif_seek(gif->fd, 0, GT_FS_SEEK_CUR);
        gif->plain_text(gif, tx, ty, tw, th, cw, ch, fg, bg);
        _gif_seek(gif->fd, sub_block, GT_FS_SEEK_SET);
    } else {
        /* Discard plain text metadata. */
        _gif_seek(gif->fd, 13, GT_FS_SEEK_CUR);
    }
    /* Discard plain text sub-blocks. */
    discard_sub_blocks(gif);
}

static void
read_graphic_control_ext(gd_GIF *gif)
{
    uint8_t rdit;
    uint8_t buffer[4] = {0};

    /* Discard block size (always 0x04). */
    _gif_seek(gif->fd, 1, GT_FS_SEEK_CUR);
    _gif_read(gif->fd, buffer, sizeof(buffer));
    rdit = buffer[0];
    gif->gce.disposal = (rdit >> 2) & 3;
    gif->gce.input = rdit & 2;
    gif->gce.transparency = rdit & 1;
    // gif->gce.delay = read_num(gif->fd);
    gif->gce.delay = _get_uint16(&buffer[1]);
    gif->gce.tindex = buffer[3];
    /* Skip block terminator. */
    _gif_seek(gif->fd, 1, GT_FS_SEEK_CUR);
}

static void
read_comment_ext(gd_GIF *gif)
{
    if (gif->comment) {
        uint32_t sub_block = _gif_seek(gif->fd, 0, GT_FS_SEEK_CUR);
        gif->comment(gif);
        _gif_seek(gif->fd, sub_block, GT_FS_SEEK_SET);
    }
    /* Discard comment sub-blocks. */
    discard_sub_blocks(gif);
}

static void
read_application_ext(gd_GIF *gif)
{
    char app_id[8];
    char app_auth_code[3];
    uint8_t buffer[11] = {0};

    /* Discard block size (always 0x0B). */
    _gif_seek(gif->fd, 1, GT_FS_SEEK_CUR);
    _gif_read(gif->fd, buffer, sizeof(buffer));
    /* Application Identifier. */
    gt_memcpy(app_id, buffer, sizeof(app_id));
    /* Application Authentication Code. */
    gt_memcpy(app_auth_code, &buffer[sizeof(app_id)], sizeof(app_auth_code));

    if (!strncmp(app_id, "NETSCAPE", sizeof(app_id))) {
        /* Discard block size (0x03) and constant byte (0x01). */
        _gif_seek(gif->fd, 2, GT_FS_SEEK_CUR);
        gif->loop_count = read_num(gif->fd);
        /* Skip block terminator. */
        _gif_seek(gif->fd, 1, GT_FS_SEEK_CUR);
    } else if (gif->application) {
        uint32_t sub_block = _gif_seek(gif->fd, 0, GT_FS_SEEK_CUR);
        gif->application(gif, app_id, app_auth_code);
        _gif_seek(gif->fd, sub_block, GT_FS_SEEK_SET);
        discard_sub_blocks(gif);
    } else {
        discard_sub_blocks(gif);
    }
}

static void
read_ext(gd_GIF *gif)
{
    uint8_t label;

    _gif_read(gif->fd, &label, 1);
    switch (label) {
    case 0x01:
        read_plain_text_ext(gif);
        break;
    case 0xF9:
        read_graphic_control_ext(gif);
        break;
    case 0xFE:
        read_comment_ext(gif);
        break;
    case 0xFF:
        read_application_ext(gif);
        break;
    default:
        GT_LOGD("GIF", "unknown extension: %02X", label);
    }
}

static Table *
new_table(int key_size)
{
    int key;
    int init_bulk = MAX(1 << (key_size + 1), 0x100);
    Table *table = gt_mem_malloc(sizeof(*table) + sizeof(Entry) * init_bulk);
    if (table) {
        table->bulk = init_bulk;
        table->nentries = (1 << key_size) + 2;
        table->entries = (Entry *) &table[1];
        for (key = 0; key < (1 << key_size); key++)
            table->entries[key] = (Entry) {1, 0xFFF, key};
    }
    return table;
}

/* Add table entry. Return value:
 *  0 on success
 *  +1 if key size must be incremented after this addition
 *  -1 if could not realloc table */
static int
add_entry(Table **tablep, uint16_t length, uint16_t prefix, uint8_t suffix)
{
    Table *table = *tablep;
    if (table->nentries == table->bulk) {
        table->bulk *= 2;
        table = gt_mem_realloc(table, sizeof(*table) + sizeof(Entry) * table->bulk);
        if (!table) return -1;
        table->entries = (Entry *) &table[1];
        *tablep = table;
    }
    Entry * entry = &table->entries[table->nentries];
    entry->length = length;
    entry->prefix = prefix;
    entry->suffix = suffix;
    ++table->nentries;
    if ((table->nentries & (table->nentries - 1)) == 0)
        return 1;
    return 0;
}

static uint16_t
get_key(gd_GIF *gif, int key_size, uint8_t *sub_len, uint8_t *shift, uint8_t *byte)
{
    int bits_read;
    int rpad;
    int frag_size;
    uint16_t key;

    key = 0;
    for (bits_read = 0; bits_read < key_size; bits_read += frag_size) {
        rpad = (*shift + bits_read) % 8;
        if (rpad == 0) {
            /* Update byte. */
            if (*sub_len == 0) {
                _gif_read(gif->fd, sub_len, 1); /* Must be nonzero! */
                if (*sub_len == 0)
                    return 0x1000;
            }
            _gif_read(gif->fd, byte, 1);
            (*sub_len)--;
        }
        frag_size = MIN(key_size - bits_read, 8 - rpad);
        key |= ((uint16_t) ((*byte) >> rpad)) << bits_read;
    }
    /* Clear extra bits to the left. */
    key &= (1 << key_size) - 1;
    *shift = (*shift + key_size) % 8;
    return key;
}

/* Compute output index of y-th input line, in frame of height h. */
static int
interlaced_line_index(int h, int y)
{
    int p; /* number of lines in current pass */

    p = (h - 1) / 8 + 1;
    if (y < p) /* pass 1 */
        return y * 8;
    y -= p;
    p = (h - 5) / 8 + 1;
    if (y < p) /* pass 2 */
        return y * 8 + 4;
    y -= p;
    p = (h - 3) / 4 + 1;
    if (y < p) /* pass 3 */
        return y * 4 + 2;
    y -= p;
    /* pass 4 */
    return y * 2 + 1;
}

/* Decompress image pixels.
 * Return 0 on success or -1 on out-of-memory (w.r.t. LZW code table). */
static int
read_image_data(gd_GIF *gif, int interlace)
{
    uint8_t sub_len, shift, byte;
    int init_key_size, key_size, table_is_full = 0;
    int frm_off, frm_size, str_len = 0, i, p, x, y;
    uint16_t key, clear, stop;
    int ret;
    Table *table;
    Entry entry = {0};
    uint32_t start, end;

    _gif_read(gif->fd, &byte, 1);
    key_size = (int) byte;
    if (key_size < 2 || key_size > 8) {
        return -1;
    }

    start = _gif_seek(gif->fd, 0, GT_FS_SEEK_CUR);
    discard_sub_blocks(gif);
    end = _gif_seek(gif->fd, 0, GT_FS_SEEK_CUR);
    _gif_seek(gif->fd, start, GT_FS_SEEK_SET);
    clear = 1 << key_size;
    stop = clear + 1;
    table = new_table(key_size);
    key_size++;
    init_key_size = key_size;
    sub_len = shift = 0;
    key = get_key(gif, key_size, &sub_len, &shift, &byte); /* clear code */
    frm_off = 0;
    ret = 0;
    frm_size = gif->fw*gif->fh;
    while (frm_off < frm_size) {
        if (key == clear) {
            key_size = init_key_size;
            table->nentries = (1 << (key_size - 1)) + 2;
            table_is_full = 0;
        } else if (!table_is_full) {
            ret = add_entry(&table, str_len + 1, key, entry.suffix);
            if (ret == -1) {
                gt_mem_free(table);
                return -1;
            }
            if (table->nentries == 0x1000) {
                ret = 0;
                table_is_full = 1;
            }
        }
        key = get_key(gif, key_size, &sub_len, &shift, &byte);
        if (key == clear) continue;
        if (key == stop || key == 0x1000) break;
        if (ret == 1) key_size++;
        entry = table->entries[key];
        str_len = entry.length;
        for (i = 0; i < str_len; i++) {
            p = frm_off + entry.length - 1;
            x = p % gif->fw;
            y = p / gif->fw;
            if (interlace)
                y = interlaced_line_index((int) gif->fh, y);
            gif->frame[(gif->fy + y) * gif->width + gif->fx + x] = entry.suffix;
            if (entry.prefix == 0xFFF)
                break;
            else
                entry = table->entries[entry.prefix];
        }
        frm_off += str_len;
        if (key < table->nentries - 1 && !table_is_full)
            table->entries[table->nentries - 1].suffix = entry.suffix;
    }
    gt_mem_free(table);
    if (key == stop) {
        _gif_read(gif->fd, &sub_len, 1); /* Must be zero! */
    }
    _gif_seek(gif->fd, end, GT_FS_SEEK_SET);
    return 0;
}

/* Read image.
 * Return 0 on success or -1 on out-of-memory (w.r.t. LZW code table). */
static int
read_image(gd_GIF *gif)
{
    uint8_t buffer[9] = {0};
    uint8_t fisrz;
    int interlace;

    /* Image Descriptor. */
    _gif_read(gif->fd, buffer, sizeof(buffer));
    gif->fx = _get_uint16(&buffer[0]);
    gif->fy = _get_uint16(&buffer[2]);

    if (gif->fx >= gif->width || gif->fy >= gif->height)
        return -1;

    gif->fw = _get_uint16(&buffer[4]);
    gif->fh = _get_uint16(&buffer[6]);

    gif->fw = MIN(gif->fw, gif->width - gif->fx);
    gif->fh = MIN(gif->fh, gif->height - gif->fy);

    fisrz = buffer[8];
    interlace = fisrz & 0x40;
    /* Ignore Sort Flag. */
    /* Local Color Table? */
    if (fisrz & 0x80) {
        /* Read LCT */
        gif->lct.size = 1 << ((fisrz & 0x07) + 1);
        _gif_read(gif->fd, gif->lct.colors, 3 * gif->lct.size);
        gif->palette = &gif->lct;
    } else
        gif->palette = &gif->gct;
    /* Image Data. */
    return read_image_data(gif, interlace);
}

static void
render_frame_rect(gd_GIF *gif, uint8_t *buffer, uint8_t * mask)
{
    int i, j, k;
    uint8_t index, *color;
    uint32_t cnt = 0;
    gt_color_t c;

    i = gif->fy * gif->width + gif->fx;
    for (j = 0; j < gif->fh; j++) {
        for (k = 0; k < gif->fw; k++) {
            index = gif->frame[(gif->fy + j) * gif->width + gif->fx + k];
            color = &gif->palette->colors[index * 3];
            if (!gif->gce.transparency || index != gif->gce.tindex) {
#if GT_COLOR_DEPTH == 32
            cnt = (i + k) * 3;
            buffer[cnt]     = color[2];
            buffer[cnt + 1] = color[1];
            buffer[cnt + 2] = color[0];
            mask[i + k]     = GT_OPA_100;
#elif GT_COLOR_DEPTH == 16
            cnt = (i + k) << 1;
            c = gt_color_make(color[0], color[1], color[2]);
            buffer[cnt]     = c.full & 0xff;
            buffer[cnt + 1] = c.full >> 8;
            mask[i + k]     = GT_OPA_100;
#elif GT_COLOR_DEPTH == 8
            cnt = i + k;
            c = gt_color_make(color[0], color[1], color[2]);
            buffer[cnt] = c.full;
            mask[cnt]   = GT_OPA_100;
#elif GT_COLOR_DEPTH == 1
            cnt = (i + k) << 1;
            uint8_t b = ( color[0] | color[1] | color[2] );
            buffer[cnt] = b > 128 ? 1 : 0;
            buffer[cnt] = GT_OPA_100;

#endif
            }
        }
        i += gif->width;
    }
}

static void
dispose(gd_GIF *gif)
{
    int i, j, k;
    uint8_t *bgcolor;
    uint32_t cnt = 0;
    uint8_t opa = GT_OPA_100;
    gt_color_t color;

    if(gif->gce.transparency) opa = GT_OPA_0;

    switch (gif->gce.disposal) {
    case 2: /* Restore to background color. */
        bgcolor = &gif->palette->colors[gif->bgindex * 3];
        i = gif->fy * gif->width + gif->fx;

        for (j = 0; j < gif->fh; j++) {
            for (k = 0; k < gif->fw; k++) {
#if GT_COLOR_DEPTH == 32
            cnt = (i + k) * 3;
            gif->canvas[cnt]     = bgcolor[2];
            gif->canvas[cnt + 1] = bgcolor[1];
            gif->canvas[cnt + 2] = bgcolor[0];
            gif->mask[i + k]     = opa;
#elif GT_COLOR_DEPTH == 16
            cnt = (i + k) << 1;
            color = gt_color_make(bgcolor[0], bgcolor[1], bgcolor[2]);
            gif->canvas[cnt]     = color.full & 0xff;
            gif->canvas[cnt + 1] = color.full >> 8;
            gif->mask[i + k]     = opa;
#elif GT_COLOR_DEPTH == 8
            cnt = i + k;
            color = gt_color_make(bgcolor[0], bgcolor[1], bgcolor[2]);
            gif->canvas[cnt] = color.full;
            gif->mask[cnt]   = opa;
#elif GT_COLOR_DEPTH == 1
            cnt = i + k;
            uint8_t b = bgcolor[0] | bgcolor[1] | bgcolor[2];
            gif->canvas[cnt] = b > 128 ? 1 : 0;
            gif->mask[cnt]   = opa;
#endif
            }
            i += gif->width;
        }
        break;
    case 3: /* Restore to previous, i.e., don't update canvas.*/
        break;
    default:
        /* Add frame non-transparent pixels to canvas. */
        render_frame_rect(gif, gif->canvas, gif->mask);
        break;
    }
}

/* Return 1 if got a frame; 0 if got GIF trailer; -1 if error. */
int
gd_get_frame(gd_GIF *gif)
{
    char sep;

    dispose(gif);
    _gif_read(gif->fd, (uint8_t * )&sep, 1);
    while (sep != ',') {
        if (sep == ';')
            return 0;
        if (sep == '!')
            read_ext(gif);
        else return -1;
        _gif_read(gif->fd, (uint8_t * )&sep, 1);
    }
    if (read_image(gif) == -1)
        return -1;
    return 1;
}

void
gd_render_frame(gd_GIF *gif, uint8_t *buffer, uint8_t * mask)
{
    render_frame_rect(gif, buffer, mask);
}

int
gd_is_bgcolor(gd_GIF *gif, uint8_t color[3])
{
    return !memcmp(&gif->palette->colors[gif->bgindex*3], color, 3);
}

void
gd_rewind(gd_GIF *gif)
{
    _gif_seek(gif->fd, gif->anim_start, GT_FS_SEEK_SET);
}

void
gd_close_gif(gd_GIF *gif)
{
    gt_mem_free(gif->frame);
    _gif_close(gif->fd);
    gt_mem_free(gif);
}

#endif  /** GT_USE_GIF */
