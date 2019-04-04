#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[])
{
    char* p = getenv("LOGFILENAME");

    if(strcmp(p, "(null)") == 0)
    {
        putenv("LOGFILENAME = registos.txt");
    }

    printf("%s", p);

    return 0;
}