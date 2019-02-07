#include "logger.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef LOG

#ifdef LOG_DEBUG
void dbg_log(FILE* stream, char* format, ...) {
	va_list args;
	va_start(args, format);
	fprintf(stream, format, args);
	va_end(args);
}
#else
void dbg_log(void* a, ...) {
}
#endif

#define COUNT_MAX 128
#define STR_MAX 256

char dir_path[STR_MAX] = "";
BOOL init_done = FALSE;

FILE* init_log(char* file_name, BOOL* new_line, BOOL dir_conf) {
	char file_path[STR_MAX];
	dbg_log(stdout, "Starting init...\n");
	if(!init_done) {
		init_done = TRUE; // so I don't create multiple folders (due to, you know, the nature of time itself)

		dbg_log(stdout, "Generating time info init...\n");
		// getting localized time
		time_t raw = time(NULL);
		dbg_log(stdout, "Getting local time...\n");
		struct tm* tm_info = localtime(&raw);

		// creating log directory if it doesn't exist
		struct stat st = {0};
		if (stat("logs/", &st) == -1) {
			dbg_log(stdout, "Creating logs directory...");
			mkdir("logs/", 0700);
		}

		dbg_log(stdout, "Generating dir path...\n");

		// creating the dir name
		sprintf(file_path, "logs/%d-%d-%d %d-%d-%d", (tm_info->tm_year + 1900), (tm_info->tm_mon + 1), tm_info->tm_mday, tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
		unsigned int count = 0;
		if (dir_conf || stat(file_path, &st) == -1) { // Se la cartella ancora non esiste, la creo normalmente
			strcat(file_path, "/");
			mkdir(file_path, 0700);
		} else {  // Se la cartella esiste, cerco di crearne una nuova fino a che non ne trovo una inesistente
			char str_test[STR_MAX];
			do {
				++count;
				char str_cat[STR_MAX] = "";
				strcpy(str_test, file_path);
				sprintf(str_cat, " (%d)/", count);
				strcat(str_test, str_cat);
			} while (stat(str_test, &st) != -1 || count > COUNT_MAX);
			strcpy(file_path, str_test);
			mkdir(file_path, 0700);
		}

		// saving it for future use
		strcpy(dir_path, file_path);
	}
	else {
		strcpy(file_path, dir_path);
	}

	dbg_log(stdout, "Generating file name...\n");
	// creating the file name
	char file[STR_MAX];
	snprintf(file, STR_MAX - 1, "%d %s.log", getpid(), file_name);
	strcat(file_path, file);

	dbg_log(stdout, "Generating file path %s...\n", file_path);
	// opening the actual file
	FILE* f = fopen(file_path, "w");

	dbg_log(stdout, "Writing on file...\n");

	// starting log file info
	fprintf(f, "LOG INIT... Done.\nThis log data in this file is meant for debugging purposes.\n\n");

	*new_line = TRUE;
	dbg_log(stdout, "Log file: Init Done for %s.\n", file_path);
	return f;
}

void log_nfo(FILE* file, BOOL* new_line, BOOL prefix, char* format, ...) {
	dbg_log(stdout, "Generating time info...\n");
	// getting localized time
	time_t raw = time(NULL);
	dbg_log(stdout, "Getting local time...\n");
	struct tm* tm_info = localtime(&raw);

	dbg_log(stdout, "Initializing va data...\n");
	// getting args list for the pring
	va_list log_data;
	va_start(log_data, format);

	dbg_log(stdout, "Generating formatted string...\n");
	// generating custom format STRING
	char str_format[STR_MAX] = "";
	if (prefix) {  // adding the prefix
		dbg_log(stdout, "Adding new line... (%d)\n", (*new_line));
		if (!(*new_line)) { // adding new line at the head
			strcpy(str_format, "\n");
		}
		sprintf(str_format, "[%d:%d:%d] ", tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
	}
	// adding the actual format
	strcat(str_format, format);

	dbg_log(stdout, "Writing on log file...\n");
	// generating complete log data
	char str_output[STR_MAX] = "";
	vsprintf(str_output, str_format, log_data);
	size_t str_len = strlen(str_output);

	// checking if terminating with new line
	if (str_output[str_len - 1] == '\n') {
		*new_line = TRUE;
		dbg_log(stdout, "Got new line @ end of line in:\n\t%s", str_output);
	}
	else {
		*new_line = FALSE;
	}

	// printing on file
	fprintf(file, "%s", str_output);
	dbg_log(stdout, "Closing va data...\n");
	va_end(log_data);
	dbg_log(stdout, "Writing Done.");
}

void end_log(FILE* file, BOOL* new_line) {
	dbg_log(stdout, "Ending log file... ");
	if(!(*new_line)) {  // adding new line if necessary
		dbg_log(stdout, "Adding new line... (%d)\n", *new_line);
		fprintf(file, "\n");
	}
	// closing the log file info
	fprintf(file, "\nLOG END... Done.\nCLOSING FILE... Done.");
	*new_line = FALSE; // resetting flag
	dbg_log(stdout, "Closing file... ");
	// closing the actual file
	fclose(file);
	free(file);

	dbg_log(stdout, "Done.\n");
}

#else

#include <stdio.h>
#include <stdarg.h>

FILE* init_log(void* a, ...){
	return NULL;
}
void log_nfo(void* a, ...){
}
void end_log(void* a, ...){
}

#endif
