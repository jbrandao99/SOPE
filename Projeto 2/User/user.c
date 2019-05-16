#include "sope.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int num_password;
int num_accounts;
int num_delay;
int num_operacao;
req_header_t user;
req_value_t value;


//FAZ NA MAIN POR ISSO E QUE TEM ARGV
void verifyArgs(char *args[]){

   num_accounts=atoi(args[1]);
   if(num_accounts>MAX_BANK_ACCOUNTS || num_accounts<1)
   {
     printf("Numero invalido de conta %d\n",MAX_BANK_ACCOUNTS);
     exit(1);
   }

    num_password= strlen(args[2]);    //Tamanho da pass
    if(num_password < MIN_PASSWORD_LEN || num_password > MAX_PASSWORD_LEN){
        printf("Password invalida, tente com %d a %d caracteres\n", MIN_PASSWORD_LEN, MAX_PASSWORD_LEN);
        exit(1);
    }

    num_delay = atoi(args[3]);
    if(num_delay>MAX_OP_DELAY_MS || num_delay<1)
    {
      printf("Delay invalido %d\n", MAX_OP_DELAY_MS);
      exit(1);
    }

    num_operacao = atoi(args[4]);
    if(num_operacao<0 || num_operacao>3)
    {
    printf("Codigo de operacao invalido %d\n",num_operacao);
    exit(1);
    }

}

void sincronizeUser(char *args[])  //FALTA PID
{
  user.account_id= atoi(args[1]);
  strcpy(user.password,args[2]);
  user.op_delay_ms=atoi(args[3]);

}

void adminAcess(int code)
{
  if(code==OP_CREATE_ACCOUNT)
  {
    //
  }
  if(code==OP_SHUTDOWN)
  {
    //
  }
  else
  {
    printf("Code only available to user acess %d\n",code);
    exit(1);
  }


}

void userAcess(int code)
{
  if(code==OP_BALANCE)
  {
    //
  }
  if(code==OP_TRANSFER)
  {
    //
  }
  else
  {
    printf("Code only available to client acess %d\n",code);
    exit(1);
  }


}




int main(int argc, char *argv[]) {

if(argc!=6)
{
  exit(1);
}
verifyArgs(argv);
sincronizeUser(argv);
if(num_accounts==ADMIN_ACCOUNT_ID)
{
  adminAcess(num_operacao);
}
else
{
  userAcess(num_operacao);
}








}
