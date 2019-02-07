// binding the library if and only if it hasn't been binded yet
#ifndef _LIB_H_
#define _LIB_H_

#include "bool.h"
#include "sync.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

// number of semaphores
#define NUM_OF_SEM 14
// odd student synch semaphore
#define WAIT_ALL_ODD 0
// even student synch semaphore
#define WAIT_ALL_EVEN 1
// manager semaphore to wait 0
#define MANAGER_WAIT_0 2
// manager semaphore to wait the end of the child
#define WAIT_END 3
// shared memory mutex
#define SHM_MUTEX 4
// student semaphore to wait 0
#define STUDENT_WAIT_0 5
// mutex semaphore shared between child
#define MSG_MUTEX 6
// student semaphore to wait result
#define WAIT_RESULT 7
// odd reader/writer semaphore
#define RW_MUTEX_ODD 8
// even reader/writer semaphore
#define RW_MUTEX_EVEN 9
// odd reader semaphore
#define R_MUTEX_ODD 10
// even reader semaphore
#define R_MUTEX_EVEN 11
// odd writer semaphore
#define W_MUTEX_ODD 12
// even writer semaphore
#define W_MUTEX_EVEN 13

// macro checking if a FUNC (that can be anything, from an actual
// function call to an arithmetic operation) has generated any error,
// printing said error data and closing the program
#define DO_AND_TEST(FUNC) if ((FUNC) < 0) {\
	if(errno != EINTR){\
		fprintf(stderr,"Fatal error (#%d): %s || In file: %s || in function: %s || at line: %d.\n",errno,strerror(errno), __FILE__,__func__, __LINE__	);\
		exit(EXIT_FAILURE);\
	}\
}

// structure representing the shared memory data structure
// the structure contains the following data:
// - even = table containing all the students with even registration number
// - odd = table containing all the students with off registration number
// - groups = table containing basic info about the forming groups
// - evenSize = number of elements in even array
// - offSize = number of elements in odd array
// - groupsSize = number of groups formed
// - evenMean = mean of the grade of the evens
// - oddMean = mean of the grade of the odds
// - maxG2OddOptimalGroup = estimation of the max number grup composed by 2 odd students
// - maxG3OddOptimalGroup = estimation of the max number grup composed by 3 odd students
// - maxG4OddOptimalGroup = estimation of the max number grup composed by 4 odd students
// - g2_OddOpenGroup = the current number of open group composed by odd student with nOfElems == 2
// - g3_OddOpenGroup = the current number of open group composed by odd student with nOfElems == 3
// - g4_OddOpenGroup = the current number of open group composed by odd student with nOfElems == 4
// - maxG2EvenOptimalGroup = estimation of the max number grup composed by 2 even students
// - maxG3EvenOptimalGroup = estimation of the max number grup composed by 3 even students
// - maxG4EvenOptimalGroup = estimation of the max number grup composed by 4 even students
// - g2_EvenOpenGroup = the current number of open group composed by even student with nOfElems == 2
// - g3_EvenOpenGroup = the current number of open group composed by even student with nOfElems == 3
// - g4_EvenOpenGroup = the current number of open group composed by even student with nOfElems == 4
// - oddReaderCount = number of readers among the odds
// - oddWriterCount = number of writers among the odds
// - EvenReaderCount = number of readers among the evens
// - EvenWriterCount = number of writers among the evens
// - oddInviteReserved = if TRUE then a odd student (which can't belong to a group with his same prefernce) is inviting, and no one else can invite
// - evenInviteReserved = if TRUE then a even student (which can't belong to a group with his same prefernce) is inviting, and no one else can invite
// - timeOut = if TRUE then the time is out
#define SHM_STRUCT struct shm { \
	studentInfo even[pop_size]; \
	studentInfo odd[pop_size]; \
	group groups[pop_size]; \
	unsigned int evenSize; \
	unsigned int oddSize; \
	unsigned int groupsSize; \
	unsigned short evenMean; \
	unsigned short oddMean; \
	int maxG2OddOptimalGroup; \
	int maxG3OddOptimalGroup; \
	int maxG4OddOptimalGroup; \
	int oddG2OpenGroup; \
	int oddG3OpenGroup; \
	int oddG4OpenGroup; \
	int maxG2EvenOptimalGroup; \
	int maxG3EvenOptimalGroup; \
	int maxG4EvenOptimalGroup; \
	int evenG2OpenGroup; \
	int evenG3OpenGroup; \
	int evenG4OpenGroup; \
	unsigned int oddReaderCount; \
	unsigned int oddWriterCount; \
	unsigned int evenReaderCount; \
	unsigned int evenWriterCount; \
	BOOL oddInviteReserved; \
	BOOL evenInviteReserved; \
	BOOL evenG2InviteReserved; \
	BOOL evenG3InviteReserved; \
	BOOL evenG4InviteReserved; \
	BOOL oddG2InviteReserved; \
	BOOL oddG3InviteReserved; \
	BOOL oddG4InviteReserved; \
	BOOL timeOut; \
};

// structure represnting a group
// the structure containst the following data:
// - member = a matrix containg all the student regNum and grade belonging to this group
// - size = numbero of element in array
// - isClosed = control value: TRUE -> the group is closed, FALSE -> otherwise
typedef struct group{
	unsigned int member[4][2];
	unsigned short size;
	BOOL isClosed;
} group;

// structure representing a student
// the structure contains the following data:
// - regNum | matricola = the registration number of the student in the system
// - grade | voto_AdE = the evaluation obtained by the student at the past exam
// - nOfElems | nof_elems = the prefered group size of the student
// - inviteLest = the number of inviteLeft
// - myGroupLeader = the Leader of my group if == -1 then i'm not in a group else if ==myRegNum then i'm a group leader
typedef struct student_info {
	long regNum;			// matricola
	unsigned short grade;			// voto_AdE
	unsigned short nOfElems;	// nof_elems
	unsigned int inviteLeft;
	unsigned int finalGrade;
	long myGroupLeader;
	BOOL inviting;
} studentInfo;

#endif
