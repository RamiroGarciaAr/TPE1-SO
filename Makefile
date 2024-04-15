CC = gcc
CFLAGS = -Wall 
all: app slave view

app: app.c shm.h
	$(CC) $(CFLAGS) -std=c99 -fsanitize=address -g -o app app.c shm.c -lrt -pthread -D_XOPEN_SOURCE=500 

slave: slave.c
	$(CC) $(CFLAGS) -o slave slave.c slave.h

view: view.c shm.h
	$(CC) $(CFLAGS) -std=c99 -o view view.c shm.c -lrt -pthread -D_XOPEN_SOURCE=500 

clean:
	rm -f app slave view resultado.txt