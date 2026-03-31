/**
 * @file gt_example_textarea.c
 * @author Feyoung
 * @brief Textarea widget example
 * @version 0.1
 * @date 2026-03-27
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_textarea.h"

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/

/* global functions / API interface -------------------------------------*/

/**
 * @brief Create textarea with mixed style text.
 */
gt_obj_st * gt_examples_textarea(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);

    gt_obj_st * ta = gt_textarea_create(screen);
    gt_obj_set_pos(ta, 10, 10);
    gt_obj_set_size(ta, 300, 200);
    gt_textarea_set_font_size(ta, 16);
    gt_textarea_set_font_color(ta, gt_color_black());
    gt_textarea_set_bg_color(ta, gt_color_hex(0xF5F5F5));
    gt_textarea_set_border_width(ta, 1);
    gt_textarea_set_border_color(ta, gt_color_hex(0xCCCCCC));
    gt_textarea_set_radius(ta, 4);
    gt_textarea_set_space(ta, 0, 4);

    /* Page 1 - Introduction */
    gt_textarea_set_text(ta, "=== GT-HMI-Engine Textarea ===\n");
    gt_textarea_add_str(ta, "Rich text with mixed styles.\n\n", GT_FONT_STYLE_NONE, gt_color_hex(0x666666));

    /* Page 1 - Bold / italic / underline */
    gt_textarea_add_str(ta, "Style Demo:\n", GT_FONT_STYLE_BOLD, gt_color_hex(0x0055CC));
    gt_textarea_add_str(ta, "Bold: ", GT_FONT_STYLE_BOLD, gt_color_black());
    gt_textarea_add_str(ta, "important keyword\n", GT_FONT_STYLE_BOLD, gt_color_hex(0xCC0000));
    gt_textarea_add_str(ta, "Italic: ", GT_FONT_STYLE_NONE, gt_color_black());
    gt_textarea_add_str(ta, "emphasis text\n", GT_FONT_STYLE_ITALIC, gt_color_hex(0x007700));
    gt_textarea_add_str(ta, "Normal: plain readable text\n\n", GT_FONT_STYLE_NONE, gt_color_black());

    /* Page 2 - Multi-line paragraph */
    gt_textarea_add_str(ta, "--- Paragraph ---\n", GT_FONT_STYLE_BOLD, gt_color_hex(0x884400));
    gt_textarea_add_str(ta, "The quick brown fox jumps over\n", GT_FONT_STYLE_NONE, gt_color_black());
    gt_textarea_add_str(ta, "the lazy dog. Pack my box with\n", GT_FONT_STYLE_NONE, gt_color_black());
    gt_textarea_add_str(ta, "five dozen liquor jugs.\n\n", GT_FONT_STYLE_NONE, gt_color_black());

    /* Page 2 - Color spectrum */
    gt_textarea_add_str(ta, "--- Colors ---\n", GT_FONT_STYLE_BOLD, gt_color_hex(0x884400));
    gt_textarea_add_str(ta, "Red   ", GT_FONT_STYLE_BOLD, gt_color_hex(0xFF2222));
    gt_textarea_add_str(ta, "Green   ", GT_FONT_STYLE_BOLD, gt_color_hex(0x22AA22));
    gt_textarea_add_str(ta, "Blue   ", GT_FONT_STYLE_BOLD, gt_color_hex(0x2255FF));
    gt_textarea_add_str(ta, "Orange\n\n", GT_FONT_STYLE_BOLD, gt_color_hex(0xFF8800));

    /* Page 3 - Log-style output */
    gt_textarea_add_str(ta, "--- Log Output ---\n", GT_FONT_STYLE_BOLD, gt_color_hex(0x884400));
    gt_textarea_add_str(ta, "[INFO]  ", GT_FONT_STYLE_BOLD, gt_color_hex(0x0088CC));
    gt_textarea_add_str(ta, "System initialized OK\n", GT_FONT_STYLE_NONE, gt_color_black());
    gt_textarea_add_str(ta, "[WARN]  ", GT_FONT_STYLE_BOLD, gt_color_hex(0xFF8800));
    gt_textarea_add_str(ta, "Memory usage above 80%%\n", GT_FONT_STYLE_NONE, gt_color_black());
    gt_textarea_add_str(ta, "[ERROR] ", GT_FONT_STYLE_BOLD, gt_color_hex(0xFF2222));
    gt_textarea_add_str(ta, "Device not responding\n", GT_FONT_STYLE_NONE, gt_color_black());
    gt_textarea_add_str(ta, "[DEBUG] ", GT_FONT_STYLE_BOLD, gt_color_hex(0x888888));
    gt_textarea_add_str(ta, "Frame rate: 60 fps\n\n", GT_FONT_STYLE_NONE, gt_color_hex(0x444444));

    /* Page 3 - Numeric data */
    gt_textarea_add_str(ta, "--- Sensor Data ---\n", GT_FONT_STYLE_BOLD, gt_color_hex(0x884400));
    gt_textarea_add_str(ta, "Temp:     ", GT_FONT_STYLE_NONE, gt_color_hex(0x555555));
    gt_textarea_add_str(ta, "36.5 C\n", GT_FONT_STYLE_BOLD, gt_color_hex(0xFF4400));
    gt_textarea_add_str(ta, "Humidity: ", GT_FONT_STYLE_NONE, gt_color_hex(0x555555));
    gt_textarea_add_str(ta, "62 %%\n", GT_FONT_STYLE_BOLD, gt_color_hex(0x0077CC));
    gt_textarea_add_str(ta, "Pressure: ", GT_FONT_STYLE_NONE, gt_color_hex(0x555555));
    gt_textarea_add_str(ta, "1013 hPa\n", GT_FONT_STYLE_BOLD, gt_color_hex(0x007744));
    gt_textarea_add_str(ta, "Battery:  ", GT_FONT_STYLE_NONE, gt_color_hex(0x555555));
    gt_textarea_add_str(ta, "87 %%\n\n", GT_FONT_STYLE_BOLD, gt_color_hex(0x44AA00));

    /* Page 4 - Network status */
    gt_textarea_add_str(ta, "--- Network ---\n", GT_FONT_STYLE_BOLD, gt_color_hex(0x884400));
    gt_textarea_add_str(ta, "SSID:    ", GT_FONT_STYLE_NONE, gt_color_hex(0x555555));
    gt_textarea_add_str(ta, "GT-HMI-AP\n", GT_FONT_STYLE_BOLD, gt_color_black());
    gt_textarea_add_str(ta, "IP:      ", GT_FONT_STYLE_NONE, gt_color_hex(0x555555));
    gt_textarea_add_str(ta, "192.168.1.100\n", GT_FONT_STYLE_BOLD, gt_color_hex(0x0055CC));
    gt_textarea_add_str(ta, "Signal:  ", GT_FONT_STYLE_NONE, gt_color_hex(0x555555));
    gt_textarea_add_str(ta, "-52 dBm  Good\n", GT_FONT_STYLE_BOLD, gt_color_hex(0x22AA22));
    gt_textarea_add_str(ta, "TX/RX:   ", GT_FONT_STYLE_NONE, gt_color_hex(0x555555));
    gt_textarea_add_str(ta, "1.2 MB / 3.4 MB\n\n", GT_FONT_STYLE_NONE, gt_color_black());

    /* Page 4 - Task list */
    gt_textarea_add_str(ta, "--- Task List ---\n", GT_FONT_STYLE_BOLD, gt_color_hex(0x884400));
    gt_textarea_add_str(ta, "[x] ", GT_FONT_STYLE_BOLD, gt_color_hex(0x22AA22));
    gt_textarea_add_str(ta, "Initialize display driver\n", GT_FONT_STYLE_NONE, gt_color_black());
    gt_textarea_add_str(ta, "[x] ", GT_FONT_STYLE_BOLD, gt_color_hex(0x22AA22));
    gt_textarea_add_str(ta, "Load font resources\n", GT_FONT_STYLE_NONE, gt_color_black());
    gt_textarea_add_str(ta, "[x] ", GT_FONT_STYLE_BOLD, gt_color_hex(0x22AA22));
    gt_textarea_add_str(ta, "Register input device\n", GT_FONT_STYLE_NONE, gt_color_black());
    gt_textarea_add_str(ta, "[ ] ", GT_FONT_STYLE_BOLD, gt_color_hex(0xAAAAAA));
    gt_textarea_add_str(ta, "Connect to server\n", GT_FONT_STYLE_NONE, gt_color_hex(0x888888));
    gt_textarea_add_str(ta, "[ ] ", GT_FONT_STYLE_BOLD, gt_color_hex(0xAAAAAA));
    gt_textarea_add_str(ta, "Sync configuration\n\n", GT_FONT_STYLE_NONE, gt_color_hex(0x888888));

    /* Page 5 - Event log */
    gt_textarea_add_str(ta, "--- Event Log ---\n", GT_FONT_STYLE_BOLD, gt_color_hex(0x884400));
    gt_textarea_add_str(ta, "08:01:02  ", GT_FONT_STYLE_NONE, gt_color_hex(0x888888));
    gt_textarea_add_str(ta, "Boot complete\n", GT_FONT_STYLE_NONE, gt_color_black());
    gt_textarea_add_str(ta, "08:01:05  ", GT_FONT_STYLE_NONE, gt_color_hex(0x888888));
    gt_textarea_add_str(ta, "Touch panel ready\n", GT_FONT_STYLE_NONE, gt_color_black());
    gt_textarea_add_str(ta, "08:01:10  ", GT_FONT_STYLE_NONE, gt_color_hex(0x888888));
    gt_textarea_add_str(ta, "UI rendered in 42 ms\n", GT_FONT_STYLE_NONE, gt_color_black());
    gt_textarea_add_str(ta, "08:02:33  ", GT_FONT_STYLE_NONE, gt_color_hex(0x888888));
    gt_textarea_add_str(ta, "User tapped button #3\n", GT_FONT_STYLE_NONE, gt_color_black());
    gt_textarea_add_str(ta, "08:03:17  ", GT_FONT_STYLE_NONE, gt_color_hex(0x888888));
    gt_textarea_add_str(ta, "Screen brightness: 80%%\n", GT_FONT_STYLE_NONE, gt_color_black());
    gt_textarea_add_str(ta, "08:05:00  ", GT_FONT_STYLE_NONE, gt_color_hex(0x888888));
    gt_textarea_add_str(ta, "Auto-sleep triggered\n\n", GT_FONT_STYLE_NONE, gt_color_hex(0xFF8800));

    /* Page 5 - Version info */
    gt_textarea_add_str(ta, "--- Version Info ---\n", GT_FONT_STYLE_BOLD, gt_color_hex(0x884400));
    gt_textarea_add_str(ta, "Engine:  ", GT_FONT_STYLE_NONE, gt_color_hex(0x555555));
    gt_textarea_add_str(ta, "GT-HMI-Engine v2.1\n", GT_FONT_STYLE_BOLD, gt_color_black());
    gt_textarea_add_str(ta, "Build:   ", GT_FONT_STYLE_NONE, gt_color_hex(0x555555));
    gt_textarea_add_str(ta, "2026-03-31 release\n", GT_FONT_STYLE_NONE, gt_color_black());
    gt_textarea_add_str(ta, "Chip:    ", GT_FONT_STYLE_NONE, gt_color_hex(0x555555));
    gt_textarea_add_str(ta, "Cortex-M7 @ 480 MHz\n", GT_FONT_STYLE_NONE, gt_color_black());
    gt_textarea_add_str(ta, "Flash:   ", GT_FONT_STYLE_NONE, gt_color_hex(0x555555));
    gt_textarea_add_str(ta, "2 MB  RAM: 1 MB\n\n", GT_FONT_STYLE_NONE, gt_color_black());

    /* End marker */
    gt_textarea_add_str(ta, "--- End of Demo ---\n", GT_FONT_STYLE_BOLD, gt_color_hex(0x884400));
    gt_textarea_add_str(ta, "Scroll up to review all content.\n", GT_FONT_STYLE_ITALIC, gt_color_hex(0x888888));

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
