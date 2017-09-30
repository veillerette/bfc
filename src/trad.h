#ifndef __HEAD_TRAD__
#define __HEAD_TRAD__

#include "lexical.h"
#include <string.h>

#define NWIV(i,v,k) Nasm_WriteInstruction(f,i,v##k)
#define NWI(i,v) Nasm_WriteInstruction(f,i,v)
#define NWL(l) Nasm_WriteLabel(f,l)



typedef enum
{
	BASIC                  = 0x0000,
	SHORT_STACK            = 0x0002,
	INT_STACK              = 0x0004,
	LONG_STACK             = 0x0008,
	INFINITE_STACK         = 0x0010,
	O1                     = 0x0020
} Trad_Option;

int Trad_Nasm(FILE *f, BFInstrList *bf, Trad_Option opt);

char *Nasm_GetPathDest(const char *source);

FILE *Trad_OpenDestFile(const char *path);

#endif