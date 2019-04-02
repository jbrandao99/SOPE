
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{

    char* hash256final = "";

    snprintf(hash256final,62,"%d",execl("/usr/bin/sha256sum", "ls", argv[1], NULL));

    return 0;
} 
