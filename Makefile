CC = gcc
CFLAGS = -Wall

OBJS = configuration.o file-properties.o files-list.o main.o messages.o processes.o sync.o utility.o

PROJET_LP25: $(OBJS)
	$(CC) $(CFLAGS) -o PROJET_LP25 $(OBJS)

configuration.o: configuration.c configuration.h defines.h
	$(CC) $(CFLAGS) -c configuration.c

file-properties.o: file-properties.c file-properties.h defines.h
	$(CC) $(CFLAGS) -c file-properties.c

files-list.o: files-list.c files-list.h defines.h
	$(CC) $(CFLAGS) -c files-list.c

main.o: main.c defines.h
	$(CC) $(CFLAGS) -c main.c

messages.o: messages.c messages.h defines.h
	$(CC) $(CFLAGS) -c messages.c

processes.o: processes.c processes.h defines.h
	$(CC) $(CFLAGS) -c processes.c

sync.o: sync.c sync.h defines.h
	$(CC) $(CFLAGS) -c sync.c

utility.o: utility.c utility.h defines.h
	$(CC) $(CFLAGS) -c utility.c

clean:
	rm -f PROJET_LP25 *.o