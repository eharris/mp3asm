/*  mp3asm: an mp3 frameeditor.
 *
 *  utils.c : all sorts of handy little functions.
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
#include "utils.h"
#include <unistd.h>
#include <stdio.h>

/*
 * tmalloc: mallocs cleanly
 *
 */
void
*tmalloc (size_t size)
{
  void *mem;
  
  if (!(mem = malloc(size)))
    {
      fprintf(stderr, "Out of memory.");
      exit(ENOMEM);
    }
  return (mem);
}

/*
 * tmalloc: mallocs cleanly
 *
 */
void
*tcalloc (int n, size_t size)
{
  void *mem;
  
  if (!(mem = calloc(n, size)))
    {
      fprintf(stderr, "Out of memory.");
      exit(ENOMEM);
    }
  return (mem);
}

/*
 * trealloc: reallocs cleanly.
 *
 */
void *
trealloc (void *ptr, size_t size)
{
  void *mem;
  if (!(mem = realloc(ptr, size)))
    {
      fprintf(stderr, "Out of memory.");
      exit(ENOMEM);
    }
  return (mem);
}

/*
 * print_std: prints log.buf to stderr
 *               takes min verbosity needed to show
 */
void
print_std (int verb)
{
  if ((verb < 0) || ((verb <= verbosity) && !quiet))
    fprintf (stderr, "%s", log.buf);
  return;
}

/*
 * print_log: prints log.buf to stderr
 *             takes min verbosity needed to show
 */
void
print_log (int verb)
{
  if ((verb <= verbosity) && log.file && log.file)
    fprintf (log.file, "%s", log.buf);
  return;
}

/*
 * print_all: prints log.buf to stderr & file
 *            takes min verbosity needed to show
 */
void
print_all (int verb)
{
  print_std (verb);
  print_log (verb);
  return;
}

/*
 * mp3ropen: opens the mp3 file for read, returns int/-1
 *
 */
FILE *
mp3ropen(char *name) /* infile */
{
  FILE *file = fopen (name, "r");

  if (!file)
    {
      sprintf(log.buf, "%s: Unable to read from %s.\n", me, name);
      print_all (-1);
      exit (EX_NOINPUT);
    }
  return (file);
}

/*
 * mp3wopen: opens the mp3 file for write, returns *file/-1
 *           if exists then it takes another filename
 *
 */
FILE *
mp3wopen(char **name, int layer)
{
  FILE *file;
  char *basename = NULL, *name2 = strcpy (tmalloc ((10 + strlen (*name)) * sizeof (char)), *name);
  char *count = strcpy (tmalloc (3 * sizeof (char)), "000");
  char *extension = (layer == 3) ? ".mp3" : ((layer == 2) ? ".mp2" : ".mp1") ;

  while (!access (name2, F_OK))
    {
      sprintf (log.buf, "File %s already exists.\n", name2);
      print_all (0);
      
      if (!basename)
	{
	  char *po = name2 + strlen (*name) - 4;
	  if (strncasecmp (po, ".mp", 3))
	    {
	      sprintf (log.buf, "%s: Unable to write to %s, this is not an mp1/2/3 file\n", me, name2);
	      print_all (-1);
	      exit (EX_CANTCREAT);
	    }
	  *po = 0;
	  basename = strcpy (tmalloc (strlen (name2) + 1), name2);
	}
      if (count[2] == '9') /* I think 1000 files will sufise */
	{
	  if (count[1] == '9')
	    {
	      count[0]++;
	      count[1] = '0';
	    }
	  else
	    count[1]++;
	  count[2] = '0';
	}
      else
	count[2]++;

      strcpy (name2, basename);
      strcat (name2 + strlen (name2), count);
      strcat (name2 + strlen (name2), extension);
    }

  free (basename);
  file = fopen (name2, "wx");
  if (!file)
    {
      sprintf (log.buf, "%s: Unable to write to %s\n", me, name2);
      print_all (-1);
      exit (EX_CANTCREAT);
    }
  free (*name);  /* change filename, we wont want it printin the wrong inf */
  name = &name2;

  sprintf (log.buf, "Writing to %s\n", *name);
  print_all (0);

  return (file);
}

/*
 * logopen: opens the logfile.
 *
 */

int
logopen(void) /* for appending only, logfile... */
{
  if(!log.name)
    return (1);       /* other functions will not use it either then */

  log.file = fopen(log.name, "w");
  if (!log.file)
    {
      sprintf(log.buf, "Unable to write to %s, not logging output\n", log.name);
      print_std (-1);
      log.name = NULL;
      return (0);
    }
  return(1);
}


/*
 * Buffer section
 *
 */
/*
#ifndef DEBUG_BUFFER
#define DEBUG_BUFFER
#endif*/

/*
 * init_buf: initialises a buffer.
 *
 */

buffer_t *
init_buf (int size)
{
  buffer_t *buffer;

  buffer = (buffer_t *)malloc(sizeof(buffer_t));
  buffer->data = (unsigned char *)malloc(size * sizeof(unsigned char));

  buffer->size = size;
  buffer->used = 0;
  buffer->begin = 0;
  buffer->end = 0;
  buffer->eof = 0;
#ifdef DEBUG_BUFFER
  sprintf(log.buf, "BUFFER: initialised buffer\nBUFFER: begin: %d end: %d used: %d size: %d\n", buffer->begin, buffer->end, buffer->used, buffer->size);
  print_log (10);
#endif /* DEBUG_BUFFER */

  return (buffer);
}

/*
 * free_buf: handles deallocation of a buffer.
 *
 */

void
free_buf(buffer_t *buffer)
{
  if (buffer->used)
    {
      sprintf(log.buf, "Warning: Emptying a not completely empty buffer (%p,%d/%d)\n", buffer, buffer->used, buffer->size);
      print_all (0);
    }
  free(buffer->data);
  free(buffer);
}

/*
 * fill_buf_from_file: (re)fills the buffer from file
 *                     returns -1 on error, and 0 on eof, 1 on succes
 *
 */

int
fill_buf_from_file (buffer_t *buffer, FILE *file)
{
  int readb, count, count1;
  unsigned char *ptr;
  
  count = (buffer->size - buffer->used);

  while (count)
    {
      count1 = ((buffer->end + count) > buffer->size) ? (buffer->size - buffer->end) : count;
      ptr = (unsigned char *)(buffer->data + buffer->end);
      
      while (count1)
	{
	  if (feof(file))
	    {
	      buffer->eof++;
	      return (0);
	    }
	  if ((readb = fread(ptr, sizeof(char), count1, file)) < 0)
	    {
	      if (errno == EINTR)
		continue;
	      else
		{
		  sprintf (log.buf, "%s: Error reading from file to buffer.\n", me);
		  print_all (-1);
		  perror ("read()");
		  return (-1);
		}
	    }
#ifdef DEBUG_BUFFER
	  sprintf (log.buf, "BUFFER: Reading from file to buffer: %d \n", readb);
	  print_log (10);
#endif /* DEBUG_BUFFER */

	  count1 -= readb;
	  count -= readb;
	  ptr += readb;
	  buffer->end += readb;
	  buffer->used += readb;
	}
      if (buffer->end == buffer->size) 
	buffer->end = 0;
    }

#ifdef DEBUG_BUFFER
  sprintf(log.buf, "BUFFER: begin: %d end: %d used: %d size: %d\n", buffer->begin, buffer->end, buffer->used, buffer->size);
  print_log (10);
#endif /* DEBUG_BUFFER */
  
  return (1);
}

/*
 * read_buf: reads from a buffer into a pointer.
 *           returns 0 on eob, 1 success, -1 read error.
 */

int
read_buf (buffer_t *buffer, unsigned char *data, int offset, int count)
{
  int location, count1;
  unsigned char  *ptr;

  if ((offset + count) > (buffer->used))
    {
      sprintf(log.buf, "%s: Request for more bytes (%d + %d) than there are in buffer (%d/%d).\n", me, count, offset, buffer->used, buffer->size);
      print_log (10);
      return (0);
    }
  location = buffer->begin + offset;
  while (location > buffer->size)
    location -= buffer->size;
  
  while (count)
    {
      count1 = ((location + count) > buffer->size) ? (buffer->size - location) : count;
      ptr =(unsigned char *)(buffer->data + location);
      
#ifdef DEBUG_BUFFER
      sprintf (log.buf, "BUFFER: Reading %d from buffer (@%d from %d/%d)\n", count1, location, buffer->used, buffer->size);
      print_log (10);
#endif /* DEBUG_BUFFER */  
      
      if (data != memcpy(data , ptr, count1 * sizeof(unsigned char)))
	{
	  sprintf (log.buf, "%s: Error reading from buffer.\n", me);
	  print_all (-1);
	  return (-1);
	}
      
      count -= count1;
      data += count1;
      location += count1;
      if (location == buffer->size) 
	location = 0;
    }    

#ifdef DEBUG_BUFFER
  sprintf(log.buf, "BUFFER: begin: %d end: %d used: %d size: %d\n", buffer->begin, buffer->end, buffer->used, buffer->size);
  print_log (10);
#endif /* DEBUG_BUFFER */
  return (1);
}

/*
 * free_buf: frees bytes from the buffer.
 *
 */

int
rem_buf (buffer_t *buffer, int count)
{
  if (count)
    {
      if (count > buffer->used)
	{
#ifdef DEBUG_BUFFER
	  sprintf(log.buf, "%s: Cant free bytes that rnt in the buffer.\n", me);
	  print_log (10);
#endif /* DEBUG_BUFFER */
	  return (0);
	}
      
      if ((buffer->begin + count) > buffer->size)
	buffer->begin = count - buffer->size + buffer->begin;
      else 
	buffer->begin += count;
      
      buffer->used -= count;
    }
#ifdef DEBUG_BUFFER
  sprintf(log.buf, "BUFFER: freed %d from buffer (%d)\nBUFFER: begin: %d end: %d used: %d size: %d\n", count, buffer->size, buffer->begin, buffer->end, buffer->used, buffer->size);
  print_log (10);
#endif /* DEBUG_BUFFER */
  
  return (1);
}

/*
 * cut_buf: cuts 'count' bytes from one buffer and appends them to the other.
 *          returns -1 if the 2nd buf is too full, 0 when first is empty
 */

int
cut_buf (buffer_t *buffera, buffer_t *bufferb, int count)
{
  int count1;

  if (count > buffera->used)
    {
#ifdef DEBUG_BUFFER
      sprintf(log.buf, "%s: Request for more bytes (%d) than there are in buffer A (%d/%d).\n", me, count, buffera->used, buffera->size);
      print_log (10);
#endif /* DEBUG_BUFFER */
      return (0);
    }    
  if (count > (bufferb->size - bufferb->used))
    {
      sprintf(log.buf, "%s: Request for buffer overload received (%d + %d > %d\n", me, count, bufferb->used, bufferb->size);
      print_all (-1);
      return (-1);
    }
  
  while (count)
    {
      count1 = count;
      
      if ((buffera->begin + count) > buffera->size)
	count1 = buffera->size - buffera->begin;
      
      if (((bufferb->end + count1) > bufferb->size) && ((bufferb->size - bufferb->end) < count1))
	count1 = bufferb->size - bufferb->end;

#ifdef DEBUG_BUFFER      
      sprintf (log.buf, "BUFFER: Reading %d from buffer (@%d from %d/%d) to buffer (@%d from %d/%d) \n", count1, buffera->begin, buffera->used, buffera->size, bufferb->end, bufferb->used, bufferb->size);
      print_log (10);
#endif /* DEBUG_BUFFER */      
      
      memcpy(bufferb->data + bufferb->end, buffera->data + buffera->begin, count1 * sizeof(unsigned char));
      
      count -= count1;
      
      buffera->begin += count1;
      if (buffera->begin == buffera->size) 
	buffera->begin = 0;
      buffera->used -= count1;
      
      bufferb->end += count1;
      if ( bufferb->end == bufferb->size) 
	bufferb->end = 0;
      bufferb->used += count1;
    }    
  
#ifdef DEBUG_BUFFER
  sprintf(log.buf, "BUFFER: 1) begin: %d end: %d used: %d size: %d\n", buffera->begin, buffera->end, buffera->used, buffera->size);
  print_log (10);
  sprintf(log.buf, "BUFFER: 2) begin: %d end: %d used: %d size: %d\n", bufferb->begin, bufferb->end, bufferb->used, bufferb->size);
  print_log (10);
#endif /* DEBUG_BUFFER */
  
  return (1);
}

/*
 * write_buf: writes count from data to buffer, returns -1 on full buffer.
 *
 */
int
write_buf (unsigned char *data, buffer_t *buffer, int count)
{
  int count1, temp = 0;

  if (count > (buffer->size - buffer->used))
    {
      sprintf(log.buf, "%s: Request for buffer overload received (%d + %d > %d\n", me, count, buffer->used, buffer->size);
      print_all (-1);
      return (-1);
    }
  
  while (count)
    {
      count1 = count;
      
      if ((buffer->size - buffer->end) < count1)
	count1 = buffer->size - buffer->end;

#ifdef DEBUG_BUFFER      
      sprintf (log.buf, "BUFFER: Reading %d from data (%p) to buffer (@%d from %d/%d) \n", count1, data, buffer->end, buffer->used, buffer->size);
      print_log (10);
#endif /* DEBUG_BUFFER */      
      
      memcpy (buffer->data + buffer->end, data + temp, count1 * sizeof(unsigned char));
      
      count -= count1;
      temp += count1;
      buffer->end += count1;
      if (buffer->end == buffer->size) 
	buffer->end = 0;
      buffer->used += count1;
    }    
  
#ifdef DEBUG_BUFFER
  sprintf(log.buf, "BUFFER: 2) begin: %d end: %d used: %d size: %d\n", buffer->begin, buffer->end, buffer->used, buffer->size);
  print_log (10);
#endif /* DEBUG_BUFFER */

  
  return (1);
}

/*
 * write_file_from_buf: (re)fills the buffer from file
 *                     returns -1 on error, and 0 on eof, 1 on succes
 *
 */
int
print_buf (buffer_t *buffer, int count)
{
  int count1;
  unsigned char *ptr;

  ptr = buffer->data + buffer->begin;
  count1 = ((buffer->begin + count) > buffer->size) ? (buffer->size - buffer->begin) : count;

  while (count)
    {
      while (count1)
	{
	  fprintf (log.file, "%x", *ptr);
	  count1 --;
	  count --;
	  ptr ++;
	}
      if (ptr == (buffer->data + buffer->size))
	  ptr = buffer->data;
      count1 = count;
    }
  fprintf (log.file, "\n");
  
  return (1);
}

/*
 * write_file_from_buf: (re)fills the buffer from file
 *                     returns -1 on error, and 0 on eof, 1 on succes
 *
 */
int
write_file_from_buf (buffer_t *buffer, FILE *file, int count)
{
  int writeb, count1;
  unsigned char *ptr;
  
  while (count)
    {
      count1 = ((buffer->begin + count) > buffer->size) ? (buffer->size - buffer->begin) : count;
      
      ptr = (unsigned char *)(buffer->data + buffer->begin);
      
      while (count1)
	{
	  if ((writeb = fwrite(ptr, sizeof(unsigned char), count1, file)) < 0)
	    {
	      if (errno == EINTR)
		continue;
	      else
		{
		  sprintf (log.buf, "%s: Error writing from buffer to file.\n", me);
		  print_all (-1);
		  perror ("fwrite");
		  return (-1);
		}
	    }
#ifdef DEBUG_BUFFER
	  sprintf (log.buf, "BUFFER: writing to file from buffer: %d \n", writeb);
	  print_log (10);
#endif /* DEBUG_BUFFER */

	  count1 -= writeb;
	  count -= writeb;
	  ptr += writeb;
	  buffer->begin += writeb;
	  buffer->used -= writeb;
	}
      if (buffer->begin == buffer->size) 
	buffer->begin = 0;
    }

#ifdef DEBUG_BUFFER
  sprintf(log.buf, "BUFFER: begin: %d end: %d used: %d size: %d\n", buffer->begin, buffer->end, buffer->used, buffer->size);
  print_log (10);
#endif /* DEBUG_BUFFER */
  
  return (1);
}

/*
 * write_file_from_buf: (re)fills the buffer from file
 *                     returns -1 on error, and 0 on eof, 1 on succes
 *
 */
int
print_data (unsigned char *data, int count)
{
  unsigned char *ptr;

  ptr = data;

  while (count)
    {
      fprintf (log.file, "%x", *ptr);
      count --;
      ptr ++;
    }
  fprintf (log.file, "\n");
  
  return (1);
}

/* EOF */
