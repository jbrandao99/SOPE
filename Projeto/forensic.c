#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {

    struct stat buf;
    struct tm* mytm;
    char t[20];
    time_t mytime;
    
    mytime = time(&buf.st_mtime);
    mytm = localtime(&mytime);
    
    strftime(t,20,"%Y-%m-%dT%H:%M:%S",mytm);

    if(argc == 2)
    {
        stat(argv[1],&buf);

        printf("%s,", argv[1]);
        printf("%ld,",buf.st_size);
        printf((buf.st_mode & S_IRUSR) ? "r" : "");
        printf((buf.st_mode & S_IWUSR) ? "w" : "");
        printf((buf.st_mode & S_IXUSR) ? "x" : ",");
        printf("%s,",t);
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