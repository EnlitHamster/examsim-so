#include "sync.h"
#include "lib.h"
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/stat.h>

int init1(int semId, int semNum) {
	union semun arg;
	arg.val = 1;
	return semctl(semId, semNum, SETVAL, arg);
}

int init0(int semId, int semNum) {
	union semun arg;
	arg.val = 0;
	return semctl(semId, semNum, SETVAL, arg);
}

int P(int semId, int semNum) {
	struct sembuf sops;
	sops.sem_num = semNum;
	sops.sem_op = -1;
	sops.sem_flg = 0;
	return semop(semId, &sops, 1);
}

int V(int semId, int semNum) {
	struct sembuf sops;
	sops.sem_num = semNum;
	sops.sem_op = 1;
	sops.sem_flg = 0;
	return semop(semId, &sops, 1);
}

int wait0(int semId, int semNum) {
	struct sembuf sops;
	sops.sem_num = semNum;
	sops.sem_op = 0;
	sops.sem_flg = 0;
	return semop(semId, &sops, 1);
}

int semVal(int semId, int semNum) {
	union semun arg;
	return semctl(semId, semNum, GETVAL, arg);
}
