/*  mp3asm: an mp3 frameeditor.
 *
 *  mp3asm.c: something should hold int main :)
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
#include "parse.h"
#include "utils.h"

/* stream.c */

extern stream_t *read_stream (FILE *file);
extern void print_stream_inf(stream_t *stream, char *name);
extern int write_stream (stream_t *stream, char **filename);
extern int process_input (stream_t *stream, long startframe, long endframe);
extern int process_output (stream_t *stream);
extern void merge_streams (stream_t *streama, stream_t *streamb);

/* utils.c */

extern void *tmalloc (size_t size);
extern void *trealloc (void *ptr, size_t size);
extern void print_std (int verb);
extern int logopen (void);
extern FILE *mp3ropen(const char *name);
extern FILE *mp3wopen(const char *name);

/* parse.c */

extern void parse_args (int argc, char *argv[]);

int verbosity, inputs, quiet, info; 
char *me;
logfile_t log;
input_t **input;
output_t *output;

/*
 * getprogname: sets me to be the name of the executable.
 *
 */

char *
getprogname(const char *argv0)
{
  char *string;
  (string = strrchr(argv0, '/')) ? string++ : (string = (char *)argv0);
  return(string);
}

/*
 * new_input: adds and inits a new input
 *
 */

void
new_input (void)
{
  ++inputs;

  input = trealloc(input, inputs*sizeof(input_t *));
  input[inputs - 1] = tmalloc(sizeof(input_t));

  input[inputs - 1]->name = NULL;
  input[inputs - 1]->file = NULL;
  input[inputs - 1]->startframe = 0;
  input[inputs - 1]->readframes = 0;
  input[inputs - 1]->endframe = 0;
  input[inputs - 1]->use_id3 = 0;
  input[inputs - 1]->stream = NULL;
}

/*
 * init_global:
 *
 */
void
init_global (char *progname)
{
  me = strcpy((char *)tmalloc(strlen(progname) + 1), progname);

  log.name = NULL;
  log.file = NULL;

  verbosity = 0;
  quiet = 0;

  inputs = 0; 
  new_input ();

  output = (output_t *)tmalloc(sizeof(output_t));
  output->name = NULL;
  output->file = NULL;
  output->stream = NULL;
  output->write_crc = 0;  /* not used yet */
}

/*
 * open_inputs: opens & reads the inputstreams
 *
 */

static void
open_inputs (void)
{
  int i;

  for (i = 0; i < inputs; ++i)
    {
      input[i]->file = mp3ropen (input[i]->name);
      input[i]->stream = read_stream (input[i]->file);
      if (!input[i]->stream)
	{
	  sprintf (log.buf, "bad input file specified\n");
	  print_std (-1);
	  exit (EX_NOINPUT);
	}
      print_stream_inf (input[i]->stream, input[i]->name);
      fclose (input[i]->file);
      input[i]->file = NULL;

      process_input (input[i]->stream, input[i]->startframe, input[i]->endframe);
    }
}
/*
 * open_inputs: opens & reads the inputstreams
 *
 */

static void
write_output (void)
{
  int i;

  if (!inputs)
    {
      sprintf (log.buf, "Please provide valid inputfiles...\n");
      print_std (-1);
      exit (EX_USAGE);
    }
    
  for (i = 1; i < inputs; ++i)
    {
      /*fprintf (stderr, "%d\n", i);*/
      if (input[i]->use_id3)
	{
	  if (input[i]->stream->tag)
	    {
	      input[0]->stream->tag = input[i]->stream->tag;
	      input[i]->stream->tag = NULL;
	    }
	  else
	    {
	      sprintf (log.buf, "Unable to use a non-existent id3 tag.\n");
	      print_std (0);
	    }
	}
      else
	free (input[i]->stream->tag);
    }
  
  output->stream = input[0]->stream;
  input[0]->stream = NULL;

  for (i = 1; i < inputs; ++i)
    {
      merge_streams (output->stream, input[i]->stream);
      input[i]->stream = NULL;
    }
  free (input);
  inputs = 0;
  
  process_output (output->stream);

  write_stream (output->stream, &output->name);
}

/*
 * main
 *
 */ 
int
main(int argc, char *argv[])
{ 
  init_global (getprogname(argv[0]));

  parse_args (argc, argv);

  logopen ();

  open_inputs ();

  if (output->name)
    write_output ();

  exit (EX_OK);
}

/* EOF */
