/*  mp3asm: an mp3 frameeditor.
 *
 *  mp3asm.h : main includes file.
 *
 *  Copyright (C) 2001  Luc Verhaegen (_Death_@Undernet(IRC))
 *                                    <dw_death_@hotmail.com>
 *  Copyright (C) 1996-1997 Olli Fromme <olli@fromme.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* DEBUG STUFF - Enables extra output */

#ifndef HAVE_MP3ASM_H
#define HAVE_MP3ASM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <argz.h>
#include <ctype.h>

#define LOGBUFSIZE 4096

typedef struct logfile_t
{
  char buf[4096];
  char *name; /* name of the file to write to */
  FILE *file;
} logfile_t;

extern int verbosity;
extern int quiet;
extern int info; /* if set to 1 -> print header info on all files and exit */
extern char *me; /* name of the executable */
extern logfile_t log; /* file to log to */
extern int inputs;

#endif /* HAVE_MP3ASM_H */

/* EOF */

