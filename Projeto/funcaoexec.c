#include "funcaoexec.h"

char *funcaoexec(char *argv, char *funcao)
{
    int link[2];
    __pid_t pid;
    char foo[4096];
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

        wait(NULL);
        return strdup(&foo[0]);
    }
}