/**
 * @file gt_qrcode.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-08-24 16:37:09
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_qrcode.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../font/gt_font.h"
#include "../others/gt_assert.h"
#include "../extra/qrcode/gt_qrcode_src.h"

#if GT_CFG_ENABLE_QRCODE == 1
/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_QRCODE
#define MY_CLASS    &gt_qr_code_class


// buff size
#define _QRDataBuf_SIZE  (4072)//(4072)
#define _QRCodeDataBuf_SIZE   _QRDataBuf_SIZE  // (3706)

/* private typedef ------------------------------------------------------*/


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct _gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_qr_code_class = {
    ._init_cb = _init_cb,
    ._deinit_cb = _deinit_cb,
    ._event_cb = _event_cb,
    .type = OBJ_TYPE,
    .size_instance = (sizeof(gt_obj_st)),
    .size_style = (sizeof(_gt_style_qrcode_st))
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static void _Data_ReMap(unsigned char* getdate,unsigned char* putdata, unsigned char width, unsigned char high, unsigned char style)
{
    unsigned int i,j,wbyte;
    unsigned char i_8;
    wbyte = (width+7)/8;
    if( style == 0 )	//竖置横排 转 横置横排 ( Y-->W )
    {
    for( i = 0; i < high; i++ )
        for( j = 0; j < width; j++ )
        {
            i_8 = i/8;
            if((*(putdata+i_8*width+j)&(0x01<<(i%8))) > 0)
                getdate[wbyte*i+j/8] |= (0x80>>(j%8));
            else
                getdate[wbyte*i+j/8] &= (~(0x80>>(j%8)));
        }
    }
    else if( style == 1 )	//竖置横排 (W--> Y)
    {
        for( i = 0; i < high; i++ )
            for( j = 0; j < width; j++ )
            {
            i_8 = i/8;
            if((*(putdata+wbyte*i+j/8)&(0x80>>(j%8))) > 0)
                getdate[i_8*width+j] |= (0x01<<(i%8));
            else
                getdate[i_8*width+j] &= (~(0x01<<(i%8)));
            }
    }
}


static inline void _gt_qr_code_init_widget(gt_obj_st * qr_code) {
    _gt_style_qrcode_st * style = qr_code->style;
    unsigned char *qr_data_buff = NULL;
    unsigned char *qr_code_buff = NULL;

    qr_data_buff = gt_mem_malloc(_QRDataBuf_SIZE);
    qr_code_buff = gt_mem_malloc(_QRCodeDataBuf_SIZE);
    if(!gt_qrcode_init(style->version , qr_data_buff , qr_code_buff))
    {
        GT_LOGE(GT_LOG_TAG_GUI , "qrcode init err!");
        goto err_ret;
    }
    
    gt_memset_0(qr_data_buff , _QRDataBuf_SIZE);
    gt_memset_0(qr_code_buff , _QRCodeDataBuf_SIZE);

    if(!gt_qrcode_get(Mode_Unicode , style->ec_level , style->mask_patt , style->str , style->str_len))
    {
        GT_LOGE(GT_LOG_TAG_GUI , "get qrcode data err!");
        goto err_ret;
    }
    
    /* base shape */
    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.is_fill       = 1;
    rect_attr.radius        = 0;
    rect_attr.bg_opa        = GT_OPA_100;
    rect_attr.border_width  = 0;
    rect_attr.fg_color      = gt_color_white();
    rect_attr.bg_color      = gt_color_black();
    rect_attr.border_color  = gt_color_black();
    rect_attr.gray = 1;
    rect_attr.data_gray = qr_code_buff;

    qr_code->area.w = 4 * style->version + 17;
    qr_code->area.h = ((qr_code->area.w + 7) >> 3) << 3;
    
    if(  ((qr_code->area.h * qr_code->area.h) >> 3 ) > _QRCodeDataBuf_SIZE )
    {
        GT_LOGE(GT_LOG_TAG_GUI , "get qrcode buff size is small!");
        goto err_ret;
    }
    
    gt_memset_0(qr_code_buff , _QRCodeDataBuf_SIZE);
    _Data_ReMap(qr_code_buff,qr_data_buff, qr_code->area.w, qr_code->area.h, 0 );
    
    qr_code->area.w = ((qr_code->area.w + 7) >> 3) << 3;
    GT_LOGD( GT_LOG_TAG_GUI,"qrcode w = %d , h = %d" , qr_code->area.w , qr_code->area.h);

    // 
    gt_mem_free(qr_data_buff);
    qr_data_buff = NULL;

    GT_LOGD(GT_LOG_TAG_UI , "barcode gray = %d" , rect_attr.gray);
    // 1:base shape
    draw_bg(qr_code->draw_ctx, &rect_attr, &qr_code->area);

err_ret:
    if(qr_data_buff)
    {
        gt_mem_free(qr_data_buff);
        qr_data_buff = NULL;
    }
    if(qr_code_buff)
    {
        gt_mem_free(qr_code_buff);
        qr_code_buff = NULL;
    }
}

/**
 * @brief obj init qr_code widget cb
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");

    _gt_qr_code_init_widget(obj);
}

/**
 * @brief obj deinit cb
 *
 * @param obj
 */
static void _deinit_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "start deinit_cb");
    /* code */
}


/**
 * @brief obj event handler cb
 *
 * @param obj
 * @param e event
 */
static void _event_cb(struct _gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code = gt_event_get_code(e);
    switch(code) {
        case GT_EVENT_TYPE_DRAW_START: {
            GT_LOGD(GT_LOG_TAG_GUI, "start draw");
            if( gt_obj_check_scr(obj) ){
                extern void gt_disp_ref_area(gt_area_st * area);
                gt_disp_ref_area(&obj->area);
            }
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;
        }
        case GT_EVENT_TYPE_DRAW_END: {
            GT_LOGD(GT_LOG_TAG_GUI, "end draw");
            break;
        }
        case GT_EVENT_TYPE_CHANGE_CHILD_REMOVE: {/* remove child from screen but not delete */
            GT_LOGD(GT_LOG_TAG_GUI, "child remove");
			break;
        }
        case GT_EVENT_TYPE_CHANGE_CHILD_DELETE: {/* delete child */
            GT_LOGD(GT_LOG_TAG_GUI, "child delete");
            break;
        }
        case GT_EVENT_TYPE_INPUT_CLICKING: {  /* add clicking style and process clicking event */
            GT_LOGD(GT_LOG_TAG_GUI, "clicking");

            break;
        }
        case GT_EVENT_TYPE_INPUT_SCROLL: {
            GT_LOGD(GT_LOG_TAG_GUI, "scroll");
            break;
        }
        case GT_EVENT_TYPE_INPUT_PROCESS: {  /* remove clicking style and process click event */
            GT_LOGD(GT_LOG_TAG_GUI, "process");
            /* code here */

            gt_event_send(obj, GT_EVENT_TYPE_INPUT_PROCESSED, NULL);
            break;
        }
        case GT_EVENT_TYPE_INPUT_PROCESSED: {/* click event finish */
            GT_LOGD(GT_LOG_TAG_GUI, "processed");
            break;
        }
        default:
            break;
    }
}


static void _gt_qrcode_init_style(gt_obj_st * qr_code)
{
    _gt_style_qrcode_st * style = (_gt_style_qrcode_st * )qr_code->style;
    /* code */
    // http://www.gaotongfont.cn/
    gt_memset(style, 0, sizeof(_gt_style_qrcode_st));
    style->str = gt_mem_malloc(strlen("http://www.gaotongfont.cn/")+1);
    gt_memcpy(style->str, "http://www.gaotongfont.cn/" , strlen("http://www.gaotongfont.cn/")+1);
    style->str_len = strlen(style->str);
    style->version = GT_FAMILY_QRCODE_VERSION_10;
    style->ec_level = ECLevel_H;
    style->mask_patt = MaskPattern4;
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief create a qr_code obj
 *
 * @param parent qr_code's parent element
 * @return gt_obj_st* qr_code obj
 */
gt_obj_st * gt_qrcode_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    _gt_qrcode_init_style(obj);
    return obj;
}


void gt_qrcode_style_set_version(gt_obj_st * qr_code , gt_family_t family)
{
    _gt_style_qrcode_st * style = qr_code->style;
    style->version = family;
}

void gt_qrcode_style_set_str(gt_obj_st * qr_code , char* str)
{
    _gt_style_qrcode_st * style = qr_code->style;
    if( NULL != style->str)
    {
        gt_mem_free(style->str);
    }
    uint16_t size = strlen(str);
    style->str = gt_mem_malloc(size + 1);
    gt_memcpy(style->str, str, size);
    style->str[size] = '\0';
    // gt_event_send(barcode, GT_EVENT_TYPE_DRAW_START, NULL);
}

#endif
/* */
/* end ------------------------------------------------------------------*/
