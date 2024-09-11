/**
 * @file gt_crc.c
 * @author Feyoung
 * @brief CRC8 CRC16 CRC32 calculation
 * @version 0.1
 * @date 2024-09-11 13:56:20
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "./gt_crc.h"

#if GT_USE_CRC

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/
#if GT_ENABLED_CRC8
/**
 * @brief The SHT75 humidity sensor is capable of calculating an 8 bit CRC checksum to
 *      ensure data integrity. The lookup table crc8_table[] is used to recalculate
 *      the CRC.
 */
static GT_ATTRIBUTE_LARGE_RAM_ARRAY const uint8_t sht75_crc8_table[] = {
	0,   49,  98,  83,  196, 245, 166, 151, 185, 136, 219, 234, 125, 76,  31,  46,
	67,  114, 33,  16,  135, 182, 229, 212, 250, 203, 152, 169, 62,  15,  92,  109,
	134, 183, 228, 213, 66,  115, 32,  17,  63,  14,  93,  108, 251, 202, 153, 168,
	197, 244, 167, 150, 1,   48,  99,  82,  124, 77,  30,  47,  184, 137, 218, 235,
	61,  12,  95,  110, 249, 200, 155, 170, 132, 181, 230, 215, 64,  113, 34,  19,
	126, 79,  28,  45,  186, 139, 216, 233, 199, 246, 165, 148, 3,   50,  97,  80,
	187, 138, 217, 232, 127, 78,  29,  44,  2,   51,  96,  81,  198, 247, 164, 149,
	248, 201, 154, 171, 60,  13,  94,  111, 65,  112, 35,  18,  133, 180, 231, 214,
	122, 75,  24,  41,  190, 143, 220, 237, 195, 242, 161, 144, 7,   54,  101, 84,
	57,  8,   91,  106, 253, 204, 159, 174, 128, 177, 226, 211, 68,  117, 38,  23,
	252, 205, 158, 175, 56,  9,   90,  107, 69,  116, 39,  22,  129, 176, 227, 210,
	191, 142, 221, 236, 123, 74,  25,  40,  6,   55,  100, 85,  194, 243, 160, 145,
	71,  118, 37,  20,  131, 178, 225, 208, 254, 207, 156, 173, 58,  11,  88,  105,
	4,   53,  102, 87,  192, 241, 162, 147, 189, 140, 223, 238, 121, 72,  27,  42,
	193, 240, 163, 146, 5,   52,  103, 86,  120, 73,  26,  43,  188, 141, 222, 239,
	130, 179, 224, 209, 70,  119, 36,  21,  59,  10,  89,  104, 255, 206, 157, 172
};
#endif  /** GT_ENABLED_CRC8 */

#if GT_ENABLED_CRC16

static bool crc16_init_flag         = false;
static GT_ATTRIBUTE_LARGE_RAM_ARRAY uint16_t crc16_table[256] = {0};

#endif  /** GT_ENABLED_CRC16 */

#if GT_ENABLED_CRC32

static bool crc32_init_flag         = false;
static GT_ATTRIBUTE_LARGE_RAM_ARRAY uint32_t crc32_table[256] = {0};

#endif  /** GT_ENABLED_CRC32 */

/* macros ---------------------------------------------------------------*/

#if GT_ENABLED_CRC16
    #define _crc16_calc(crc, val)     ((crc >> 8) ^ crc16_table[ (crc ^ (uint16_t)val) & 0x00FF ])
#endif  /** GT_ENABLED_CRC16 */

#if GT_ENABLED_CRC32
    #define _crc32_calc(crc, val)     ((crc >> 8) ^ crc32_table[ (crc ^ (uint32_t)val) & 0x000000FFul ])
#endif  /** GT_ENABLED_CRC32 */

/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

#if GT_ENABLED_CRC16
/**
 * @brief For optimal performance uses the CRC16 routine a lookup table with values
 * that can be used directly in the XOR arithmetic in the algorithm. This
 * lookup table is calculated by the init_crc16_tab() routine, the first time
 * the CRC function is called.
 */
static void init_crc16_tab(void) {
	uint16_t i, j;
	uint16_t crc;
	uint16_t c;

	for (i = 0; i < 256; ++i) {
		crc = 0;
		c   = i;
		for (j = 0; j < 8; ++j) {
			if ( (crc ^ c) & 0x0001 ) crc = ( crc >> 1 ) ^ GT_CRC16_POLY;
			else                      crc =   crc >> 1;

			c = c >> 1;
		}
		crc16_table[i] = crc;
	}
	crc16_init_flag = true;
}
#endif  /** GT_ENABLED_CRC16 */

#if GT_ENABLED_CRC32
/**
 * @brief For optimal speed, the CRC32 calculation uses a table with pre-calculated
 *      bit patterns which are used in the XOR operations in the program.
 */
void init_crc32_tab(void) {
	uint32_t i, j;
	uint32_t crc;

	for (i = 0; i < 256; ++i) {
		crc = i;
		for (j=0; j<8; j++) {
			if ( crc & 0x00000001L ) crc = ( crc >> 1 ) ^ GT_CRC32_POLY;
			else                     crc =   crc >> 1;
		}
		crc32_table[i] = crc;
	}
}
#endif  /** GT_ENABLED_CRC32 */

/* global functions / API interface -------------------------------------*/
#if GT_ENABLED_CRC8
uint8_t gt_crc_8(const uint8_t * input_str, size_t num_bytes)
{
	const uint8_t * ptr = input_str;
	size_t a = 0;
	uint8_t crc = GT_CRC8_START;

	if (NULL == ptr) { return crc; }
    for (; a < num_bytes; ++a) {
		crc = sht75_crc8_table[(*ptr++) ^ crc];
	}
	return crc;
}

uint8_t gt_update_crc_8(uint8_t crc, uint8_t val) {
	return sht75_crc8_table[val ^ crc];
}
#endif  /** GT_ENABLED_CRC8 */

#if GT_ENABLED_CRC16
uint16_t gt_crc_16(const uint8_t * input_str, size_t num_bytes)
{
	uint16_t crc = GT_CRC16_START;
	const uint8_t * ptr = input_str;
	size_t a = 0;

	if ( ! crc16_init_flag ) init_crc16_tab();

	if (NULL == ptr) { return crc; }
    for (; a < num_bytes; ++a) {
        crc = _crc16_calc(crc, *ptr++);
	}
	return crc;
}

uint16_t gt_crc16_mod_bus(const uint8_t * input_str, size_t num_bytes)
{
	uint16_t crc = GT_CRC16_START_MOD_BUS;
	const uint8_t * ptr = input_str;
	size_t a = 0;

	if ( ! crc16_init_flag ) init_crc16_tab();

	if (NULL == ptr) { return crc; }
	for (; a < num_bytes; ++a) {
        crc = _crc16_calc(crc, *ptr++);
	}
	return crc;
}

uint16_t gt_update_crc_16(uint16_t crc, uint8_t c)
{
	if ( ! crc16_init_flag ) init_crc16_tab();

	return _crc16_calc(crc, c);
}
#endif  /** GT_ENABLED_CRC16 */

#if GT_ENABLED_CRC32
uint32_t gt_crc_32(const uint8_t * input_str, size_t num_bytes)
{
	uint32_t crc = GT_CRC_START_32;
	const uint8_t * ptr = input_str;
	size_t a = 0;

	if ( ! crc32_init_flag ) init_crc32_tab();

	if (NULL == ptr) { return crc; }
    for (a = 0; a < num_bytes; ++a) {
        crc = _crc32_calc(crc, *ptr++);
	}
	return (crc ^ 0xFFFFFFFFul);
}

uint32_t gt_update_crc_32(uint32_t crc, uint8_t c)
{
	return _crc32_calc(crc, c);
}
#endif  /** GT_ENABLED_CRC32 */


/* end ------------------------------------------------------------------*/

#endif  /** GT_USE_CRC */
