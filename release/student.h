#ifndef _STUDENT_H_
#define _STUDENT_H_

#include "lib.h"

// possible message types definition
#define ANSWER 0
#define INVITE 1
// macro initialize and write student data
#define INIT_AND_WRITE(ARRAY, ARRAY_SIZE) { ARRAY[ARRAY_SIZE].grade = 18 + rand() % 13; \
		                                        ARRAY[ARRAY_SIZE].nOfElems = getPref(g2, g3, g4); \
		                                        ARRAY[ARRAY_SIZE].regNum = getpid(); \
		                                        ARRAY[ARRAY_SIZE].myGroupLeader = -1; \
		                                        ARRAY[ARRAY_SIZE].inviteLeft = nof_invites; \
		                                        ARRAY[ARRAY_SIZE].inviting = FALSE;}

#define btoc(b) (b) ? 'V' : 'X'
#define select_par(delta,par_a,par_b) (delta) ? (par_a) : (par_b)

// structure representing a message content
// the structure contains the following data:
// - type = either ANSWER or INVITE
// - regNum = regNum of the sender
// - nOfElems = the nOfElems of the sender
// - data = either the grade of the sender or the answer
//			to an invite (in said case TRUE or FALSE)
typedef struct message_data {
	short type;
	long regNum;
	unsigned short nOfElems;
	unsigned short data;
} message;

// structure representing the actual message
// the structure contains the following data:
// - mtype = the PID of the reciever
// - mtext = the content of the message (see the previous struct)
typedef struct msgbuf {
	long mtype;
	message mtext;
} msgbuf;

// function used to randomically get the nOfElems value
// PARAMETERS:
// - rate of student with nOfElems = 2
// - rate of student with nOfElems = 3
// - rate of student with nOfElems = 4
// RETURN:
// + nOfElems value
short getPref(int, int, int);

// function used to reject all the invite received
// PARAMETERS:
// - Id of the message queue
// - my regNum
// - array of the invite received
// - size of the array
// RETURN:
// + num of invite rejected
int rejectAll(int, long, message *, int);

// function used to reject all the invite received
// PARAMETERS:
// - Id of the message queue
// - my regNum
// - array of the invite received
// - size of the array
// RETURN:
// + num of invite rejected
int rejectAllExcept(int, long, long, message *, int);

// function used to reject all the invite received
// PARAMETERS:
// - Id of the message queue
// - my regNum
// - regNum of the student that invited me
// - my grade if is a positive answer, 0 otherwise
void answerToInvite(int, long, long, int);

// function used to reject all the invite received
// PARAMETERS:
// - Id of the message queue
// - a pointer to my data
// - my grade
// - regNum of the student that i'm going to invite
// RETURN:
// + 0 on success, -1 otherwise
int sendInvite(int, studentInfo *, unsigned int, long);

// function used to find the worst student to invite
// PARAMETERS:
// - a pointer to the student data
// - the array of student where i'm going to search a student (odd or even)
// - the size of the array
// - the type of student to find, if FALSE then the research is made belong all the studeent
// if TRUE is made belong the student with my same nOfElems
// RETURN:
// + a pointer to the worst student info
studentInfo * findWorstStudent(studentInfo *, studentInfo *, int, BOOL);

// function used to find the worst student to invite
// PARAMETERS:
// - pointer to the student data
// - the array of student where i'm going to search a student (odd or even)
// - the size of the array
// - the type of student to find, if FALSE then the research is made belong all the studeent
// - if TRUE is made belong the student with my same nOfElems
// RETURN:
// + a pointer to the best student info
studentInfo * findBestStudent(studentInfo *, studentInfo *, int, BOOL);

// function used to find the worst student to invite
// PARAMETERS:
// - the array of all the invite i've received
// - the size of the array
// - the type of invite to find, if 0 then the research is made belong all the invite
// if == myData->nOfElems then is made belong the student with my same nOfElems
// RETURN:
// + a pointer to the best invite
message * findBestInvite(message *, unsigned int, unsigned short);

// function used to find the worst student to invite
// PARAMETERS:
// - the array of all the invite i've received
// - the size of the array
// - the type of invite to find, if 0 then the research is made belong all the invite
// if == myData->nOfElems then is made belong the student with my same nOfElems
// RETURN:
// + a pointer to the best invite
message * findWorseInvite(message *, unsigned int, unsigned short);

// function used to find the best grade in a group
// PARAMETERS:
// - the matrix containing the student regNum and grade
// - the size of the of the groyo
// RETURN:
// + the best grade in the group
unsigned short maxGroupGrade(unsigned int member[4][2], unsigned short size);

// function used to find the worst grade in a group
// PARAMETERS:
// - the matrix containing the student regNum and grade
// - the size of the of the groyo
// RETURN:
// + the worst grade in the group
unsigned short minGroupGrade(unsigned int member[4][2], unsigned short);

// search a group in the groups array
// PARAMETERS:
// - the group leader of the group
// - the array of groups
// - the size of the array
// RETURN:
// + a pointer to the group info
group * searchGroup(long, group *, unsigned int);

// a function used to create a group
// PARAMETERS:
// - a pointer to the student data
// - the array of groups
// - a pointer to the size of the array
// - the id the of the semaphore set
// RETURN:
// + a pointer to the group info
group * createGroup(studentInfo*, group *, unsigned int *, int);

// a function used to check if the student can be invited
// PARAMETERS:
// - a pointer to the student data
// - the array of groups
// - the array of student with my same type of regNum (odd or even)
// - the size of the array of student
// - the size of the array of group
// - a pointer to the current num of open group
// RETURN:
// + TRUE if the student can be invited, FALSE otherwise
BOOL canBeInvited(studentInfo *, group *, studentInfo *, int, unsigned int, int *);

// a function used to check if the student can be invited
// PARAMETERS:
// - a pointer to the student data
// RETURN:
// + TRUE if the student is a group leader, FALSE otherwise
BOOL isGroupLeader(studentInfo *);

// a function used to check if a group is closed
// PARAMETERS:
// - the regNum of a groupLeader
// - the arrayof group
// - the size of the array
// RETURN:
// + TRUE if the group is closed, FALSE otherwise
BOOL isClosed(long, group *, unsigned int);

// a function used to check if the student can be invited
// PARAMETERS:
// - a pointer to the student data
// - the array of student with my same type of regNum (odd or even)
// - the size of the student array
// RETURN:
// + TRUE if the group is closed, FALSE otherwise
BOOL canInvite(studentInfo *, studentInfo *, int);

// a function used to initialize the student
// PARAMETERS
// - the key of the semaphore set
// - the key of the shared memory segment
// - the key of the message queue
// RETURN
// + a pointer to the shared memory segment
void* init_ipc(key_t, key_t, key_t);

// a function used to initialize global student data and write them on the shm
// PARAMETERS
// - a pointer to the shared memory segment
// - a parameter used to determ the parity of the student
// RETURNN
// + a pointer to my Data in the shared memory
void* init_mydata(void*, BOOL);

// - a pointer to the shared memory segment
// - a parameter used to determ the parity of the student
void init_locals(void*, BOOL);

// a function to determ whether the student should find an optimal student or not
// PARAMETERS
// - a pointer to the function that need to be used to search the optimal student (findWorseStudent or findBestStudent)
// RETURN
// + NULL if the student should not find an optimal student or if the optimalStudent wasn't found
// 	 a pointer to the studentInfo otherwise
studentInfo* findOptStud(studentInfo* (*fFindStud)(studentInfo *, studentInfo *, int, BOOL));

// a function used to accept an invite and reject all the other one
// PARAMETERS
// - the array containing all the invite received
// - the struct containing the invite to accept
// - the size of the array
// - a pointer to the number of rejectionLeft
// RETURN
// + the function does not return anything but the rejectionLeft will be modified
void acceptAndReject(message*, message*, int*, int*);

// a function used to analyse all the invite received and choose which of them shoudl acceptt
// PARAMETERS
// - a pointer to the function that need to be used to search the optimal student (findWorseStudent or findBestStudent)
// - a pointer to the function that need to be used to search the optimal ivnite (findWorseInvite or findBestInvite)
// - the array containg all the invite received
// - the size of the array
// - a pointer to the number of rejectionLeft
// RETURN
// + the function does not return anything but the rejectionLeft will be modified
void analyseInvite(studentInfo* (*fFindStud)(studentInfo *, studentInfo *, int, BOOL), message* (*fFindInv)(message *, unsigned int, unsigned short), message*, int*, int*);

// a function used to analyse all the answer received and to add student to the groups (if they accepted the invite)
// PARAMETERS
// - a pointer to the shared memory segment
// - the array containg all the answer
// - size of the array
// RETURN
// + on success return 0, that is the new size of the array containig all the answer
int analyseAnswer(void*, message *, int);

// a function used to search the optimal student to invite, and to invite him
// PARAMETERS
// - a pointer to the function that need to be used to search the optimal student (findWorseStudent or findBestStudent)
// - my grade or the grade of the best/worst student of my group
void tryToInvite(studentInfo* (*fFind)(studentInfo*, studentInfo*, int, BOOL), int);
#endif
