#include "tratastring.h"

int main()
{
    char *teste;
    teste = typestring("file.txt: ASCII text","file.txt");
    printf("%s", teste);
    return 0;
}