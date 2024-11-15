/**
 * @file gt_serial_var.h
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2024-09-23 17:13:08
 * @copyright Copyright (c) 2014-2024, Company Genitop. Co., Ltd.
 */
#ifndef _GT_SERIAL_VAR_H_
#define _GT_SERIAL_VAR_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../../gt_conf.h"
#include "../../utils/gt_serial.h"
#include "../../others/gt_types.h"
#include "../../widgets/gt_obj.h"

#if GT_USE_SERIAL
/* define ---------------------------------------------------------------*/
#ifndef GT_USE_SERIAL_VAR
    /**
     * @brief Serial var
     *      [default: 0]
     */
    #define GT_USE_SERIAL_VAR    01
#endif
#endif  /** GT_USE_SERIAL */
//
#if GT_USE_SERIAL_VAR && GT_USE_BIN_CONVERT
//
#define GT_VAR_REG_LEN  2
#define GT_VAR_IS_VP(reg, vp) ((reg) == (vp))
#define GT_VAR_IS_SP(reg, sp, len_dsc) (((reg) >= (sp)) && ((reg) < (sp) + (len_dsc)))
#define GT_VAR_GET_SP_REG(reg) (reg & 0x7F)
#define GT_VAR_IS_SYS_CONFIG(reg) ((reg) < (0x1000))
/* typedef --------------------------------------------------------------*/

typedef enum{
    VAR_TYPE_ICON           = 0x00,
    VAR_TYPE_ANIM_ICON      = 0x01,
    VAR_TYPE_SLIDER         = 0x02,
    VAR_TYPE_ARTISTIC       = 0x03,
    VAR_TYPE_PIC            = 0x04,
    VAR_TYPE_ICON_ROTATE    = 0x05,     // unrealized
    VAR_TYPE_BIT            = 0x06,
    VAR_TYPE_ICON_PAN       = 0x07,     // unrealized
    VAR_TYPE_ICON_SUP       = 0x08,     // unrealized
    VAR_TYPE_DATA_COPY      = 0x09,     // unrealized

    VAR_TYPE_NUMB           = 0x10,
    VAR_TYPE_TEXT           = 0x11,
    VAR_TYPE_RTC            = 0x12,
    VAR_TYPE_HEX            = 0x13,

    VAR_TYPE_ROLLER         = 0x15,

    VAR_TYPE_PROGRESS_BAR   = 0x23,

    VAR_TYPE_QR_CODE        = 0x25,
    VAR_TYPE_AREA_LIGHT     = 0x26,
    VAR_TYPE_VALUE_PASSING  = 0x30,

    VAR_TYPE_INPUT          = 0x40,
    VAR_TYPE_KEYPAD         = 0x41,

}gt_var_type_et;

typedef struct{
    uint8_t prio;
    gt_var_type_et type; // @ gt_var_type_et
    uint16_t sp;
    uint16_t len_dsc;
    uint16_t vp;
}gt_var_head_st;

typedef struct{
    gt_obj_st* obj;
    uint8_t* buffer;
    uint32_t len;
    gt_id_t id;
}gt_var_st;

typedef struct gt_serial_pack_buffer_s gt_var_value_st;


typedef gt_res_t (*create_cb_t)(gt_var_st* );
typedef gt_res_t (*set_value_cb_t)(gt_var_st* , gt_var_value_st* );

typedef struct{
    gt_var_type_et type;
    create_cb_t create_cb;
    set_value_cb_t set_value_cb;
}gt_var_handler_item_st;

typedef struct {
    const gt_var_handler_item_st* list;
    gt_var_st* var;
    gt_var_value_st *value;
    uint32_t count;
    uint8_t type;
}gt_var_handler_st;



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief init serial var
 * @param load true: load 22 file; false: load 22 file;
 *
 */
void gt_serial_var_init(bool load);

/**
 * @brief get var buffer pointer
 *
 * @param reg
 * @param res_ptr get the pointer to the buffer
 * @param len
 * @return uint16_t
 */
uint16_t gt_serial_var_buffer_get_pointer(uint16_t reg, const uint8_t** res_ptr, uint16_t len);
/**
 * @brief get var buffer
 *
 * @param reg   is sp or vp
 * @param data
 * @param len byte length
 * @return int [0] : success [-1] : error
 */
int gt_serial_var_buffer_get_reg(uint16_t reg, uint8_t* data, uint16_t len);

/**
 * @brief
 *
 * @param reg
 * @param eof_value
 * @return uint32_t The byte length of the data
 */
uint32_t gt_serial_var_buffer_get_len_by_eof(uint16_t reg, uint16_t eof_value);

/**
 * @brief get var buffer
 *
 * @param value
 * @param data
 * @param len
 * @return int [0] : success [-1] : error
 */
int gt_serial_var_buffer_get(gt_var_value_st* value, uint8_t* data, uint16_t len);

/**
 * @brief set var buffer
 *
 * @param reg   is sp or vp
 * @param data
 * @param len
 * @return int [0] : success [-1] : error
 */
int gt_serial_var_buffer_set_reg(uint16_t reg, uint8_t* data, uint16_t len);

/**
 * @brief set var buffer
 *
 * @param value
 * @return int [0] : success [-1] : error
 */
int gt_serial_var_buffer_set(gt_var_value_st* value);


/**
 * @brief Check value is sys config
 *
 * @param value
 * @return int [0] : is sys config [-1] : not sys config
 */
int gt_serial_var_check_value_is_sys_config(gt_var_value_st* value);


/**
 * @brief Check value is set this variable
 *
 * @param var
 * @param value
 * @return int  [-1]: error  [0]: is vp reg  [0x80|X]: is sp reg @ GT_VAR_GET_SP_REG
 */
int gt_serial_var_check_value_is_set_this(gt_var_st *var, gt_var_value_st* value);

/**
 * @brief parse var head cb
 *
 * @param user_data
 * @param buffer
 * @param len
 * @return gt_res_t [GT_RES_OK] : success [other] : error
 */
gt_res_t gt_serial_var_head_parse(void * user_data, uint8_t const * const buffer, uint32_t len);
//
/**
 * @brief Create var
 *
 * @param var
 * @return gt_res_t [GT_RES_OK] : success [other] : error
 */
gt_res_t gt_serial_var_create(gt_var_st* var);

/**
 * @brief set var value
 *
 * @param var
 * @param value buffer: 0x0000(vp / sp) + (0x00 ...)data, len: byte length
 * @return gt_res_t [GT_RES_OK] : success [other] : error
 */
gt_res_t gt_serial_var_set_value(gt_var_st* var, gt_var_value_st* value);

/**
 * @brief get var value
 *
 * @param value
 * @param res_buffer read data in to res_buffer
 * @return uint16_t read len
 */
uint16_t gt_serial_var_get_value(gt_var_value_st * value, uint8_t * res_buffer);

#endif  /** GT_USE_SERIAL_SHOW && GT_USE_BIN_CONVERT */
//
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_SERIAL_VAR_H_


/* end of file ----------------------------------------------------------*/


