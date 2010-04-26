#ifndef _MYDEBUG_H_
#define _MYDEBUG_H_

/**
 * @file
 * @author Holger Schurig
 *
 * @section DESCRIPTION
 *
 * Define DEBUGLVL to 0, 1, 2 or 3 and then include this file.
 *
 * Depending on the debug level, the following macros are disabled or
 * re-routed to myDebug():
 * - 0: all debug is off (this is the default)
 * - 1: enable MYDEBUG()
 * - 2: enable MYDEBUG() and MYTRACE()
 * - 3: enable MYDEBUG(), MYTRACE() and MYVERBOSE()
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 */


/*!
 * Enable syslog() logging
 *
 * Global define. If defined, then MYDEBUG(), MYTRACE() and MYVERBOSE() will
 * emit their output (via myDebug()) also to syslog.
 */
#define USE_SYSLOG


/*!
 * Internal debug formatter.
 *
 * Function to emit the actual text, normally only used by MYDEBUG(),
 * MYVERBOSE() and MYTRACE(). The formatted debug output will be sent to
 * Qt's MessageHandler.
 *
 * @param fmt     printf-like format string
 * @param ...     arguments
 */
void myDebug(const char* fmt, ...) __attribute__((format (printf, 1, 2)));


#ifndef DEBUGLVL
#define DEBUGLVL 0
#endif

/*!
 * Use instead printf() for debug output.
 */
#if DEBUGLVL
#  define MYDEBUG myDebug
#else
#  define MYDEBUG 1 ? (void)0 : myDebug
#endif

/*!
 * Use instead printf() for debug output. Most often used at the beginning
 * of functions or members.
 */
#if DEBUGLVL > 1
#  define MYTRACE myDebug
#else
#  define MYTRACE 1 ? (void)0 : myDebug
#endif

/*!
 * Use instead printf() for debug output. Most ofte used for really chatty output.
 */
#if DEBUGLVL > 2
#  define MYVERBOSE myDebug
#else
#  define MYVERBOSE 1 ? (void)0 : myDebug
#endif


/*!
 * Hex dump function
 *
 * Create hex dump of same variable contents
 *
 * @param data       address of region to dump
 * @param len        length (in bytes) to dump
 * @param with_addr  if true, then prepend each line with an address. The address
 *                   starts at offset 0. If false, just do the plain hex dump.
 */
void dump(const void *data, int len, bool with_addr=false);


#endif
