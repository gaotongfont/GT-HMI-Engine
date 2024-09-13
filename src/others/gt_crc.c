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

static bool crc8_init_flag         = false;
static GT_ATTRIBUTE_LARGE_RAM_ARRAY uint8_t crc8_table[256] = {0};

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
    #define _crc16_calc(crc, val)     		((crc >> 8) ^ crc16_table[ (crc ^ (uint16_t)val) & 0x00FF ])
#endif  /** GT_ENABLED_CRC16 */

#if GT_ENABLED_CRC32
    #define _crc32_calc(crc, val)     		((crc >> 8) ^ crc32_table[ (crc ^ (uint32_t)val) & 0x000000FFul ])
#endif  /** GT_ENABLED_CRC32 */

/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
#if GT_ENABLED_CRC8
static void init_crc8_tab(void) {
	uint16_t i, j;
	uint8_t crc;

	for (i = 0; i < 256; ++i) {
		crc = i;
		for (j = 0; j < 8; ++j) {
			if (crc & 0x80) crc = (crc << 1) ^ GT_CRC8_POLY;
			else            crc = (crc << 1);
		}
		crc8_table[i] = crc;
	}
	crc8_init_flag = true;
}
#endif	/** GT_ENABLED_CRC8 */

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
	crc32_init_flag = true;
}
#endif  /** GT_ENABLED_CRC32 */

/* global functions / API interface -------------------------------------*/
#if GT_ENABLED_CRC8
uint8_t gt_crc8(const uint8_t * input_str, size_t num_bytes)
{
	const uint8_t * ptr = input_str;
	size_t a = 0;
	uint8_t crc = GT_CRC8_START;

	if ( ! crc8_init_flag ) init_crc8_tab();

	if (NULL == ptr) { return crc; }
    for (; a < num_bytes; ++a) {
		crc = crc8_table[(*ptr++) ^ crc];
	}
	return crc;
}

uint8_t gt_update_crc8(uint8_t crc, uint8_t val) {
	return crc8_table[val ^ crc];
}
#endif  /** GT_ENABLED_CRC8 */

#if GT_ENABLED_CRC16
uint16_t gt_crc16(const uint8_t * input_str, size_t num_bytes)
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

uint16_t gt_update_crc16(uint16_t crc, uint8_t c)
{
	if ( ! crc16_init_flag ) init_crc16_tab();

	return _crc16_calc(crc, c);
}
#endif  /** GT_ENABLED_CRC16 */

#if GT_ENABLED_CRC32
uint32_t gt_crc32(const uint8_t * input_str, size_t num_bytes)
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

uint32_t gt_update_crc32(uint32_t crc, uint8_t c)
{
	return _crc32_calc(crc, c);
}
#endif  /** GT_ENABLED_CRC32 */


/* end ------------------------------------------------------------------*/

#endif  /** GT_USE_CRC */
