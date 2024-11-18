C_FLAGS = gcc -g -Wall -pthread -O3
SRC = src
LIBS = $(wildcard $(SRC)/*.c)

all:
	$(C_FLAGS) partition.c $(LIBS) -o partition

purge:
	rm partition

debug: C_FLAGS += -DDEBUG
debug: all
