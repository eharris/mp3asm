/*  mp3asm: an mp3 frameeditor.
 *
 *  stream.c: readin, writing & sorting of streams
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

#ifndef HAVE_STREAM_H
#define HAVE_STREAM_H

typedef struct stream_t
{
  int maj_version;  /* 1, 2 */
  int min_version;  /* 5 -> 2.5 */
  int layer; /* 1, 2, 3 */
  int samples; /* per frame */
  int isize; /* side info size */
  int cbr; /* boolean */
  float avkbps;
  int freq;
  int mode; /* 0 = stereo | 1 = joint stereo | 2 = dual chan | 3 = mono */
  int crc;  
  int private;
  int copyright;
  int original;
  unsigned char *tag;
  
  long count;

  struct frame_t *first; /* pointer to first frame of the stream */
  struct frame_t *last;
} stream_t;

#endif /* HAVE_STREAM_H */

/* EOF */
