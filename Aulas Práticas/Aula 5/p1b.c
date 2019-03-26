#include <unistd.h>
#include <stdlib.h> 
#include <stdio.h> 
#include <sys/types.h> 

struct numeros
    {
        /* data */
        int num1;
        int num2;
    };

int main(void) {       
    int fd[2];            
    pid_t pid;            
    struct numeros n;            
    
if (pipe(fd) < 0) {             
    fprintf(stderr, "pipe error\n");                  
    exit(1);            
    }            
    
if ( (pid = fork()) < 0) {             
    fprintf(stderr,      "fork error\n");                  
    exit(2);            }            
else if (pid > 0) {  
    scanf("%d %d", &n.num1, &n.num2);         /* pai */             
    close(fd[0]);
    write(fd[1],&n,sizeof(n));
    close(fd[1]);      
    }      
else      {     
    struct numeros n2;                   /*      filho      */                  
    close(fd[1]);           /* fecha lado emissor do pipe */                                     
    read(fd[0],&n2,sizeof(n2));
    printf("\nSum: %d\n", n2.num1 + n2.num2);
    printf("Sub: %d\n", n2.num1 - n2.num2);
    printf("Mult: %d\n", n2.num1 * n2.num2);
    printf("Div: %d\n", n2.num1 / n2.num2);
    close(fd[0]); 
    } 
}