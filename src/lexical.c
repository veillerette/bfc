#include "lexical.h"

extern InstrPos InstrPos_Set(int line, int col)
{
	InstrPos temp;
	temp.line = line;
	temp.col = col;
	return temp;
}

extern BFInstrList *BFInstrList_Alloc(void)
{
       BFInstrList *temp = (BFInstrList *)malloc(sizeof(BFInstrList));
       if(NULL == temp)
       {
	       perror("malloc");
	       exit(EXIT_FAILURE);
       }
       
       temp->lst = (BFInstrVal *)malloc(sizeof(BFInstrVal) * BFInstrList_BASE);
       if(NULL == temp->lst)
       {
	       perror("malloc");
	       exit(EXIT_FAILURE);
       }
       temp->pos = (InstrPos *)malloc(sizeof(InstrPos) *  BFInstrList_BASE);
       if(NULL == temp->pos)
       {
	       perror("malloc");
	       exit(EXIT_FAILURE);
       }
       
       temp->n = 0;
       temp->m = BFInstrList_BASE;
       
       temp->is_in = 0;
       temp->is_out = 0;
       
       return temp;
}

extern void BFInstrList_Free(BFInstrList **bfins)
{
	if(*bfins)
	{
		if((*bfins)->lst != NULL)
			free((*bfins)->lst);
		
		if((*bfins)->pos != NULL)
			free((*bfins)->pos);
			
		free(*bfins);
		*bfins = NULL;
	}
}

BFInstrVal BFInstrVal_Set(BFInstr ins, int val)
{
	BFInstrVal temp;
	temp.ins = ins;
	temp.val = val;
	return temp;
}

int BFInstrList_Add(BFInstrList *bfins, BFInstr instr, int val, int line, int col)
{
	if((NULL == bfins) || (NULL == bfins->lst))
		return 0;
	
	if(bfins->n == bfins->m)
	{
		bfins->m *= 2;
		bfins->lst = realloc(bfins->lst, sizeof(BFInstrVal) * bfins->m);
		if(NULL == bfins->lst)
		{
			perror("realloc");
			exit(EXIT_FAILURE);
		}
		
		bfins->pos = realloc(bfins->pos, sizeof(InstrPos) * bfins->m);
		if(NULL == bfins->pos)
		{
			perror("realloc");
			exit(EXIT_FAILURE);
		}
	}
	
	bfins->lst[bfins->n] = BFInstrVal_Set(instr, val);
	bfins->pos[bfins->n] = InstrPos_Set(line, col);
	bfins->n++;
	
	return 1;
}

extern BFInstrList *BFInstr_Open(const char *path)
{
	FILE *f = NULL;
	int c;
	BFInstrList *lst = NULL;
	int choose = 0, val;
	int line = 1, col = 1;
	if((NULL == path))
		return NULL;
	
	f = fopen(path, "r");
	if(NULL == f)
	{
		fprintf(stderr, "Can't open %s : file can't be found\n", path);
		exit(EXIT_FAILURE);
	}
	
	lst = BFInstrList_Alloc();
	
	while((c = fgetc(f)) != EOF)
	{
		switch(c)
		{
			case '>': choose = ADD; val = 1; break;
			case '<': choose = SUB; val = 1; break;
			case '+': choose = INC; val = 1; break;
			case '-': choose = DEC; val = 1; break;
			case '.': choose = OUT; val = 0; lst->is_out = 1; break;
			case ',': choose = IN; val = 0; lst->is_in = 1; break;
			case '[': choose = BRANCH; val = 0; break;
			case ']': choose = BREAK; val = 0; break;
			
			case '\n':
				col = 1;
				line++;
			default:
				choose = -1;
				break;
		}
		if(choose != -1)
			if(!BFInstrList_Add(lst, choose, val, line, col))
			{
				fprintf(stderr, "[DevError 2v]\n");
				exit(EXIT_FAILURE);
			}
		col++;
	}
	
	fclose(f);
	
	return lst;
}

extern int VerifBF(const BFInstrList *bfins, const char *path)
{
	int i;
	BFInstrVal ins;
	int branches=0;
	BFError error = 0;
	BFWarning warn = 0;
	BFInstr last = INS_UNUSED;
	
	if((NULL == bfins) || (NULL == bfins->lst))
		return 0;
	
	for(i = 0; i < bfins->n; i++)
	{
		ins = bfins->lst[i];
		switch(ins.ins)
		{
			case BRANCH:
				branches++;
				if(branches > BRANCHES_LIMIT)
				{
					error |= LIMIT_BRANCH_EXCESS;
					fprintf(stderr,"%s:%d:%d: error: Limit \
					loop recursion excess\n",
						path, bfins->pos[i].line,
						bfins->pos[i].col);
					return 0;
				}
				break;
			case BREAK:
				branches--;
				if(branches < 0)
				{
					error |= NEGATIVE_BRANCH;
					fprintf(stderr,"%s:%d:%d: error: found \
']' without an oppening '['\n",
						 path, bfins->pos[i].line,
						 bfins->pos[i].col);
					return 0;
				}
				if(BRANCH == last)
				{
					warn |= INFINITE_LOOP;
					fprintf(stderr, "%s:%d:%d: warning: \
infinite loop detected\n",
						path, bfins->pos[i].line,
						bfins->pos[i].col);
				}
				break;
			case IN:
				if(IN == last)
				{
					error |= DOUBLE_IN;
					fprintf(stderr, "%s:%d:%d: warning: \
double input on the same memory\n",
						path, bfins->pos[i].line,
						bfins->pos[i].col);
				}
			
			default:
				break;
		}
		last = ins.ins;
	}
	
	if(branches)
	{
		error |= BRANCH_NOT_CLOSED;
		fprintf(stderr,"%s:%d:%d: error: at least %d \
'[' not closed properly\n",
			 path, bfins->pos[bfins->n].line,
			 bfins->pos[bfins->n].col + 1, branches);
		return 0;
		
	}
	return 1;
}

extern void BFList_Debug(BFInstrList *bf)
{
	int i;
	
	if((NULL == bf) || (NULL == bf->lst) || (NULL == bf->pos))
	{
		fprintf(stderr, "bf is NULL\n");
		return;
	}
	
	printf("BFInstrList : %d/%d instr. \n", bf->n, bf->m);
	
	for(i = 0; i < bf->n; i++)
	{
		switch(bf->lst[i].ins)
		{
			case ADD:            printf("ADD"); break;
			case SUB:            printf("SUB"); break;
			case INC:            printf("INC"); break;
			case DEC:            printf("DEC"); break;
			case OUT:            printf("OUT"); break;
			case IN:             printf("IN"); break;
			case BRANCH:         printf("BRANCH");break;
			case BREAK:          printf("BREAK");break;
			default:             printf("UNUSED");break;
		}
		
		printf("%d", bf->lst[i].val);
			
		if(i == bf->n-1)
			putchar('\n');
		else
			printf("  ");
	}
}

int BF_CanBeFusion(BFInstr a, BFInstr b)
{
	if((a == ADD || a == SUB) && (b == ADD || b == SUB))
		return 1;
	if((a == INC || a == DEC) && (b == INC || b == DEC))
		return 1;
	return 0;
}

int BFInstr_isNegative(BFInstr ins)
{
	if(ins == SUB || ins == DEC)
		return 1;
	return 0;
}

extern BFInstrList *BFList_Optimise1(BFInstrList *source)
{
	BFInstrList *res = NULL;
	int i,j;
	BFInstrVal ins;
	BFInstrVal current;
	
	res = BFInstrList_Alloc();
	
	i = 0;
	while(i < source->n)
	{
		ins = source->lst[i];
		
		if(ins.ins == IN)
			res->is_in = 1;
		else if(ins.ins == OUT)
			res->is_out = 1;
			
		if(ins.ins == SUB || ins.ins == DEC)
			ins.val = -ins.val;
			
		j = i+1;
		current = source->lst[j];
		while(BF_CanBeFusion(ins.ins, current.ins))
		{
			if(BFInstr_isNegative(current.ins))
				ins.val -= current.val;
			else
				ins.val += current.val;
			j++;
			if(j >= source->n)
				break;
			current = source->lst[j];
		}
		i = j;
		
		if(ins.val < 0)
		{
			if(ins.ins == ADD)
				ins.ins = SUB;
			else if(ins.ins == INC)
				ins.ins = DEC;
			ins.val = -ins.val;
		}
		else if(ins.val > 0)
		{
			if(ins.ins == SUB)
				ins.ins = ADD;
			else if(ins.ins == DEC)
				ins.ins = INC;
		}
		else if(ins.ins >= ADD && ins.ins <= DEC)
			continue;
		
		if(!BFInstrList_Add(res, ins.ins, ins.val, 0, 0))
		{
			fprintf(stderr, "[DevError 2o]\n");
			exit(EXIT_FAILURE);
		}
	}
	
	return res;
}


