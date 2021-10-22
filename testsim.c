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

struct mesg_buffer {
  long mesg_type;
  char mesg_text[100];
} message;

int recieveMsg(){
	key_t key;
  int msgid;
  
  key = ftok("config.h", 'a');
  
  msgid = msgget(key, 0666 | IPC_CREAT);
  
  msgrcv(msgid, &message, sizeof(message)+1, 1, 0);
  
  printf("MsgQueue received: %s \n", message.mesg_text);

	return msgid;
}

char* getTime(){
	time_t now = time(0);
	char* dt = ctime(&now);
	return dt;
}

void logmsg(int sleeper, int repeat){
	FILE *fp;
	fp = fopen("log.txt", "w");

	int pid = (int)getpid();

	int i;
	for (i = 0; i < repeat; i++){
		char *timer = getTime();
		fprintf(fp, "PID: %d ", pid);
		fprintf(fp, "Iteration %d of %d ", (i+1), repeat);
		fprintf(fp, "%s", timer);
		sleep(sleeper);
	}
}


int main(int argc, char *argv[]){
	int shmid;
	key_t key;
	char *shm;
	char *s;

	srand (time(NULL));

	int num1;
	int num2;

	num1 = (rand() % (5 + 1 - 1) + 1);
	num2 = (rand() % (5 + 1 - 1) + 1);

	printf("Sleep factor is %d\n", num1);
	printf("Repeat factor is %d\n", num2);


	key = 8837;

	shmid = shmget(key, SHSIZE, 0666);
	if(shmid < 0){
		perror("runsim: Error: Shared memory setup failed");
		exit(EXIT_FAILURE);
	}

	shm = shmat(shmid, NULL, 0);

	if(shm == (char *) -1){
		perror("runsim: Error: Shared memory attach");
		exit(EXIT_FAILURE);
	}

	for(s = shm; *s != 0; s++){
		//recieveMsg();
		logmsg(num1, num2);
	}

	*shm = '*';

	return 0;
}
