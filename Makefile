CC = gcc
CFLAGS = -Wall -g -I./include
SRC = $(wildcard ./jogo/*.c)
OBJ = $(SRC:.c=.o)
EXEC = snake

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(EXEC) $(OBJ)