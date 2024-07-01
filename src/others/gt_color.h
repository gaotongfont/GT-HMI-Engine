/**
 * @file gt_color.h
 * @author yongg
 * @brief THe color implementation
 * @version 0.1
 * @date 2022-05-13 10:13:45
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_COLOR_H_
#define _GT_COLOR_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "stdint.h"
#include "../gt_conf.h"
#include "gt_types.h"
#include "gt_math.h"
#include "gt_assert.h"

/* define ---------------------------------------------------------------*/

enum {
    GT_OPA_TRANSP = 0,
    GT_OPA_0      = 0,
    GT_OPA_5      = 12,
    GT_OPA_10     = 25,
    GT_OPA_15     = 38,
    GT_OPA_20     = 51,
    GT_OPA_25     = 64,
    GT_OPA_30     = 76,
    GT_OPA_35     = 89,
    GT_OPA_40     = 102,
    GT_OPA_45     = 114,
    GT_OPA_50     = 127,
    GT_OPA_55     = 140,
    GT_OPA_60     = 153,
    GT_OPA_65     = 165,
    GT_OPA_70     = 178,
    GT_OPA_75     = 190,
    GT_OPA_80     = 204,
    GT_OPA_85     = 217,
    GT_OPA_90     = 229,
    GT_OPA_95     = 241,
    GT_OPA_100    = 255,
    GT_OPA_HALF   = GT_OPA_50,
    GT_OPA_COVER  = GT_OPA_100,

    GT_OPA_MIN    = 3,          /* Opacities below this will be transparent */
    GT_OPA_MAX    = 252,        /* Opacities above this will fully cover GT */
};

#if GT_COLOR_DEPTH == 1
#define GT_COLOR_SIZE 8
#elif GT_COLOR_DEPTH == 8
#define GT_COLOR_SIZE 8
#elif GT_COLOR_DEPTH == 16
#define GT_COLOR_SIZE 16
#elif GT_COLOR_DEPTH == 32
#define GT_COLOR_SIZE 32
#else
#error "Invalid GT_COLOR_DEPTH in GT_conf.h! Set it to 1, 8, 16 or 32!"
#endif

// #define GT_COLOR_GET_OBJ(_buf, x, y, w, h)          (_buf[ (int)( (y) * (w) + x) ])
// #define GT_COLOR_CLR_BUF(obj_buf, len, val)         do{int __idx__=0;while(__idx__<len){obj_buf[__idx__].full=val; __idx__++;}}while(0)
// #define GT_COLOR_CPY_BUF(dst, src,len, val)         do{int __idx__=0;while(__idx__<len){dst[__idx__].full=src[__idx__].full;__idx__++;}}while(0)
// #define GT_COLOR_SET_BUF(obj_buf, x, y, w, h, val)  (obj_buf[ (int)((y)*(w) + (x)) ].full = val)
// #define GT_COLOR_GET_BUF(obj_buf, x, y, w, h)       (obj_buf[ (int)((y)*(w) + (x)) ].full)

#define GT_COLOR_SET(obj,val)           (obj.full=val)
#define GT_COLOR_GET(obj)               (obj.full)

#if GT_COLOR_DEPTH == 16 && GT_COLOR_16_SWAP == 1
#define GT_COLOR_SET_RGB(obj,r,g,b)     {obj.ch.green_h=(g>>3);obj.ch.red=(r);obj.ch.blue=(b);obj.ch.green_l=(g & 0x7);}
#else
#define GT_COLOR_SET_RGB(obj,r,g,b)     {obj.ch.red=(r);obj.ch.green=(g);obj.ch.blue=(b);}
#endif

/*---------------------------------------
 * Macros for all existing color depths
 * to set/get values of the color channels
 *------------------------------------------*/
#if GT_COLOR_DEPTH == 1
# define GT_COLOR_SET_R1(c, v) (c).ch.red = (uint8_t)((v) & 0x1)
# define GT_COLOR_SET_G1(c, v) (c).ch.green = (uint8_t)((v) & 0x1)
# define GT_COLOR_SET_B1(c, v) (c).ch.blue = (uint8_t)((v) & 0x1)
# define GT_COLOR_SET_A1(c, v) do {} while(0)

# define GT_COLOR_GET_R1(c) (c).ch.red
# define GT_COLOR_GET_G1(c) (c).ch.green
# define GT_COLOR_GET_B1(c) (c).ch.blue
# define GT_COLOR_GET_A1(c) 0xFF

# define _GT_COLOR_ZERO_INITIALIZER1 {0x00}
# define GT_COLOR_MAKE1(r8, g8, b8)  {(uint8_t)((b8 >> 7) | (g8 >> 7) | (r8 >> 7))}

#elif GT_COLOR_DEPTH == 8
# define GT_COLOR_SET_R8(c, v) (c).ch.red = (uint8_t)((v) & 0x7U)
# define GT_COLOR_SET_G8(c, v) (c).ch.green = (uint8_t)((v) & 0x7U)
# define GT_COLOR_SET_B8(c, v) (c).ch.blue = (uint8_t)((v) & 0x3U)
# define GT_COLOR_SET_A8(c, v) do {} while(0)

# define GT_COLOR_GET_R8(c) (c).ch.red
# define GT_COLOR_GET_G8(c) (c).ch.green
# define GT_COLOR_GET_B8(c) (c).ch.blue
# define GT_COLOR_GET_A8(c) 0xFF

# define _GT_COLOR_ZERO_INITIALIZER8 {{0x00, 0x00, 0x00}}
# define GT_COLOR_MAKE8(r8, g8, b8) {{(uint8_t)((b8 >> 6) & 0x3U), (uint8_t)((g8 >> 5) & 0x7U), (uint8_t)((r8 >> 5) & 0x7U)}}

#elif GT_COLOR_DEPTH == 16
# define GT_COLOR_SET_R16(c, v) (c).ch.red = (uint8_t)((v) & 0x1FU)
#if GT_COLOR_16_SWAP == 0
# define GT_COLOR_SET_G16(c, v) (c).ch.green = (uint8_t)((v) & 0x3FU)
#else
# define GT_COLOR_SET_G16(c, v) {(c).ch.green_h = (uint8_t)(((v) >> 3) & 0x7); (c).ch.green_l = (uint8_t)((v) & 0x7);}
#endif  /** GT_COLOR_16_SWAP */
# define GT_COLOR_SET_B16(c, v) (c).ch.blue = (uint8_t)((v) & 0x1FU)
# define GT_COLOR_SET_A16(c, v) do {} while(0)

# define GT_COLOR_GET_R16(c) (c).ch.red
#if GT_COLOR_16_SWAP == 0
# define GT_COLOR_GET_G16(c) (c).ch.green
#else
# define GT_COLOR_GET_G16(c) (((c).ch.green_h << 3) + (c).ch.green_l)
#endif  /** GT_COLOR_16_SWAP */
# define GT_COLOR_GET_B16(c) (c).ch.blue
# define GT_COLOR_GET_A16(c) 0xFF

#if GT_COLOR_16_SWAP == 0
# define _GT_COLOR_ZERO_INITIALIZER16  {{0x00, 0x00, 0x00}}
# define GT_COLOR_MAKE16(r8, g8, b8) {{(uint8_t)((b8 >> 3) & 0x1FU), (uint8_t)((g8 >> 2) & 0x3FU), (uint8_t)((r8 >> 3) & 0x1FU)}}
#else
# define _GT_COLOR_ZERO_INITIALIZER16 {{0x00, 0x00, 0x00, 0x00}}
# define GT_COLOR_MAKE16(r8, g8, b8) {{(uint8_t)((g8 >> 5) & 0x7U), (uint8_t)((r8 >> 3) & 0x1FU), (uint8_t)((b8 >> 3) & 0x1FU), (uint8_t)((g8 >> 2) & 0x7U)}}
#endif  /** GT_COLOR_16_SWAP */

#endif  /** GT_COLOR_DEPTH */

# define GT_COLOR_SET_R32(c, v) (c).ch.red = (uint8_t)((v) & 0xFF)
# define GT_COLOR_SET_G32(c, v) (c).ch.green = (uint8_t)((v) & 0xFF)
# define GT_COLOR_SET_B32(c, v) (c).ch.blue = (uint8_t)((v) & 0xFF)
# define GT_COLOR_SET_A32(c, v) (c).ch.alpha = (uint8_t)((v) & 0xFF)

# define GT_COLOR_GET_R32(c) (c).ch.red
# define GT_COLOR_GET_G32(c) (c).ch.green
# define GT_COLOR_GET_B32(c) (c).ch.blue
# define GT_COLOR_GET_A32(c) (c).ch.alpha

# define _GT_COLOR_ZERO_INITIALIZER32  {{0x00, 0x00, 0x00, 0x00}}
# define GT_COLOR_MAKE32(r8, g8, b8) {{b8, g8, r8, 0xff}} /*Fix 0xff alpha*/

/*---------------------------------------
 * Macros for the current color depth
 * to set/get values of the color channels
 *------------------------------------------*/
#define GT_COLOR_SET_R(c, v) GT_CONCAT(GT_COLOR_SET_R, GT_COLOR_DEPTH)(c, v)
#define GT_COLOR_SET_G(c, v) GT_CONCAT(GT_COLOR_SET_G, GT_COLOR_DEPTH)(c, v)
#define GT_COLOR_SET_B(c, v) GT_CONCAT(GT_COLOR_SET_B, GT_COLOR_DEPTH)(c, v)
#define GT_COLOR_SET_A(c, v) GT_CONCAT(GT_COLOR_SET_A, GT_COLOR_DEPTH)(c, v)

#define GT_COLOR_GET_R(c) GT_CONCAT(GT_COLOR_GET_R, GT_COLOR_DEPTH)(c)
#define GT_COLOR_GET_G(c) GT_CONCAT(GT_COLOR_GET_G, GT_COLOR_DEPTH)(c)
#define GT_COLOR_GET_B(c) GT_CONCAT(GT_COLOR_GET_B, GT_COLOR_DEPTH)(c)
#define GT_COLOR_GET_A(c) GT_CONCAT(GT_COLOR_GET_A, GT_COLOR_DEPTH)(c)

/*********************************************/
/*                  color                    */
/*********************************************/
#if GT_COLOR_16_SWAP == 0
# define _GT_COLOR_ZERO_INITIALIZER16  {{0x00, 0x00, 0x00}}
# define GT_COLOR_MAKE16(r8, g8, b8) {{(uint8_t)((b8 >> 3) & 0x1FU), (uint8_t)((g8 >> 2) & 0x3FU), (uint8_t)((r8 >> 3) & 0x1FU)}}
#else
# define _GT_COLOR_ZERO_INITIALIZER16 {{0x00, 0x00, 0x00, 0x00}}
# define GT_COLOR_MAKE16(r8, g8, b8) {{(uint8_t)((g8 >> 5) & 0x7U), (uint8_t)((r8 >> 3) & 0x1FU), (uint8_t)((b8 >> 3) & 0x1FU), (uint8_t)((g8 >> 2) & 0x7U)}}
#endif

# define _GT_COLOR_ZERO_INITIALIZER32  {{0x00, 0x00, 0x00, 0x00}}
# define GT_COLOR_MAKE32(r8, g8, b8) {{b8, g8, r8, 0xff}} /*Fix 0xff alpha*/

#define _GT_COLOR_ZERO_INITIALIZER GT_CONCAT(_GT_COLOR_ZERO_INITIALIZER, GT_COLOR_DEPTH)
#define GT_COLOR_MAKE(r8, g8, b8) GT_CONCAT(GT_COLOR_MAKE, GT_COLOR_DEPTH)(r8, g8, b8)

/* Adjust color mix functions rounding. GPUs might calculate color mix (blending) differently.
 * 0: round down, 64: round up from x.75, 128: round up from half, 192: round up from x.25, 254: round up */
#define GT_COLOR_MIX_ROUND_OFS (GT_COLOR_DEPTH == 32 ? 0: 128)

/* typedef --------------------------------------------------------------*/

#if GT_COLOR_DEPTH == 1 || GT_COLOR_DEPTH == 8
typedef uint8_t gt_color_val_t;
#elif GT_COLOR_DEPTH == 16
typedef uint16_t gt_color_val_t;
#elif GT_COLOR_DEPTH == 32
typedef uint32_t gt_color_val_t;
#endif

typedef union {
    union {
        uint8_t blue : 1;
        uint8_t green : 1;
        uint8_t red : 1;
    } ch;
    uint8_t full; /*must be declared first to set all bits of byte via initializer list*/
} gt_color1_t;

typedef union {
    struct {
        uint8_t blue : 2;
        uint8_t green : 3;
        uint8_t red : 3;
    } ch;
    uint8_t full;
} gt_color8_t;

typedef union {
    struct {
#if GT_COLOR_16_SWAP == 0
        uint16_t blue : 5;
        uint16_t green : 6;
        uint16_t red : 5;
#else
        uint16_t green_h : 3;
        uint16_t red : 5;
        uint16_t blue : 5;
        uint16_t green_l : 3;
#endif
    } ch;
    uint16_t full;
} gt_color16_t;

typedef union {
    struct {
        uint8_t blue;
        uint8_t green;
        uint8_t red;
        uint8_t alpha;
    } ch;
    uint32_t full;
} gt_color32_t;

typedef GT_CONCAT3(uint, GT_COLOR_SIZE, _t) gt_color_int_t;
typedef GT_CONCAT3(gt_color, GT_COLOR_DEPTH, _t) gt_color_t;

typedef uint8_t gt_opa_t;


/* macros ---------------------------------------------------------------*/

/* static functions / API inteface-------------------------------------- */
static inline gt_color_t gt_color_make(uint8_t r, uint8_t g, uint8_t b) {
    return (gt_color_t)GT_COLOR_MAKE(r, g, b);
}

/**
 * @brief
 *
 * @param c Must be 24bit RGB888
 * @return gt_color_t
 */
static inline gt_color_t gt_color_hex(uint32_t c) {
    return gt_color_make((uint8_t)((c >> 16) & 0xFF), (uint8_t)((c >> 8) & 0xFF), (uint8_t)(c & 0xFF));
}

/**
 * @brief
 *
 * @param hex_val According to the GT_COLOR_DEPTH width
 * @return gt_color_t
 */
static inline gt_color_t gt_color_set(gt_color_val_t hex_val) {
    gt_color_t ret = {
#if GT_COLOR_16_SWAP == 0
        .full = hex_val,
#else
        .full = ((hex_val & 0xff) << 8) | (hex_val >> 8),
#endif
    };
    return ret;
}

static inline uint32_t gt_color_to32(gt_color_t color) {
#if GT_COLOR_DEPTH == 1
    if(color.full == 0)
        return 0xFF000000;
    else
        return 0xFFFFFFFF;
#elif GT_COLOR_DEPTH == 8
    gt_color32_t ret;
    GT_COLOR_SET_R32(ret, GT_COLOR_GET_R(color) * 36); /*(2^8 - 1)/(2^3 - 1) = 255/7 = 36*/
    GT_COLOR_SET_G32(ret, GT_COLOR_GET_G(color) * 36); /*(2^8 - 1)/(2^3 - 1) = 255/7 = 36*/
    GT_COLOR_SET_B32(ret, GT_COLOR_GET_B(color) * 85); /*(2^8 - 1)/(2^2 - 1) = 255/3 = 85*/
    GT_COLOR_SET_A32(ret, 0xFF);
    return ret.full;
#elif GT_COLOR_DEPTH == 16
    /**
     * The floating point math for conversion is:
     *  valueto = valuefrom * ( (2^bitsto - 1) / (float)(2^bitsfrom - 1) )
     * The faster integer math for conversion is:
     *  valueto = ( valuefrom * multiplier + adder ) >> divisor
     *   multiplier = FLOOR( ( (2^bitsto - 1) << divisor ) / (float)(2^bitsfrom - 1) )
     *
     * Find the first divisor where ( adder >> divisor ) <= 0
     *
     * 5-bit to 8-bit: ( 31 * multiplier + adder ) >> divisor = 255
     * divisor  multiplier  adder  min (0)  max (31)
     *       0           8      7        7       255
     *       1          16     14        7       255
     *       2          32     28        7       255
     *       3          65     25        3       255
     *       4         131     19        1       255
     *       5         263      7        0       255
     *
     * 6-bit to 8-bit: 255 = ( 63 * multiplier + adder ) >> divisor
     * divisor  multiplier  adder  min (0)  max (63)
     *       0           4      3        3       255
     *       1           8      6        3       255
     *       2          16     12        3       255
     *       3          32     24        3       255
     *       4          64     48        3       255
     *       5         129     33        1       255
     *       6         259      3        0       255
     */

    gt_color32_t ret;
    GT_COLOR_SET_R32(ret, (GT_COLOR_GET_R(color) * 263 + 7) >> 5);
    GT_COLOR_SET_G32(ret, (GT_COLOR_GET_G(color) * 259 + 3) >> 6);
    GT_COLOR_SET_B32(ret, (GT_COLOR_GET_B(color) * 263 + 7) >> 5);
    GT_COLOR_SET_A32(ret, 0xFF);
    return ret.full;
#elif GT_COLOR_DEPTH == 32
    return color.full;
#endif
}

/**
 * Get the brightness of a color
 * @param color a color
 * @return the brightness [0..255]
 */
static inline uint8_t gt_color_brightness(gt_color_t color)
{
    gt_color32_t c32;
    c32.full        = gt_color_to32(color);
    uint16_t bright = (uint16_t)(3u * GT_COLOR_GET_R32(c32) + GT_COLOR_GET_B32(c32) + 4u * GT_COLOR_GET_G32(c32));
    return (uint8_t)(bright >> 3);
}

static inline gt_color_t gt_color_white(void) { return gt_color_make(0xff, 0xff, 0xff); }
static inline gt_color_t gt_color_black(void) { return gt_color_make(0x00, 0x00, 0x00); }
static inline gt_color_t gt_color_red(void) { return gt_color_make(0xFF, 0x00, 0x00); }
static inline gt_color_t gt_color_yellow(void) { return gt_color_make(0xFF, 0xFF, 0x00); }
static inline gt_color_t gt_color_blue(void) { return gt_color_make(0x00, 0x00, 0xFF); }
static inline gt_color_t gt_color_gray(void) { return gt_color_make(0x80, 0x80, 0x80); }
static inline gt_color_t gt_color_dark_gray(void) { return gt_color_make(0x40, 0x40, 0x40); }
static inline gt_color_t gt_color_bright_gray(void) { return gt_color_make(0xc0, 0xc0, 0xc0); }
static inline gt_color_t gt_color_orange(void) { return gt_color_make(0xFF, 0xA5, 0x00); }


/**
 * Mix two colors with a given ratio.
 * @param c1 the first color to mix (usually the foreground)
 * @param c2 the second color to mix (usually the background)
 * @param mix The ratio of the colors. 0: full `c2`, 255: full `c1`, 127: half `c1` and half`c2`
 * @return the mixed color
 */
static inline gt_color_t gt_color_mix(gt_color_t c1, gt_color_t c2, uint8_t mix)
{
    gt_color_t ret;

#if 1 == GT_COLOR_DEPTH
    /*GT_COLOR_DEPTH == 1*/
    ret.full = mix > GT_OPA_50 ? c1.full : c2.full;
#elif 16 == GT_COLOR_DEPTH
    if (mix > GT_OPA_MAX) {
        return c1;
    } else if (mix < GT_OPA_MIN) {
        return c2;
    }
    mix = (mix + 4) >> 3;

    #if GT_COLOR_16_SWAP
    c1.full = (GT_COLOR_GET_R16(c1) << 11) | (GT_COLOR_GET_G16(c1) << 5) | GT_COLOR_GET_B16(c1);
    c2.full = (GT_COLOR_GET_R16(c2) << 11) | (GT_COLOR_GET_G16(c2) << 5) | GT_COLOR_GET_B16(c2);
    #endif

    /** https://stackoverflow.com/questions/18937701/combining-two-16-bits-rgb-colors-with-alpha-blending/50012418#50012418 */
    /** 0x7E0F81F = 0b00000111111000001111100000011111 */
    uint32_t fg = ((c1.full << 16) | c1.full) & 0x07E0F81F;
    uint32_t bg = ((c2.full << 16) | c2.full) & 0x07E0F81F;
    bg += ((fg - bg) * mix) >> 5;
    bg &= 0x07E0F81F;

    #if GT_COLOR_16_SWAP
    GT_COLOR_SET_G16(ret, (bg >> 21) & 0x3F);
    GT_COLOR_SET_R16(ret, bg >> 11);
    GT_COLOR_SET_B16(ret, bg);
    #else
    ret.full = (uint16_t)((bg >> 16) | bg);
    #endif
#else
    /* GT_COLOR_DEPTH == 8 or 32 */
    GT_COLOR_SET_R(ret, GT_UDIV255((uint16_t)GT_COLOR_GET_R(c1) * mix + GT_COLOR_GET_R(c2) *
                                   (255 - mix) + GT_COLOR_MIX_ROUND_OFS));
    GT_COLOR_SET_G(ret, GT_UDIV255((uint16_t)GT_COLOR_GET_G(c1) * mix + GT_COLOR_GET_G(c2) *
                                   (255 - mix) + GT_COLOR_MIX_ROUND_OFS));
    GT_COLOR_SET_B(ret, GT_UDIV255((uint16_t)GT_COLOR_GET_B(c1) * mix + GT_COLOR_GET_B(c2) *
                                   (255 - mix) + GT_COLOR_MIX_ROUND_OFS));
    GT_COLOR_SET_A(ret, 0xFF);
#endif

    return ret;
}


/* global functions / API interface -------------------------------------*/

/**
 * @brief fill the color into the buffer
 *
 * @param color_arr which buffer save color.
 * @param len The buffer data size.
 * @param color Which color value
 */
void gt_color_fill(gt_color_t * color_arr, uint32_t len, gt_color_t color);

gt_color_t gt_color_focus(void);
void gt_color_focus_set(uint32_t col);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_COLOR_H_
