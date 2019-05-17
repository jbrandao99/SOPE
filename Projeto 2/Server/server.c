#include "../sope.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

bank_account_t bank_accounts[MAX_BANK_ACCOUNTS];
unsigned int num_accounts = 0;
int fd, fd_dummy;
int num_banks;
int slog;
static const char characters[] = "0123456789abcdef";
sem_t sem1, sem2;
int val1, val2;
pthread_t *balcao;

//NS SE SE VAI USAR
/* bool login(uint32_t id, char *pass)
{
    for (unsigned int i = 0; i < num_accounts; i++)
    {
        if (bank_accounts[i].account_id == id)
        {
            //falta password (hash)
            return true;
        }
    }
    return false;
} */

void createServerFIFO()
{
    if (mkfifo(SERVER_FIFO_PATH, 0660) < 0)
    {
        if (errno == EEXIST)
        {
            printf("FIFO %s alreay exists\n", SERVER_FIFO_PATH);
        }
        else
        {
            printf("Can't create FIFO\n");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        printf("FIFO %s successfully created\n", SERVER_FIFO_PATH);
    }
}

void destroyServerFIFO()
{
    if (unlink(SERVER_FIFO_PATH) < 0)
    {
        printf("Error when destroying FIFO %s\n", SERVER_FIFO_PATH);
    }
    else
    {
        printf("FIFO %s has been destroyed\n", SERVER_FIFO_PATH);
        exit(EXIT_SUCCESS);
    }
}

void openServerFIFO()
{
    if ((fd = open(SERVER_FIFO_PATH, O_RDONLY)) != -1)
    {
        printf("FIFO %s openned in READONLY mode\n", SERVER_FIFO_PATH);
    }

    if ((fd_dummy = open(SERVER_FIFO_PATH, O_WRONLY)) != -1)
    {
        printf("FIFO %s openned in WRITEONLY mode\n", SERVER_FIFO_PATH);
    }
}

void closeServerFIFO()
{
    close(fd);
    close(fd_dummy);
}

void shutdown()
{
    chmod(SERVER_FIFO_PATH, S_IRUSR | S_IRGRP | S_IROTH);
}

int balance(uint32_t id)
{
    bank_account_t bank_account;
    for (unsigned int i = 0; i < num_accounts; i++)
    {
        if (bank_accounts[i].account_id == id)
        {
            bank_account = bank_accounts[i];
            break;
        }
    }
    return bank_account.balance;
}

void verifyArgs(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Wrong number of arguments\n");
        exit(EXIT_FAILURE);
    }

    num_banks = atoi(argv[1]);
    if (num_banks > MAX_BANK_OFFICES || num_banks <= 0)
    {
        printf("Invalid Number of Eletronic Counters: %d\n", MAX_BANK_OFFICES);
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
}

char *getHash(char *password, char *salt)
{

    char cmd[1024] = "echo -n ";
    strcat(cmd, password);
    strcat(cmd, salt);
    strcat(cmd, " | sha256sum");

    FILE *fp;
    fp = popen(cmd, "r");
    if (fp == NULL)
    {
        printf("Failed to run command\n");
        exit(EXIT_FAILURE);
    }

    char output[1024];
    fscanf(fp, "%[^\n]", output);
    pclose(fp);
    return strdup(&output[0]);
}

char *getSalt()
{
    char *salt = (char *)malloc((SALT_LEN + 1) * sizeof(char));
    unsigned int i;
    for (i = 0; i < SALT_LEN; i++)
    {
        int n = rand() % (int)(sizeof(characters) - 1);
        salt[i] = characters[n];
    }
    salt[i] = '\0';

    return salt;
}

bool isAccount(uint32_t id)
{
    for (unsigned int i = 0; i < num_accounts; i++)
    {
        if (bank_accounts[i].account_id == id)
        {
            return true;
        }
    }
    return false;
}

void addAccount(bank_account_t bank_account)
{
    bank_accounts[num_accounts] = bank_account;
    num_accounts++;
}

int createAccount(uint32_t id, uint32_t balance, char *password)
{
    bank_account_t bank_account;
    char *salt;

    if (isAccount(id))
    {
        return RC_ID_IN_USE;
    }

    bank_account.account_id = id;
    bank_account.balance = balance;
    salt = getSalt();
    strcpy(bank_account.salt, salt);
    strcpy(bank_account.hash, getHash(password, salt));
    addAccount(bank_account);
    return RC_OK;
}

bank_account_t *getAccount(uint32_t id)
{
    bank_account_t *bank_account = (bank_account_t *)malloc(sizeof(bank_account_t));
    bank_account = NULL;

    for (unsigned int i = 0; i < num_accounts; i++)
    {
        if (bank_accounts[i].account_id == id)
        {
            bank_account = &bank_accounts[i];
            break;
        }
    }
    return bank_account;
}

int transfer(uint32_t src_id, uint32_t dest_id, uint32_t amount)
{
    if (!isAccount(src_id))
    {
        return RC_OTHER;
    }

    if (!isAccount(dest_id))
    {
        return RC_ID_NOT_FOUND;
    }

    if (src_id == dest_id)
    {
        return RC_SAME_ID;
    }

    bank_account_t *src_account, *dest_account;
    src_account = getAccount(src_id);
    dest_account = getAccount(dest_id);

    if (src_account->balance < amount)
    {
        return RC_NO_FUNDS;
    }

    if (dest_account->balance + amount > MAX_BALANCE)
    {
        return RC_TOO_HIGH;
    }

    src_account->balance -= amount;
    dest_account->balance += amount;

    return RC_OK;
}

void openServerFile()
{
    slog = open(SERVER_LOGFILE, O_WRONLY|O_CREAT|O_EXCL, 0664);

    if (slog == -1)
    {
        printf("Error opening server log file\n");
        exit(EXIT_FAILURE);
    }
}

void closeServerFile()
{
    close(slog);
}

void esperaBalcao()
{
    for (int i = 0; i < num_banks; i++)
    {
        pthread_join(balcao[i], NULL);
    }  
}

void createSemaphore()
{
    sem_init(&sem1,0,num_banks);
    sem_getvalue(&sem1,&val1);

    //logSyncMechSem(STDOUT_FILENO, MAIN_THREAD_ID, SYNC_OP_SEM_INIT, SYNC_ROLE_PRODUCER, 0, val1);

    sem_init(&sem2, 0, 0);
    sem_getvalue(&sem2, &val2);
    
    //logSyncMechSem(STDOUT_FILENO, MAIN_THREAD_ID, SYNC_OP_SEM_INIT, SYNC_ROLE_PRODUCER, 0, val2);
}

int main(int argc, char *argv[])
{
    verifyArgs(argc, argv);

    srand(time(NULL));

    createSemaphore();

    createAccount(ADMIN_ACCOUNT_ID, 0, argv[2]);

    //logAccountCreation(STDOUT_FILENO, MAIN_THREAD_ID, &bank_accounts[ADMIN_ACCOUNT_ID]);

    openServerFile();

    createServerFIFO();

    openServerFIFO();

    esperaBalcao();

    closeServerFIFO();

    destroyServerFIFO();

    closeServerFile();

    return 0;
}
