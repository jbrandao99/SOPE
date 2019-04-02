#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

char (*filetype(char * argv));

int main(int argc, char *argv[]) {

    struct stat buf;
    char tm[20]; //time modified
    char tc[20]; //time created
    char* type;

    if(argc == 2)
    {
        stat(argv[1],&buf);
    
        strftime(tm,20,"%Y-%m-%dT%H:%M:%S",localtime(&(buf.st_mtime)));
        strftime(tc,20,"%Y-%m-%dT%H:%M:%S",localtime(&(buf.st_ctime)));

        printf("%s,", argv[1]);
        printf("%ld,",buf.st_size);
        type = filetype(argv[1]);
        printf("%s,",type);
        printf((buf.st_mode & S_IRUSR) ? "r" : "");
        printf((buf.st_mode & S_IWUSR) ? "w" : "");
        printf((buf.st_mode & S_IXUSR) ? "x" : ",");
        printf("%s,",tc);
        printf("%s",tm);

        printf("\n");
        exit(EXIT_SUCCESS);

    }
    else
    {
        perror("No arguments!");
        exit(EXIT_FAILURE);
    }
    

    return 0;
}

char (*filetype(char * argv))
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
        execlp("file", "file", argv, NULL);
    }
    else
    {

        close(link[1]);
        read(link[0], foo, sizeof(foo));
        for(int i=0; i<(strlen(argv)+2); i++){
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
        return final;
        wait(NULL);
    }
}