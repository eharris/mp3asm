/*  mp3asm: an mp3 frameeditor.
 *
 *  frame.c : how about those frames huh.
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
#include "frame.h"
#include "stream.h"

/*
#ifndef FRAME_DEBUG
#define FRAME_DEBUG
#endif
#ifndef SIDE_DEBUG
#define SIDE_DEBUG
#endif
#ifndef POINTER_DEBUG
#define POINTER_DEBUG
#endif
#ifndef WRITE_DEBUG
#define WRITE_DEBUG
#endif*/

#define FRAMEBUF_SIZE 5

/* utils.c */
extern void *tmalloc (size_t size);
extern void *tcalloc (int n, size_t size);
extern void *trealloc (void *ptr, size_t size);
extern void print_log (int verb);
extern void print_all (int verb);
extern buffer_t *init_buf (int size);
extern int read_buf (buffer_t *buffer, unsigned char *data, int offset, int count);
extern int rem_buf (buffer_t *buffer, int count);
extern int cut_buf (buffer_t *buffera, buffer_t *bufferb, int count);
extern int write_buf (unsigned char *data, buffer_t *buffer, int count);
extern int write_file_from_buf (buffer_t *buffer, FILE *file, int count);
extern int print_buf (buffer_t *buffer, int count);
extern int print_data (unsigned char *data, int count);

/* tag.c */
extern int write_tag_v1 (stream_t *stream, FILE *file);

/*
 * isheader: 
 *
 */
static int
isheader (unsigned char head[4])
{
  if ((head[0] == 0xff) && ((head[1] & 0xe0) != 0xe0))
    { 
      sprintf(log.buf, "%s: No syncbits found at the start of header %x%x%x%x.\n", me, head[0], head[1], head[2], head[3]);
      print_log (10);
      return (0);
    }
  if ((head[1] & 0x18) == 0x08)
    {
      sprintf(log.buf, "%s: Bad mpeg audio version specified in %x%x%x%x.\n", me, head[0], head[1], head[2], head[3]);
      print_log (10);
      return (0);
    }
  if (!(head[1] & 0x06))
    {
      sprintf(log.buf, "%s: Bad mpeg layer specified in %x%x%x%x.\n", me, head[0], head[1], head[2], head[3]);
      print_log (10);
      return (0);
    }
  if ((head[2] & 0xf0) == 0xf0)
    {
      sprintf(log.buf, "%s: No valid bitrate specified in %x%x%x%x.\n", me, head[0], head[1], head[2], head[3]);
      print_log (10);
      return (0); 
    }
  if ((head[2] & 0x0c) == 0x0c)
    {
      sprintf(log.buf, "%s: No valid sampling frequency specified in %x%x%x%x.\n", me, head[0], head[1], head[2], head[3]);
      print_log (10);
      return (0); 
    }
  return(1);
}

/*
 * samestream: returns boolean for comparing 2 headers
 *             takes into account bitrate/mode def/padding
 */
static int
samestream (unsigned char head1[4], unsigned char head2[4])
{
  if ((head1[1] == head2[1]) && ((head1[2] & 0x0c) == (head2[2] & 0x0c)) && ((head1[3] & 0xc3) == (head2[3] & 0xc3)))
    return (1);
  return(0);
}

/*
 * search header: searches for next header from beginning of buffer
 *                returns -1 when no valid header is found
 *                or the position from buf->begin
 *
 */

int
search_first_header (buffer_t *buffer, stream_t *stream, int *count, header_t **heads, int pos)
{
  int i = 0, k;
  unsigned char head[4];
 
  while (1)
    switch (read_buf(buffer, head, i, 4))
      {
      case 0:
	sprintf(log.buf, "%s: No valid mp3header found in this buffer.\n", me);
	print_log (10);
 	return(-1); /* fill the buffer */
	
      case 1:
	if (head[0] == 0xff && isheader(head))
	  {
	    for (k = 0; k < (*count); k++)
	      {
		if (samestream (head, (*heads)[k].head))
		  {
		    sprintf (log.buf, "other header: %x.%x.%x.%x, count = %d\n", (*heads)[k].head[0], (*heads)[k].head[1], (*heads)[k].head[2], (*heads)[k].head[3], (*heads)[k].count);
		    print_log (10);
		    if ((*heads)[k].count == 3)
		      {
                        int temp = (*heads)[k].pos;
			stream->head = tmalloc(4 * sizeof (unsigned char));
			memcpy (stream->head, (*heads)[k].head, 4);
			free (*heads);
			return (temp);
		      }
		    (*heads)[k].count++;
		    break;
		  }
	      }
	    if (k == (*count))
	      {
		(*count)++;
		*heads = trealloc (*heads, (*count) * sizeof (header_t));
		memcpy ((*heads)[k].head, head, 4);
                (*heads)[k].pos = pos + i;
		(*heads)[k].count = 1;
	      }
	  }
	i++;
	break;
	
      default:
	sprintf(log.buf, "%s: Unable to read from buffer.\n", me);
	print_all (0);
	return(-2); /* flush the buffer */
      }
}


/*
 * search header: searches for next header from beginning of buffer
 *                returns -1 when no valid header is found
 *                or the position from buf->begin
 *
 */
int
search_header (buffer_t *buffer, stream_t *stream)
{
  int i = 0;
  unsigned char *head = tmalloc (4 * sizeof (unsigned char));

  while (1)
    switch (read_buf(buffer, head, i, 4))
      {
      case 0:
	sprintf(log.buf, "%s: No valid mp3header found in this buffer.\n", me);
	print_log (10);
	return(-1); /* fill the buffer */
	
      case 1:
	if (head[0] == 0xff)
	  {
	    if(samestream(head, stream->head))
	      {
		if (stream->last->head) /* first frame - already set */
		  free (stream->last->head);
		stream->last->head = head;
		return(i);
	      }
	  }
	i++;
	break;
	
      default:
	sprintf(log.buf, "%s: Unable to read from buffer.\n", me);
	print_all (0);
	return(-2); /* flush the buffer */
      }
}

/*
 *   1st index:  0 = "MPEG 1.0",   1 = "MPEG 2.0"
 *   2nd index:  0 = "Layer 3",   1 = "Layer 2",   2 = "Layer 1"
 *   3rd index:  bitrate index from frame header
 */

int kbpstab[2][3][16] =
{{
  {0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, -1},
  {0, 32, 48, 56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 384, -1},
  {0, 32, 40, 48,  56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, -1}
},
 {
   {0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256, -1},
   {0,  8, 16, 24, 32, 40, 48,  56,  64,  80,  96, 112, 128, 144, 160, -1},
   {0,  8, 16, 24, 32, 40, 48,  56,  64,  80,  96, 112, 128, 144, 160, -1}
 }};

static const unsigned freqtab[9] =
        {44100, 48000, 32000, 22050, 24000, 16000, 11025, 12000, 8000};

/*
 * parse_static_stream_inf: parses the first header and puts the neccessary
 *                          inf in the stream struct
 */

static void
parse_static_header_inf(stream_t *stream)
{
  frame_t *frame = stream->first;

  switch (frame->head[1] & 0x18) {/* mpeg version */
  case 0x00:
    stream->maj_version = 2; /* mpeg 2.5 */
    stream->min_version = 5;
    stream->freq = freqtab[6 + ((0x0c & frame->head[2]) >> 2)];
    break;
  case 0x10:
    stream->maj_version = 2; /* mpeg 2 */
    stream->freq = freqtab[3 + ((0x0c & frame->head[2]) >> 2)];
    break;  
  case 0x18:
    stream->maj_version = 1; /* mpeg 1 */
    stream->freq = freqtab[(0x0c & frame->head[2]) >> 2];
    break;
  }
  
  switch (frame->head[1] & 0x06) {/* layer */
  case 0x06:
    stream->layer = 1;
    stream->samples = 384;
    break;
  case 0x04:
    stream->layer = 2;
    stream->samples = 1152;
    break;  
  case 0x02:
    stream->layer = 3;
    stream->samples = (stream->maj_version == 1) ? 1152 : 576;
    break; 
  }

  stream->mode = ((frame->head[3] & 0xc0) >> 6);
  
  if (stream->maj_version == 1)
    stream->isize = (stream->mode == 3) ? 17 : 32;
  else
    stream->isize = (stream->mode == 3) ? 9 : 17;
  
  stream->cbr = 1;
  stream->crc = !(frame->head[1] & 0x01);
  stream->private = (frame->head[2] & 0x01);
  stream->copyright = (frame->head[3] & 0x08) >> 3;
  stream->original = (frame->head[3] & 0x04) >> 2;
}

/*
 * parse_frame_header: Parses a header for the variable inf.
 *
 */

void
parse_frame_header (stream_t *stream)
{
  frame_t *frame = stream->last;

  sprintf (log.buf, "%x.%x.%x.%x\n", frame->head[0], frame->head[1], frame->head[2], frame->head[3]);
  print_log (10);
  
  if (stream->count < 0)
    parse_static_header_inf(stream);
  
  frame->kbps = kbpstab[stream->maj_version - 1][stream->layer - 1][(0xf0 & frame->head[2]) >> 4];
  
  if (stream->count < 0)
    stream->avkbps = frame->kbps;
  else if (!stream->cbr)
    stream->avkbps += (frame->kbps - stream->avkbps) / (stream->count + 1);
  else if (stream->cbr && (stream->avkbps != frame->kbps))
    stream->cbr = 0;
  
  frame->hsize = (frame->kbps * 125 * stream->samples) / stream->freq;
  
  if (0x02 & frame->head[2])
    {
      frame->padded = 1;
      if (stream->layer == 1)
	frame->hsize += 4;
      else
	frame->hsize++;
    }
  frame->dsize = frame->hsize - stream->isize - 4;
}

/*
 * init_frame: inits and returns a fresh frame.
 *
 */
static frame_t *
init_frame (void) 
{
  frame_t *frame;
  
  frame = tmalloc(sizeof(frame_t));
  
  frame->prev = NULL;
  frame->next = NULL;

  frame->head = NULL;
  frame->info = NULL;
  frame->data = NULL;
  
  frame->hsize = 0;
  frame->dsize = 0;
  frame->backref = 0;
  frame->kbps = 0;
  
  return(frame);
}

/*
 * free_frame: frees the given frame from the stream.
 *
 */
static void
free_frame (stream_t *stream, frame_t *frame)
{
  if (frame->prev && frame->next)
    {
      frame->prev->next = frame->next;
      frame->next->prev = frame->prev;
    }
  else if (!frame->prev && !frame->next)
    stream->first = stream->last = NULL;
  else if (!frame->prev)
    {
      stream->first = frame->next;
      frame->next->prev = NULL;
    }
  else
    {
      stream->last = frame->prev;
      frame->prev->next = NULL;
    }

  free (frame->data);
  free (frame->info);
  free (frame);
}

/*
 * free_first_frame: needed to provide nice abstraction
 *
 */
void
free_first_frame (stream_t *stream)
{
  free(stream->first->info);
  free(stream->first->data);
  stream->first = stream->first->next;
  free(stream->first->prev);
  stream->first->prev = NULL;
}

/*
 * read_frame: reads a full frame from the buffer with the
 *             count the position of the first header.
 *
 */
static int
read_datasize (stream_t *stream)
{
  int gr0_ch0_size = 0, gr0_ch1_size = 0, gr1_ch0_size = 0, gr1_ch1_size = 0;
  int total, temp;
  frame_t *frame = stream->last;

  if (stream->isize == 32) /* mpeg 1 stereo */
    {
      gr0_ch0_size = ((frame->info[2] & 0x0f) << 8) | frame->info[3];
      gr0_ch1_size = ((frame->info[9] & 0x01) << 11)
	| (frame->info[10] << 3) | (frame->info[11] >> 5);
      gr1_ch0_size = ((frame->info[17] & 0x3f) << 6) |
	(frame->info[18] >> 2);
      gr1_ch1_size = ((frame->info[24] & 0x07) << 9) |
	(frame->info[25] << 1) | (frame->info[26] >> 7);
    } 
  else if (stream->isize == 9) /* mpeg 2 mono */
    gr0_ch0_size = ((frame->info[1] & 0x7f) << 5) | (frame->info[2] >> 3);
  else /* sisize == 17 */
    if (stream->maj_version == 1) /* mpeg 1 mono */
      {
	gr0_ch0_size = ((frame->info[2] & 0x1f) << 7) | (frame->info[3] >> 1);
	gr1_ch0_size = ((frame->info[9] & 0x03) << 10) |
	  (frame->info[10] << 2) | (frame->info[11] >> 6);
      }
    else /* mpeg 2 stereo */
      {
	gr0_ch0_size = ((frame->info[1] & 0x3f) << 4) | (frame->info[2] >> 4);
	gr0_ch1_size = ((frame->info[9] & 0x7f) << 5) | (frame->info[10] >> 3);
      }
  
  total = (gr0_ch0_size + gr0_ch1_size + gr1_ch0_size + gr1_ch1_size) / 8;
  if ((temp = (gr0_ch0_size + gr0_ch1_size + gr1_ch0_size + gr1_ch1_size) % 8))
      total++;

  return (total);
}

/*
 * read_frame: reads a full frame from the buffer with the
 *             count the position of the first header.
 *
 *             returns 0 on eof, -1 on error
 */
int
read_frame (stream_t *stream, buffer_t *filebuf, buffer_t *databuf)
{
  int temp;
#ifdef SIDE_DEBUG
  int k;
#endif /* SIDE_DEBUG */
  frame_t *frame = init_frame ();

  if (stream->count < 0)
    {
      stream->first = frame;
      stream->last = frame;
    }
  else
    {
      frame->prev = stream->last;
      stream->last->next = frame;
      stream->last = frame;
    }      
  switch (temp = search_header(filebuf, stream))
    {
    case -2:
      free_frame (stream, frame);
      return (-1); /* stop readin this file */
    case -1:
      free_frame (stream, frame);
      return (0); /* end of buffer */
    default:
      break;
    }
  
  parse_frame_header(stream);
  
  sprintf (log.buf, "Reading frame %ld\n", stream->count + 1);
  print_log (10);  
#ifdef FRAME_DEBUG
  sprintf(log.buf, "Frame %ld: %d kbps, header to header size = %d\n", stream->count + 1, frame->kbps, frame->hsize);
  print_log (10);
#endif
  
  if (stream->layer < 3)
    {
      if (filebuf->used <= frame->hsize)
	{
	  free_frame(stream, frame);
	  return (0);
	}

      frame->dsize = frame->hsize - 4; /* paddin at layer 1/2 = padded null */
      frame->data = tmalloc(frame->dsize * sizeof(unsigned char));
      read_buf(filebuf, frame->data, 4, frame->dsize);
      rem_buf(filebuf, frame->hsize);
    }
  else
    {
      frame->info = tmalloc(stream->isize * sizeof(unsigned char));
      switch (read_buf (filebuf, frame->info, 4 + temp + 2*stream->crc, stream->isize))
	{
	case -1:
	  return (-1); /* stop readin this file */
	case 0:
	  free_frame (stream, frame);
	  return (0); /* end of buffer */
	default:
	  break;
	}	
      
#ifdef SIDE_DEBUG
      sprintf(log.buf, "sideinfo = ");
      print_log (10);
      for ( k = 0; k < stream->isize; k++)
	{
	  sprintf(log.buf, ".%x", frame->info[k]);
	  print_log (10);
	}
      sprintf(log.buf, " \n");
      print_log (10);
#endif  
      
      if (stream->maj_version == 1)
	frame->backref = frame->info[0] << 1 | frame->info[1] >> 7;
      else
	frame->backref = frame->info[0];
      
      frame->dsize = read_datasize(stream);
      
      if (filebuf->used <= (frame->hsize + 3))
	if (!filebuf->eof || (filebuf->eof && (frame->dsize + stream->isize + 4 + 2*stream->crc) > (databuf->used + filebuf->used)))
	  {
	    free_frame(stream, frame);
	    return (0);
	  }

      if (temp)
	{
	  sprintf (log.buf, "Too much space between headers: %d bytes!\n", temp);
	  print_all (0);
	  cut_buf(filebuf, databuf, temp);
	}

      rem_buf(filebuf, 4 + stream->isize + 2 * stream->crc);

#ifdef FRAME_DEBUG
      sprintf(log.buf, "            Backref: %d, datasize: %d\n", frame->backref, frame->dsize);
      print_log (10);
#endif

      if (databuf->used < frame->backref)
	{
	  sprintf(log.buf, "Error: bad stream formattin, bad backref!\n");
	  print_all (0);
          frame->dsize = 0;
	 /* free_frame (stream, frame);
	    return (-1); */
	}
      else
	rem_buf(databuf, databuf->used - frame->backref);

      if (filebuf->used < (frame->hsize - 4 - stream->isize - 2*stream->crc))
	cut_buf(filebuf, databuf, filebuf->used);
      else
	cut_buf(filebuf, databuf, frame->hsize - 4 - stream->isize - 2*stream->crc);
      
      if (databuf->used < frame->dsize) /*skip frame data!!!*/
	{
	  sprintf(log.buf, "Error: bad stream formattin, databuffer underrun.\n");
 	  print_all (0);
          frame->dsize = 0;
	 /* free_frame (stream, frame);
	    return (-1);*/
	}
      if (frame->dsize)
	{
	  frame->data = tmalloc(frame->dsize * sizeof(unsigned char));
	  read_buf(databuf, frame->data, 0, frame->dsize);
	  rem_buf(databuf, frame->dsize);
	}
    }

#ifdef POINTER_DEBUG
  sprintf (log.buf, "frame %ld; prev: %p, next: %p, info: %p, data: %p\n", stream->count + 1, frame->prev, frame->next, frame->info, frame->data);
  print_log (10);
#endif


  stream->count++;
  if (!filebuf->used && filebuf->eof)
    return (0);
  else
    return (1);
}

/*
 * process_frames:
 *
 */
int
process_frames (stream_t *stream, long startframe, long endframe)
{
  frame_t *frame = stream->first;
  unsigned long count = 0;

#ifdef POINTER_DEBUG
      sprintf (log.buf, "frame %ld; prev: %p, next: %p, info: %p, data: %p\n", count, frame->prev, frame->next, frame->info, frame->data);
      print_log (10);
#endif /* POINTER_DEBUG */
  
  while (frame)
    {
      if ((count < startframe) || (endframe && (count > endframe)) || !frame->dsize)
	{
	  if (frame->next)
	    {
	      frame = frame->next;
	      free_frame (stream, frame->prev);
	    }
	  else /* last frame */
	    {
	      free_frame (stream, frame);
	      frame = NULL;
	    }
	  stream->count--;
          sprintf (log.buf, "Removed frame %ld\n", count);
	    print_log (10);
	}
      else
	frame = frame->next;
      count++;
    }
  return (1);
}
/*
 * empty_info:
 *
 */
static unsigned char *
empty_info (stream_t *stream)
{
  unsigned char *info = tcalloc (stream->isize, sizeof (unsigned char));
  memset (info, 0x00, stream->isize);
  
  if (stream->maj_version == 1)
    {
      if (stream->mode == 3)
	{
	  info[1] = 0x03;
	  info[2] = 0xc0;
	  info[4] = 0x01; /* global gain */
	  info[5] = 0xa4;
	  info[6] = 0x1c; /* window switch & block type */
	  info[12] = 0x34; /* global gain */
	  info[13] = 0xf0;
	}
      else
	{
	  info[1] = 0x0f; /* scfsi */
	  info[2] = 0xf0;
	  /* granule 0 channel 0 */
	  info[5] = 0x69; /* global gain = 210 */
	  /* in the computation of the gain per frequency 210 is
	     substracted, so the equation becomes 0 */
	  info[6] = 0x07; /* windows switching flag & blocktype */
	  /* granule 0 channel 1 */
	  info[12] = 0x0d;
	  info[13] = 0x20;
	  info[14] = 0xe0;
	  /* granule 1 channel 0 */
	  info[19] = 0x01; /* global gain */
	  info[20] = 0xa4;
	  /* granule 1 channel 0 */
	  info[27] = 0x34;
	  info[28] = 0x80; /* global gain */
	}
    }
  else
    {
      if (stream->mode == 3)
	{
	  info[3] = 0x01;
	  info[4] = 0x48;
	  info[6] = 0xe0;
	  info[11] = 0x03;
	  info[12] = 0x48;
	  info[13] = 0x01;
	  info[14] = 0xc0;
	}
      else
	{
	  info[3] = 0x03;
	  info[4] = 0x48;
	  info[5] = 0x01;
	  info[6] = 0xc0;
	}
    }
  return (info);
}

/*
 * write_emptyframe: 
 *
 */
static int
write_empty_startframe (stream_t *stream)
{
  frame_t *frame = init_frame ();

  frame->next = stream->first;
  stream->first = frame->next->prev = frame;
  stream->count++;
  frame->hsize = frame->next->hsize;
  frame->dsize = 0;
  frame->kbps = frame->next->kbps;
  frame->head = tmalloc (4 * sizeof(unsigned char));
  memcpy (frame->head, frame->next->head, 4);
  frame->backref = 0;
  frame->info = empty_info (stream);

  return (1);
}

/*
 * write_empty_frame: 
 *
 */
static int
write_empty_frame (stream_t *stream, frame_t *prevframe)
{
  frame_t *frame = init_frame ();

  frame->next = prevframe->next;
  prevframe->next = frame->next->prev = frame;
  stream->count++;
  frame->hsize = frame->next->hsize;
  frame->dsize = 0;
  frame->kbps = frame->next->kbps;
  frame->head = tmalloc (4 * sizeof(unsigned char));
  memcpy (frame->head, frame->next->head, 4);
  frame->backref = 0;
  frame->info = empty_info (stream);

  return (1);
}

/*
 * calc_backref: 
 *
 */
int
calc_backref (stream_t *stream)
{
  if (stream->layer == 3)
    {
      int backref = 0, count = 0, highest = 0;
      int limit = (stream->maj_version == 1) ? 511 : 255; 
      frame_t *frame = stream->first;
      
      while (frame)
	{
	  frame->backref = backref;
	  
	  /* write backref */
	  frame->info[0] &= 0x00;
	  if (stream->maj_version == 1)
	    frame->info[1] &= 0x7f;
	  if (backref)
	    {
	      if (stream->maj_version == 1)
		{
		  frame->info[0] |= backref >> 1;
		  frame->info[1] |= (backref & 0x01) << 7;
		}
	      else
		frame->info[0] |= backref;
	    }
#ifdef FRAME_DEBUG	  
	  sprintf (log.buf, "frame %d: hsize: %d, dsize: %d, backref: %d\n", count, frame->hsize, frame->dsize, frame->backref);
	  print_log (10);
#endif	  
	  backref += (frame->hsize - 4 - stream->isize - frame->dsize);
	  
	  if (backref > limit)
	    backref = limit;
	  else if (backref < 0)
	    {
	      sprintf (log.buf, "Unable to decently format stream, frame size (%d) exceeds available space (%d)\n", frame->dsize, frame->dsize + backref);
	      print_log (2);

	      if ((limit - highest + backref) < 0) /* local underrun */
		{
		  frame = frame->prev;
		  count--;
		  if (stream->maj_version == 1)
		    backref = (frame->info[0] << 1) & (frame->info[1] >> 7);
		  else
		    backref = frame->info[0];
		  write_empty_frame (stream, frame);
		}
	      else /* needs another frame at the start of the stream */
		{
		  write_empty_startframe (stream);
		  count = backref = 0;
		  frame = stream->first;
		}
	      continue;
	    }
	  
	  if (backref > highest)
	    highest = backref;
	  
	  frame = frame->next;
	  count++;
	}
    }
  return (1);
}

/*
 * write_frame: writes to the framebuf and starts the next frame
 *
 *             returns 0 on eof, -1 on error
 */
int
write_frames (stream_t *stream, FILE *file)
{
  frame_t *frame = stream->first;
  buffer_t *framebuf = init_buf (FRAMEBUF_SIZE * 1024);
  
  if (stream->layer != 3)
      while (frame)
      {
	print_data (frame->head, 4);
	write_buf (frame->head, framebuf, 4);
	write_buf (frame->data, framebuf, frame->dsize);
	print_buf (framebuf, frame->hsize);
	write_file_from_buf (framebuf, file, frame->hsize);
	frame = frame->next;
      }
  else
    {
      frame_t *dataframe = stream->first;
      int count, offset = 0, backref = 0, temp;
      int fcount = 0, dfcount = 0;
      char *padding = tmalloc (1024 * sizeof (char));
      memset (padding, 0x00, 1024);
      /* frame = the frame that is now being filled out */
      /* dataframe = the one that provides for the data to fill this frame */
      /* count counts the still available bytes in this frame */
      /* offset = the offset in the data of the dataframe */

      while (frame)
	{
	  count = frame->hsize;
#ifdef WRITE_DEBUG
	  sprintf (log.buf, "FRAME: %d\n", fcount);
	  print_log (10);
#endif
	  write_buf (frame->head, framebuf, 4);
	  count -= 4;
	  write_buf (frame->info, framebuf, stream->isize);
	  count -= stream->isize;
#ifdef WRITE_DEBUG
	  sprintf (log.buf, "wrote header & sideinfo (%d) to framebuf\n", 4 + stream->isize);
	  print_log (10);
#endif
  
	  while (count && dataframe)
	    {
	      if (dataframe->dsize - offset)
		{
		  while (count && (temp = backref - (dataframe->backref - offset)) > 0)
		    { /* pad with 0 */

		      if (temp > count)
			temp = count;
		      write_buf (padding, framebuf, temp);
		      backref -= temp;
		      count -= temp;
#ifdef WRITE_DEBUG
		      sprintf (log.buf, "wrote %d null bytes to framebuf (backref = %d, count = %d)\n", temp, backref, count);
		      print_log (10);
#endif
		    }
		  if (!count)
		    break;
		  if (count < (dataframe->dsize - offset))
		    {
		      write_buf (dataframe->data + offset, framebuf, count);
		      offset += count;
		      backref -= count;
#ifdef WRITE_DEBUG
		      sprintf (log.buf, "wrote %d bytes from data to framebuf (backref = %d, offset = %d, count = 0)\n", count, backref, offset);
		      print_log (10);
#endif
		      count = 0;
		      break;
		    }
		  else
		    {
		      int temp = dataframe->dsize - offset;
		      write_buf (dataframe->data + offset, framebuf, temp);
		      backref -= temp;
		      count -= temp;
#ifdef WRITE_DEBUG
		      sprintf (log.buf, "wrote %d bytes from data to framebuf (backref = %d, count = %d)\n", temp, backref, count);
		      print_log (10);
#endif
		    }
		}
	      backref += (dataframe->hsize - stream->isize - 4);
#ifdef WRITE_DEBUG
	      sprintf (log.buf, "backref: %d\n", backref);
	      print_log (10);
#endif
	      dataframe = dataframe->next;
	      offset = 0;
	      dfcount++;
#ifdef WRITE_DEBUG
	      sprintf (log.buf, "DATAFRAME: %d\n", dfcount);
	      print_log (10);
#endif
	    }
	  if (!dataframe && count)
	    {
	      write_buf (padding, framebuf, count);	    
	      sprintf (log.buf, "padded frame %d with %d null bytes\n", fcount, count);
	      print_log (10);
	    }
	  write_file_from_buf (framebuf, file, frame->hsize);
	  frame = frame->next;
	  fcount++;
	}
    }
  write_tag_v1 (stream, file);
  return (1);
}

/* EOF */
