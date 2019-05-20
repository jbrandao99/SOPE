all: server user

server: server.c
		gcc -Wall -Werror -Wextra -pthread -lrt -lpthread server.c -o server

user: user.c
		gcc -Wall -Werror -Wextra -pthread -lrt -lpthread user.c -o user

clean:
		rm -f server
		rm -f user

.PHONY: all run