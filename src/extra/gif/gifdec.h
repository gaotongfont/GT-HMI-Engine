#ifndef GIFDEC_H
#define GIFDEC_H

#include <stdint.h>
#include "../../gt_conf.h"

#if GT_USE_GIF

#include "../../others/gt_types.h"
#include "../../core/gt_fs.h"

#if GT_USE_FILE_HEADER
#include "../../hal/gt_hal_file_header.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct gd_Palette {
    int size;
    uint8_t colors[0x100 * 3];
} gd_Palette;

typedef struct gd_GCE {
    uint16_t delay;
    uint8_t tindex;
    uint8_t disposal;
    int input;
    int transparency;
} gd_GCE;

typedef struct gd_GIF {
    gt_fs_fp_st * fd;
    uint32_t anim_start;
    uint16_t width;
    uint16_t height;
    uint16_t depth;
    uint16_t loop_count;
    gd_GCE gce;
    gd_Palette *palette;
    gd_Palette lct, gct;
    void (*plain_text)(
        struct gd_GIF *gif, uint16_t tx, uint16_t ty,
        uint16_t tw, uint16_t th, uint8_t cw, uint8_t ch,
        uint8_t fg, uint8_t bg
    );
    void (*comment)(struct gd_GIF *gif);
    void (*application)(struct gd_GIF *gif, char id[8], char auth[3]);
    uint16_t fx, fy, fw, fh;
    uint8_t bgindex;
    uint8_t *canvas, *frame, *mask;
} gd_GIF;

gd_GIF * gd_open_gif(const char *fname);

#if GT_USE_FILE_HEADER
gd_GIF * gd_fh_open_gif(gt_file_header_param_st const * const file_header);
#endif

int gd_get_frame(gd_GIF *gif);
void gd_render_frame(gd_GIF *gif, uint8_t *buffer, uint8_t * mask);
int gd_is_bgcolor(gd_GIF *gif, uint8_t color[3]);
void gd_rewind(gd_GIF *gif);
void gd_close_gif(gd_GIF *gif);

#ifdef __cplusplus
}
#endif

#endif  /** GT_USE_GIF */

#endif /* GIFDEC_H */
