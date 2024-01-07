CC = gcc
CFLAGS = -Wall -L/usr/lib -lssl -lcrypto
SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)
TARGET = PROJET_LP25

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.c
	$(CC) -c $< $(CFLAGS)

clean:
	rm -f $(OBJ) $(TARGET)