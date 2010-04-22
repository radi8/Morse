#ifndef _MYDEBUG_H_
#define _MYDEBUG_H_

#define USE_SYSLOG

void myDebug(const char* fmt, ...) __attribute__((format (printf, 1, 2)));

#ifndef DEBUGLVL
#define DEBUGLVL 0
#endif

#if DEBUGLVL
#  define MYDEBUG myDebug
#else
#  define MYDEBUG 1 ? (void)0 : myDebug
#endif
#if DEBUGLVL > 1
#  define MYTRACE myDebug
#else
#  define MYTRACE 1 ? (void)0 : myDebug
#endif
#if DEBUGLVL > 2
#  define MYVERBOSE myDebug
#else
#  define MYVERBOSE 1 ? (void)0 : myDebug
#endif


void dump(const unsigned char *data, int len, bool with_addr=false);


#endif
