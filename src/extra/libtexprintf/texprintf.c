#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "boxes.h"
#include "drawbox.h"
#include "parser.h"
#include "lexer.h"
#include "stringutils.h"
#include "error.h"

#if GT_USE_LIB_TEX_PRINTF

int TEXPRINTF_LW=0;
char * TEXPRINTF_FONT="text";
int TEXPRINTF_FCW=2;
int TEXPRINTF_WCW=2;

int TEXPRINTF_ERR=0;

int texprintf(const char *format, ...)
{
	va_list ap;
   	char *str;
	int Na=255, np;
	box root;

	ResetErrors();
	FCSPACES=TEXPRINTF_FCW;
	WCSPACES=TEXPRINTF_WCW;
	if ((WCSPACES<1)||(WCSPACES>2))
	{
		_TEX_FPRINTF(stderr,"Error: TEXPRINTF_WCW out of range, wide characters can occupy either 1 or 2 character spaces\n");
		exit(1);
	}
	if ((FCSPACES<1)||(FCSPACES>2))
	{
		_TEX_FPRINTF(stderr,"Error: TEXPRINTF_FCW out of range, wide characters can occupy either 1 or 2 character spaces\n");
		exit(1);
	}

	str=_TEX_MALLOC(Na*sizeof(char));
	va_start (ap, format);
	np=vsnprintf(str, Na*sizeof(char), format, ap);
	while (np>=Na)
	{
		Na+=255;
		str=_TEX_REALLOC(str, Na*sizeof(char));
		va_start (ap, format);
		np=vsnprintf(str, Na*sizeof(char), format, ap);
	}

	root=ParseString(str, TEXPRINTF_LW, TEXPRINTF_FONT);
	BoxPos(&root);
	np=PrintBox(&root);
	FreeBox(&root);
	_TEX_FREE(str);
	TEXPRINTF_ERR=ERRORSTATE;
	return np;
}

char * texstring(const char *str)
{
	box root;
	char *res;

	ResetErrors();
	FCSPACES=TEXPRINTF_FCW;
	WCSPACES=TEXPRINTF_WCW;
	if ((WCSPACES<1)||(WCSPACES>2))
	{
		_TEX_FPRINTF(stderr,"Error: TEXPRINTF_WCW out of range, wide characters can occupy either 1 or 2 character spaces\n");
		// exit(1);
		return NULL;
	}
	if ((FCSPACES<1)||(FCSPACES>2))
	{
		_TEX_FPRINTF(stderr,"Error: TEXPRINTF_FCW out of range, wide characters can occupy either 1 or 2 character spaces\n");
		// exit(1);
		return NULL;
	}

	root=ParseString((char*)str, TEXPRINTF_LW, TEXPRINTF_FONT);
	BoxPos(&root);
	res=DrawBox(&root);
	FreeBox(&root);
	TEXPRINTF_ERR=ERRORSTATE;

	return res;
}


char * stexprintf(const char *format, ...)
{
   	va_list ap;
   	char *res;
   	char *str;
   	int Na=255, np;

   	str=_TEX_MALLOC(Na*sizeof(char));
   	va_start (ap, format);
   	np=vsnprintf(str, Na*sizeof(char), format, ap);
   	while (np>=Na)
   	{
		Na+=255;
		str=_TEX_REALLOC(str, Na*sizeof(char));
		va_start (ap, format);
		np=vsnprintf(str, Na*sizeof(char), format, ap);
	}

	res = texstring(str);
	_TEX_FREE(str);
	return res;
}

void texfree(void *ptr)
{
    _TEX_FREE(ptr);
}

int ftexprintf(FILE *f, const char *format, ...)
{
   	va_list ap;
   	char *res;
   	char *str;
   	int Na=255, np;
	box root;

	ResetErrors();
	FCSPACES=TEXPRINTF_FCW;
	WCSPACES=TEXPRINTF_WCW;
	if ((WCSPACES<1)||(WCSPACES>2))
	{
		_TEX_FPRINTF(stderr,"Error: TEXPRINTF_WCW out of range, wide characters can occupy either 1 or 2 character spaces\n");
		exit(1);
	}
	if ((FCSPACES<1)||(FCSPACES>2))
	{
		_TEX_FPRINTF(stderr,"Error: TEXPRINTF_FCW out of range, wide characters can occupy either 1 or 2 character spaces\n");
		exit(1);
	}

   	str=_TEX_MALLOC(Na*sizeof(char));
   	va_start (ap, format);
   	np=vsnprintf(str, Na*sizeof(char), format, ap);
   	while (np>=Na)
   	{
		Na+=255;
		str=_TEX_REALLOC(str, Na*sizeof(char));
		va_start (ap, format);
		np=vsnprintf(str, Na*sizeof(char), format, ap);
	}

	root=ParseString(str, TEXPRINTF_LW, TEXPRINTF_FONT);
	BoxPos(&root);
	res=DrawBox(&root);
	FreeBox(&root);
	_TEX_FREE(str);
	np=strlen(res);
	_TEX_FPRINTF(f,"%s",res);
	_TEX_FREE(res);
	TEXPRINTF_ERR=ERRORSTATE;

	return np;
}

void texboxtree(const char *format, ...)
{
   	va_list ap;
   	char *str;
   	int Na=255, np;
	box root;

	ResetErrors();
	FCSPACES=TEXPRINTF_FCW;
	WCSPACES=TEXPRINTF_WCW;
	if ((WCSPACES<1)||(WCSPACES>2))
	{
		_TEX_FPRINTF(stderr,"Error: TEXPRINTF_WCW out of range, wide characters can occupy either 1 or 2 character spaces\n");
		exit(1);
	}
	if ((FCSPACES<1)||(FCSPACES>2))
	{
		_TEX_FPRINTF(stderr,"Error: TEXPRINTF_FCW out of range, wide characters can occupy either 1 or 2 character spaces\n");
		exit(1);
	}

   	str=_TEX_MALLOC(Na*sizeof(char));
   	va_start (ap, format);
   	np=vsnprintf(str, Na*sizeof(char), format, ap);
   	while (np>=Na)
   	{
		Na+=255;
		str=_TEX_REALLOC(str, Na*sizeof(char));
		va_start (ap, format);
		np=vsnprintf(str, Na*sizeof(char), format, ap);
	}

	root=ParseString(str, TEXPRINTF_LW, TEXPRINTF_FONT);
	BoxPos(&root);
	DrawBoxTree(&root);
	FreeBox(&root);
	_TEX_FREE(str);
	TEXPRINTF_ERR=ERRORSTATE;

}

void texlistsymbols()
{
	ListSymbols();								/* link to lexer.o */
}

char *texsymbols_str()
{
	return Symbols_Str();								/* link to lexer.o */
}

void texerrors()
{
	ERRORSTATE=TEXPRINTF_ERR;
	E_Messages();						/* link to error.o */
	ResetErrors();
}

/* Like texerror(), but return "; "-delimited string instead of
   writing to stderr.  Caller should _TEX_FREE return value with texfree. */
char *texerrors_str()
{
	char *s;
	ERRORSTATE=TEXPRINTF_ERR;
	s = E_Messages_str();						/* link to error.o */
	ResetErrors();
	return s;
}

void SetStyleASCII()
{
	style=&STYLE_ASC;
}
void SetStyleUNICODE()
{
	style=&STYLE_UNI;
}
void ToggleMapSuperSub()
{
	style->mapsupersub=!style->mapsupersub;
}
void ToggleAvoidCombining()
{
	style->avoidcombining=!style->avoidcombining;
}

#endif /* GT_USE_LIB_TEX_PRINTF */
