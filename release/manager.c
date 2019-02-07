#include "manager.h"
#include <limits.h>
#include <math.h>
#include <sys/wait.h>

//--------------------------//
// GLOBAL MANAGER VARIABLES //
//--------------------------//

// Size of odd registry number students
unsigned int oddSize;
// Size of even registry number students
unsigned int evenSize;
// List of all odd registry number students
studentInfo * odd;
// List of all even registry number students
studentInfo * even;

int main(void) {
	// Initializing data, reading from config file.
	init();

	// Initializing the shared memory structure based on the
	// opt.conf data
	SHM_STRUCT;

	//----------------------//
	// SHARED STUFF INIT... //
	//----------------------//

	// Generating shared area keys
	key_t semKey = ftok("./manager.c", 1);
	key_t shmKey = ftok("./manager.c", 2);
	key_t msgKey = ftok("./manager.c", 3);

	// Generating the actual shared areas
	int shmId;
	struct shm* shmStruct = (struct shm*) init_shm(shmKey, &shmId);
	int semId = init_sems(semKey);
	int msgId = init_msgq(msgKey);

	//-------------------//
	// STUDENTS INIT...  //
	//-------------------//

	// Converting the data for the student to strings
	char* shmKeyStr = ktoa(shmKey);
	char* semKeyStr = ktoa(semKey);
	char* msgKeyStr = ktoa(msgKey);
	char* maxRejectStr = itoa(max_rejects);
	char* nofInvitesStr = itoa(nof_invites);
	char* popSizeStr = itoa(pop_size);
	char* g2Str = itoa(g2);
	char* g3Str = itoa(g3);
	char* g4Str = itoa(g4);

	// Generating the students...
	int iPop;
	for (iPop = 0; iPop < pop_size; ++iPop) {
		switch (fork()) {   // Generating child process
		case -1:    // ERROR CASE
			fprintf(stderr, "Fatal error (#%d): %s.\n",errno,strerror(errno));
			exit(EXIT_FAILURE);
		case 0:     // CHILD CASE
		{
			char* args[] = {"student", shmKeyStr, semKeyStr, msgKeyStr, maxRejectStr, nofInvitesStr, popSizeStr, g2Str, g3Str, g4Str, NULL};

			// Starting the execution of the student code...
			DO_AND_TEST(execve("./student.out", args, NULL));
		}
		}
	}

	// Waiting until all children have been initialized
	wait0(semId, MANAGER_WAIT_0);

	init_shm_struct(shmStruct); // Initializing data of the shared memory

	// Sorting the vectors
	bubbleSort(shmStruct->even, shmStruct->evenSize);
	bubbleSort(shmStruct->odd, shmStruct->oddSize);

	// Setting the alarm based on SIM_TIME
	struct sigaction sa;
	sigset_t my_mask;
	sa.sa_handler = alarmHandler;
	sa.sa_flags = 0;
	sigemptyset(&my_mask);
	sa.sa_mask = my_mask;
	DO_AND_TEST(sigaction(SIGALRM, &sa, NULL));
	DO_AND_TEST(alarm(sim_time));

	// Authorize child to proceed
	P(semId, STUDENT_WAIT_0);

	// Waiting until all children have ended or SIM_TIME has passed
	if(wait0(semId, WAIT_END) == -1) {
		if(errno == EINTR) {
			shmStruct->timeOut = TRUE;
			DO_AND_TEST(wait0(semId, WAIT_END));
		}
		else{
			fprintf(stderr, "Fatal error (#%d): %s.\n",errno,strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
	// Assign final grade to the student
	for(int i = 0; i < shmStruct->oddSize; i++) {
		assignGrade(shmStruct->odd+i, shmStruct->groups, shmStruct->groupsSize);
	}
	for(int i = 0; i < shmStruct->evenSize; i++) {
		assignGrade(shmStruct->even+i, shmStruct->groups, shmStruct->groupsSize);
	}
	DO_AND_TEST(P(semId, WAIT_RESULT));
	// Wait child to die
	alarm(0);
	int status;
	while(wait(&status) > 0){
		if(!WIFEXITED(status)){
			fprintf(stderr, "Fatal error: a student has died due to an unknown cause\n");
			exit(EXIT_FAILURE);
		}
	}
	if(errno != ECHILD) {
		fprintf(stderr, "Fatal error (#%d): %s.\n",errno,strerror(errno));
		exit(EXIT_FAILURE);
	}

	// stampa media e statistiche
	stats(shmStruct->odd, shmStruct->oddSize, shmStruct->even, shmStruct->evenSize, shmStruct->groups, shmStruct->groupsSize);
	printf("AdE mean: %f\n", ((sumAll(shmStruct->even, shmStruct->evenSize) + sumAll(shmStruct->odd, shmStruct->oddSize))/((float)(shmStruct->evenSize + shmStruct->oddSize))));
	printf("SO mean : %f\n\n", finalMean(shmStruct->odd, shmStruct->oddSize, shmStruct->even, shmStruct->evenSize, shmStruct->groups, shmStruct->groupsSize));

	// Cleaning memory
	free(shmKeyStr);
	free(semKeyStr);
	free(msgKeyStr);
	free(maxRejectStr);
	free(nofInvitesStr);
	free(popSizeStr);
	free(g2Str);
	free(g3Str);
	free(g4Str);
	
	// detaching and de-allocating the SM and the other IPC facilities
	shmdt(shmStruct);
	DO_AND_TEST(shmctl(shmId, IPC_RMID, NULL));
	printf("Shared memory removed\n");
	DO_AND_TEST(semctl(semId, 0, IPC_RMID));
	printf("Semaphores removed\n");
	DO_AND_TEST(msgctl(msgId, 0, IPC_RMID));
	printf("Message queue removed\n");

	return 0;
}

//-----------------//
//    FUNCTIONS    //
//-----------------//

void init() {
	// Retrieving the data from the configuration file
	int r = scan("opt.conf");

	// Checking if config data is correct
	if (!r) {
		fprintf(stderr, "[MANAGER] Invalid file!\n");
		exit(EXIT_FAILURE);
	}
}

int init_sems(key_t semKey) {
	// generating semaphores (SEM)...
	int semId;  // SEM ID
	DO_AND_TEST(semId = semget(semKey, NUM_OF_SEM, IPC_CREAT | IPC_EXCL | 0660));
	// initializing all semaphores
	unsigned short semVals[NUM_OF_SEM];
	semVals[WAIT_ALL_ODD] = 0;
	semVals[WAIT_ALL_EVEN] = 0;
	semVals[MANAGER_WAIT_0] = pop_size;
	semVals[WAIT_END] = pop_size;
	for(int i = SHM_MUTEX; i<NUM_OF_SEM; i++) {
		semVals[i] = 1;
	}
	union semun arg;
	arg.array = semVals;
	DO_AND_TEST(semctl(semId, 0, SETALL, arg));
	return semId;
}

void* init_shm(key_t shmKey, int* shmId) {
	SHM_STRUCT;
	// generating shared memory (SM) area...
	DO_AND_TEST(*shmId = shmget(shmKey, sizeof(struct shm),
	                            IPC_CREAT | IPC_EXCL | 0660));

	struct shm* shmStruct = (struct shm*) malloc(sizeof(struct shm));

	// binding the SM area...
	DO_AND_TEST(shmStruct = (struct shm*) shmat(*shmId, NULL, 0));

	shmStruct->evenSize = 0;
	shmStruct->oddSize = 0;
	shmStruct->groupsSize = 0;
	return shmStruct;
}

int init_msgq(key_t msgKey) {
	// generating message queue (MSG)...
	int msgId;
	DO_AND_TEST(msgId = msgget(msgKey, IPC_CREAT | IPC_EXCL | 0660));
	return msgId;
}

void init_shm_struct(void* pShm) {
	SHM_STRUCT;

	struct shm* shmStruct = (struct shm*) pShm;

	shmStruct->evenMean = percentile(shmStruct->even, shmStruct->evenSize, 66);
	shmStruct->oddMean = percentile(shmStruct->odd, shmStruct->oddSize, 66);
	shmStruct->maxG2OddOptimalGroup = countOccurence(shmStruct->odd, shmStruct->oddSize, 2)/2;
	shmStruct->maxG3OddOptimalGroup = countOccurence(shmStruct->odd, shmStruct->oddSize, 3)/3;
	shmStruct->maxG4OddOptimalGroup = countOccurence(shmStruct->odd, shmStruct->oddSize, 4)/4;
	shmStruct->maxG2EvenOptimalGroup = countOccurence(shmStruct->even, shmStruct->evenSize, 2)/2;
	shmStruct->maxG3EvenOptimalGroup = countOccurence(shmStruct->even, shmStruct->evenSize, 3)/3;
	shmStruct->maxG4EvenOptimalGroup = countOccurence(shmStruct->even, shmStruct->evenSize, 4)/4;
	shmStruct->oddG2OpenGroup = 0;
	shmStruct->oddG3OpenGroup = 0;
	shmStruct->oddG4OpenGroup = 0;
	shmStruct->evenG2OpenGroup = 0;
	shmStruct->evenG3OpenGroup = 0;
	shmStruct->evenG4OpenGroup = 0;
	shmStruct->oddReaderCount = 0;
	shmStruct->oddWriterCount = 0;
	shmStruct->evenReaderCount = 0;
	shmStruct->evenWriterCount = 0;
	shmStruct->oddInviteReserved = FALSE;
	shmStruct->evenInviteReserved = FALSE;
	shmStruct->oddG2InviteReserved = FALSE;
	shmStruct->oddG3InviteReserved = FALSE;
	shmStruct->oddG4InviteReserved = FALSE;
	shmStruct->evenG2InviteReserved = FALSE;
	shmStruct->evenG3InviteReserved = FALSE;
	shmStruct->evenG4InviteReserved = FALSE;
	shmStruct->timeOut = FALSE;
}

char* ktoa(key_t key) {
	// maximum characters length of a key
	unsigned int size = floor(log10(key) + 2);
	//unsigned int size = 100;
	char* buff; // buffer containing the string representation of the key
	DO_AND_TEST(buff= (char*) calloc(size, sizeof(char)));
	DO_AND_TEST(sprintf(buff, "%d", key));  // converting the key to string...
	return buff;
}

char* itoa(int integer) {
	// maximum characters length of a key
	unsigned int size = floor(log10(integer) + 2);
	//unsigned int size = 100;
	char* buff; // buffer containing the string representation of the key
	DO_AND_TEST(buff= (char*) calloc(size, sizeof(char)));
	DO_AND_TEST(sprintf(buff, "%d", integer));  // converting the key to string...
	return buff;
}

void alarmHandler(int sig){
	printf("-- TIME OUT ! --\n");
}

void stats(studentInfo * odd, size_t oddSize, studentInfo * even, size_t evenSize, group* groups, size_t groupsSize) {
	unsigned int closed = 0;
	unsigned int notclosed = 0;
	unsigned int g1Group = 0;
	unsigned int g2Group = 0;
	unsigned int g3Group = 0;
	unsigned int g4Group = 0;

	for (unsigned int i = 0; i < groupsSize; i++) {
		if (!groups[i].isClosed) {
			notclosed++;
		}
		else {
			if (groups[i].size == 1) {
				g1Group++;
			}
			else if (groups[i].size == 2) {
				g2Group++;
			}
			else if (groups[i].size == 3) {
				g3Group++;
			}
			else if (groups[i].size == 4) {
				g4Group++;
			}
			closed++;
		}
	}

	unsigned int total_groups = g1Group + g2Group + g3Group + g4Group;

	printf("\nTotal number of groups: %d\n\n", total_groups);
	printf("Groups of 2 members: %d\n", g2Group);
	printf("Groups of 3 members: %d\n", g3Group);
	printf("Groups of 4 members: %d\n\n", g4Group);

	printf("Closed groups: %d\n", closed);
	printf("Open groups: %d\n\n", notclosed);

	unsigned int expectedG2 = countOccurence(even, evenSize, 2) + countOccurence(odd, oddSize, 2);
	unsigned int expectedG3 = countOccurence(even, evenSize, 3) + countOccurence(odd, oddSize, 3);
	unsigned int expectedG4 = countOccurence(even, evenSize, 4) + countOccurence(odd, oddSize, 4);

	printf("Expected G1:\t0\n");
	printf("Effective G1:\t%d\n\n", g1Group * 1);
	printf("Expected G2:\t%d\n", expectedG2);
	printf("Effective G2:\t%d\n\n", g2Group * 2);
	printf("Expected G3:\t%d\n", expectedG3);
	printf("Effective G3:\t%d\n\n", g3Group * 3);
	printf("Expected G4:\t%d\n", expectedG4);
	printf("Effective G4:\t%d\n\n", g4Group * 4);

	unsigned int evenOptimalStudent = 0;
	unsigned int oddOptimalStudent = 0;
	group * myGroup = NULL;
	for(int i = 0; i < evenSize; i++){
		myGroup = searchGroup(even[i].myGroupLeader, groups, groupsSize);
		if(myGroup != NULL){
			BOOL find = FALSE;
			for(int j = 0; j < myGroup->size && !find; j++){
				find = myGroup->member[j][0] == even[i].regNum;
			}
			if(find && myGroup->size == even[i].nOfElems && myGroup->isClosed){
				++evenOptimalStudent;
			}
		}
	}
	for(int i = 0; i< oddSize; i++){
		myGroup = searchGroup(odd[i].myGroupLeader, groups, groupsSize);
		if(myGroup != NULL){
			BOOL find = FALSE;
			for(int j = 0; j < myGroup->size && !find; j++){
				find = myGroup->member[j][0] == odd[i].regNum;
			}
			if(find && myGroup->size == odd[i].nOfElems && myGroup->isClosed){
				++oddOptimalStudent;
			}
		}
	}

	printf("evenSize: %zu\n", evenSize);
	printf("oddSize: %zu\n", oddSize);
	printf("Even students whose group is optimal:\t%d\n", evenOptimalStudent);
	printf("Odd students whose group is optimal:\t%d\n", oddOptimalStudent);
	printf("\nTotal students whose group is optimal:\t%d\n\n", evenOptimalStudent + oddOptimalStudent);

	unsigned int gradesCounter[13] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  unsigned int projGradesCounter[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  for (unsigned int i = 0; i < oddSize; ++i) {
		++gradesCounter[odd[i].grade - 18];

    if (odd[i].finalGrade == 0) {
      ++projGradesCounter[0];
    }
    else {
      ++projGradesCounter[odd[i].finalGrade - 14];
    }
  }

  for (unsigned int i = 0; i < evenSize; ++i) {
    ++gradesCounter[even[i].grade - 18];

    if (even[i].finalGrade == 0) {
      ++projGradesCounter[0];
    }
    else {
      ++projGradesCounter[even[i].finalGrade - 14];
    }
  }

  printf("\tNumber of students per grade:\n");
  printf("\tAdE\tSO\n");
  for (short i = 0; i < 17; ++i) {
    if (i < 4) {
      printf("%d\t/\t%d\n", (i == 0 ? 0 : (i + 14)), projGradesCounter[i]);
    }
    else {
      printf("%d\t%d\t%d\n", (i + 14), gradesCounter[i - 4], projGradesCounter[i]);
    }
  }
  printf("\n");
}

int countOccurence(studentInfo * studentArray, size_t size, unsigned short nOfElems){
	int count = 0;
	for(int i = 0; i < size; i++) {
		if(studentArray[i].nOfElems == nOfElems) {
			++count;
		}
	}
	return count;
}

unsigned int sumAll(studentInfo arr[], size_t arr_len){
	unsigned int sum = 0;
	for(size_t i = 0; i < arr_len; i++) {
		sum += arr[i].grade;
	}
	return sum;
}

unsigned short mean(studentInfo arr[], size_t arr_len){
	unsigned int sum = sumAll(arr, arr_len);
	return (sum / arr_len);
}

void bubbleSort(studentInfo arr[], size_t arr_len) {
	size_t len = arr_len - 1;

	for (size_t i = 0; i < arr_len; i++) {
		for (size_t j = 0; j < len; j++) {
			if (arr[j].grade > arr[j + 1].grade) {
				studentInfo tmp = arr[j];
				arr[j] = arr[j + 1];
				arr[j + 1] = tmp;
			}

			len--;
		}
	}
}

group * searchGroup(long leaderRegNum, group * groups, size_t groupSize){
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

unsigned short maxGroupGrade(unsigned int member[4][2], size_t size){
	unsigned short max = member[0][1];
	for(int i = 0; i<size; i++) {
		if(member[i][1] > max) {
			max = (unsigned short)member[i][1];
		}
	}
	return max;
}

void assignGrade(studentInfo * myData, group * groups, size_t groupSize){
	unsigned short grade = 0;
	if(myData->myGroupLeader != -1) {
		group * myGroup = searchGroup(myData->myGroupLeader, groups, groupSize);
		BOOL find = FALSE;
		for(int i = 0; i<myGroup->size && !find; i++){
			find = myGroup->member[i][0] == myData->regNum;
		}
		if(myGroup->isClosed && find) {
			grade = maxGroupGrade(myGroup->member, myGroup->size);
			if(myGroup->size != myData->nOfElems)
				grade -= 3;
		}
	}
	myData->finalGrade = grade;
}

float finalMean(studentInfo * odd, size_t oddSize, studentInfo * even, size_t evenSize, group * groups, size_t groupsSize){
	unsigned int sum = 0;
	for(int i = 0; i<oddSize; i++) {
		sum += odd[i].finalGrade;
	}
	for(int i = 0; i<evenSize; i++) {
		sum += even[i].finalGrade;
	}
	return sum/(float)(oddSize + evenSize);
}

int percentile(studentInfo* students, size_t size, int p) {
	int index = (p * size / 100);
	return students[index].grade;
}
