#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>


int main(int argc, char* argv[], char* envp[])
{
    int fd = open("file.txt",O_WRONLY | O_CREAT | O_RDONLY,0600);

    if(fd == -1)
    {
        perror("Error opening or creating the file");
        exit(1);
    }

    write(fd,"Nice job!\n", 11);

    close(fd);

    return 0;
}