#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

    struct stat buf;

    if(argc == 2)
    {
        stat(argv[1],&buf);

        printf("%s,", argv[1]);
        printf("%ld,",buf.st_size);
        printf("%s,", ctime(&buf.st_mtime));
        printf((buf.st_mode & S_IRUSR) ? "r" : "-");
        printf((buf.st_mode & S_IWUSR) ? "w" : "-");
        printf((buf.st_mode & S_IXUSR) ? "x" : "-");

        exit(EXIT_SUCCESS);

    }
    else
    {
        perror("No arguments!");
        exit(EXIT_FAILURE);
    }
    

    return 0;
}