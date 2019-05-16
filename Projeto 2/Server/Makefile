all: server

server: server.c
		gcc -Wall -Werror -Wextra -pthread -lrt -lpthread server.c -o server

clean:
		rm -f server

run: server
		./server

.PHONY: all run