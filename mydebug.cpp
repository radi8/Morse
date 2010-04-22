#define DEBUGLVL 0
#include "mydebug.h"

#include <qglobal.h>
#include <stdio.h>		// for snprintf
#include <stdlib.h>		// for abort
#include <stdarg.h>

#ifdef USE_SYSLOG
#include <syslog.h>		// for syslog
#endif


static void MessageHandler(QtMsgType type, const char *msg)
{
	const char *t = "Unknown: ";
#ifdef USE_SYSLOG
	int level = LOG_DEBUG;
#endif

	switch (type) {
	case QtDebugMsg:
		t = "";
		break;
	case QtWarningMsg:
		t = "Warnung: ";
#ifdef USE_SYSLOG
		level = LOG_WARNING;
#endif
		break;
	case QtCriticalMsg:
		t = "Fehler: ";
		break;
	case QtFatalMsg:
		t = "Fatal: ";
#ifdef USE_SYSLOG
		level = LOG_CRIT;
#endif
		break;
	}

	// Send to syslog
#ifdef USE_SYSLOG
	openlog("cradler", 0, LOG_USER);
	syslog(level, msg);
	closelog();
#endif

	// Send to stdout
	fputs(t, stdout);
	puts(msg);
	fflush(stdout);

	if (type == QtFatalMsg)
		abort();
}


void myDebug(const char *fmt, ...)
{
	char buf[256];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	MessageHandler(QtDebugMsg, buf);
	va_end(ap);
}


void dump(const unsigned char *p, int len, bool with_addr)
{
	int i, thisline;
	int offset = 0;

	while (offset < len) {
		if (with_addr)
			printf("%04x ", offset);
		thisline = len - offset;
		if (thisline > 16) {
			thisline = 16;
		}
		for (i = 0; i < thisline; i++) {
			printf("%02x ", p[i]);
		}
		for (; i < 16; i++) {
			printf("   ");
		}
		for (i = 0; i < thisline; i++) {
			printf("%c", (p[i] >= 0x20 && p[i] < 0x7f) ? p[i] : '.');
		}
		printf("\n");
		offset += thisline;
		p += thisline;
	}
	fflush(stdout);
}


static void init_debug(void) __attribute__((__constructor__));

static void init_debug(void)
{
	qInstallMsgHandler(MessageHandler);
}
