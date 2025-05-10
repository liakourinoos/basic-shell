OBJS = source/mysh.o source/array_handling.o source/basic_shell.o source/history.o source/lists.o
SOURCE = source/mysh.c source/array_handling.c source/basic_shell.c source/history.c source/lists.c
HEADER = includes/array_handling.h includes/basic_shell.h includes/defines.h includes/history.h includes/lists.h
OUT = mysh
CC = gcc
FLAGS = -g -c -Iincludes

$(OUT):$(OBJS)
	$(CC) -g $(OBJS) -o $@

source/mysh.o : source/mysh.c
	$(CC) $(FLAGS) source/mysh.c -o source/mysh.o

source/array_handling.o : source/array_handling.c
	$(CC) $(FLAGS) source/array_handling.c -o source/array_handling.o

source/basic_shell.o : source/basic_shell.c
	$(CC) $(FLAGS) source/basic_shell.c -o source/basic_shell.o

source/history.o : source/history.c
	$(CC) $(FLAGS) source/history.c -o source/history.o

source/lists.o : source/lists.c
	$(CC) $(FLAGS) source/lists.c -o source/lists.o

clean:
	rm -f $(OBJS) $(OUT)