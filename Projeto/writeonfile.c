#include "writeonfile.h"

void WriteOnFile(int argc, char *argv[])
{
    int fd;
    fd = open("file.txt", O_WRONLY | O_APPEND);
    dup2(fd, STDOUT_FILENO);
    WriteOnSTDOUT(argc, argv);
    close(fd);
}