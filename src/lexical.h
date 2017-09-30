#ifndef __HEAD_LEXICAL__
#define __HEAD_LEXICAL__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BFInstrList_BASE 200
#define BRANCHES_LIMIT 200

typedef enum
{
	LIMIT_BRANCH_EXCESS	= 0x01,
	NEGATIVE_BRANCH		= 0x02,
	BRANCH_NOT_CLOSED	= 0x04,
	ERR_UNUSED		= 0xff
} BFError;

typedef enum
{
	INFINITE_LOOP,
	DOUBLE_IN,
	WAR_UNUSED
} BFWarning;

typedef enum
{
	ADD,           SUB,
	INC,           DEC,
	OUT,           IN,
	BRANCH,        BREAK,
	INS_UNUSED
} BFInstr;

typedef struct InstrPos InstrPos;
struct InstrPos
{
	int line;
	int col;
};

typedef struct BFInstrVal BFInstrVal;
struct BFInstrVal
{
	BFInstr ins;
	int val;
	int ptr;
};

typedef struct BFInstrList BFInstrList;
struct BFInstrList
{
	BFInstrVal *lst;
	InstrPos *pos;
	int n;
	int m;
	
	int is_in;
	int is_out;
};

BFInstrVal BFInstrValPtr_Set(BFInstr ins, int val, int ptr);

BFInstrVal BFInstrVal_Set(BFInstr ins, int val);

extern InstrPos InstrPos_Set(int line, int col);

extern BFInstrList *BFInstrList_Alloc(void);

extern void BFInstrList_Free(BFInstrList **bfins);

int BFInstrList_Add(BFInstrList *bfins, BFInstr instr, int val, int ptr, int line, int col);

extern BFInstrList *BFInstr_Open(const char *path);

extern int VerifBF(const BFInstrList *bfins, const char *path);

extern void BFList_Debug(BFInstrList *bf);

extern BFInstrList *BFList_Optimise1(BFInstrList *source);

#endif
