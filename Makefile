CC=g++
CFLAGS=-g -Wall -pthread -I./
LDFLAGS= -lpthread -lm -lgtest

OBJECTS=$(SUBSRCS:.c=.o)
EXEC=maglevhash_test

all: $(EXEC)

$(EXEC): $(wildcard *.c) $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

clean:
	$(RM) $(EXEC)

.PHONY: $(EXEC)
