#include "sope.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

bank_account_t bank_accounts[MAX_BANK_ACCOUNTS];
int num_accounts;
int fd, fd_dummy;
int num_bancos;
int num_password;

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
    if(unlink(SERVER_FIFO_PATH) < 0)
        {
            printf("Error when destroying FIFO %s\n",SERVER_FIFO_PATH);
        }
        else
        {
            printf("FIFO %s has been destroyed\n", SERVER_FIFO_PATH);
            exit(EXIT_SUCCESS);
        }
}

void openServerFIFO()
{
    if((fd = open(SERVER_FIFO_PATH, O_RDONLY)) != -1)
    {
        printf("FIFO %s openned in READONLY mode\n", SERVER_FIFO_PATH);
    }
    
    if((fd_dummy = open(SERVER_FIFO_PATH, O_WRONLY)) != -1)
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
    chmod(SERVER_FIFO_PATH, S_IRUSR|S_IRGRP|S_IROTH);
}

int balance(uint32_t id)
{
    bank_account_t bank_account;
    for (unsigned int i = 0; i < num_accounts; i++)
    {
        if(bank_accounts[i].account_id == id)
        {
            bank_account = bank_accounts[i];
            break;
        }
    }
    return bank_account.balance;
}

void verifyArgs(char *argv[]){

    num_bancos = atoi(argv[1]);
    if(num_bancos>MAX_BANK_OFFICES||num_bancos<=0)
    {
        printf("Numero de balcoes eletronicos invalido %d\n",MAX_BANK_OFFICES);
        exit(1);
    }
    num_password= strlen(argv[2]);    //Tamanho da pass
    if(num_password < MIN_PASSWORD_LEN || num_password > MAX_PASSWORD_LEN){
        printf("Password invalida, tente com %d a %d caracteres\n", MIN_PASSWORD_LEN, MAX_PASSWORD_LEN);
        exit(1);
    }
}
