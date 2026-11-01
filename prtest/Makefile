# prtest orchestrator build. The harness is compiled by the orchestrator
# at runtime against the student's libft.a; this Makefile only builds prtest.
CC      ?= cc
CFLAGS  ?= -std=c11 -Wall -Wextra -Werror -O0 -g
SRC     := $(wildcard src/*.c)
OBJ     := $(SRC:.c=.o)
BIN     := prtest

.PHONY: all clean
all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

src/%.o: src/%.c src/prtest.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(BIN)
