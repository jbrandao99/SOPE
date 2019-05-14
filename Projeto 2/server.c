#include "sope.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

bank_account_t bank_accounts[MAX_BANK_ACCOUNTS];
int num_accounts;

bool login(uint32_t id, char* pass)
{
    for(unsigned int i = 0; i < num_accounts; i++)
    {
        if(bank_accounts[i].account_id == id)
        {
            //falta password (hash)
            return true;
        }
    }
    return false;
}

void createServerFIFO()
{
    if(mkfifo(SERVER_FIFO_PATH, 0660) < 0)
    {
        if(errno == EEXIST)
        {
        printf("FIFO %s alreay exists\n", SERVER_FIFO_PATH);
        }
        else
        {
            printf("Can't create FIFO\n");
        }
        
        if(unlink(SERVER_FIFO_PATH) < 0)
        {
            printf("Error when destroying FIFO %s\n",SERVER_FIFO_PATH);
        }
        else
        {
            printf("FIFO %s has been destroyed\n", SERVER_FIFO_PATH);
        }
        exit(0);
    }
}

