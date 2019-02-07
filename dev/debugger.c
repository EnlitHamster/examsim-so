#include <stdlib.h>
#include <string.h>
#include "debugger.h"

#ifdef DEBUG_SEVERE

void dbg_svr(FILE* stream, char* format, ...) {
	va_list args;
	va_start(args, format);	// Generating arguments array
	vfprintf(stream, format, args);	// Printing on stream
	va_end(args);
}
#else
void dbg_svr(void* a, ...) {
}
#endif

#ifdef DEBUG
void dbg(FILE* stream, char* format, ...) {
	va_list args;
	va_start(args, format);	// Generating arguments array
	vfprintf(stream, format, args);	// Printing on stream
	va_end(args);
}
#else
void dbg(void* a, ...) {
}
#endif
