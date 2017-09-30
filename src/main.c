
#define _POSIX_C_SOURCE 3
#include <unistd.h>

#include "lexical.h"
#include "trad.h"

#define TESTFILE "test.bf"

int main(int argc, char *argv[])
{
	BFInstrList *lst = NULL;
	BFInstrList *opti = NULL;
	char *pathdest = NULL;
	FILE *f = NULL;
	int isopti = 0;
	int opt;
	
	while((opt = getopt(argc, argv, "O")) != -1)
	{
		switch(opt)
		{
			case 'O':
				isopti = 1;
				break;
			default:
				break;
		}
	}
	
	if(optind >= argc)
	{
		fprintf(stderr, "fatal error: no input file\n");
		exit(EXIT_FAILURE);
	}
	
	lst = BFInstr_Open(argv[optind]);
	
	
	if(!VerifBF(lst, argv[optind]))
	{
		BFInstrList_Free(&lst);
		exit(EXIT_FAILURE);
	}
	
	pathdest = Nasm_GetPathDest(argv[optind]);
	f = Trad_OpenDestFile(pathdest);
	
	if(pathdest != NULL)
		free(pathdest);
	
	if(isopti)
	{
		opti = BFList_Optimise1(lst);
		Trad_Nasm(f, opti, O1);
		BFList_Debug(opti);
		BFInstrList_Free(&opti);
	}
	else
	{
		Trad_Nasm(f, lst, SHORT_STACK);	
	}
	
	
	BFInstrList_Free(&lst);
	
	fclose(f);
	
	argc = argc;
	argv = argv;
	return 0;
}
