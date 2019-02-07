#ifndef _SYNC_H_
#define _SYNC_H_

union semun {
	int val;
	struct semid_ds* buf;
	unsigned short* array;
#ifdef __linux__
	struct seminfo* __buf;
#endif
};

extern int init1(int, int);
extern int init0(int, int);
extern int P(int, int);
extern int V(int, int);
extern int wait0(int, int);
extern int semVal(int, int);

#endif
