/* -*- c-file-style: "linux" -*-
 *
 * Copyright (C) 2001 by Martin Pool <mbp@samba.org>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/**
 * \section tls
 *
 * tls -- Trivial recursive ls, for comparing two directories after
 * running an rsync.
 *
 * The problem with using the system's own ls is that some features
 * have little quirks that make directories look different when for
 * our purposes they're the same -- for example, the BSD braindamage
 * about setting the mode on symlinks based on your current umask.
 *
 * All the filenames must be given on the command line -- tls does not
 * even read directories, let alone recurse.  The typical usage is
 * "find|sort|xargs tls".
 *
 * The format is not exactly the same as any particular Unix ls(1).
 *
 * A key requirement for this program is that the output be "very
 * reproducible."  So we mask away information that can accidentally
 * change.
 **/



#include "rsync.h"

#define PROGRAM "tls"

/* These are to make syscall.o shut up. */
int dry_run = 0;
int read_only = 1;
int list_only = 0;


static void failed (char const *what,
		    char const *where)
{
	fprintf (stderr, PROGRAM ": %s %s: %s\n",
		 what, where, strerror (errno));
	exit (1);
}



static void list_file (const char *fname)
{
	struct stat buf;
	char permbuf[PERMSTRING_SIZE];
	struct tm *mt;

	if (do_lstat(fname, &buf) == -1)
		failed ("stat", fname);

	/* On some BSD platforms the mode bits of a symlink are
	 * undefined.  */
	if (S_ISLNK(buf.st_mode))
		buf.st_mode &= ~0777;
		
	/* The size of anything but a regular file is probably not
	 * worth thinking about. */
	if (!S_ISREG(buf.st_mode))
		buf.st_size = 0;

	permstring(permbuf, buf.st_mode);

	mt = gmtime(&buf.st_mtime);

	/* TODO: Perhaps escape special characters in fname? */

	/* NB: need to pass size as a double because it might be be
	 * too large for a long. */
	printf("%s %12.0f %6d.%-6d %04d-%02d-%02d %02d:%02d:%02d %s\n",
	       permbuf, (double) buf.st_size,
	       buf.st_uid, buf.st_gid,
	       mt->tm_year + 1900,
	       mt->tm_mon + 1,
	       mt->tm_mday,
	       mt->tm_hour,
	       mt->tm_min,
	       mt->tm_sec,
	       fname);
}


int main (int argc, char *argv[])
{
	if (argc < 2) {
		fprintf (stderr, "usage: " PROGRAM " DIR ...\n"
			 "Trivial file listing program for portably checking rsync\n");
		return 1;
	}

	for (argv++; *argv; argv++) {
		list_file (*argv);
	}

	return 0;
}
