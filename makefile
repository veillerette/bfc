CC=colorgcc
CFLAGS=-ansi -Wall -Wextra -O3
CLIBS=
EXE=prog
OBJ=obj/
SRC=src/
INCL=src/
FILEC:= $(wildcard $(SRC)*.c)
FILEO:= $(patsubst $(SRC)%.c,$(OBJ)%.o,$(FILEC))

$(EXE): $(FILEO)
	$(CC) -o $(EXE) $^ $(CFLAGS) $(CLIBS)

$(OBJ)main.o:$(SRC)main.c
	@if [ ! -d "$(OBJ)" ]; then mkdir $(OBJ); fi;	
	$(CC) -o $@ -c $< $(CFLAGS)
	
$(OBJ)%.o: $(SRC)%.c $(INCL)%.h
	@if [ ! -d "$(OBJ)" ]; then mkdir $(OBJ); fi;	
	$(CC) -o $@ -c $< $(CFLAGS)
	
clean:
	rm -f $(EXE)
	rm -f *~
	if [ -d "$(OBJ)" ]; then rm -f $(OBJ)*.o; rmdir $(OBJ); fi;	

wc:
	@wc $(SRC)*.c $(INCL)*.h Makefile | sort -n

