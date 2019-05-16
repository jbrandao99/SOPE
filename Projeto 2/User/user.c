#include "../sope.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int num_accounts;
int num_delay;
int op_number;
req_header_t user;
req_value_t value;

void verifyArgs(int argc, char *argv[])
{

  if (argc != 6)
  {
    printf("Wrong number of arguments\n");
    exit(EXIT_FAILURE);
  }

  num_accounts = atoi(argv[1]);
  if (num_accounts > MAX_BANK_ACCOUNTS || num_accounts < 0)
  {
    printf("Invalid Number of Accounts: %d\n", MAX_BANK_ACCOUNTS);
    exit(EXIT_FAILURE);
  }

  char size_pass[MAX_PASSWORD_LEN + 1];
  strcpy(size_pass, argv[2]);
  if (strlen(size_pass) < MIN_PASSWORD_LEN || strlen(size_pass) > MAX_PASSWORD_LEN)
  {
    printf("Invalid Password. Must have more than %d and less than %d characters\n", MIN_PASSWORD_LEN, MAX_PASSWORD_LEN);
    exit(EXIT_FAILURE);
  }
  size_pass[strlen(size_pass)] = '\0';

  num_delay = atoi(argv[3]);
  if (num_delay > MAX_OP_DELAY_MS || num_delay < 1)
  {
    printf("Invalid delay: %d\n", MAX_OP_DELAY_MS);
    exit(EXIT_FAILURE);
  }

  op_number = atoi(argv[4]);
  if (op_number < 0 || op_number > 3)
  {
    printf("Invalid Operation Code: %d\n", op_number);
    exit(EXIT_FAILURE);
  }
}

void sincronizeUser(char *args[]) //FALTA PID
{
  user.account_id = atoi(args[1]);
  strcpy(user.password, args[2]);
  user.op_delay_ms = atoi(args[3]);
}

void adminAcess(int code)
{
  if (code == OP_CREATE_ACCOUNT)
  {
    //
  }
  if (code == OP_SHUTDOWN)
  {
    //
  }
  else
  {
    printf("Code only available to user acess %d\n", code);
    exit(1);
  }
}

void userAcess(int code)
{
  if (code == OP_BALANCE)
  {
    //
  }
  if (code == OP_TRANSFER)
  {
    //
  }
  else
  {
    printf("Code only available to client acess %d\n", code);
    exit(1);
  }
}

int main(int argc, char *argv[])
{

  verifyArgs(argc, argv);
  sincronizeUser(argv);
  if (num_accounts == ADMIN_ACCOUNT_ID)
  {
    adminAcess(num_operacao);
  }
  else
  {
    userAcess(num_operacao);
  }
}
