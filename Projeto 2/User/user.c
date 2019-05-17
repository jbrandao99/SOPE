#include "../sope.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

int num_account;
int num_delay;
int op_number;
int ulog;
char size_pass[MAX_PASSWORD_LEN + 1];
char args[3][1000];

req_header_t req_header;
req_value_t req_value;
req_create_account_t req_create_account;
req_transfer_t req_transfer;
tlv_request_t message;

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

bool checkArgs(int op)
{
  if (atoi(args[0]) < 1 || atoi(args[0]) > MAX_BANK_ACCOUNTS)
  {
    printf("Invalid Account ID!\n");
    return false;
  }
  else if (strtoul(args[1],NULL,10) < MIN_BALANCE || strtoul(args[1],NULL,10) > MAX_BALANCE)
  {
    printf("Invalid account balance!\n");
    return false;
  }

  if (op == OP_CREATE_ACCOUNT)
  {
    if (strlen(args[2]) < MIN_PASSWORD_LEN || strlen(args[2]) > MAX_PASSWORD_LEN)
    {
      printf("Invalid Password!\n");
      return false;
    }
  }

  return true;
}

void createAccount()
{
  if (!checkArgs(OP_CREATE_ACCOUNT))
  {
    exit(EXIT_FAILURE);
  }

  req_value.header = req_header;
  req_create_account.account_id = atoi(args[0]);
  req_create_account.balance = atoi(args[1]);
  strcpy(req_create_account.password, args[2]);
  req_value.create = req_create_account;
}

void transfer()
{
  if (!checkArgs(OP_TRANSFER))
  {
    exit(EXIT_FAILURE);
  }

  req_value.header = req_header;
  req_transfer.account_id = atoi(args[0]);
  req_transfer.amount = atoi(args[1]);
  req_value.transfer = req_transfer;
}

void openUserFile()
{
  ulog = open(USER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0664);

  if (ulog == -1)
  {
    printf("Error opening user log file\n");
    exit(EXIT_FAILURE);
  }
}

void closeServerFile()
{
  close(ulog);
}

void setArgs(char* arg5)
{
  char fifthArg[1000];
  memcpy(fifthArg, arg5, strlen(arg5) + 1);
  char *token;
  token = strtok(fifthArg, " ");

  for (int i = 0; token != NULL; i++)
  {
    strcpy(args[i], token);
    token = strtok(NULL, " ");
  }
}

void operation()
{
  switch (op_number)
  {
  case (OP_CREATE_ACCOUNT):
    createAccount();
    break;
  case (OP_BALANCE):
    req_value.header = req_header;
    break;
  case (OP_TRANSFER):
    transfer();
    break;
  case (OP_SHUTDOWN):
    req_value.header = req_header;
    break;
  }
}

void setMessage()
{
  message.value = req_value;
  message.type = op_number;
  message.length = sizeof(message);
}

void sendMessage(){

}

void receiveMessage(){

}

int main(int argc, char *argv[])
{
  verifyArgs(argc, argv);

  sincronizeHeader();

  setArgs(argv[5]);

  operation();

  setMessage();

  sendMessage();

  receiveMessage();

  return 0;
}
