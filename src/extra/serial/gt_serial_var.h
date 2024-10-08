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
    VAR_TYPE_ICON = 0,
    VAR_TYPE_ANIM_ICON,
    VAR_TYPE_SLIDER,

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
 *
 */
void gt_serial_var_init(void);
/**
 * @brief get var buffer
 *
 * @param reg   is sp or vp
 * @param data
 * @param len
 * @return int [0] : success [-1] : error
 */
int gt_serial_var_buffer_get_reg(uint16_t reg, uint8_t* data, uint16_t len);

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
 * @param value
 * @return gt_res_t [GT_RES_OK] : success [other] : error
 */
gt_res_t gt_serial_var_set_value(gt_var_st* var, gt_var_value_st* value);

#endif  /** GT_USE_SERIAL_SHOW && GT_USE_BIN_CONVERT */
//
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_SERIAL_VAR_H_


/* end of file ----------------------------------------------------------*/


