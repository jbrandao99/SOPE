all: user

user: user.c
		gcc -Wall -Werror -Wextra -pthread -lrt -lpthread user.c -o user

clean:
		rm -f user

run: user
		./user

.PHONY: all run