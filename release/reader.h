#ifndef _READER_H_
#define _READER_H_

#include <stdio.h>
#include <stdlib.h>
#include "bool.h"

extern int g2, g3, g4, max_rejects, nof_invites, sim_time, pop_size;

// function controlling possible overflow of INT variables
// PARAMETERS:
// - [STRING]: string representing the number
// RETURN:
// + true = the number causes overflow
// + false = the number is convertible to integer
BOOL check_overflow(char*);

// function used to update a given parameter of the reader
// PARAMETERS:
// - [BOOL]: pointer to the flag containing if the key has already been set
// - [INT]: pointer to the key to update
// - [INT]: value to which the key must be set
// RETURN:
// + true = the key has been set
// + false = the key wasn't set (due to flag TRUE or errors)
BOOL set_opt(BOOL*, int*, int);

// special wrapper of set_opt for the groups percentage
// PARAMETERS:
// - [FILE]: pointer to the config file
// - [BOOL]: pointer to the flag to pass to set_opt
// - [INT]: pointer to the key to pass to set_opt
// - [STRING]: string used for the check routine
// RETURN:
// + true = the key has been set
// + false = the key wasn't set (due to flag TRUE or errors)
BOOL set_group_perc(FILE*, BOOL*, int*, char*);

// function scanning the config file updating the global parameters
// PARAMETERS:
// - [STRING]: path to the config file
// RETURN:
// + -1 = the config isn't is valid
// + not -1 = the config is valid
extern int scan(char*);

#endif
