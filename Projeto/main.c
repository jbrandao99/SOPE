#include "tratastring.h"

int main(int argc,char* argv[])
{
    printf("%d\n", argc);
    char* teste = funcaoexec(argv[1],"file");
    printf("%s", teste);
    return 0;
}