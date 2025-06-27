#ifndef _PARSER_H_
#define _PARSER_H_

#include "tex_config.h"

#if GT_USE_LIB_TEX_PRINTF

#include "drawchars.h"
extern Style STYLE_UNI;
extern Style STYLE_ASC;
extern Style * style;
extern int avoidcombining;
box ParseString(char *string, int LW, char *font);

#endif /* GT_USE_LIB_TEX_PRINTF */
#endif /* _PARSER_H_ */
