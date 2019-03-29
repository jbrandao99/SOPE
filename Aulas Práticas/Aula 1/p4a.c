#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]){

    for(unsigned int i = 0; i < atoi(argv[2]); i++)
    {
            printf("Hello %s!\n", argv[1]);
    }

    return 0;
}