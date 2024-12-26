/**
 * @file gt_types.h
 * @author yongg
 * @brief The global data type defined
 * @version 0.1
 * @date 2022-05-12 16:27:54
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_TYPES_H_
#define _GT_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "stdint.h"
#include "stdbool.h"

/* define ---------------------------------------------------------------*/

// If __UINTPTR_MAX__ or UINTPTR_MAX are available, use them to determine arch size
#if defined(__UINTPTR_MAX__) && __UINTPTR_MAX__ > 0xFFFFFFFF
#define GT_ARCH_64

#elif defined(UINTPTR_MAX) && UINTPTR_MAX > 0xFFFFFFFF
#define GT_ARCH_64

// Otherwise use compiler-dependent means to determine arch size
#elif defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__) || defined (__aarch64__)
#define GT_ARCH_64

#endif

#define REDUCE_DEFAULT     (2)

/* typedef --------------------------------------------------------------*/

#if defined(__cplusplus) || __STDC_VERSION__ >= 199901L
    /*If c99 or newer,  use the definition of uintptr_t directly from <stdint.h>*/
    typedef uintptr_t gt_uintptr_t;
    typedef intptr_t gt_intptr_t;
#else
/*Otherwise, use the arch size determination*/
    #ifdef GT_ARCH_64
        typedef uint64_t gt_uintptr_t;
        typedef int64_t gt_intptr_t;
    #else
        typedef uint32_t gt_uintptr_t;
        typedef int32_t gt_intptr_t;
    #endif
#endif

typedef uint32_t gt_addr_t;

/**
 * @brief screen id
 */
typedef int16_t gt_scr_id_t;

typedef int16_t gt_size_t;

typedef float gt_float_t;

/**
 * @brief Font family type
 */
typedef uint16_t gt_family_t;

/**
 * @brief color opacity value
 */
typedef uint8_t gt_opt_t;

/**
 * @brief widget id
 */
typedef int16_t gt_id_t;

/**
 * @brief Screen stack size
 */
typedef int16_t gt_stack_size_t;

/**
 * @brief The header index type
 */
typedef int16_t gt_file_header_idx_t;

/**
 * @brief radius unit
 */
typedef uint16_t gt_radius_t;

/**
 * @brief layout property value
 */
typedef uint16_t gt_layout_prop_t;

/**
 * @brief The gap size value
 */
typedef uint8_t gt_gap_t;

typedef uint8_t gt_font_width_t;

/**
 * @brief The screen turn next screen animation type
 */
typedef enum gt_scr_anim_type_e {
    GT_SCR_ANIM_TYPE_NONE = 0,
    GT_SCR_ANIM_TYPE_MOVE_LEFT,     // Move the interface to the left
    GT_SCR_ANIM_TYPE_MOVE_RIGHT,    // Move the interface to the right
    GT_SCR_ANIM_TYPE_MOVE_UP,       // Move the interface to the up
    GT_SCR_ANIM_TYPE_MOVE_DOWN,     // Move the interface to the down

    GT_SCR_ANIM_TYPE_COVER_LEFT,    // Cover the interface from the left
    GT_SCR_ANIM_TYPE_COVER_RIGHT,   // Cover the interface from the right
    GT_SCR_ANIM_TYPE_COVER_UP,      // Cover the interface from the up
    GT_SCR_ANIM_TYPE_COVER_DOWN,    // Cover the interface from the down

    GT_SCR_ANIM_TYPE_TOTAL,
}gt_scr_anim_type_et;

/**
 * @brief widget type enum definition
 */
typedef enum gt_obj_type_e {
    GT_TYPE_UNKNOWN     = -3,       ///< unknown type
    GT_TYPE_LAYER_TOP   = -2,       ///< [CORE USED] layer top, used to display floating widgets
    GT_TYPE_SCREEN      = -1,       ///< screen type
    GT_TYPE_OBJ         = 0,        ///< base obj type
    GT_TYPE_BTN         = 1,        ///< button type
    GT_TYPE_IMG         = 2,        ///< img type
    GT_TYPE_LAB         = 3,        ///< label type
    GT_TYPE_TEXTAREA    = 4,        ///< textarea type
    GT_TYPE_CANVAS      = 5,        ///< canvas
    GT_TYPE_OPTION      = 6,        ///< option
    GT_TYPE_GROUP       = 7,        ///< group
    GT_TYPE_CHECKBOX    = 8,        ///< checkbox
    GT_TYPE_RADIO       = 9,        ///< radio box
    GT_TYPE_INPUT       = 10,       ///< input
    GT_TYPE_SWITCH      = 11,       ///< switch
    GT_TYPE_SLIDER      = 12,       ///< slider
    GT_TYPE_PROCESS_BAR = 13,       ///< progress bar
    GT_TYPE_KEYPAD      = 14,       ///< keypad
    GT_TYPE_IMGBTN      = 15,       ///< imgbtn
    GT_TYPE_BARCODE     = 16,       ///< barcode
    GT_TYPE_QRCODE      = 17,       ///< qrcode
    GT_TYPE_LINE        = 18,       ///< line
    GT_TYPE_PLAYER      = 19,       ///< player
    GT_TYPE_INPUT_NUMBER= 20,       ///< input number
    GT_TYPE_CLOCK       = 21,       ///< clock
    GT_TYPE_WORDART     = 22,       ///< wordart
    GT_TYPE_VIEW_PAGER  = 23,       ///< view pager
    GT_TYPE_GIF         = 24,       ///< gif
    GT_TYPE_RECT        = 25,       ///< rect
    GT_TYPE_LISTVIEW    = 26,       ///< listview
    GT_TYPE_TEMPLATE    = 27,       ///< template
    GT_TYPE_ROLLER      = 28,       ///< roller
    GT_TYPE_BTNMAP      = 29,       ///< button map
    GT_TYPE_DIALOG      = 30,       ///< dialog, floating widget
    GT_TYPE_CHAT        = 31,       ///< chat
    GT_TYPE_STATUS_BAR  = 32,       ///< status bar, floating widget
    GT_TYPE_GRAPH       = 33,       ///< graph
    GT_TYPE_MEDIA_PLAYER= 34,       ///< media player
    GT_TYPE_BIT_IMG     = 35,       ///< bit img
    GT_TYPE_ARC         = 36,       ///< arc
    GT_TYPE_SELECT      = 37,       ///< select
    GT_TYPE_TABLE       = 38,       ///< table
    GT_TYPE_MARKDOWN    = 39,       ///< markdown

    GT_TYPE_TOTAL,                  ///< count total of type
}gt_obj_type_et;

/**
 * @brief graphs type enum definition
 */
typedef enum gt_graphs_type_e {
    GT_GRAPHS_TYPE_POINT = 0,
    GT_GRAPHS_TYPE_LINE,
    GT_GRAPHS_TYPE_CURVE,
    GT_GRAPHS_TYPE_BAR,
}gt_graphs_type_et;

typedef enum gt_brush_type_e {
    GT_BRUSH_TYPE_ROUND = 0,
    GT_BRUSH_TYPE_RECT,
}gt_brush_type_et;

enum gt_layout_type_e {
    GT_LAYOUT_TYPE_FIXED = 0,
    GT_LAYOUT_TYPE_FLEX,

    _GT_LAYOUT_TYPE_TOTAL,
};

typedef gt_layout_prop_t gt_layout_type_t;

enum gt_layout_flex_direction_e {
    GT_LAYOUT_FLEX_DIR_ROW = 0,
    GT_LAYOUT_FLEX_DIR_ROW_REVERSE,
    GT_LAYOUT_FLEX_DIR_COLUMN,
    GT_LAYOUT_FLEX_DIR_COLUMN_REVERSE,

    _GT_LAYOUT_FLEX_DIR_TOTAL,
};

typedef gt_layout_prop_t gt_layout_flex_direction_t;

enum gt_layout_justify_content_e {
    GT_LAYOUT_JUSTIFY_CONTENT_START = 0,        /** left to right, or top to bottom */
    GT_LAYOUT_JUSTIFY_CONTENT_END,              /** right to left, or bottom to top */
    GT_LAYOUT_JUSTIFY_CONTENT_CENTER,           /** the center expend to both sides */
    GT_LAYOUT_JUSTIFY_CONTENT_SPACE_BETWEEN,    /** space between, Gap = item_count - 1 */
    GT_LAYOUT_JUSTIFY_CONTENT_SPACE_AROUND,     /** space around, Gap = item_count */
    GT_LAYOUT_JUSTIFY_CONTENT_SPACE_EVENLY,     /** space evenly, Gap = item_count + 1 */

    _GT_LAYOUT_JUSTIFY_CONTENT_TOTAL,
};

typedef gt_layout_prop_t gt_layout_justify_content_t;

enum gt_layout_align_items_e {
    GT_LAYOUT_ALIGN_ITEMS_START = 0,    /** top or left border align */
    GT_LAYOUT_ALIGN_ITEMS_END,          /** bottom or right border align */
    GT_LAYOUT_ALIGN_ITEMS_CENTER,       /** center align */

    _GT_LAYOUT_ALIGN_ITEMS_TOTAL,
};

typedef gt_layout_prop_t gt_layout_align_items_t;

enum gt_layout_shrink_e {
    GT_LAYOUT_SHRINK_DISABLE = 0,   /** All items will be display full size */
    GT_LAYOUT_SHRINK_ENABLE,        /** All items display area limited by parent area */
};

typedef gt_layout_prop_t gt_layout_shrink_t;

typedef struct gt_point_s {
    gt_size_t x;
    gt_size_t y;
}gt_point_st;

typedef struct gt_point_f_s {
    gt_float_t x;
    gt_float_t y;
}gt_point_f_st;

typedef struct gt_padding_s {
    gt_size_t left;
    gt_size_t right;
    gt_size_t top;
    gt_size_t bottom;
}gt_padding_st;

/**
 * @brief return status
 */
typedef enum {
    GT_RES_FAIL = -1,   ///< failed
    GT_RES_OK   = 0,    ///< success
    GT_RES_INV  = 1,    ///< invalid
}gt_res_t;

/**
 * @brief area information
 */
typedef struct gt_area_s {
    gt_size_t x;    ///< x position
    gt_size_t y;    ///< y position
    uint16_t w;     ///< The width of area
    uint16_t h;     ///< The height of area
}gt_area_st;

typedef struct gt_range_s {
    gt_float_t start;
    gt_float_t end;
}gt_range_st;

typedef struct gt_area_abs_s {
    gt_size_t left;
    gt_size_t right;
    gt_size_t top;
    gt_size_t bottom;
}gt_area_abs_st;

/**
 * @brief widget selected state
 */
typedef enum {
    GT_STATE_NONE = 0,
    GT_STATE_PRESSED,   // pressed
}gt_state_et;


/* macros ---------------------------------------------------------------*/

#define _GT_CONCAT3(x, y, z) x ## y ## z
#define GT_CONCAT3(x, y, z) _GT_CONCAT3(x, y, z)

#define _GT_CONCAT(x, y) x ## y
#define GT_CONCAT(x, y) _GT_CONCAT(x, y)

#define GT_UNUSED(_val) ((void)_val)

/* global functions / API interface -------------------------------------*/

static inline uint32_t gt_area_get_size(gt_area_st area){ return (area.w  * area.h); }

/**
 * @brief area reduction
 *
 * @param area gt_area_st
 * @param r_pix Reduced pixel
 * @return gt_area_st
 */
static inline gt_area_st gt_area_reduce(gt_area_st area , gt_size_t r_pix)
{
    gt_area_st box_area = area;
    box_area.x += r_pix;
    box_area.y += r_pix;
    box_area.w -= (r_pix << 1);
    box_area.h -= (r_pix << 1);
    return box_area;
}

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_TYPES_H_
