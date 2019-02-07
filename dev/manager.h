#ifndef _MANAGER_H_
#define _MANAGER_H_

#include "lib.h"
#include "reader.h"

//------------//
// PROTOTYPES //
//------------//

// Init methods

// Initializing the basic data of the manager
// in particular, initializing the log file, if LOGGING is enabled
// and reading the configuration parameters from the config file
void init();

// Creating the semaphores area through IPC lib
// PARAMETERS
// - [KEY]: the key from which to generate the semaphores
// RETURN:
// + The semaphores ID
int init_sems(key_t);

// Creating the shared memory area through IPC lib
// PARAMETERS
// - [KEY]: the key from which to generate the shared memory
// - [INT]: the shared memory ID
// RETURN:
// + pointer to the shared memory area struct
void* init_shm(key_t, int*);

// Creating the message queue through IPC lib
// PARAMETERS
// - [KEY]: the key from which to generate the message queue
// RETURN:
// + The messague queue ID
int init_msgq(key_t);

// Initializing the value of all the shared memory area
// to their base values
// PARAMETERS:
// - [POINTER]: pointer to the shared memory area
void init_shm_struct(void*);

// Utility methods

// Generates the string representing the value of the key
// PARAMETERS:
// - [KEY]: key to convert
// RETURN:
// + The string containing the key
char* ktoa(key_t);

// Generates the string representing the number
// PARAMETERS:
// - [INT]: number to convert
// RETURN:
// + The string containing the number
char* itoa(int);

// Calculates the mean of the grades of a pool of students
// PARAMETERS:
// - [STUDENT ARRAY]: pool of students
// - [SIZE]: number of students
// RETURN:
// + The mean grade
unsigned short mean(studentInfo*, size_t);

// Sorts a pool of students using the bubble sort algorithm
// PARAMETERS:
// - [STUDENT ARRAY]: pool of students
// - [SIZE]: number of students
void bubbleSort(studentInfo*, size_t);

// Counts the number of students having the same group
// size preference in a pool of students
// PARAMETERS:
// - [STUDENT ARRAY]: pool of students
// - [SIZE]: number of students
// - [SHORT]: prefered group size
// RETURN:
// + Number of occurences of the preference
int countOccurence(studentInfo*, size_t, unsigned short);

// Calculates the sum of the grades in a pool of students
// PARAMETERS:
// - [STUDENT ARRAY]: pool of students
// - [SIZE]: number of students
// RETURN:
// + Summatory of the grades
unsigned int sumAll(studentInfo *, size_t);

// Sets the grade of all students depending on their prefered
// group size, the actual group size and the maximum grade in
// the group
// PARAMETERS:
// - [STUDENT ARRAY]: pool of students
// - [GROUP ARRAY]: students' groups
// - [SIZE]: number of students
void assignGrade(studentInfo *, group *, size_t);

// Calculates the mean at the end of the group creation phase
// PARAMETERS:
// - [STUDENT ARRAY]: odd registry number students
// - [SIZE]: number of odd registry number students
// - [STUDENT ARRAY]: even registry number students
// - [SIZE]: number of even registry number students
// - [GROUP ARRAY]: students' groups
// - [SIZE]: number of groups
// RETURN:
// + Mean of the final grades
float finalMean(studentInfo *, size_t, studentInfo *, size_t, group *, size_t);

// Calculates the grades' n-th percentile in a pool of students
// PARAMETERS:
// - [STUDENT ARRAY]: pool of students
// - [SIZE]: number of students
// - [INT]: n-th percentile
// RETURN:
// + n-th percentile value
int percentile(studentInfo *, size_t, int);

// Output methods

// Outputs statistics about the groups
// PARAMETERS:
// - [STUDENT ARRAY]: odd registry number students
// - [SIZE]: number of odd registry number students
// - [STUDENT ARRAY]: even registry number students
// - [SIZE]: number of even registry number students
// - [GROUP ARRAY]: students' groups
// - [SIZE]: number of groups
void stats( studentInfo *, size_t, studentInfo *, size_t, group*, size_t);

// Outputs informations about the student and controls
// if it is part of a group that is optimal for himself
// PARAMETERS:
// - [STUDENT]: student to control
// - [GROUP ARRAY]: students' groups
// RETURN:
// + 0 = not optimal
// + 1 = optimal
int printStudentInfo(studentInfo*, group*);

// Outputs informations about the shared memory
// PARAMETERS:
// - [STUDENT ARRAY]: odd registry number students
// - [SIZE]: number of odd registry number students
// - [STUDENT ARRAY]: even registry number students
// - [SIZE]: number of even registry number students
// - [GROUP ARRAY]: students' groups
// - [SIZE]: number of groups
void printShm(studentInfo*, size_t, studentInfo*, size_t, group*, size_t);

// Handlers

// The handler for the alarm signal
// PARAMETERS:
// - [INT]: signal to handle
void alarmHandler(int);

#endif
