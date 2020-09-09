/*  mp3asm: an mp3 frameeditor.
 *
 *  parse.h : parses the command line input.
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


#ifndef HAVE_PARSE_H
#define HAVE_PARSE_H
#include "stream.h"

typedef struct input_t
{
  char *name; /* name of the input file */
  FILE *file;
  
  long startframe;
  long readframes;
  long endframe;
  int use_id3;
  stream_t *stream;
} input_t;

extern input_t **input;

typedef struct output_t
{
  void *stream;
  char *name; 
  FILE *file;
  
  int write_crc;
} output_t;

extern output_t *output;

#endif /* HAVE_PARSE_H */

/* EOF */ 
