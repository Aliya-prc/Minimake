CC = gcc
CFLAGS = -std=c99 -pedantic -Werror -Wall -Wextra -Wvla

SRC = src/minimake.c src/build.c src/flags.c src/parse.c
OBJ = $(SRC:.c=.o)

all: minimake

minimake: $(OBJ)
	$(CC) -o minimake $(OBJ)

check: minimake
	./tests/tests.sh

clean:
	rm $(RM) $(OBJ) minimake

.PHONY: clean check all
