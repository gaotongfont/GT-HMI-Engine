#ifndef _PARSDEF_H_
#define _PARSDEF_H_

#include "lexer.h"

#if GT_USE_LIB_TEX_PRINTF

typedef struct {
	char           *name;
	PRSDEF          P; /* the PRSDEF enummerator type is declared in lexer.h */
	int 			Nargs;
	int 			Nopt;
} KEYWORD;


typedef struct {
	char *name;
	SCALABLE_DELIMITER D;
} DELIMITER;


typedef struct {
	PRSDEF          P;
	unsigned int comb;		// combining diacritical mark
	unsigned int alt;		// alternative character to put above or below (0 if no suitable character exists)
	unsigned int altascii;	// ascii alternative to put above/below
} CombiningMarks;


typedef struct {
	char *name;
	unsigned int unicode;
} Symbol;


typedef struct {
	char *name;
	float scale; // wscale is always in units monospace width
} LengthUnit;


extern const KEYWORD Keys[];
extern const KEYWORD Envs[];
extern const DELIMITER DelTable[];
extern const CombiningMarks Combining[];
extern const Symbol Symbols[];
extern const Symbol * TEXPRINTF_SYMBOLS;
extern float AspectRatio;
extern const LengthUnit Lengths[];

#endif // GT_USE_LIB_TEX_PRINTF
//
#endif // _PARSDEF_H
