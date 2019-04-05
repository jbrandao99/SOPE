#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>
#include <getopt.h>

char *filetype(char *argv, char *funcao);
void WriteOnFile(int argc, char *argv, char* hash,char* out);
void WriteOnSTDOUT(int argc, char *argv, char* hash);
void getDirectory(char *argv);
void isDirectory(char *argv);

char *filetype(char *argv, char *funcao)
{
    int link[2];
    __pid_t pid;
    char foo[4096];
    char final[4096];
    memset(foo, 0x00, 4096);
    memset(final, 0x00, 4096);
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

void WriteOnFile(int argc, char *argv, char* hash, char* out)
{
    int fd;
    fd = open(out, O_WRONLY | O_APPEND);
    dup2(fd, STDOUT_FILENO);
    WriteOnSTDOUT(argc, argv, hash);
    close(fd);
}

void WriteOnSTDOUT(int argc, char *argv, char* hash)
{
    struct stat buf;
    char tm[20]; //time modified
    char tc[20]; //time created
    char *type;
    char *hash_sha1;
    char *hash_sha256;
    char *hash_md5;

    if (argc > 0)
    {
        stat(argv, &buf);
        strftime(tm, 20, "%Y-%m-%dT%H:%M:%S", localtime(&(buf.st_mtime)));
        strftime(tc, 20, "%Y-%m-%dT%H:%M:%S", localtime(&(buf.st_ctime)));

        printf("%s,", argv);
        type = filetype(argv, "file");
        printf("%s,", type);
        printf("%ld,", buf.st_size);
        printf((buf.st_mode & S_IRUSR) ? "r" : "");
        printf((buf.st_mode & S_IWUSR) ? "w" : "");
        printf((buf.st_mode & S_IXUSR) ? "x" : ",");
        printf("%s,", tc);
        printf("%s", tm);
        if(strcmp(hash,"") == 0)
        {
            printf("%s\n", hash);
            return;
        }
        else if(strcmp(hash,"md5") == 0)
        {
            printf(",");
            hash_md5 = filetype(argv, "md5sum");
            printf("%s\n", hash_md5);
            return;
        }
        else if(strcmp(hash,"sha1") == 0)
        {
            printf(",");
            hash_sha1 = filetype(argv, "sha1sum");
            printf("%s\n", hash_sha1);
            return;
        }
        else if(strcmp(hash,"sha256") == 0)
        {
            printf(",");
            hash_sha256 = filetype(argv, "sha256sum");
            printf("%s\n", hash_sha256);
            return;
        }
        else if(strcmp(hash,"md5,sha1") == 0)
        {
            printf(",");
            hash_md5 = filetype(argv, "md5sum");
            printf("%s,", hash_md5);
            hash_sha1 = filetype(argv, "sha1sum");
            printf("%s\n", hash_sha1);
            return;
        }
        else if(strcmp(hash,"md5,sha256") == 0)
        {
            printf(",");
            hash_md5 = filetype(argv, "md5sum");
            printf("%s,", hash_md5);
            hash_sha256 = filetype(argv, "sha256sum");
            printf("%s\n", hash_sha256);
            return;
        }
        else if(strcmp(hash,"sha1,sha256") == 0)
        {
            printf(",");
            hash_sha1 = filetype(argv, "sha1sum");
            printf("%s,", hash_sha1);
            hash_sha256 = filetype(argv, "sha256sum");
            printf("%s\n", hash_sha256);
            return;
        }
        else
        {
            printf(",");
            hash_md5 = filetype(argv, "md5sum");
            printf("%s,", hash_md5);
            hash_sha1 = filetype(argv, "sha1sum");
            printf("%s,", hash_sha1);
            hash_sha256 = filetype(argv, "sha256sum");
            printf("%s\n", hash_sha256);
            return;
        }
    }
    else
    {
        perror("No arguments!");
    }
}

void isDirectory(char *argv)
{
    char *type = filetype(argv, "file");
    if (strcmp("directory", type) == 0)
    {
        getDirectory(argv);
    }
    else
        WriteOnSTDOUT(2, argv,"");
}

void getDirectory(char *argv)
{
    DIR *dir;
    struct dirent *dent;
    dir = opendir(argv);
    pid_t pid;
    char path[4096];
    strcpy(path, argv);
    char aux[4069];
    if (dir != NULL)
    {
        while ((dent = readdir(dir)) != NULL)
        {
            if (strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0)
                continue;

            strcpy(aux, path);
            strcat(aux, "/");
            strcat(aux, dent->d_name);
            //printf("%s\n", aux);
            //printf("%s\n", filetype(aux, "file"));
            //printf("////////////////////////////////////////////////\n");
            if (strcmp("directory", filetype(aux, "file")) != 0)
            {
                isDirectory(aux);
                continue;
            }

            pid = fork();
            if (pid == 0)
            {
                strcat(path, "/");
                strcat(path, dent->d_name);
                //printf("%s\n", path);
                //printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
                getDirectory(path);
                break;
            }
            else
            {
                wait(NULL);
            }
            //printf(dent->d_name);
            //printf("\n");
            //isDirectory(dent->d_name);
        }
    }
    closedir(dir);
}

int main(int argc, char *argv[])
{
    if (argc == 2)
    {
        WriteOnSTDOUT(2,argv[1],"");
    }
    else if ((argc == 3) && (strcmp(argv[1],"-r")== 0))
    {
        isDirectory(argv[2]);
    }
    else if (argc == 4) {
        if(strcmp(argv[1],"-h") == 0)
        {
            WriteOnSTDOUT(4,argv[3],argv[2]);
        }
        else if (strcmp(argv[1],"-o") == 0) 
        {
            WriteOnFile(4,argv[3],"",argv[2]);
        }
        else
        {
            exit(1);
        }
        
        
    }
    else
    {
        perror("ERROR!");
    }
    
    return 0;

}