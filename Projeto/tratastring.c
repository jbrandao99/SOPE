#include "tratastring.h"

char *sha256string(char *hash)
{
    char final[4096];
    for (unsigned int i = 0; i < 64; i++)
    {
        final[i] = hash[i];
    }
    return strdup(&final[0]);
}

char *sha1string(char *hash)
{
    char final[4096];
    for (unsigned int i = 0; i < 40; i++)
    {
        final[i] = hash[i];
    }
    return strdup(&final[0]);
}

char *md5string(char *hash)
{
    char final[4096];
    for (unsigned int i = 0; i < 32; i++)
    {
        final[i] = hash[i];
    }
    return strdup(&final[0]);
}

char *typestring(char *foo, char *argv)
{
    char *final;
    for (unsigned int i = 0; i < (strlen(argv) + 2); i++)
    {
        foo[i] = '0';
    }
    unsigned int k = 0;
    int t = 0;
    while (k < strlen(foo))
    {
        if (foo[k] != '0' && foo[k] != '\n' && foo[k] != ',')
        {
            final[t] = foo[k];
            t++;
            k++;
            continue;
        }
        k++;
    }
    return strdup(&final[0]);
}
