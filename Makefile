# Makefile for the tableau project.

CC=g++
CFLAGS=-g -Wall

ALL=prove php h gamma statman

all: $(ALL)

prove: prove.o kes3.o ke.o analytic.o tableau.o formula.o
	$(CC) -o $@ $^

php: php.o formula.o

h: h.o formula.o

gamma: gamma.o formula.o

statman: statman.o formula.o

clean:
	-rm -f *.o $(ALL)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $^ -o $@ 

