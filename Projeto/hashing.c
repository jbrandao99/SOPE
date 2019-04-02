#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char *argv[])
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
        execlp("sha256sum", "sha256sum", argv[1], NULL);
    }
    else
    {

        close(link[1]);
        read(link[0], foo, sizeof(foo));
        for(int i=0; i<(strlen(argv[1])+2); i++){
            foo[i]='0';
        }
        int k=0;
        int t=0;
        while(k<strlen(foo)){
            if(foo[k]!='0'){
                final[t]=foo[k];
                t++;
                k++;
                continue;
            }
            k++;
        }
        printf("Output:%s\n", final);
        wait(NULL);
    }
    return 0;
}