OBJS = $(wildcard src/*.c)

CC = gcc

CFLAGS = -Wall -Wextra -Wpedantic

LDFLAGS = -lSDL2 -lm

BIN = sim

all : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(BIN) $(LDFLAGS)
