// SPDX-FileCopyrightText: 2021 KUNBUS GmbH <support@kunbus.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef REVPI_SERIAL_H
#define REVPI_SERIAL_H

#include <stdarg.h>
#include <stdio.h>

#if (!defined(NDEBUG))
# define DEBUG 1
#else
# define DEBUG 0
#endif

typedef enum error_level {
	LEVEL_DEBUG,
	LEVEL_WARN,
	LEVEL_ERROR
} error_level_t;

static void log_print(const error_level_t level, const char *fmt, ...)
{
	const char *lstr;
	va_list args;
	switch (level)
	{
	case LEVEL_DEBUG:
		lstr = "DEBUG: ";
		break;
	case LEVEL_WARN:
		lstr = "WARNING: ";
		break;
	case LEVEL_ERROR:
		lstr = "ERROR: ";
		break;
	default:
		lstr = "UNKNOWN: ";
		break;
	}

	fflush(stdout);
	flockfile(stderr);
	fputs(lstr, stderr);
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fflush(stderr);
	funlockfile(stderr);
}

#define dbg_print(...) \
	do { if (DEBUG) log_print(LEVEL_DEBUG, ##__VA_ARGS__); } while (0)

#define warn_print(...) \
	log_print(LEVEL_WARN, ##__VA_ARGS__)


#define err_print(...) \
	log_print(LEVEL_ERROR, ##__VA_ARGS__)

#endif /* INCLUDE GARD */
