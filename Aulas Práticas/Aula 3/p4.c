#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h>  
#include <string.h> 

int main(void)
{
    if(fork() > 0)
    {
        printf("Hello ");
    }
    else
    {
        printf("World!");
    }
    return 0;
}