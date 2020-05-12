# utilizing previous code from CIS 330
sources = $(wildcard *.c)
objects = $(addsuffix .o, $(basename $(sources)))
object = $(basename $(sources))
flags = -g -W -Wall -std=c11 -DDEBUG=1 -DTEST=0

%.o : %.c
	gcc -c $(flags) $< -o $@

clean :
	rm $(object)

all: part1 part2 part3 part4 cpubound	iobound
