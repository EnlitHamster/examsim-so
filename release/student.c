#include "student.h"

unsigned short R_MUTEX;
unsigned short W_MUTEX;
unsigned short RW_MUTEX;
unsigned short WAIT_ALL;

// Global working variables
int percentile, studentSize;
int * openOptimalGroup, * possibleOptimalGroup;
studentInfo * studentArray, * myData; // a pointer to the data written on the shm
BOOL * inviteReserved, * optInviteReserved;
unsigned int * readerCount, * writerCount;

int g2, g3, g4, pop_size, nof_invites, max_rejects;
int semId, shmId, msgId;

group * myGroup = NULL; //point to the group whom i'm leader
int pendingInvites = 0;
BOOL optimalLeader = FALSE;

BOOL * timeOut;

int main(int argc, char ** argv) {
	// recovering data from father
	key_t shmKey = (key_t)atoi(argv[1]);
	key_t semKey = (key_t)atoi(argv[2]);
	key_t msgKey = (key_t)atoi(argv[3]);
	max_rejects = atoi(argv[4]);
	nof_invites = atoi(argv[5]);
	pop_size = atoi(argv[6]);
	g2 = atoi(argv[7]);
	g3 = atoi(argv[8]);
	g4 = atoi(argv[9]);

	// working variable
	msgbuf receivedMessage; //struct containing the receivedMessage
	message answer[pop_size]; // array containing the answer received
	message invite[pop_size]; // array containing the invite received
	int inviteSize = 0, answerSize = 0;
	int rejectionLeft = max_rejects;
	BOOL allowedToInvite = FALSE;

	// Getting shared stuff...
	SHM_STRUCT;
	struct shm* shmStruct = (struct shm*) init_ipc(semKey, shmKey, msgKey);

	// initialize and write datas on shm
	myData = (studentInfo*) init_mydata(shmStruct, getpid() % 2 == 0);

	// decrement MANAGER_WAIT_0 by 1
	DO_AND_TEST(P(semId, MANAGER_WAIT_0));

	// waiting for manager authorization to proceed
	DO_AND_TEST(wait0(semId, STUDENT_WAIT_0));

	// Initializing working variable
	init_locals(shmStruct, myData->regNum % 2 == 0);

	while(!(*timeOut) && (myData->myGroupLeader == -1 || (myData->myGroupLeader == myData->regNum && (myGroup == NULL || !myGroup->isClosed)))) {

		DO_AND_TEST(wait0(semId, WAIT_ALL));
		DO_AND_TEST(V(semId, WAIT_ALL));
		// Synch with the writer

		DO_AND_TEST(P(semId, R_MUTEX));
		++(*readerCount);
		if(*readerCount == 1) {
			DO_AND_TEST(P(semId, RW_MUTEX));
		}
		DO_AND_TEST(V(semId, R_MUTEX));
		// Entering reading phase
		// There are no writer

		myData->inviting = FALSE;
		//read all the message received and filter them
		while (msgrcv(msgId, &receivedMessage, sizeof(message), myData->regNum,  IPC_NOWAIT)!=-1) {
			if(receivedMessage.mtext.type == ANSWER) {
				answer[answerSize] = receivedMessage.mtext;
				++answerSize;
			}
			else{
				invite[inviteSize] = receivedMessage.mtext;
				++inviteSize;
			}
		}
		if(errno!=ENOMSG) {
			// the msgrcv failed
			exit(EXIT_FAILURE);
		}

		// analyse all the answers
		answerSize = analyseAnswer(shmStruct, answer, answerSize);

		if(inviteSize > 0) {
			if(pendingInvites > 0 || myData->myGroupLeader != -1) {
				// if i have some pending invites or i'm in a group then i reject all the invites
				// reject all the invites
				rejectAll(msgId, myData->regNum, invite, inviteSize);
				inviteSize = 0;
			}
			else if(rejectionLeft == 0) {
				// accept the first invite
				answerToInvite(msgId, myData->regNum, invite[0].regNum, myData->grade);
				myData->myGroupLeader = invite[0].regNum;
				// reject all the remaining
				// invite + 1, point to all the remainig invite message;
				// there are inviteSize - 1 invite messaege left;
				rejectAll(msgId, myData->regNum, invite+1, inviteSize-1);
				inviteSize = 0;
			}
			else{
				if(myData->grade < percentile) {
					analyseInvite(&findBestStudent, &findBestInvite, invite, &inviteSize, &rejectionLeft);
				}
				else{
					analyseInvite(&findWorstStudent, &findWorseInvite, invite, &inviteSize, &rejectionLeft);
				}
			}
		}

		DO_AND_TEST(P(semId, R_MUTEX));
		// checking if i can be invited or if there are student left to invite
		if(!(*timeOut) && pendingInvites == 0 && !canBeInvited(myData, shmStruct->groups, studentArray, studentSize, shmStruct->groupsSize, openOptimalGroup) && !canInvite(myData, studentArray, studentSize)) {
			// i cannot be invited my anyone and i have no invite left
			// so i close the group
			if(myData->myGroupLeader == -1) {
				// create a group by yourself
				myGroup = createGroup(myData, shmStruct->groups, &shmStruct->groupsSize, semId);
				myGroup->isClosed = TRUE;
			}
			else{
				// close your group
				if(myData->myGroupLeader == myData->regNum) {
					if(myGroup != NULL) {
						// closing the group
						myGroup->isClosed = TRUE;
						if(optimalLeader) {
							--(*openOptimalGroup);
						}
					}
					else{
						// renounce to my invite privilege
						myData->myGroupLeader = -1;
						if(optimalLeader) {
							--(*openOptimalGroup);
							++(*possibleOptimalGroup);
						}
					}
				}
			}
		}

		// checking if i can invite
		if(myData->myGroupLeader == -1 || (myData->myGroupLeader == myData->regNum && (myGroup == NULL || !myGroup->isClosed))) {
			// trying to get an invite permission
			if(!allowedToInvite && myData->myGroupLeader == -1 && myData->inviteLeft > 0 && (*possibleOptimalGroup > 0 || (*possibleOptimalGroup <= 0 && *openOptimalGroup <= 0))) {
				// invite permission obtained
				allowedToInvite = TRUE;
				if(*possibleOptimalGroup > 0) {
					// i can create an optimal group
					--(*possibleOptimalGroup);
					optimalLeader = TRUE;
					myData->myGroupLeader = myData->regNum;
					++(*openOptimalGroup);
				}
			}

			if(allowedToInvite && (((optimalLeader && !(*optInviteReserved)) || (!optimalLeader && !(*inviteReserved))))) {
				int size = (myGroup == NULL) ? 0 : myGroup->size;
				// checking if i can invite
				myData->inviting = pendingInvites == 0 && myData->inviteLeft > 0 && (myData->myGroupLeader == -1 || (myData->myGroupLeader == myData->regNum && size < myData->nOfElems));
				if(!optimalLeader) {
					*inviteReserved = myData->inviting;
				}
				else{
					*optInviteReserved = myData->inviting;
				}
			}
		}

		--(*readerCount);
		if(*readerCount == 0) {
			DO_AND_TEST(V(semId, RW_MUTEX));
		}
		DO_AND_TEST(V(semId, R_MUTEX));
		// Exiting reader phase

		if(myData->inviting) {
			// I' inviting
			DO_AND_TEST(P(semId, W_MUTEX));
			++(*writerCount);
			if(*writerCount == 1) {
				DO_AND_TEST(P(semId, RW_MUTEX));
			}
			DO_AND_TEST(V(semId, W_MUTEX));
			// Entering reading phase
			// There are no writer

			if(myGroup == NULL) {
				// im not a group leader yet
				if(myData->grade >= percentile) {
					tryToInvite(&findWorstStudent, myData->grade);
				}
				else{
					tryToInvite(&findBestStudent, myData->grade);
				}
			}
			else{
				if(maxGroupGrade(myGroup->member, myGroup->size)>=percentile) {
					tryToInvite(&findWorstStudent, maxGroupGrade(myGroup->member, myGroup->size));
				}
				else{
					tryToInvite(&findBestStudent, myData->grade);
				}
			}

			DO_AND_TEST(P(semId, W_MUTEX));
			if(!optimalLeader) {
				*inviteReserved = FALSE;
			}
			else{
				*optInviteReserved = FALSE;
			}
			--(*writerCount);
			if(*writerCount == 0) {
				DO_AND_TEST(V(semId, RW_MUTEX));
			}
			DO_AND_TEST(V(semId, W_MUTEX));
			// Exiting writer phase
		}
		// Synch with phase
		DO_AND_TEST(P(semId, WAIT_ALL));
	}

	DO_AND_TEST(P(semId, WAIT_END));
	DO_AND_TEST(wait0(semId, WAIT_RESULT));
	printf("[%d]\tMy AdE grade = %d\tMy SO grade = %d\n", getpid(), myData->grade, myData->finalGrade);

	shmdt(shmStruct);
	return 0;
}

//----------------//
//    FUNCTION    //
//----------------//

group * searchGroup(long leaderRegNum, group * groups, unsigned int groupSize){
	BOOL find = FALSE;
	int i = 0;
	while(i<groupSize && !find) {
		if(groups[i].member[0][0] == leaderRegNum)
			find = TRUE;
		else
			i++;
	}
	return groups + i;
}

group * createGroup(studentInfo* myData, group * groups, unsigned int * groupsSize, int semId){
	DO_AND_TEST(P(semId, SHM_MUTEX));
	group * newGroup = NULL;
	myData->myGroupLeader = myData->regNum;
	newGroup = groups+(*groupsSize);
	newGroup->member[0][0] = myData->regNum;
	newGroup->member[0][1] = myData->grade;
	newGroup->size = 1;
	newGroup->isClosed = FALSE;
	++(*groupsSize);
	DO_AND_TEST(V(semId, SHM_MUTEX));
	return newGroup;
}


BOOL canInvite(studentInfo * myData, studentInfo * studentArray, int size){
	int i = 0;
	BOOL canInvite = FALSE;
	while(i<size && !canInvite) {
		canInvite = studentArray[i].regNum != myData->regNum && studentArray[i].myGroupLeader == -1;
		i++;
	}
	return canInvite && myData->inviteLeft > 0 && (myData->myGroupLeader == myData->regNum || myData->myGroupLeader == -1);
}

BOOL canBeInvited(studentInfo * myData, group * groups, studentInfo * studentArray, int size, unsigned int groupSize, int * openOptimalGroup){
	BOOL canBeInvited = FALSE;
	if(myData->myGroupLeader == -1) {
		// i'm not in a group and i'm not a group leader
		canBeInvited = *openOptimalGroup > 0;
		for(int i = 0; i < size && !canBeInvited; i++) {
			if (studentArray[i].regNum != myData->regNum && (studentArray[i].myGroupLeader == -1 || (isGroupLeader(studentArray+i) && !isClosed(studentArray[i].regNum, groups, groupSize))) && studentArray[i].inviteLeft > 0) {
				canBeInvited = TRUE;
			}
		}
	}
	return canBeInvited;
}

BOOL isGroupLeader(studentInfo * student){
	return student->regNum == student->myGroupLeader;
}

BOOL isClosed(long groupLeader, group * groups, unsigned int groupSize){
	int i;
	BOOL find = FALSE;
	for(i = 0; i<groupSize && !find; i++) {
		if(groups[i].member[0][0] == groupLeader && groups[i].isClosed) {
			find = TRUE;
		}
	}
	return find;
}

unsigned short maxGroupGrade(unsigned int member[4][2], unsigned short size){
	unsigned short max = member[0][1];
	for(int i = 0; i<size; i++) {
		if(member[i][1] > max) {
			max = (unsigned short)member[i][1];
		}
	}
	return max;
}

unsigned short minGroupGrade(unsigned int member[4][2], unsigned short size){
	unsigned short min = member[0][1];
	for(int i = 0; i<size; i++) {
		if(member[i][1] < min) {
			min = (unsigned short)member[i][1];
		}
	}
	return min;
}

int sendInvite(int msgId, studentInfo * myData, unsigned int grade,long invitedStudent) {
	msgbuf inviteMessage;
	message invite;
	invite.type = INVITE;
	invite.regNum = myData->regNum;
	invite.nOfElems = myData->nOfElems;
	invite.data = grade;
	inviteMessage.mtype = invitedStudent;
	inviteMessage.mtext = invite;
	if(msgsnd(msgId, &inviteMessage, sizeof(invite), IPC_NOWAIT) == -1) {
		if(errno != EAGAIN) {
			fprintf(stderr,"Fatal error (#%d): %s || In file: %s || in function: %s || at line: %d.\n",errno,strerror(errno), __FILE__,__func__, __LINE__ );
			exit(EXIT_FAILURE);
		}
		else {
			return -1;
		}
	}
	return 0;
}

void answerToInvite(int msgId, long myRegNum, long senderId, int data){
	msgbuf answerMessage;
	message answer;
	answer.type = ANSWER;
	answer.regNum = myRegNum;
	answer.nOfElems = 0; // the value of this field does not matter
	answer.data = data;
	answerMessage.mtype = senderId;
	answerMessage.mtext = answer;
	if(msgsnd(msgId,&answerMessage, sizeof(answer), 0) == -1) {
		if(errno != EAGAIN) {
			fprintf(stderr,"Fatal error (#%d): %s || In file: %s || in function: %s || at line: %d.\n",errno,strerror(errno), __FILE__,__func__, __LINE__ );
			exit(EXIT_FAILURE);
		}
		else{
			printf("CODA PIENA!\n");
			exit(EXIT_FAILURE);
		}
	}
}

/*
   If mode == FALSE :   then the nOfElems preference does not matter
                      so the return value is best student
                      beside me
   If mode == TRUE :  then i return the best student with my same nOfElems
                      preference
 */
studentInfo * findBestStudent(studentInfo * myData, studentInfo * studentArray, int size, BOOL mode){
	int studentIndex = size - 1;
	BOOL find = FALSE;
	if(!mode) {
		// cycle until i find the best student whose regNum is different from mine
		while(studentIndex >= 0 && !find) {
			if(studentArray[studentIndex].regNum != myData->regNum && studentArray[studentIndex].myGroupLeader == -1 && !studentArray[studentIndex].inviting)
				find = TRUE;
			else
				--studentIndex;
		}
	}
	else{
		while(studentIndex >= 0 && !find) {
			if(studentArray[studentIndex].regNum != myData->regNum && studentArray[studentIndex].nOfElems == myData->nOfElems && studentArray[studentIndex].myGroupLeader == -1 && !studentArray[studentIndex].inviting)
				find = TRUE;
			else
				--studentIndex;
		}
	}
	if(!find) {
		// i have not found a student beside me
		return NULL;
	}
	else{
		return studentArray+studentIndex;
	}
}

studentInfo * findWorstStudent(studentInfo * myData, studentInfo * studentArray, int size, BOOL mode){
	unsigned int studentIndex = 0;
	BOOL find = FALSE;
	if(!mode) {
		// cycle until i find the worse student whose regNum is different from mine
		while(studentIndex <= size && !find) {
			if(studentArray[studentIndex].regNum != myData->regNum && studentArray[studentIndex].myGroupLeader == -1 && !studentArray[studentIndex].inviting) {
				find = TRUE;
			}
			else
				++studentIndex;
		}
	}
	else{
		// cycle until i find the worse student whose regNum is different from mine
		while(studentIndex <= size && !find) {
			if(studentArray[studentIndex].regNum != myData->regNum && studentArray[studentIndex].nOfElems == myData->nOfElems && studentArray[studentIndex].myGroupLeader == -1 && !studentArray[studentIndex].inviting)
				find = TRUE;
			else
				++studentIndex;
		}
	}
	if(!find) {
		// i have not found a student beside me
		return NULL;
	}
	else{
		return studentArray+studentIndex;
	}
}

message * findBestInvite(message * invite, unsigned int inviteSize, unsigned short nOfElems){
	int bestInviteIndex = -1;
	if(nOfElems == 0) {
		bestInviteIndex = 0;
		for(int i = 0; i < inviteSize; i++) {
			if(invite[i].data > invite[bestInviteIndex].data) {
				bestInviteIndex = i;
			}
		}
	}
	else{
		// find the first invite compatible with my nOfElems preference
		int i = 0;
		while(i < inviteSize && bestInviteIndex == -1) {
			if(invite[i].nOfElems == nOfElems)
				bestInviteIndex = i;
			++i;
		}
		for(; i < inviteSize; i++) {
			if(invite[i].nOfElems == nOfElems && invite[i].data > invite[bestInviteIndex].data) {
				bestInviteIndex = i;
			}
		}
	}
	if(bestInviteIndex != -1) {
		return &invite[bestInviteIndex];
	}
	else {
		return NULL;
	}
}

message * findWorseInvite(message * invite, unsigned int inviteSize, unsigned short nOfElems){
	int worseInviteIndex = -1;
	if(nOfElems == 0) {
		worseInviteIndex = 0;
		for(int i = 0; i < inviteSize; i++) {
			if(invite[i].data < invite[worseInviteIndex].data) {
				worseInviteIndex = i;
			}
		}
	}
	else{
		// find the first invite compatible with my nOfElems preference
		int i = 0;
		while(i < inviteSize && worseInviteIndex == -1) {
			if(invite[i].nOfElems == nOfElems) {
				worseInviteIndex = i;
			}
			++i;
		}
		for(; i < inviteSize; i++) {
			if(invite[i].nOfElems == nOfElems && invite[i].data < invite[worseInviteIndex].data) {
				worseInviteIndex = i;
			}
		}
	}
	if(worseInviteIndex != -1) {
		return &invite[worseInviteIndex];
	}
	else {
		return NULL;
	}
}

int rejectAllExcept(int msgId, long myRegNum, long senderId, message * invite, int inviteSize){
	for(int i = 0; i < inviteSize; i++) {
		if(invite[i].regNum != senderId) {
			answerToInvite(msgId, myRegNum, invite[i].regNum, 0);
		}
	}
	int rejectionLeft = inviteSize - 1;
	return rejectionLeft;
}

int rejectAll(int msgId, long myRegNum, message * invite, int inviteSize) {
	for(int i = 0; i < inviteSize; i++) {
		answerToInvite(msgId, myRegNum, invite[i].regNum, 0);
	}
	int rejectionLeft = inviteSize;
	return rejectionLeft;
}

short getPref(int g2, int g3, int g4) {
	// generating random value
	int val = rand() % 100;
	// selection based on probabilities
	if (val < g2) {
		return 2;
	}
	else if (val < g2 + g3) {
		return 3;
	}
	else {
		return 4;
	}
}

void* init_ipc(key_t semKey, key_t shmKey, key_t msgKey) {
	SHM_STRUCT;

	//Get the IPC facilities
	DO_AND_TEST(semId = semget(semKey, 0, 0660));
	DO_AND_TEST(shmId = shmget(shmKey, 0, 0660));
	DO_AND_TEST(msgId = msgget(msgKey, 0660));

	struct shm* shmStruct;
	DO_AND_TEST(shmStruct = (struct shm*) shmat(shmId, NULL, 0));

	return shmStruct;
}

void init_locals(void* pShm, BOOL delta) {
	SHM_STRUCT;
	struct shm* shmStruct = (struct shm*) pShm;

	timeOut = &(shmStruct->timeOut);
	studentSize = select_par(delta, shmStruct->evenSize, shmStruct->oddSize);
	studentArray = select_par(delta, shmStruct->even, shmStruct->odd);
	percentile = select_par(delta, shmStruct->evenMean, shmStruct->oddMean);
	readerCount = select_par(delta, &(shmStruct->evenReaderCount), &(shmStruct->oddReaderCount));
	writerCount = select_par(delta, &(shmStruct->evenWriterCount), &(shmStruct->oddWriterCount));
	inviteReserved = select_par(delta, &(shmStruct->evenInviteReserved), &(shmStruct->oddInviteReserved));
	R_MUTEX = select_par(delta, R_MUTEX_EVEN, R_MUTEX_ODD);
	W_MUTEX = select_par(delta, W_MUTEX_EVEN, W_MUTEX_ODD);
	RW_MUTEX = select_par(delta, RW_MUTEX_EVEN, RW_MUTEX_ODD);
	WAIT_ALL = select_par(delta, WAIT_ALL_EVEN, WAIT_ALL_ODD);

	switch(myData->nOfElems) {
	case 2:
		openOptimalGroup = select_par(delta, &(shmStruct->evenG2OpenGroup), &(shmStruct->oddG2OpenGroup));
		possibleOptimalGroup = select_par(delta, &(shmStruct->maxG2EvenOptimalGroup), &(shmStruct->maxG2OddOptimalGroup));
		optInviteReserved = select_par(delta, &(shmStruct->evenG2InviteReserved), &(shmStruct->oddG2InviteReserved));
		break;
	case 3:
		openOptimalGroup = select_par(delta, &(shmStruct->evenG3OpenGroup), &(shmStruct->oddG3OpenGroup));
		possibleOptimalGroup = select_par(delta, &(shmStruct->maxG3EvenOptimalGroup), &(shmStruct->maxG3OddOptimalGroup));
		optInviteReserved = select_par(delta, &(shmStruct->evenG3InviteReserved), &(shmStruct->oddG3InviteReserved));
		break;
	case 4:
		openOptimalGroup = select_par(delta, &(shmStruct->evenG4OpenGroup), &(shmStruct->oddG4OpenGroup));
		possibleOptimalGroup = select_par(delta, &(shmStruct->maxG4EvenOptimalGroup), &(shmStruct->maxG4OddOptimalGroup));
		optInviteReserved = select_par(delta, &(shmStruct->evenG4InviteReserved), &(shmStruct->oddG4InviteReserved));
		break;
	}
}

void* init_mydata(void* pShm, BOOL delta) {
	SHM_STRUCT;
	struct shm* shmStruct = (struct shm*) pShm;

	srand(getpid());

	// Getting the array of students on which I have to work
	studentInfo* pool = select_par(delta, shmStruct->even, shmStruct->odd);
	// Getting a pointer to the var containing the size (because of operations
	// on said value, I can't simply get the value itself)
	unsigned int* size = select_par(delta, &(shmStruct->evenSize), &(shmStruct->oddSize));

	// Writing data on SHM
	DO_AND_TEST(P(semId, SHM_MUTEX));
	INIT_AND_WRITE(pool, *size);
	studentInfo* data = &(pool[*size]);
	++(*size);

	DO_AND_TEST(V(semId, SHM_MUTEX));

	return data;
}

studentInfo* findOptStud(studentInfo* (*fFindStud)(studentInfo *, studentInfo *, int, BOOL)) {
	studentInfo* optStudent = NULL;
	// my grade is lower than the percentile
	if(*possibleOptimalGroup > 0 || (*possibleOptimalGroup <= 0 && *openOptimalGroup <= 0)) {
		// I can be a group leader
		optStudent = fFindStud(myData, studentArray, studentSize, TRUE);
	}
	return optStudent;
}

void acceptAndReject(message* invites, message* msg, int* inviteSize, int* rejectionLeft) {
	// someone with a grade greater than the percentile invited me
	// or i can't reject all the invite i received
	// or i cannot invite a better student
	answerToInvite(msgId, myData->regNum, msg->regNum, myData->grade);
	myData->myGroupLeader = msg->regNum;
	*rejectionLeft -= rejectAllExcept(msgId, myData->regNum, msg->regNum, invites, *inviteSize);
	*inviteSize = 0;
}

void analyseInvite(studentInfo* (*fFindStud)(studentInfo *, studentInfo *, int, BOOL), message* (*fFindInv)(message *, unsigned int, unsigned short), message* invites, int* inviteSize, int* rejectionLeft) {
	message * optInvite; //pointer to the best invite
	studentInfo * optStudent = findOptStud(fFindStud); //pointer to the best student info

	//finding best invite
	optInvite = fFindInv(invites, *inviteSize, myData->nOfElems);
	if(optInvite != NULL) {
		// someone with my same preference invited me;
		if(optInvite->data >= percentile || (*possibleOptimalGroup <= 0) || *inviteSize>*rejectionLeft || optStudent == NULL || optStudent->grade<=optInvite->data || myData->inviteLeft == 0) {
			acceptAndReject(invites, optInvite, inviteSize, rejectionLeft);
		}
		else {
			// i can invite a better student and i can reject all the invite
			*rejectionLeft -= rejectAll(msgId, myData->regNum, invites, *inviteSize);
			*inviteSize = 0;
		}
	}
	else{
		// no one with my same preference invited me
		if(*inviteSize <= *rejectionLeft && (*openOptimalGroup > 0 || (*possibleOptimalGroup > 0 && myData->inviteLeft > 0))) {
			// i can invite a better student and i can reject all the invite
			*rejectionLeft -= rejectAll(msgId, myData->regNum, invites, *inviteSize);
			*inviteSize = 0;
		}
		else{
			// even though the one that invited me has a different group preference
			// i cannot invite a better student
			message * optRemainingInvite = fFindInv(invites, *inviteSize, 0);
			acceptAndReject(invites, optRemainingInvite, inviteSize, rejectionLeft);
		}
	}
}

int analyseAnswer(void* pShm, message * answer, int answerSize){
	SHM_STRUCT;
	struct shm* shmStruct = (struct shm*) pShm;

	for(int i = 0; i<answerSize; ++i) {
		if(answer[i].data<18) {
			// is a rejection
			--pendingInvites;
		}
		else if(!(*timeOut)){
			--pendingInvites;
			if(myGroup == NULL) {
				// create grocharup and add my info to it
				myGroup = createGroup(myData, shmStruct->groups, &(shmStruct->groupsSize), semId);
			}
			// adding member info to the group info
			myGroup->member[myGroup->size][0] = answer[i].regNum;
			myGroup->member[myGroup->size][1] = answer[i].data;
			// incrementing group size by one
			++(myGroup->size);
			// checking if the group has to be closed
			if(myGroup->size == myData->nOfElems) {
				// if the group has the same amount of element as i wish it has then i close it
				myGroup->isClosed = TRUE;
				DO_AND_TEST(P(semId, SHM_MUTEX));
				if(optimalLeader) {
					--(*openOptimalGroup);
				}
				DO_AND_TEST(V(semId, SHM_MUTEX));
			}
		}
	}
	answerSize = 0;
	return answerSize;
}

void tryToInvite(studentInfo* (*fFind)(studentInfo*, studentInfo*, int, BOOL), int grade) {
	// my grade is greater than the percentile
	// finding the worst student to invite
	studentInfo * optStudent = fFind(myData, studentArray, studentSize, TRUE);
	if(optStudent == NULL) {
		// there is no student with my same preference
		optStudent = fFind(myData, studentArray, studentSize, FALSE);
	}
	if(optStudent != NULL) {
		// there is a student to invite
		if(sendInvite(msgId, myData, grade, optStudent->regNum) != -1) {
			// invite sent
			++pendingInvites;
			--(myData->inviteLeft);
		}
	}
}
