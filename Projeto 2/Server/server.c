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

#include "../sope.h"
#include "../log.c"

bank_account_t bank_accounts[MAX_BANK_ACCOUNTS];
unsigned int num_accounts = 0;
int fd, fd_dummy;
int num_banks;
int slog;
static const char characters[] = "0123456789abcdef";
sem_t sem1;
int val1;
pthread_t *balcao;
tlv_request_t num_requests[MAX_BANK_ACCOUNTS];
unsigned int num_request = 0;
unsigned int num_request2 = 0;
pthread_mutex_t replym = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t bankm = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t req = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t create = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t balancet = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t transfert = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t shutdownt = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
unsigned int len = 0;
int userFd;
bool online = true;
unsigned int requests = 0;
unsigned int process = 0;
char *userFifo = "";

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

bool login(uint32_t id, char *pass)
{
	char buf[MAX_PASSWORD_LEN + 1];
	strcpy(buf, pass);

	for (unsigned int i = 0; i < num_accounts; i++)
	{
		if (bank_accounts[i].account_id == id)
		{
			char *hash = getHash(buf, bank_accounts[i].salt);
			if (strcmp(bank_accounts[i].hash, hash) != 0)
			{
				return true;
			}
		}
	}
	return false;
}

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
			unlink(SERVER_FIFO_PATH);
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
	pthread_mutex_lock(&bankm);
	bank_accounts[num_accounts] = bank_account;
	num_accounts++;
	pthread_mutex_unlock(&bankm);
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

int transfer(uint32_t src_id, uint32_t dest_id, uint32_t amount, uint32_t delay)
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

	usleep(delay);

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
	slog = open(SERVER_LOGFILE, O_WRONLY | O_TRUNC | O_CREAT, 0777);

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
	sem_init(&sem1, 0, 0);
	sem_getvalue(&sem1, &val1);

	logSyncMechSem(slog, MAIN_THREAD_ID, SYNC_OP_SEM_INIT, SYNC_ROLE_PRODUCER, 0, val1);
}

int getBalance(uint32_t id)
{
	bank_account_t *bank_account = getAccount(id);

	if (bank_account == NULL)
	{
		return 0;
	}

	return bank_account->balance;
}

void addRequest(tlv_request_t tlv_request)
{
	pthread_mutex_lock(&req);
	num_requests[num_request] = tlv_request;
	num_request = (num_request + 1) % MAX_BANK_ACCOUNTS;
	pthread_mutex_unlock(&req);
	return;
}

void processRequest()
{
	while (online)
	{
		tlv_request_t tlv_request;

		if (read(fd, &tlv_request, sizeof(tlv_request_t)) > 0)
		{
			logRequest(slog, tlv_request.value.header.pid, &tlv_request);

			addRequest(tlv_request);

			sem_post(&sem1);
			sem_getvalue(&sem1, &val1);
			logSyncMechSem(slog, MAIN_THREAD_ID, SYNC_OP_SEM_POST, SYNC_ROLE_PRODUCER, tlv_request.value.header.pid, val1);

			if (tlv_request.type == OP_SHUTDOWN) {
				logSyncMech(slog, MAIN_THREAD_ID, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_PRODUCER, 0);
				pthread_mutex_lock(&mutex);

				for (int i = 0; i < num_banks; i++)
				{
					requests++;
					logSyncMech(slog, MAIN_THREAD_ID, SYNC_OP_COND_SIGNAL, SYNC_ROLE_PRODUCER, tlv_request.value.header.pid);
					pthread_cond_signal(&cond);
				}

				pthread_mutex_unlock(&mutex);
				logSyncMech(slog, MAIN_THREAD_ID, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_PRODUCER, tlv_request.value.header.pid);

				break;
			}
		}
	}
}

void getRequest(tlv_request_t *request)
{
	pthread_mutex_lock(&req);
	*request = num_requests[num_request2];
	num_request2 = (num_request2 + 1) % MAX_BANK_ACCOUNTS;
	pthread_mutex_unlock(&req);
	return;
}

bool openUserFIFO(pid_t pid) {
	char *buf = malloc(sizeof(WIDTH_ID + 1));
	sprintf(buf, "%*d", WIDTH_ID, pid);

	userFifo = malloc(USER_FIFO_PATH_LEN);

	strcpy(userFifo, USER_FIFO_PATH_PREFIX);
	strcat(userFifo, buf);

	userFd = open(userFifo, O_WRONLY);
	if (userFd == -1) {
		return -1;
	}

	return 0;
}

void *processCounter(void *num)
{
	logBankOfficeOpen(slog, *(int *)num, pthread_self());

	while (online) {

		logSyncMech(slog, *(int *)num, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_CONSUMER, 0);
		pthread_mutex_lock(&mutex);

		while (!(requests > 0)) {
			logSyncMech(slog, *(int *)num, SYNC_OP_COND_WAIT, SYNC_ROLE_CONSUMER, 0);
			pthread_cond_wait(&cond, &mutex);
		}

		requests--;

		pthread_mutex_unlock(&mutex);
		logSyncMech(slog, *(int *)num, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_CONSUMER, 0);

		tlv_reply_t tlv_reply;
		tlv_request_t request;

		getRequest(&request);

		sem_getvalue(&sem1, &val1);
		logSyncMechSem(slog, *(int *)num, SYNC_OP_SEM_WAIT, SYNC_ROLE_CONSUMER, request.value.header.pid, val1);
		sem_wait(&sem1);

		if (!online)
			break;

		if (login(request.value.header.account_id, request.value.header.password)) {
			process++;

			switch (request.type) {

				/**     CREATE ACCOUNT      **/

			case OP_CREATE_ACCOUNT:

				logSyncMech(slog, *(int *)num, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_ACCOUNT, request.value.header.pid);
				pthread_mutex_lock(&create);

				logDelay(slog, *(int *)num, request.value.header.op_delay_ms);
				usleep(request.value.header.op_delay_ms * 1000);

				len += sizeof(int) + sizeof(int);

				if (request.value.header.account_id == ADMIN_ACCOUNT_ID) {
					int rc = createAccount(request.value.create.account_id, request.value.create.balance, request.value.create.password);
					tlv_reply.value.header.ret_code = rc;
					if (rc == RC_OK)
						logAccountCreation(slog, *(int *)num, &bank_accounts[num_accounts - 1]);
				}
				else if (request.value.header.account_id != ADMIN_ACCOUNT_ID) {
					tlv_reply.value.header.ret_code = RC_OP_NALLOW;
				}

				else {
					tlv_reply.value.header.ret_code = RC_OTHER;
				}
				tlv_reply.value.header.account_id = *(int *)num;

				len += sizeof(*(int *)num) + sizeof(tlv_reply.value.header.ret_code);

				pthread_mutex_unlock(&create);
				logSyncMech(slog, *(int *)num, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_ACCOUNT, request.value.header.pid);

				break;


				/**     CHECK BALANCE       **/

			case OP_BALANCE:

				logSyncMech(slog, *(int *)num, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_ACCOUNT, request.value.header.pid);
				pthread_mutex_lock(&balancet);

				logDelay(slog, *(int *)num, request.value.header.op_delay_ms);
				usleep(request.value.header.op_delay_ms * 1000);

				len += sizeof(int) + sizeof(int);

				if (request.value.header.account_id != ADMIN_ACCOUNT_ID) {
					tlv_reply.value.balance.balance = getBalance(request.value.header.account_id);
					len += sizeof(int);
				}

				else if (request.value.header.account_id == ADMIN_ACCOUNT_ID) {
					tlv_reply.value.header.ret_code = RC_OP_NALLOW;
				}
				else {
					tlv_reply.value.header.ret_code = RC_OTHER;
				}
				pthread_mutex_unlock(&balancet);
				logSyncMech(slog, *(int *)num, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_ACCOUNT, request.value.header.pid);

				break;


				/**     SERVER SHUTDOWN     **/

			case OP_SHUTDOWN:

				logSyncMech(slog, *(int *)num, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_CONSUMER, request.value.header.pid);
				pthread_mutex_lock(&shutdownt);

				logDelay(slog, *(int *)num, request.value.header.op_delay_ms);
				usleep(request.value.header.op_delay_ms * 1000);

				len += sizeof(request.value.header.account_id) + sizeof(int);

				if (request.value.header.account_id == ADMIN_ACCOUNT_ID) {
					shutdown();
					tlv_reply.value.header.ret_code = RC_OK;
					tlv_reply.value.shutdown.active_offices = process - 1;
					len += sizeof(int);
				}

				else if (request.value.header.account_id != ADMIN_ACCOUNT_ID) {
					tlv_reply.value.header.ret_code = RC_OP_NALLOW;
				}

				else {
					tlv_reply.value.header.ret_code = RC_OTHER;
				}

				pthread_mutex_unlock(&shutdownt);
				logSyncMech(slog, *(int *)num, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_CONSUMER, request.value.header.pid);

				break;


				/**     TRANSFERENCIA       **/

			case OP_TRANSFER:

				logSyncMech(slog, *(int *)num, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_ACCOUNT, request.value.header.pid);
				pthread_mutex_lock(&transfert);

				len += sizeof(int) + sizeof(int);
				if (request.value.header.account_id == ADMIN_ACCOUNT_ID) {
					tlv_reply.value.header.ret_code = RC_OP_NALLOW;
				}

				else if (request.value.header.account_id != ADMIN_ACCOUNT_ID) {
					logDelay(slog, *(int *)num, request.value.header.op_delay_ms);
					tlv_reply.value.header.ret_code =
						transfer(request.value.header.account_id, request.value.transfer.amount, request.value.transfer.account_id, request.value.header.op_delay_ms * 1000);
					len += sizeof(request.value.transfer.amount);
				}
				else {
					tlv_reply.value.header.ret_code = RC_OTHER;
				}

				bank_account_t *account = getAccount(request.value.header.account_id);
				tlv_reply.value.transfer.balance = account->balance; // New balance

				pthread_mutex_unlock(&transfert);
				logSyncMech(slog, *(int *)num, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_ACCOUNT, request.value.header.pid);

				break;

				/**     DEFAULT     **/
			default:
				break;
			}
			process--; // Fim do processo
		}

		else {
			len += 2 * sizeof(int);
			tlv_reply.value.header.account_id = *(int *)num;
			tlv_reply.value.header.ret_code = RC_LOGIN_FAIL;
		}

		logSyncMech(slog, *(int *)num, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_CONSUMER, request.value.header.pid);
		pthread_mutex_lock(&replym);

		tlv_reply.length = len;
		tlv_reply.type = request.type;

		logRequest(slog, request.value.header.pid, &request);

		if (openUserFIFO(request.value.header.pid) != 0) {
			tlv_reply.value.header.ret_code = RC_USR_DOWN;
		}

		logReply(slog, *(int *)num, &tlv_reply);

		write(userFd, &tlv_reply, sizeof(tlv_reply_t));

		close(userFd);

		len = 0; // Reset length to 0 (zero)

		pthread_mutex_unlock(&replym);
		logSyncMech(slog, *(int *)num, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_CONSUMER, request.value.header.pid);
	}

	logBankOfficeClose(slog, *(int *)num, pthread_self());

	pthread_exit(NULL);
}

void createCounter()
{
	pthread_t balcao[num_banks];
	int num_counter[num_banks];
	for (int i = 0; i < num_banks; i++)
	{
		num_counter[i] = i + 1;
		if (pthread_create(&balcao[i], NULL, processCounter, &num_counter[i]) != 0)
		{
			printf("Error creating counter\n");
			exit(EXIT_FAILURE);
		}
	}
}

void destroyMutex()
{
	pthread_mutex_destroy(&mutex);
	pthread_mutex_destroy(&bankm);
	pthread_mutex_destroy(&req);
}

int main(int argc, char *argv[])
{
	verifyArgs(argc, argv);

	srand(time(NULL));

	openServerFile();

	createSemaphore();

	createCounter();

	logSyncMech(slog, MAIN_THREAD_ID, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_ACCOUNT, 0);

	pthread_mutex_lock(&mutex);

	logDelay(slog, MAIN_THREAD_ID, 0);

	createAccount(ADMIN_ACCOUNT_ID, 0, argv[2]);

	logAccountCreation(slog, MAIN_THREAD_ID, &bank_accounts[ADMIN_ACCOUNT_ID]);

	pthread_mutex_unlock(&mutex);

	logSyncMech(slog, MAIN_THREAD_ID, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_ACCOUNT, 0);

	createServerFIFO();

	openServerFIFO();

	processRequest();

	esperaBalcao();

	closeServerFIFO();

	destroyServerFIFO();

	closeServerFile();

	destroyMutex();

	return 0;
}
