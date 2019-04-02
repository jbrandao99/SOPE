#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

char *filetype(char *argv, char *funcao);

int main(int argc, char *argv[])
{

    struct stat buf;
    char tm[20]; //time modified
    char tc[20]; //time created
    char *type;
    char *hash_sha1;
    char *hash_sha256;
    char *hash_md5;

    if (argc == 2)
    {
        stat(argv[1], &buf);

        strftime(tm, 20, "%Y-%m-%dT%H:%M:%S", localtime(&(buf.st_mtime)));
        strftime(tc, 20, "%Y-%m-%dT%H:%M:%S", localtime(&(buf.st_ctime)));

        printf("%s,", argv[1]);
        type = filetype(argv[1], "file");
        printf("%s,", type);
        printf("%ld,", buf.st_size);
        printf((buf.st_mode & S_IRUSR) ? "r" : "");
        printf((buf.st_mode & S_IWUSR) ? "w" : "");
        printf((buf.st_mode & S_IXUSR) ? "x" : ",");
        printf("%s,", tc);
        printf("%s", tm);
        printf(",");
        hash_md5 = filetype(argv[1], "md5sum");
        printf("%s,", hash_md5);
        hash_sha1 = filetype(argv[1], "sha1sum");
        printf("%s,", hash_sha1);
        hash_sha256 = filetype(argv[1], "sha256sum");
        printf("%s", hash_sha256);

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

<<<<<<< HEAD
char* filetype(char* argv,char* funcao)
=======
char *filetype(char *argv, char *funcao)
>>>>>>> 6fbdb029d6daef6f73456f3c10be4b0ba904a44a
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
        }else if (strcmp("md5sum", funcao) == 0)
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
