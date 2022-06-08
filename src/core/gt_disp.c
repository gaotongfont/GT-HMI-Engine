/**
 * @file gt_disp.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-05-11 14:59:50
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

 /* include --------------------------------------------------------------*/
#include "../gt_conf.h"
#include "../widgets/gt_obj.h"



/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/





/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
void gt_disp_set_src(gt_obj_st * scr) {
    // disp->scr = scr;
}

gt_obj_st * gt_disp_get_scr() {
    // return disp->scr;
}

static void _obj_foreach(gt_obj_st * parent){
    uint16_t idx = 0;
    gt_obj_st * _obj_temp = NULL;
    const gt_obj_class_st * _class = parent->class;
    while( idx < parent->cnt_child ){
        _obj_temp = parent->child[idx];
        
        _obj_temp->class = _class;
        _obj_temp->class->_init_cb(_obj_temp, _class );

        if( _obj_temp->cnt_child != 0 ){
            _obj_foreach(_obj_temp);
        }
        idx++;
    }
}

void gt_load_scr(gt_obj_st * scr) {
    if( scr->class->type != GT_TYPE_OBJ ){
		GT_LOGV(GT_LOG_TAG_GUI, "this is not a screen obj->%d",scr->class->type);
        return;
    }
    _obj_foreach(scr);
}




/* end ------------------------------------------------------------------*/
