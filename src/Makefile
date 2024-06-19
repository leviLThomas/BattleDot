BUILD_DIR=../build/
CC = bear -- gcc
C_FLAGS=-std=gnu99 -pipe -g -Wall -Wextra -Wpedantic 
LDFLAGS = -pthread

all: client server

client:	client.c 
	$(CC) $(C_FLAGS) -pthread -o $(BUILD_DIR)client client.c

server:	server.c battledot.c circular_linked_list.c
	$(CC) $(C_FLAGS) -o $(BUILD_DIR)server server.c battledot.c circular_linked_list.c $(LDFLAGS)

test:
	./server &
	./client -j -n Levi -x 5 -y 3
	./client -j -n Omar -x 7 -y 2
	./client -j -n Ken -x 6 -y 9
