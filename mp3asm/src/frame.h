/*  mp3asm: an mp3 frameeditor.
 *
 *  frame.h : how about those frames huh.
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

#ifndef HAVE_FRAME_H
#define HAVE_FRAME_H

  typedef struct header_t
  {
    unsigned char head[4];
    int pos;
    int count;
  } header_t;

typedef struct frame_t
{
  struct frame_t *prev;
  struct frame_t *next;

  unsigned char *head;      /* frame header */
  unsigned char *info;      /* side info, max. 34 bytes */
  unsigned char *data;      /* frame data */

  int hsize;         /* space between two headers */
  int dsize;         /* size of frame data (deinterleaved) */
  int padded;       
  int backref;       /* layer 3 only*/
  int kbps;
} frame_t;

#endif /* HAVE_FRAME_H */

/* EOF */
