#ifndef _DEBUGGER_H_
#define _DEBUGGER_H_

#include <stdarg.h>
#include <stdio.h>

#if defined(DEBUG) && !defined(DEBUG_SEVERE)
#define DEBUG_SEVERE
#endif

#ifdef DEBUG_SEVERE
// Outputs a debug info of severe level on the designeted
// stream.
extern void dbg_svr(FILE*, char*, ...);
#else
extern void dbg_svr(void*, ...);
#endif

#ifdef DEBUG
// Outpus a debug info of default level on the designeted
// stream.
extern void dbg(FILE*, char*, ...);
#else
extern void dbg(void*, ...);
#endif

#endif
