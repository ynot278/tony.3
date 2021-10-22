#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/ipc.h>
#include <string.h>
#include <sys/shm.h>
#include <errno.h>
#include <sys/types.h> 
#include <sys/wait.h> 
#include <signal.h>
#include <limits.h>
#include <sys/msg.h>
#include <time.h>

#include "config.h"

#define SHSIZE 100

pid_t *pidList;
int shmid2;

struct mesg_buffer{
	long mesg_type;
	char mesg_text[100];
} message;

int createProcessList(){
	int i = 0;

	key_t key = ftok("testing.data", 'a');
	shmid2 = shmget(key, sizeof(pid_t) * N_LICENSE, IPC_CREAT | 0666);
	if (shmid2 == -1){
		perror("runsim: Error: Shared memory setup failed");
	}

	pidList = (pid_t *)shmat(shmid2, 0, 0);
	if (pidList == (pid_t *) -1){
		perror("runsim: Error: Shared memory attach");
	}

	for (i = 0; i < N_LICENSE; i++)
		pidList[i] = 0;

	return shmid2;
}

void killProcesses(){
	int i = 0;
	for (i = 0; i < N_LICENSE; i++){
		kill(pidList[i], SIGKILL);
	}
}

void processIntSig(){
	printf("Killing processes");
	killProcesses();
	exit(EXIT_SUCCESS);
}

int returnEmptyProcess(){
	int i = 0;
	for (i = 0; i < N_LICENSE; i++){
		if (pidList[i] == 0)
			return i;
	}
	return -1;
}

void removePID(pid_t p){
	int i = 0;
	for (i = 0; i < N_LICENSE; i++){
		if (pidList[i] == p){
			pidList[i] = 0;
			break;
		}
	}
}

int createMsgQueue(){
	key_t key;
  int msgid;
  
  key = ftok("config.h", 'a');
  
  msgid = msgget(key, 0666 | IPC_CREAT);
  message.mesg_type = 1;
	memcpy(message.mesg_text, "test", 4);
  
  msgsnd(msgid, &message, sizeof(message)+1, 0);
  
  if(msgctl(msgid, IPC_RMID, NULL) == -1){
		perror("runsim: Error: msgctl");
		exit(EXIT_FAILURE);
	}

	return msgid;
}

pid_t create_child(){
	pid_t pid;

	if ((pid = fork()) == -1){
		perror("runsim: Error: Create child failed");
		exit(EXIT_FAILURE);
	}

	if (pid == 0){
		execl("./testsim", "./testsim", (char*)NULL);
	}

	return pid;
}

void clearShm(){
	if(shmdt(pidList) == -1){
		perror("runsim: error: Failed to detach");
		exit(EXIT_FAILURE);
	}

	if(shmctl(shmid2, IPC_RMID, NULL) == -1){
		perror("runsim: Error: Failed to remove shmid");
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char *argv[]){

	signal(SIGINT, processIntSig);
	signal(SIGALRM, processIntSig);
	alarm(TIMER);

	srand(time(NULL));

	if (argc != 2){
		printf("Usage: ./runsim n (n = runs up to n application processes at a time).\n");
		exit(EXIT_FAILURE);
	}

	int opt;

	while ((opt = getopt(argc, argv, "h")) != -1){
		switch (opt){
			case 'h':
				printf("Usage: ./runsim n (n = runs up to n application processes at a time).\n");
				exit(EXIT_SUCCESS);
				break;
			default:
				printf("Usage: ./runsim n (n = runs up to n application processes at a time).\n");
				exit(EXIT_SUCCESS);
		}
	}

	int processes;
	char *point;
	errno = 0;
	long conv = strtol(argv[1], &point, 10);

	if (errno != 0 || *point != '\0' || conv > N_LICENSE || conv < 1){
		printf("Number of licenses should not be greater than 20.");
		exit(EXIT_FAILURE);
	} else
		processes = conv;

	int shmid;
	key_t key;
	char *shm;
	char *s;
	int pidIndex = 0;
	int count = 0;
	pid_t p;

	if(count == N_LICENSE){
		p = wait(NULL);
		count--;
		removePID(p);
	}

	count++;
	createProcessList();

	int i;
	for(i = 0; i < processes; i++){
		pidList[pidIndex] = create_child();
		createMsgQueue();
		pidIndex++;
	}

	if ((p = waitpid(-1, NULL, WNOHANG)) != 0){
		removePID(p);
	}

	key = 8837;

	shmid = shmget(key, SHSIZE, IPC_CREAT | 0666);
	if(shmid < 0){
		perror("runsim: Error: Shared memory setup failed");
		exit(EXIT_FAILURE);
	}

	shm = shmat(shmid, NULL, 0);

	if(shm == (char *) -1){
		perror("runsim: Error: Shared memory attach");
		exit(EXIT_FAILURE);
	}

	memcpy(shm, "test", 4);

	s = shm;
	s += 4;

	*s = 0;

	while(*shm != '*')
		sleep(1);

	clearShm();
		
	if(shmdt(shm) == -1){
		perror("runsim: Error: Failed to detach");
		exit(EXIT_FAILURE);
	}

	if(shmctl(shmid, IPC_RMID, NULL) == -1){
		perror("runsim: Error: Failed to remove shmid");
		exit(EXIT_FAILURE);
	}
	return 0;
}