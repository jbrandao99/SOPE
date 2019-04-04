#include <unistd.h>
#include <stdlib.h> 
#include <stdio.h> 
#include <sys/types.h> 

int main(void) {       
    int a[2], fd[2];            
    pid_t pid;            
    //char line[MAXLINE];            
    
if (pipe(fd) < 0) {             
    fprintf(stderr, "pipe error\n");                  
    exit(1);            
    }            
    
if ( (pid = fork()) < 0) {             
    fprintf(stderr,      "fork error\n");                  
    exit(2);            }            
else if (pid > 0) {  
    scanf("%d %d", &a[0], &a[1]);         /* pai */             
    close(fd[0]);
    write(fd[1],a,2*sizeof(int));
    close(fd[1]);      
    }      
else      {       
    int b[2];                 /*      filho      */                  
    close(fd[1]);           /* fecha lado emissor do pipe */                                     
    read(fd[0],b,2*sizeof(int));
    printf("\nSum: %d\n", b[0] + b[1]);
    printf("Sub: %d\n", b[0] - b[1]);
    printf("Mult: %d\n", b[0] * b[1]);
    printf("Div: %d\n", b[0] / b[1]);
    close(fd[0]); 
    } 
}