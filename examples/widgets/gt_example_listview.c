/**
 * @file gt_example_listview.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2022-10-10 14:37:42
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_listview.h"


/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES


/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/

void _listview_event_cb(gt_event_st * e)
{
    printf("listview event\n");
}


/* static functions -----------------------------------------------------*/
void gt_examples_listview(void)
{
    int i = 0;
    char * names[] = { "foo1", "foo2", "foo3", "foo4", "foo5", "foo6", "foo7" };
    int length = sizeof(names) / sizeof(char *);

    gt_obj_st * screen;
    screen = gt_obj_create(NULL);

    gt_obj_st * listview = gt_listview_create(screen);
    for (i = 0; i < length; i++) {
        gt_listview_add_item(listview, names[i]);
    }
    gt_listview_set_selected_item_by_idx(listview, 2);
	gt_obj_set_pos(listview, 0, 0);
    gt_obj_set_size(listview, 120, 60);
    gt_obj_add_event_cb(listview, _listview_event_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    gt_disp_load_scr(screen);
}


/* global functions / API interface -------------------------------------*/


#endif
/* end ------------------------------------------------------------------*/
