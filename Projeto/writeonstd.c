#include "writeonstd.h"
#include "funcaoexec.h"
#include "tratastring.h"

void writeonstd(char *argv)
{
    struct stat buf;
    char tm[20]; //time modified
    char tc[20]; //time created
    char *out;
    char *type;

    stat(argv, &buf);
    strftime(tm, 20, "%Y-%m-%dT%H:%M:%S", localtime(&(buf.st_mtime)));
    strftime(tc, 20, "%Y-%m-%dT%H:%M:%S", localtime(&(buf.st_ctime)));

    printf("%s,", argv);
    out = funcaoexec(argv,"file");
    type = typestring(type,argv);
    printf("%s,", type);
    printf("%ld,", buf.st_size);
    printf((buf.st_mode & S_IRUSR) ? "r" : "");
    printf((buf.st_mode & S_IWUSR) ? "w" : "");
    printf((buf.st_mode & S_IXUSR) ? "x" : ",");
    printf("%s,", tc);
    printf("%s", tm);
    printf("\n");
}
