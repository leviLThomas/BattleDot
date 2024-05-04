CC = gcc
C_FLAGS=-std=gnu89 -g -Wall

all: battledot.o battledot_client battledot_server

battledot.o: battledot.c battledot.h
	$(CC) $(C_FLAGS) -c -o battledot.o battledot.c

battledot_client:	battledot_client.c
	$(CC) $(C_FLAGS) -pthread -o battledot_client battledot_client.c

battledot_server:	battledot_server.c battledot.o battledot.h 
	$(CC) $(C_FLAGS) -pthread -o battledot_server battledot.o battledot_server.c

test:
	./battledot_server &

	./battledot_client -m LUK -j -x 3 -y 9
	./battledot_client -m JON -j -x 6 -y 10
	./battledot_client -m MAK -j -x 4 -y 3
	./battledot_client -m KAS -j -x 7 -y 8
	./battledot_client -m LZA -j -x 1 -y 5
	./battledot_client -m LOW -j -x 1 -y 1
	./battledot_client -m UQI -j -x 2 -y 7
	./battledot_client -m IOW -j -x 5 -y 8
	./battledot_client -m OWP -j -x 7 -y 2
	./battledot_client -m OWL -j -x 9 -y 7

	./battledot_client -s 

	cat ./battledot_server.log

clean:
	rm -f *.o all
	rm -f battledot_client
	rm -f battledot_server
	rm -f serv_socket
	rm -f clients/client_socket_*
	rm -f battledot_server.log
