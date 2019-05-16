#include "../sope.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int num_account;
int num_delay;
int op_number;
char size_pass[MAX_PASSWORD_LEN + 1];
req_header_t req_header;

void verifyArgs(int argc, char *argv[])
{

  if (argc != 6)
  {
    printf("Wrong number of arguments\n");
    exit(EXIT_FAILURE);
  }

  num_account = atoi(argv[1]);
  if (num_account > MAX_BANK_ACCOUNTS || num_account < 0)
  {
    printf("Invalid Number of Accounts: %d\n", MAX_BANK_ACCOUNTS);
    exit(EXIT_FAILURE);
  }

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

void sincronizeHeader()
{
    req_header.pid = getpid();
    req_header.account_id = num_account;
    strcpy(req_header.password, size_pass);
    req_header.op_delay_ms = num_delay;
}

int main(int argc, char *argv[])
{

  verifyArgs(argc, argv);

  sincronizeHeader();
}
