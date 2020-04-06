# utilizing previous code from CIS 330
sources = $(wildcard *.c)
objects = $(addsuffix .o, $(basename $(sources)))
flags = -g -W -Wall -std=c11 -DDEBUG=1 -DTEST=0
target = a.out


$(target) : $(objects)
	gcc -o $(target) $(objects)

%.o : %.c
	gcc -c $(flags) $< -o $@
clean :
	rm $(target) $(objects)
valgrind-file:
	valgrind --leak-check=full --tool=memcheck ./a.out -f input.txt > log.txt 2>&1
valgrind-int:
	valgrind --leak-check=full --tool=memcheck ./a.out > log.txt 2>&1
