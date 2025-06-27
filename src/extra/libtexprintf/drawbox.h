#ifndef _DRAWBOX_H_
#define _DRAWBOX_H_

#include "tex_config.h"

#if GT_USE_LIB_TEX_PRINTF

char * DrawBox(box *b); /* puts a formatted equation in a multiline string */
int PrintBox(box *b); /* dumps an equation to screen */
void DrawBoxTree(box *b);

#endif /* GT_USE_LIB_TEX_PRINTF */
//
#endif /* _DRAWBOX_H_ */