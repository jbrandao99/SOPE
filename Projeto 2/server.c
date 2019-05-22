#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>

#include "sope.h"
#include "log.c"

#define NEW_MASK 0000

#define FIFO_RW_MODE 0777
#define FIFO_READ_MODE 0444

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER; // Mutex

pthread_mutex_t bank_lock = PTHREAD_MUTEX_INITIALIZER; // Mutex de acesso ao banco
pthread_mutex_t req = PTHREAD_MUTEX_INITIALIZER; // Mutex de acesso ao buffer de pedidos


/* Mutexs de operacoes a realizar por cada balcao */
pthread_mutex_t create_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t transfer_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t shutdown_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t balance_lock = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t reply_lock = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

sem_t sem; // Semaforos usados entre main e balcoes
int val; // valor dos semaforos

bank_account_t bank_accounts[MAX_BANK_ACCOUNTS]; // Guarda as contas dos users
int num_accounts = 0; //numero de contas no banco

int srv_fifo_fd; // Descritor do fifo srv
int user_fifo_fd; // Descritor do fifo user
int fd_dummy;
char *user_fifo = ""; // Nome do user fifo

tlv_request_t num_requests[MAX_BANK_ACCOUNTS]; // num_requests usado para comunicao entre Produtor e Consumidor
int num_request = 0; // numero total de pedidos
int num_requests2 = 0; // numero de pedidos atendidos

int num_threads; // Numero de banco eletronicos

int length = 0; // size (bytes) of message

int slog; // fd slog.txt file

int is_open = 1; // Boolean

int pedido = 0; // Pedidos recebidos e processados

int processing = 0; // Numero de balcoes a processar o pedido

static const char carac[] = "0123456789abcdef";

char *getSalt()
  {
    unsigned int i;
  	char *salt = (char *)malloc((SALT_LEN + 1) * sizeof(char));
  	for (i = 0; i < SALT_LEN; i++)
  	{
  		int j = rand() % (int)(sizeof(carac) - 1);
  		salt[i] = carac[j];
  	}
  	salt[i] = '\0';
  	return salt;
}

char *getHash(char *pass, char *salt){
    FILE *fp;
    char buf[MAX_PASSWORD_LEN];
    strcpy(buf,pass);
    strcat(buf, salt);
    char command[256];
    char *hash  = (char *) malloc(sizeof(char) * (HASH_LEN + 1));
    sprintf(command, "echo -n %s | sha256sum",pass);
    fp = popen(command,"r");
    if(fgets(hash, HASH_LEN + 1, fp) != NULL){
        pclose(fp);
        return hash;
    }
    return NULL;
}

bank_account_t *getBankAccount(uint32_t id){
    bank_account_t *acc = (bank_account_t *)malloc(sizeof(bank_account_t));
    acc = NULL;
    for(int i = 0; i < num_accounts; i++){
        if(bank_accounts[i].account_id == id)
        {
            acc = &bank_accounts[i];
        }
    }
    return acc;
}

int login(uint32_t id, char *pass){
  bank_account_t *acc = getBankAccount(id);
  if(acc == NULL)
  {
      return false;
  }
    char newpass[MAX_PASSWORD_LEN + 1];
    strcpy(newpass,pass);
    char *hash = getHash(newpass, acc->salt);
    if(!strcmp(acc->hash, hash) == 0)
    {
        return false;
    }

    return true;
}

void makeServerFIFO(){
  if (mkfifo(SERVER_FIFO_PATH, 0660) < 0)
	{
		if (errno == EEXIST)
		{
			printf("FIFO %s alreay exists\n", SERVER_FIFO_PATH);
		}
		else
		{
			printf("Can't create FIFO\n");
		}
		unlink(SERVER_FIFO_PATH);
		exit(EXIT_FAILURE);
	}
	else
	{
		printf("FIFO %s successfully created\n", SERVER_FIFO_PATH);
		return;
	}
}

void openServerFIFO(){
  if ((srv_fifo_fd = open(SERVER_FIFO_PATH, O_RDONLY)) != -1)
  {
    printf("FIFO %s openned in READONLY mode\n", SERVER_FIFO_PATH);
  }
  if ((fd_dummy = open(SERVER_FIFO_PATH, O_WRONLY)) != -1)
  {
    printf("FIFO %s openned in WRITEONLY mode\n", SERVER_FIFO_PATH);
  }
}


void processArgs(int argc,char *argv[]){

    if(argc!=3)
    {
      printf("Wrong number of arguments\n");
      exit(EXIT_FAILURE);
    }

    num_threads = atoi(argv[1]);
    if(num_threads<=0||num_threads > MAX_BANK_OFFICES){
      printf("Invalid Number of Eletronic Counters: %d\n", MAX_BANK_OFFICES);
      exit(EXIT_FAILURE);
    }

    char size_pass[MAX_PASSWORD_LEN + 1];
  	strcpy(size_pass, argv[2]);
    if(strlen(size_pass) < MIN_PASSWORD_LEN || strlen(size_pass) > MAX_PASSWORD_LEN){
      printf("Invalid Password. Must have more than %d and less than %d characters\n", MIN_PASSWORD_LEN, MAX_PASSWORD_LEN);
      exit(EXIT_FAILURE);
    }
}

void put_request(tlv_request_t tlv_request)

{
    pthread_mutex_lock(&req);

    num_requests[num_request] = tlv_request;
    num_request = (num_request + 1) % MAX_BANK_ACCOUNTS;

    pthread_mutex_unlock(&req);
    return;
}

void get_request(tlv_request_t *request){
    pthread_mutex_lock(&req);

    *request = num_requests[num_requests2];
    num_requests2 = (num_requests2 + 1) % MAX_BANK_ACCOUNTS;

    pthread_mutex_unlock(&req);
    return;
}

int accountExists(uint32_t id){
	for (  int i = 0; i < num_accounts; i++)
	{
		if (bank_accounts[i].account_id == id)
		{
			return true;
		}
	}
	return false;
}

int createAccount(uint32_t id, uint32_t balance, char *pass){
  if(accountExists(id)){
      return RC_ID_IN_USE;
  }

    bank_account_t acc;
    acc.balance = balance;
    acc.account_id = id;
    char *salt = getSalt();
    strcpy(acc.salt, salt);
    char *newpass = pass;
    char *hash = getHash(newpass, salt);
    strcpy(acc.hash, hash);

    bank_accounts[num_accounts] = acc;
    num_accounts++;
    return RC_OK;
}

int transfer(uint32_t id_source, uint32_t id_destination,uint32_t amount, uint32_t delay){
  if(id_destination == ADMIN_ACCOUNT_ID)
  {
    return RC_OP_NALLOW;
  }
  if(!accountExists(id_destination))
  {
    return RC_ID_NOT_FOUND;
  }

  if(!accountExists(id_source))
  {
      return RC_OTHER;
  }

  if(id_source == id_destination){
      return RC_SAME_ID;
  }

  bank_account_t *acc_source=getBankAccount(id_source);
  bank_account_t *acc_destination=getBankAccount(id_destination);


  if(acc_destination == NULL){
      return RC_OTHER;
  }

  usleep(delay);

  if(acc_destination->balance+amount > MAX_BALANCE){
      return RC_TOO_HIGH;
  }

  if(acc_source->balance < amount){
      return RC_NO_FUNDS;
  }

//TRANSFERENCIA
  acc_destination->balance = acc_destination->balance + amount;
  acc_source->balance = acc_source->balance- amount;
  return RC_OK;
}


int getBalance(uint32_t id){
    bank_account_t *acc = getBankAccount(id);
    if(acc == NULL)
    {
        return 0;
    }
    return acc->balance;
}

void shutdown(){

   chmod(SERVER_FIFO_PATH,S_IRUSR|S_IRUSR|S_IROTH);
    is_open = 0;    //ENCERRA O LOOP DA FUNCAO BALCAOELETRONICO

    for(int i = 0; i <= num_threads; i++)
    {
      sem_post(&sem);
    }

}

void transferRequest(tlv_reply_t *tlv_reply,tlv_request_t request, void *num)
{
  length += sizeof(int) + sizeof(int);
  if(request.value.header.account_id == ADMIN_ACCOUNT_ID){    //SE FOR ADMINISTRADOR
      tlv_reply->value.header.ret_code = RC_OP_NALLOW;
  }
  else if(request.value.header.account_id != ADMIN_ACCOUNT_ID){   //NAO ADMINISTRADOR
      logDelay(slog, *(int *) num, request.value.header.op_delay_ms);
      tlv_reply->value.header.ret_code =transfer(request.value.header.account_id, request.value.transfer.account_id,request.value.transfer.amount,request.value.header.op_delay_ms * 1000);
      length += sizeof(request.value.transfer.amount);
  }
  else{      //OUTRO ERRO
      tlv_reply->value.header.ret_code = RC_OTHER;
  }
  bank_account_t *account = getBankAccount(request.value.header.account_id);
  tlv_reply->value.transfer.balance = account->balance; // New balance
}

void shutdownRequest(tlv_reply_t *tlv_reply,tlv_request_t request, void *num)
{
  logDelay(slog, *(int *) num, request.value.header.op_delay_ms);
  usleep(request.value.header.op_delay_ms * 1000);
  length += sizeof(request.value.header.account_id) + sizeof(int);
  if(request.value.header.account_id == ADMIN_ACCOUNT_ID){    //SE FOR ADMINISTRADOR
      shutdown();
      tlv_reply->value.header.ret_code = RC_OK;
      tlv_reply->value.shutdown.active_offices = processing-1;
      length += sizeof(int);
  }
  else if(request.value.header.account_id != ADMIN_ACCOUNT_ID){      //NAO ADMINISTRADOR
      tlv_reply->value.header.ret_code = RC_OP_NALLOW;
  }
  else{                                                            //OUTRO ERRO
      tlv_reply->value.header.ret_code = RC_OTHER;
  }
}

void balanceRequest(tlv_reply_t *tlv_reply,tlv_request_t request, void *num)
{
  logDelay(slog, *(int *) num, request.value.header.op_delay_ms);
  usleep(request.value.header.op_delay_ms * 1000);
  length += sizeof(int) + sizeof(int);
  if(request.value.header.account_id != ADMIN_ACCOUNT_ID){             //SE FOR ADMINISTRADOR
      tlv_reply->value.balance.balance = getBalance(request.value.header.account_id);
      length += sizeof(int);
  }
  else if(request.value.header.account_id == ADMIN_ACCOUNT_ID){                //NAO ADMINISTRADOR
      tlv_reply->value.header.ret_code = RC_OP_NALLOW;
  }
  else{                                                           //OUTRO ERRO
      tlv_reply->value.header.ret_code = RC_OTHER;
  }
}

void accountRequest(tlv_reply_t tlv_reply,tlv_request_t request, void *num)
{
  logDelay(slog, *(int *) num, request.value.header.op_delay_ms);
  usleep(request.value.header.op_delay_ms * 1000);
  length += sizeof(int) + sizeof(int);
  if(request.value.header.account_id == ADMIN_ACCOUNT_ID){                  //SE FOR ADMINISTRADOR
      int rc = createAccount(request.value.create.account_id, request.value.create.balance, request.value.create.password);
      tlv_reply.value.header.ret_code = rc;
      if(rc == RC_OK)
          logAccountCreation(slog, *(int *) num, &bank_accounts[num_accounts - 1]);
  }
  else if(request.value.header.account_id != ADMIN_ACCOUNT_ID){          //NAO ADMINISTRADOR
      tlv_reply.value.header.ret_code = RC_OP_NALLOW;
  }
  else{                                                                    //OUTRO ERRO
      tlv_reply.value.header.ret_code = RC_OTHER;
  }
  tlv_reply.value.header.account_id = *(int *) num;
  length += sizeof(*(int *)num) + sizeof(tlv_reply.value.header.ret_code);
}


void *balcaoEletronico(void *num){

    logBankOfficeOpen(slog,*(int *) num, pthread_self());
    tlv_reply_t tlv_reply;
    tlv_request_t request;

    while(is_open) {

        logSyncMech(slog, *(int *) num, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_CONSUMER, 0);
        pthread_mutex_lock(&mut);

        while(!(pedido > 0)){
            logSyncMech(slog, *(int *) num, SYNC_OP_COND_WAIT, SYNC_ROLE_CONSUMER, 0);
            pthread_cond_wait(&cond,&mut);
        }

        pedido--;

        pthread_mutex_unlock(&mut);
        logSyncMech(slog, *(int *) num, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_CONSUMER, 0);
        get_request(&request);
        sem_getvalue(&sem, &val);
        logSyncMechSem(slog, *(int *) num, SYNC_OP_SEM_WAIT, SYNC_ROLE_CONSUMER, request.value.header.pid, val);
        sem_wait(&sem);

        if(!is_open)
            break;

        if(!login(request.value.header.account_id, request.value.header.password)) // Verifica se consegue fazer o login
        {
          length += 2*sizeof(int);
          tlv_reply.value.header.ret_code = RC_LOGIN_FAIL;
          tlv_reply.value.header.account_id =*(int *)num;
        }
        else
        {
            processing++; // Inicio do processo
            switch(request.type){
                case OP_CREATE_ACCOUNT:
                logSyncMech(slog, *(int *) num, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_ACCOUNT, request.value.header.pid);
                pthread_mutex_lock(&create_lock);
                accountRequest(tlv_reply,request,num);
                pthread_mutex_unlock(&create_lock);
                logSyncMech(slog, *(int *) num, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_ACCOUNT, request.value.header.pid);
                break;
                case OP_BALANCE:
                logSyncMech(slog, *(int *) num, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_ACCOUNT, request.value.header.pid);
                pthread_mutex_lock(&balance_lock);
                balanceRequest(&tlv_reply,request,num);
                pthread_mutex_unlock(&balance_lock);
                logSyncMech(slog, *(int *) num, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_ACCOUNT, request.value.header.pid);
                break;
                case OP_SHUTDOWN:
                logSyncMech(slog, *(int *) num, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_CONSUMER, request.value.header.pid);
                pthread_mutex_lock(&shutdown_lock);
                shutdownRequest(&tlv_reply,request,num);
                pthread_mutex_unlock(&shutdown_lock);
                logSyncMech(slog, *(int *) num, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_CONSUMER, request.value.header.pid);
                break;
                case OP_TRANSFER:
                logSyncMech(slog, *(int *) num, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_ACCOUNT, request.value.header.pid);
                pthread_mutex_lock(&transfer_lock);
                transferRequest(&tlv_reply,request,num);
                pthread_mutex_unlock(&transfer_lock);
                logSyncMech(slog, *(int *) num, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_ACCOUNT, request.value.header.pid);
                break;
                default:
                break;
            }
            processing--; // Fim do processo
        }

        logSyncMech(slog, *(int *) num, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_CONSUMER, request.value.header.pid);
        pthread_mutex_lock(&reply_lock);

        tlv_reply.length = length;
        tlv_reply.type = request.type;
        length = 0;
        logRequest(slog, request.value.header.pid, &request);
        logReply(slog, *(int *)num, &tlv_reply);
        write(user_fifo_fd, &tlv_reply, sizeof(tlv_reply_t));
        close(user_fifo_fd);

        pthread_mutex_unlock(&reply_lock);
        logSyncMech(slog, *(int *) num, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_CONSUMER, request.value.header.pid);
    }

    logBankOfficeClose(slog, *(int *) num, pthread_self());

    pthread_exit(NULL);
}

void destroy(){
    pthread_mutex_destroy(&mut);
    pthread_mutex_destroy(&create_lock);
    pthread_mutex_destroy(&bank_lock);
    pthread_mutex_destroy(&req);
    pthread_mutex_destroy(&transfer_lock);
    pthread_mutex_destroy(&shutdown_lock);
    pthread_mutex_destroy(&balance_lock);
    pthread_mutex_destroy(&reply_lock);

    pthread_cond_destroy(&cond);
}

void openServerFile()
{
	slog = open(SERVER_LOGFILE, O_WRONLY | O_TRUNC | O_CREAT, 0777);
	if (slog == -1)

	{
		printf("Error opening server log file\n");
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char *argv[]){

    processArgs(argc,argv);

    srand(time(NULL));

    mode_t old_mask = umask(NEW_MASK);

    openServerFile();

    pthread_t balcao[num_threads]; // balcoes

    int thrarg[num_threads]; // numero correspondente ao balcao

    sem_init(&sem, 0, 0);
    sem_getvalue(&sem, &val);
    logSyncMechSem(slog, MAIN_THREAD_ID, SYNC_OP_SEM_INIT, SYNC_ROLE_PRODUCER, 0, val);

    for(int i = 0; i < num_threads; i++){
        thrarg[i] = i+1;
        if(pthread_create(&balcao[i], NULL, balcaoEletronico, &thrarg[i]) != 0){
            printf("Nao foi possivel criar thread balcao %d\n", i);
            exit(4);
        }
    }

    logSyncMech(slog, MAIN_THREAD_ID, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_ACCOUNT, 0);
    pthread_mutex_lock(&mut);
    logDelay(slog, MAIN_THREAD_ID, 0);

    createAccount(ADMIN_ACCOUNT_ID, 0, argv[2]);
    logAccountCreation(slog, MAIN_THREAD_ID, &bank_accounts[ADMIN_ACCOUNT_ID]);

    pthread_mutex_unlock(&mut);
    logSyncMech(slog, MAIN_THREAD_ID, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_ACCOUNT, 0);

    makeServerFIFO();
    openServerFIFO();

    // ==== Ler pedidos de user enquanto o FIFO SRV estiver aberto para escrita =====
    while(is_open){

        tlv_request_t request;

        if(read(srv_fifo_fd, &request, sizeof(tlv_request_t)) > 0){

            logRequest(slog, request.value.header.pid, &request);

            put_request(request);

            sem_post(&sem);
            sem_getvalue(&sem,&val);
            logSyncMechSem(slog, MAIN_THREAD_ID, SYNC_OP_SEM_POST, SYNC_ROLE_PRODUCER, request.value.header.pid, val);

            if(request.type == OP_SHUTDOWN){
                logSyncMech(slog, MAIN_THREAD_ID, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_PRODUCER, 0);
                pthread_mutex_lock(&mut);

                for(int i = 0; i < num_threads; i++){
                    pedido++;
                    logSyncMech(slog, MAIN_THREAD_ID, SYNC_OP_COND_SIGNAL, SYNC_ROLE_PRODUCER, request.value.header.pid);
                    pthread_cond_signal(&cond);
                }

                pthread_mutex_unlock(&mut);
                logSyncMech(slog, MAIN_THREAD_ID, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_PRODUCER, request.value.header.pid);

                break;
            }
            else{
                logSyncMech(slog, MAIN_THREAD_ID, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_PRODUCER, 0);
                pthread_mutex_lock(&mut);

                pedido++;
                logSyncMech(slog, MAIN_THREAD_ID, SYNC_OP_COND_SIGNAL, SYNC_ROLE_PRODUCER, request.value.header.pid);
                pthread_cond_signal(&cond);

                pthread_mutex_unlock(&mut);
                logSyncMech(slog, MAIN_THREAD_ID, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_PRODUCER, request.value.header.pid);
            }
        }

    }


    // ====== Espera pelos balcoes encerrarem =======

    for(int i = 0; i < num_threads; i++){
        pthread_join(balcao[i],NULL);
    }



    // ======== Fecha e apaga FIFO SRV ==============

    close(srv_fifo_fd);

    unlink(SERVER_FIFO_PATH);

    close(slog);

    destroy();

    umask(old_mask); // Devolve a mask original

    return 0;
}
