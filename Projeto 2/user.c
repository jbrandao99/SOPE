#include "./sope.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "./log.c"
#include <signal.h>

int num_account;
char password[MAX_PASSWORD_LEN + 1];
int num_delay;
int op_number;
unsigned int length;
char args[3][512];
req_create_account_t req_create_account;
req_value_t req_value;
req_transfer_t req_transfer;
req_header_t req_header;
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

  strcpy(password, argv[2]);
  if (strlen(password) < MIN_PASSWORD_LEN || strlen(password) > MAX_PASSWORD_LEN)
  {
    printf("Invalid Password. Must have more than %d and less than %d characters\n", MIN_PASSWORD_LEN, MAX_PASSWORD_LEN);
    exit(EXIT_FAILURE);
  }

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

int openUserFile()
{
  int ulog = open(USER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0777);

  if (ulog < 0)
  {
    exit(EXIT_FAILURE);
  }

  return ulog;
}

bool checkArgs(int op)
{
  if (atoi(args[0]) < 1 || atoi(args[0]) > MAX_BANK_ACCOUNTS)
  {
    printf("Invalid Account ID!\n");
    return false;
  }
  else if (strtoul(args[1], NULL, 10) < MIN_BALANCE || strtoul(args[1], NULL, 10) > MAX_BALANCE)
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

  req_create_account.account_id = atoi(args[0]);
  req_create_account.balance = atoi(args[1]);
  strcpy(req_create_account.password, args[2]);
  req_value.create = req_create_account;
  length += sizeof(req_create_account.account_id) + sizeof(req_create_account.balance) + strlen(req_create_account.password);
}

void transfer()
{
  if (!checkArgs(OP_TRANSFER))
  {
    exit(EXIT_FAILURE);
  }

  req_transfer.account_id = atoi(args[0]);
  req_transfer.amount = atoi(args[1]);
  req_value.transfer = req_transfer;
  length += sizeof(req_transfer.account_id) + sizeof(req_transfer.amount);
}

void setArgs(char *arg5)
{
  char fifthArg[512];
  memcpy(fifthArg, arg5, strlen(arg5) + 1);
  char *token;
  token = strtok(fifthArg, " ");

  for (int i = 0; token != NULL; i++)
  {
    strcpy(args[i], token);
    //printf("%s\n", args[i]);
    token = strtok(NULL, " ");
  }
}

void fillRequestFields()
{
  req_header.account_id = num_account;
  req_header.op_delay_ms = num_delay;
  strcpy(req_header.password, password);
  req_header.pid = getpid();
  req_value.header = req_header;
  length += sizeof(req_header.account_id) + sizeof(req_header.op_delay_ms) + strlen(req_header.password) + sizeof(req_header.pid);
}

void fillRequestMessage()
{
  message.type = op_number;
  message.value = req_value;
  message.length = length;
}

int openServerFIFO(tlv_reply_t *reply, int ulog)
{
  int fd;

  if ((fd = open(SERVER_FIFO_PATH, O_WRONLY)) < 0)
  {
    reply->length = sizeof(op_type_t) + sizeof(ret_code_t);
    reply->type = op_number;
    reply->value.header.ret_code = RC_SRV_DOWN;

    logReply(ulog, getpid(), reply);
    exit(EXIT_FAILURE);
  }

  return fd;
}

int openUserFIFO(char *userFIFO)
{
  int fd = -1;

  while (fd < 0)
  {
    if ((fd = open(userFIFO, O_RDONLY | O_NONBLOCK)) < 0)
    {
      sleep(1);
    }
  }

  return fd;
}

int readUserFIFO(tlv_reply_t *reply, int userFD)
{
  unsigned int counter = 0;
  while ((read(userFD, reply, sizeof(tlv_reply_t))) < 0)
  {
    if (counter >= FIFO_TIMEOUT_SECS)
    {
      return -1;
    }

    counter++;
    sleep(1);
  }
  return 0;
}

int main(int argc, char *argv[])
{
  verifyArgs(argc, argv);
  setArgs(argv[5]);

  tlv_reply_t *tlv_reply = (tlv_reply_t *)malloc(sizeof(tlv_reply_t));
  mode_t old_mask = umask(0000);

  //Get FIFO name
  char *pidBuffer = malloc(sizeof(WIDTH_ID));
  pid_t pid = getpid();
  sprintf(pidBuffer, "%*d", WIDTH_ID, pid);
  char *userFIFO = malloc(USER_FIFO_PATH_LEN);
  strcpy(userFIFO, USER_FIFO_PATH_PREFIX);
  strcat(userFIFO, pidBuffer);

  //Make FIFO
  if (mkfifo(userFIFO, 0777) != 0)
  {
    exit(EXIT_FAILURE);
  }

  //Check type of usage
  if (num_account == 0)
  {
    if (op_number == OP_CREATE_ACCOUNT)
    {
      createAccount();
    }
  }
  else
  {
    if (op_number == OP_TRANSFER)
    {
      transfer();
    }
  }

  //Fill request header fields
  fillRequestFields();

  //Fill request message
  fillRequestMessage();

  int ulog = openUserFile();
  logRequest(ulog, getpid(), &message);

  int serverFD = openServerFIFO(tlv_reply, ulog);

  write(serverFD, &message, sizeof(tlv_request_t));

  int userFD = openUserFIFO(userFIFO);

  if (readUserFIFO(tlv_reply, userFD) != 0)
  {
    tlv_reply->length = sizeof(op_type_t) + sizeof(ret_code_t);
    tlv_reply->type = op_number;
    tlv_reply->value.header.ret_code = RC_SRV_TIMEOUT;

    logReply(ulog, getpid(), tlv_reply);

    free(tlv_reply);
    close(ulog);
    close(userFD);
    unlink(userFIFO);
    umask(old_mask);
    exit(EXIT_FAILURE);
  }

  logReply(ulog, getpid(), tlv_reply);

  free(tlv_reply);
  close(ulog);
  close(userFD);
  unlink(userFIFO);
  umask(old_mask);

  return 0;
}
