#include "filetype.h"

char *filetype(char *argv, char *funcao)
{
    int link[2];
    __pid_t pid;
    char foo[4096];
    char final[4096];
    pipe(link);
    pid = fork();

    if (pid == 0)
    {
        dup2(link[1], STDOUT_FILENO);
        close(link[0]);
        close(link[1]);
        execlp(funcao, funcao, argv, NULL);
        exit(EXIT_SUCCESS);
    }
    else
    {
        close(link[1]);
        read(link[0], foo, sizeof(foo));

        //////////////////////////////SETTING STRING///////////////////
        if (strcmp("file", funcao) == 0)
        {
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
        }
        else if (strcmp("sha1sum", funcao) == 0)
        {
            for (unsigned int i = 0; i < 40; i++)
            {
                final[i] = foo[i];
            }
        }
        else if (strcmp("sha256sum", funcao) == 0)
        {
            for (unsigned int i = 0; i < 64; i++)
            {
                final[i] = foo[i];
            }
        }
        else if (strcmp("md5sum", funcao) == 0)
        {
            for (unsigned int i = 0; i < 32; i++)
            {
                final[i] = foo[i];
            }
        }
        //////////////////////////////SETTING STRING/////////////////////

        wait(NULL);
        return strdup(&final[0]);
    }
}