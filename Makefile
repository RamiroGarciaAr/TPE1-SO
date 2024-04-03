CC = gcc
CFLAGS = -Wall 
all: app slave #view

app: app.c
	$(CC) $(CFLAGS) -std=c99 -o app app.c

slave: slave.c
	$(CC) $(CFLAGS) -o slave slave.c slave.h

clean:
	rm -f app slave resultado.txt