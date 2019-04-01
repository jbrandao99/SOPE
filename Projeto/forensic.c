#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {

    struct stat buf;
    struct tm* mytmm; //time modified
    struct tm* mytmc; //time created
    char tm[20]; //time modified
    char tc[20]; //time created
    time_t mytimem; //time modified
    time_t mytimec;  //time created
    
    if(argc == 2)
    {
        stat(argv[1],&buf);

        mytimem = time(&buf.st_mtime);
        mytmm = localtime(&mytimem);

        mytimec = time(&buf.st_ctime);
        mytmc = localtime(&mytimec);
    
        strftime(tm,20,"%Y-%m-%dT%H:%M:%S",mytmm);
        strftime(tc,20,"%Y-%m-%dT%H:%M:%S",mytmc);

        printf("%s,", argv[1]);
        printf("%d",type);
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