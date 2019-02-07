#include "reader.h"
#include <string.h>
#include <limits.h>
#include <math.h>

//-----------//
// VARIABLES //
//-----------//

// see header file
int g2, g3, g4, max_rejects, nof_invites, pop_size, sim_time;
// controls checking if the data has been set
BOOL g2_set = FALSE;
BOOL g3_set = FALSE;
BOOL g4_set = FALSE;
BOOL max_rejects_set = FALSE;
BOOL nof_invites_set = FALSE;
BOOL pop_size_set = FALSE;
BOOL sim_time_set = FALSE;

BOOL check_overflow(char* str) {
	// converting the number to long
	long l = strtol(str, NULL, 10);
	// getting the number of digits in the string representation
	size_t str_len = strlen(str);
	// getting the maximum number of digits representable by an INT
	size_t max_len = (size_t) floor(log10(INT_MAX)) + 1;

	// if the length of the string is greater than the maximum length,
	// if the long number is greater than the greatest representable INT
	// then it is certainly not representable as INT
	if (str_len > max_len || l > INT_MAX) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

// function used to skip clusers of the same character
// PARAMETERS:
// - [FILE]: file in input
// - [CHAR]: character to skip
static void skipc(FILE* f, char character) {
	// cycle through until I find a different character
	if (character == ' ' || character == '\t') {	// in case of whitespace, I have to consider both tab and space
		char c = fgetc(f);
		while (c == ' ' || c == '\t') {
			c = fgetc(f);
		}
	}
	else {	// in any other case, I can make it more compact
		while (fgetc(f) == character);
	}

	// setting back the pointer by one, thus putting it back on the
	// first different char than character
	fseek(f, ftell(f) - 1, SEEK_SET);
}

// function used to check if the next string corresponds to the expected
// PARAMETERS:
// - [FILE]: file in input
// - [STRING]: the expected string in input
// - [SIZE]: the length of the expected string
static int check(FILE* f, char* correct_str, size_t len) {
	int r_value = 0; // "r_value" stands for "return value"
	char read_str[len + 1];  // "len + 1" because we'll add the termination character '\0'
	char value[128]; // input value
	int space_found = FALSE; // flag for finding a white-space

	// Read the target string
	read_str[0] = correct_str[0];  // the first char has already been read
	for (int i = 1; i < len; i++) {
		read_str[i] = fgetc(f);
	}
	read_str[len] = '\0';  // adding the termination char

	// If the read string is equal to the correct string, try to
	// find the correct assignment format --> attribute = value
	if (strcmp(correct_str, read_str) == 0) {
		skipc(f, ' ');
		char c = fgetc(f);

		if (c == '=') {
			skipc(f, ' ');
			c = fgetc(f);

			// Read until new line is found or an error occurs
			int i = 0;
			while (c != '\n' && r_value != -1) {

				// If the read character is the ASCII representation of
				// a digit between 0 and 9, concatenate this digit with
				// the previous value
				if (c >= 48 && c <= 57 && !space_found) {
					value[i] = c;
					i++;
				}
				else if (c == ' ' || c == '\t') {
					space_found = TRUE;
				}
				else if (c != ' ' && c != '\t') {
					r_value = -1;
				}

				c = fgetc(f);
			}
			value[i] = '\0'; // adding the termination char

			// If the loop hasn't ended because of "r_value",
			// set the return value equal to the integer
			// representation of "value" && the read value is
			// representable as an integer
			if (r_value != -1 && !check_overflow(value)) {
				r_value = atoi(value);
			}
      else {  // there was overflow
        r_value = -1;
      }
		}
		else {  // there was no = after the desired option
			r_value = -1;
		}
	}
	else { // the input didn't match the desired string
		r_value = -1;
	}

	return r_value;
}

BOOL set_opt(BOOL* set_flag, int* key, int value) {
	// the input value isn't acceptable or was already set
	if (value == -1 || (value != -1 && *set_flag)) {
		return FALSE;
	}
	else {
		// setting info
		*key = value;
		*set_flag = TRUE;
	}

	return TRUE;
}

BOOL set_group_perc(FILE* f, BOOL* g_flag, int* g, char* check_str) {
	fseek(f, ftell(f) - 1, SEEK_SET);  // setting 1 back to start checking
	int r = check(f, check_str, 2);  // getting option value

	if (!set_opt(g_flag, g, r)) {  // if there was an error setting the value
		return FALSE;
		fprintf(stderr, "[READER] Invalid %s value.\n", check_str);
	}

	return TRUE;
}

int scan(char* path) {
	// opening config file
	FILE* f = fopen(path, "r");

	// last read char from file
	char peek;
	// file valid flag
	BOOL is_valid = TRUE;

	while ((peek = fgetc(f)) != EOF && is_valid) {
		// Skip all "blank characters"
		while (peek == ' ' || peek == '\t' || peek == '\n' || peek == '\r') {
			peek = fgetc(f);
		}

		switch (peek) {
		// If read string starts with '#', It represents
		// a comment (skip all characters until '\n')
		case '#':
			while (fgetc(f) != '\n');
			break;

		// If read string starts with 'G', It represents the start of a group id
		case 'G':
			peek = fgetc(f);
			if (peek == '2') {
				is_valid = set_group_perc(f, &g2_set, &g2, "G2");
			}
			else if (peek == '3') {
				is_valid = set_group_perc(f, &g3_set, &g3, "G3");
			}
			else if (peek == '4') {
				is_valid = set_group_perc(f, &g4_set, &g4, "G4");
			}
			else {
				is_valid = FALSE;
				fprintf(stderr, "[READER] Invalid option.\n");
			}
			break;

		case 'M':
			// If the line starts with 'M', check if the entire string
			// is equal to "MAX_REJECTS" and if It is well formatted
			if (!set_opt(&max_rejects_set, &max_rejects, check(f, "MAX_REJECTS", 11))) {
				is_valid = FALSE;
				fprintf(stderr, "[READER] Invalid MAX_REJECTS value.\n");
			}
			break;

		case 'N':
			// If the line starts with 'N', check if the entire string
			// is equal to "NOF_INVITES" and if It is well formatted
			if (!set_opt(&nof_invites_set, &nof_invites, check(f, "NOF_INVITES", 11))) {
				is_valid = FALSE;
				fprintf(stderr, "[READER] Invalid NOF_INVITES value.\n");
			}
			break;

		case 'P':
			// If the line starts with 'P', check if the entire string
			// is equal to "POP_SIZE" and if It is well formatted
			if (!set_opt(&pop_size_set, &pop_size, check(f, "POP_SIZE", 8))) {
				is_valid = FALSE;
				fprintf(stderr, "[READER] Invalid POP_SIZE value.\n");
			}
			break;

		case 'S':
			// If the line starts with 'S', check if the entire string
			// is equal to "SIM_TIME" and it It is well formatted
			if (!set_opt(&sim_time_set, &sim_time, check(f, "SIM_TIME", 8))) {
				is_valid = FALSE;
				fprintf(stderr, "[READER] Invalid SIM_TIME value.\n");
			}
			break;

		default:
			is_valid = FALSE;
			fprintf(stderr, "[READER] Invalid option.\n");
			break;
		}
	}

	fclose(f);

	// If the file is well formatted but the sum of all
	// the percentages is not equal to 100, the file is invalid
	int sum = g2 + g3 + g4;
	if (is_valid &&
	    (sum != 100 || !sim_time_set || !pop_size_set ||
	     !nof_invites_set || !max_rejects_set || !g2_set ||
	     !g3_set || !g4_set)) {
		is_valid = FALSE;
		fprintf(stderr, "[READER] ERROR: Constraints not respected.\n");
	}

	printf("[READER] Reading Done.\n");
	return is_valid;
}
