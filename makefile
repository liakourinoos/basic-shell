OBJS = mysh.o array_handling.o basic_shell.o history.o lists.o
SOURCE = mysh.c array_handling.c basic_shell.c history.c lists.c
HEADER = array_handling.h basic_shell.h defines.h history.h lists.h
OUT = mysh
CC = gcc
FLAGS = -g -c
$(OUT) : $(OBJS)
	$(CC) -g $(OBJS) -o $@

mysh.o : mysh.c
	$(CC) $(FLAGS) mysh.c

array_handling.o : array_handling.c
	$(CC) $(FLAGS) array_handling.c

basic_shell.o : basic_shell.c
	$(CC) $(FLAGS) basic_shell.c

history.o : history.c
	$(CC) $(FLAGS) history.c

lists.o : lists.c
	$(CC) $(FLAGS) lists.c

clean:
	rm -f $(OBJS) $(OUT)