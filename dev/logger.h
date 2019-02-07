#ifndef _LOGGER_H_
#define _LOGGER_H_
#include <stdarg.h>
#include <stdio.h>

// Correct usage is very important!
// First of all, the logger creates a file every time you call
// the init_log routine. This routine creates the file and
// initializes the directories and the file in which the logging
// is done.
// Whenever you want to write in the log file, call the log_nfo
// with params the FILE* (that init_log generated) that is the file
// in which to log, a BOOL* that is used to store flag info (do not modify
// its value), a BOOL that is if to add the prefix with the time
// of the logging, a char* that is the format (as if printf) and
// a set of args that is the variables to substitute in the format
// (as if printf)
// At the end of the logging, call the end_log routine to close the
// file and end the log stream.

#ifdef LOG

#include "bool.h"

extern FILE* init_log(char*, BOOL*, BOOL);
extern void log_nfo(FILE*, BOOL*, BOOL, char*, ...);
extern void end_log(FILE*, BOOL*);

#else

extern FILE* init_log(void*, ...);
extern void log_nfo(void*, ...);
extern void end_log(void*, ...);

#endif

//-----------//
// STANDARDS //
//-----------//

// The following lines are not mandatory, these are just
// guidelines in order to create a tidy, easy-to-read, light-weight yet
// complete log-file.
//
// 1) Log only informations that are critical to the understanding of
//    the operations in act. E.G:
//
//    log_nfo(log_file, &log_flag, TRUE, "Opening config file...\n");
//
//    in this case, logging this makes aware of multiple informations:
//    - how far the program which the code has come
//    - what operation it is starting
//    this is critical because a file opening might cause problems like
//    seg-faults and/or faulty data input.
// 2) To get even more info, you can set it up like this
//
//    log_nfo(log_file, &log_flag, TRUE, "Opening config file... ");
//    FILE* cfg_file = fopen("config.cfg", "w");
//    log_nfo(log_file, &log_flag, FALSE, "Done.\n");
//
//    this makes the log file easy to read for the following reason:
//    you know what is happening, when and how it ended. In case of
//    seg-fault, there will be no "Done." in the log file, you know
//    something wrong happened in the opening process.
// 3) Log ALWAYS severe failure cases. this will be extremely clear
//    with an example:
//
//    log_nfo(log_file, &log_flag, TRUE, "Creating child process... ");
//    switch (fork()) {
//    case -1:
//      log_nfo(log_file, &log_flag, TRUE, "SEVERE - Process creation failed.\n");
//    break;
//    ...
//    default:
//      log_nfo(log_file, &log_flag, FALSE, "Done.\n");
//
//    as you can see, this code respects the above standards and adds
//    critical info in case of SEVERE errors, those that directly cause
//    the end of the program.
//
// The positioning and the logic of the log_nfo should be clear enough,
// use the above examples to understand how to effectivly use this tool.

#endif
