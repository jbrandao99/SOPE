all: projeto

projeto: server.c user.c
		gcc -Wall -Werror -Wextra -pthread -lrt -lpthread server.c -o server
		gcc -Wall -Werror -Wextra -pthread -lrt -lpthread user.c -o user

clean:
		rm -f server
		rm -f user

run: projeto
		./projeto

.PHONY: all run