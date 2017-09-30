#include "trad.h"

static int Nasm_HeadFile(FILE *f, Trad_Option opt, BFInstrList *bf)
{
	fprintf(f, "section .bss\n");
	fprintf(f, "\tBFTAB:\tres");
	if(opt & SHORT_STACK)
		fprintf(f, "w");
	else if(opt & INT_STACK)
		fprintf(f, "d");
	else if(opt & LONG_STACK)
		fprintf(f, "q");
	else
		fprintf(f, "b");
	fprintf(f, "\t30000\n");
	fprintf(f, "section .text\n");
	fprintf(f, "global _start\n");
	
	/*if(bf->is_out)
	       fprintf(f, "extern putchar\n");     
	if(bf->is_in)
	       fprintf(f, "extern getchar\n");
	*/
	
	fprintf(f, "\n_start:");
	
	return 1;	
}

static int Nasm_EndFile(FILE *f)
{
	f=f;
	return 1;
}

static int Nasm_WriteInstruction(FILE *f, const char *ins, const char *val)
{
	if(strlen(ins) < 1)
		return 1;
	fprintf(f, "\n\t%s", ins);
	if(strlen(val))
		fprintf(f, "\t%s", val);
	return 1;
}

static int Nasm_WriteLabel(FILE *f, const char *label)
{
	return fprintf(f, "\n%s:", label);
}

void getSizeStack(Trad_Option opt, char *dest)
{
	if(opt & SHORT_STACK)
		sprintf(dest, "word");
	else if(opt & INT_STACK)
		sprintf(dest, "dword");
	else if(opt & LONG_STACK)
		sprintf(dest, "qword");
	else
		sprintf(dest, "byte");
}

void getRegisterSize(Trad_Option opt, char *dest)
{
	if(opt & SHORT_STACK)
		sprintf(dest, "ax");
	else if(opt & INT_STACK)
		sprintf(dest, "eax");
	else if(opt & LONG_STACK)
		sprintf(dest, "rax");
	else
		sprintf(dest, "al");
}



int Nasm_WriteBody(FILE *f, BFInstrList *bf, Trad_Option opt, int *begin, int *ibranch)
{
	char strIns[20];
	char strArg[50];
	char bufReg[20];
	char size[10];
	int mybranch;
	
	int sizeN = opt & 0x0f;
	BFInstrVal ins;
	
	
	if((*begin) >= bf->n)
	       return 0;
	       
	if(!sizeN)
		sizeN = 1;
		
	getSizeStack(opt, size);
	getRegisterSize(opt, bufReg);
	
	mybranch = *ibranch;
	
	ins = bf->lst[*begin];
	
	if((opt&O1) && ins.ins == BRANCH && bf->lst[(*begin)+1].ins == DEC 
					&& bf->lst[(*begin)+2].ins == BREAK)
	{
		sprintf(strIns, "mov");
		sprintf(strArg, "%s [rbp], 0", size);
		NWI(strIns, strArg);
		(*begin) = (*begin)+3;
	}
	if((*begin) >= bf->n)
	       return 0;
	
	ins = bf->lst[*begin];
	
	switch(ins.ins)
	{
		case ADD: 
			sprintf(strIns, "add");
			sprintf(strArg, "rbp, %d", ins.val * sizeN);
			break;
			
		case SUB:
			sprintf(strIns, "sub");
			sprintf(strArg, "rbp, %d", ins.val * sizeN);
			break;
			
		case INC:
			sprintf(strIns, "add");
			sprintf(strArg, "%s [rbp], %d", size, ins.val);
			break;
			
		case DEC:
			sprintf(strIns, "sub");
			sprintf(strArg, "%s [rbp], %d", size, ins.val);
			break;
			
		case OUT:
			/*sprintf(strIns, "movzx");
			sprintf(strArg, "rdi, %s [rbp]", size);
			NWI(strIns, strArg);
			sprintf(strIns, "call");
			sprintf(strArg, "putchar");*/
			NWI("mov", "eax, 4");
			NWI("mov", "ebx, 1");
			NWI("mov", "rcx, rbp");
			NWI("mov", "edx, 1");
			sprintf(strIns, "int");
			sprintf(strArg, "0x80");
			break;
			
		case IN:
			/*NWI("call", "getchar");
			sprintf(strIns, "mov");
			sprintf(strArg, "[rbp], %s", bufReg);*/
			NWI("mov", "eax, 3");
			NWI("mov", "ebx, 0");
			NWI("mov", "rcx, rbp");
			NWI("mov", "edx, 1");
			sprintf(strIns, "int");
			sprintf(strArg, "0x80");
			break;
			
		case BRANCH:
			sprintf(strIns, "jmp");
			sprintf(strArg, ".L1%d", mybranch);
			NWI(strIns, strArg);
			sprintf(strIns, ".L0%d", *ibranch);
			NWL(strIns);
			(*begin)++;
			(*ibranch)++;
			Nasm_WriteBody(f, bf, opt, begin, ibranch);
			sprintf(strIns, ".L1%d", mybranch);
			NWL(strIns);
			sprintf(strIns, "cmp");
			sprintf(strArg, "%s [rbp], 0", size);
			NWI(strIns, strArg);
			sprintf(strIns, "jne");
			sprintf(strArg, ".L0%d", mybranch);
			break;
		
		case BREAK:
			return 1;
		default:
			break;
	}
	NWI(strIns, strArg);
	
	(*begin)++;
	return Nasm_WriteBody(f, bf, opt, begin, ibranch);
}

int Trad_Nasm(FILE *f, BFInstrList *bf, Trad_Option opt)
{
	int begin = 0;
	int branch = 0;
	if((NULL == f) || (NULL == bf))
		return 0;
	
	Nasm_HeadFile(f, opt, bf);
	
	if(!(opt&O1))
	       NWI("push", "rbp");
	       
	NWI("mov", "rbp, BFTAB");
	
	Nasm_WriteBody(f, bf, opt, &begin, &branch);
	
	if(opt&O1)
		NWI("xor", "eax, eax");
	else
	{
		NWI("mov", "eax, 0");
		NWI("pop", "rbp");
	}
	NWI("mov", "eax, 1");
	NWI("mov", "ebx, 0");
	NWI("int", "0x80");
	
	Nasm_EndFile(f);
	
	opt = opt;
	
	return 1;
}

char *Nasm_GetPathDest(const char *source)
{
	int i,j;
	char *res = NULL;
	
	if(NULL == source)
		return NULL;
	
	for(i = strlen(source)-1; i > 0; i--)
		if(source[i] == '.')
			break;
	
	res = (char *)malloc(sizeof(char) * (i+5));
	if(NULL == res)
	{
		fprintf(stderr, "Error with allocation bZJ\n");
		exit(EXIT_FAILURE);
	}
	
	for(j = 0; j <= i; j++)
		res[j] = source[j];
	strcpy(res+j, "asm");
	return res;
}


FILE *Trad_OpenDestFile(const char *path)
{
	FILE *f = NULL;
	
	if((NULL == path))
		return NULL;
	
	f = fopen(path, "w");
	if(NULL == f)
	{
		perror("fopen");
		fprintf(stderr, "Can't open \"%s\"\n", path);
		exit(EXIT_FAILURE);
	}
	
	return f;
}