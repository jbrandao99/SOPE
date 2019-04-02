#include "writeonstdout.h"

void WriteOnSTDOUT(int argc, char *argv[])
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
    }
    else
    {
        perror("No arguments!");
    }
}