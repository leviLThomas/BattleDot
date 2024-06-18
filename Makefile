CC = bear -- gcc
C_FLAGS=-std=gnu17 -pipe -g -Wall -Wextra -Wpedantic 
LDFLAGS = -pthread

all: client server

client:	client.c 
	$(CC) $(C_FLAGS) -pthread -o client client.c

server:	server.c battledot.c circular_linked_list.c
	$(CC) $(C_FLAGS) -o server server.c battledot.c circular_linked_list.c $(LDFLAGS)

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
