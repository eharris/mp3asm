/*  mp3asm: an mp3 frameeditor.
 *
 *  tag.c: reading/writing tags
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
#include "mp3asm.h"
#include "stream.h"
#include "utils.h"
#include "frame.h"


/* utils.c */
extern void *tmalloc (size_t size);
extern void print_log (int verb);
extern void print_all (int verb);
extern int read_buf (buffer_t *buffer, unsigned char *data, int offset, int count);
extern int write_buf (unsigned char *data, buffer_t *buffer, int count);

/*
 * reads the 128 byte long id3 v1 from the end of the buffer
 *
 */
int 
get_tag_v1 (stream_t *stream, buffer_t *buffer)
{
  if (!stream->tag && (buffer->used >= 128))
    {
      int temp = buffer->used - 128;
      char string[4];

      while (temp)
	{
	  read_buf(buffer, string, temp, 3);
	  string[3] = 0x00;

	  if (!strcmp ("TAG", string))
	    {
	      stream->tag = tmalloc (129 * sizeof (unsigned char));
	      read_buf(buffer, stream->tag, temp, 128);
	      stream->tag[128] = 0x00;
	      return (1);
	    }
	  temp--;
	}
    }
  return (0);
}

/*
 * writes the 128 byte long id3 v1 tag to the end of the buffer
 *
 */
int 
write_tag_v1 (stream_t *stream, FILE *file)
{
  if (stream->tag && (strlen (stream->tag) == 128))
    {
      if (fwrite(stream->tag, sizeof(unsigned char), 128, file) < 0)
	{
	  sprintf (log.buf, "%s: Error writing tag to file.\n", me);
	  print_all (-1);
	  perror ("fwrite");
	  return (-1);
	}
      return (1);
    }
  return (0);
}

/* EOF */
