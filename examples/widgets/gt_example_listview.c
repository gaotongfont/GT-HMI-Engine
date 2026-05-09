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
gt_obj_st * gt_examples_listview(void)
{
    int i = 0;
    char * names[] = {
        "foo0", "foo1", "foo2", "foo3", "foo4", "foo5", "foo6", "foo7", "foo8", "foo9",
        "foo10", "foo11", "foo12", "foo13", "foo14", "foo15", "foo16", "foo17", "foo18", "foo19",
    };
    int length = sizeof(names) / sizeof(char *);

    gt_obj_st * screen;
    screen = gt_obj_create(NULL);

    /* 垂直列表 */
    gt_obj_st * listview = gt_listview_create(screen);
    gt_obj_set_pos(listview, 0, 0);
    gt_obj_set_size(listview, 300, 200);
    gt_listview_set_item_height(listview, 40);

    for (i = 0; i < length; i++) {
        gt_listview_add_item(listview, names[i]);
    }
    gt_listview_set_selected_item_by_idx(listview, 2);
    gt_obj_add_event_cb(listview, _listview_event_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    /* 水平列表 */
    gt_obj_st * lv_hor = gt_listview_create(screen);
    gt_obj_set_pos(lv_hor, 10, 220);
    gt_obj_set_size(lv_hor, 460, 120);

    gt_listview_set_horizontal(lv_hor, true);
    gt_listview_set_item_width(lv_hor, 100);
    gt_listview_set_item_height(lv_hor, 100);
    gt_listview_set_item_reduce(lv_hor, 4);
    gt_listview_set_item_space(lv_hor, 4, 4);
    gt_listview_set_septal_line(lv_hor, true);

    char buf[32];
    for (i = 0; i < 10; i++) {
        snprintf(buf, sizeof(buf), "Item %d", i + 1);
        gt_listview_add_item(lv_hor, buf);
    }

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}


/* global functions / API interface -------------------------------------*/


#endif
/* end ------------------------------------------------------------------*/
