CC = gcc
C_FLAGS=-std=c99 -Wall -Wextra -pedantic -pthread
LDFLAGS = -pthread

all: battledot.o client server

battledot.o: battledot.c battledot.h
	$(CC) $(C_FLAGS) -c -o battledot.o battledot.c

client:	client.c battledot.o battledot.h
	$(CC) $(C_FLAGS) -pthread -o client client.c

server:	server.c battledot.o battledot.h 
	$(CC) $(C_FLAGS) -pthread -o server battledot.o server.c $(LDFLAGS)

test:
	./server &
	./client -j -n Levi -x 5 -y 3
	./client -j -n Omar -x 7 -y 2
	./client -j -n Ken -x 6 -y 9

clean:
	rm -f *.o all
	rm -f client
	rm -f server
	rm -f clients/*
