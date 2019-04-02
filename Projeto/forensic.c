#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {

    struct stat buf;
    char tm[20]; //time modified
    char tc[20]; //time created

    if(argc == 2)
    {
        stat(argv[1],&buf);
    
        strftime(tm,20,"%Y-%m-%dT%H:%M:%S",localtime(&(buf.st_mtime)));
        strftime(tc,20,"%Y-%m-%dT%H:%M:%S",localtime(&(buf.st_ctime)));

        printf("%s,", argv[1]);
        printf("%ld,",buf.st_size);
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