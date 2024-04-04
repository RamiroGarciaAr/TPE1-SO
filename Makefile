CC = gcc
CFLAGS = -Wall 
all: app slave #view

app: app.c shm.h
	$(CC) $(CFLAGS) -std=c99 -o app app.c app.h shm.c -lrt -pthread -D_XOPEN_SOURCE=500

slave: slave.c
	$(CC) $(CFLAGS) -o slave slave.c slave.h

clean:
	rm -f app slave resultado.txt