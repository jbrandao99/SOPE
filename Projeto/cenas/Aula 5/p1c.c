#include <unistd.h>
#include <stdlib.h> 
#include <stdio.h> 
#include <sys/types.h> 
#include <string.h>


int main(void) {       
    int fd[2];            
    pid_t pid;            
    char* num1;
    char* num2;
    char* num3;
    char* num4;
           
    
if (pipe(fd) < 0) {             
    fprintf(stderr, "pipe error\n");                  
    exit(1);            
    }            
    
if ( (pid = fork()) < 0) {             
    fprintf(stderr,      "fork error\n");                  
    exit(2);            }            
else if (pid > 0) {  
    scanf("%s %s", num1, num2);         /* pai */             
    close(fd[0]);
    write(fd[1],num1,strlen(num1)*sizeof(char));
    write(fd[1],num2,strlen(num2) *sizeof(char));
    close(fd[1]);      
    }      
else      {     ;                   /*      filho      */                  
    close(fd[1]);           /* fecha lado emissor do pipe */                                     
    read(fd[0],num3,strlen(num3)*sizeof(num3));
    read(fd[0],num4,strlen(num4) *sizeof(num4));
    printf("\nSum: %d\n", atoi(num3) + atoi(num4));
    printf("Sub: %d\n", atoi(num3) - atoi(num4));
    printf("Mult: %d\n", atoi(num3) * atoi(num4));
    printf("Div: %d\n", atoi(num3) / atoi(num4));
    close(fd[0]); 
    } 
}