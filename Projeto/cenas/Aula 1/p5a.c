#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[], char* envp[]){

    for(unsigned int i = 0; i < argc; i++)
    {
            printf("Variável do ambiente %d: %s!\n", i, getenv(envp[1]));
    }

    return 0;
}