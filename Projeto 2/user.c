#include "sope.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int num_password;
int num_accounts;


//FAZ NA MAIN POR ISSO E QUE TEM ARGV
void verifyArgs(char *args[]){

    num_password= strlen(argv[2]);    //Tamanho da pass
    if(num_password < MIN_PASSWORD_LEN || num_password > MAX_PASSWORD_LEN){
        printf("Password invalida, tente com %d a %d caracteres\n", MIN_PASSWORD_LEN, MAX_PASSWORD_LEN);
        exit(1);
    }

    num_accounts=atoi(args[1]);
    if(num_accounts>MAX_BANK_ACCOUNTS || num_accounts<=0)
    {
      printf("Numero invalido de conta %d\n",MAX_BANK_ACCOUNTS);
      exit(1);
    }


}
