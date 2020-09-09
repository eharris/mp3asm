/*  mp3asm: an mp3 frameeditor.
 *
 *  parse.c : parses the command line input.
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

#include <unistd.h>
#include "mp3asm.h"
#include "parse.h"
#include "utils.h"

/* mp3asm.c */
extern void new_input (void);

/* utils.c */
extern void *tmalloc (size_t size);
extern void print_std (int verb);

/*
 * implementing callbacks!
 *
 */
static void usage (void);

static void
bad_flag (char *string, int i)
{
  if (!i)
    sprintf (log.buf, "%s: Bad flag specified: \"%s\"\n", me, string);
  else 
    sprintf (log.buf, "%s: Bad flag specified: \"%s\" from \"%s\"\n", me, string+i, string);
  print_std (-1);
  exit (EX_USAGE);
}

static void
arg_needed (char flag)
{
  sprintf (log.buf, "%s: No argument specified for flag %c\n", me, flag);
  print_std (-1);
  exit (EX_USAGE);
}

static void
double_flag (char flag)
{
  sprintf (log.buf, "%s: Flag specified twice: \"%c\"\n", me, flag);
  print_std (-1);
  exit (EX_USAGE);
}

static void
bad_arg (char *string)
{
  sprintf (log.buf, "%s: Bad argument given: \"%s\"\n", me, string);
  print_std (-1);
  exit (EX_USAGE);
}

static int
incr_verbosity (char *arg)
{
  verbosity++;
  return (0);
}

static int
start_frame (char *arg)
{
  if (input[inputs - 1]->startframe) 
    return (1); 
  if ((input[inputs - 1]->startframe = atol (arg)))
    return (0);
  return (2);
}

static int
end_frame (char *arg)
{
  if (input[inputs - 1]->endframe || input[inputs - 1]->readframes) 
    return (1);
  if ((input[inputs - 1]->endframe = atol (arg)))
    return (0);
  return (2);
}

static int
read_frames (char *arg)
{
  if (input[inputs - 1]->readframes || input[inputs - 1]->endframe) 
    return (1);
  if ((input[inputs - 1]->readframes = atol (arg)))
    return (0);
  return (2);
}

static int
use_tag  (char *arg)
{
  int i = inputs;
  while (i)
    {
      i--;
      if (input[inputs - 1]->use_id3)
	return (1);
    }
  input[inputs - 1]->use_id3++;
  return (0);
}

static int
set_output (char *arg)
{
  if (output->name)
    return (1);
  output->name = strcpy (tmalloc (strlen (arg) + 1), arg);
  return (0);
}

static int
report (char *arg)
{
  char *name = tmalloc (1024 * sizeof (char));
  if (!gethostname (name, 1024))
    sprintf (log.buf, "Sending notification to the authorities; Illegal use of copyrighted material on %s\n", name);
  free (name);
  return (0);
}

static int
use_log (char *arg)
{
  if (log.name)
    return (1);
  log.name = strcpy (tmalloc (strlen (arg) + 1), arg); 
  return (0);
}

static int
print_help (char *arg)
{
  usage ();
  return (0);
}

#define NO_ARG 0
#define NEEDS_ARG 1

struct parse_flags
{
  char flag;
  char *help;
  int (*handler) (char *arg);
  int args;
};

struct parse_flags parse[] =
{
  {'v', "Increase verbosity.", incr_verbosity, NO_ARG},
  {'s', "Set the starting frame.", start_frame, NEEDS_ARG},
  {'e', "Set the ending frame.", end_frame, NEEDS_ARG},
  {'r', "Set the number of frames to read (conflicts with 'e')", read_frames, NEEDS_ARG},
  {'t', "Use id3-tag of the file. (should be used only once!)", use_tag, NO_ARG},
  {'o', "Name of the file to output to. (default = changed input filename)", set_output, NEEDS_ARG},
  {'N', "Not reporting illegal copyrighte material to the authorities.", report, NO_ARG},
  {'l', "Log to file.", use_log, NEEDS_ARG},
  {'h', "Print this help.", print_help, NO_ARG},
  { 0, NULL, NULL, NO_ARG}
};

static void
usage (void)
{ 
  int k = 0;
  
  sprintf (log.buf, "Usage: %s [options] inputfile.mp3 [...]\n", me);
  print_std (0);
  while (parse[k].flag)
    {
      sprintf (log.buf, "   -%c     %s\n", parse[k].flag, parse[k].help);
      print_std (0);
      k++;
    }
  
  exit (EX_USAGE);
}

/*
 * Parses an individual argument.
 *
 */ 

void 
parse_argument (int argc, char *argv[])
{
  int strlength, i, j, k, temp;

  for (i = 1; i < argc; ++i)
    {
      if (argv[i][0] == '-') /* flag */
	{
	  strlength = strlen ((char *)argv[i]);
	  for (j = 1 ; j < strlength; ++j)
	    {
	      k = 0;
	      while (parse[k].flag != argv[i][j])
		{
		  if (parse[k].flag == 0)
		    bad_flag (argv[i], j);
		  k++;
		}
	      if (parse[k].args == NEEDS_ARG)
		{
		  if (++j < strlength) /* this is not the last char */
		    {
		      if ((temp = parse[k].handler (argv[i] + j)) == 1)
			double_flag (argv[i][j - 1]);
		      else if (temp == 2)
			bad_arg (argv[i] + j);
		    }
		  else if (argv[++i][0] != '-')
		    {
		      if ((temp = parse[k].handler (argv[i])) == 1)
			double_flag (argv[i - 1][j - 1]);
		      else if (temp == 2)
			bad_arg (argv[i]);
		      
		    }
		  else
		    arg_needed (parse[k].flag);
		  j = strlength;
		}
	      else
		{
		  if (parse[k].handler (NULL) == 1)
		    double_flag (argv[i][j] );
		}
	    }
	}
      else /* input file */
	{
	  
	  input[inputs - 1]->name = tmalloc (strlen (argv[i]) + 1);
	   strcpy (input[inputs - 1]->name, argv[i]);
	   /*sprintf (log.buf, "%d input: %s\n", inputs - 1, input[inputs - 1]->name);
	     print_std (5);*/
	  new_input ();
	}
    }
}

/*
 * Check if the options r useable.
 *
 */

static void
check_options ()
{
  int id3=0, i;

  for (i = 0; i < inputs; ++i)
    {
      if (input[i]->readframes)
	input[i]->endframe = input[i]->startframe + input[i]->readframes;
      else if (input[i]->endframe) 
	input[i]->readframes = input[i]->endframe - input[i]->startframe;
      
      if ((input[i]->endframe && (input[i]->startframe > input[i]->endframe)) || (input[i]->startframe && (input[i]->startframe == input[i]->endframe)))
	{
	 sprintf (log.buf, "%s: Invalid options given. The number of frames to skip should be smaller than the number of the last frame to read for \"%s\".\n", me, input[i]->name);
	 print_std (-1);
 	exit (EX_USAGE);
       }
      
      if (input[i]->use_id3) {
	if (id3)
	  {
	    sprintf (log.buf, "%s: Invalid options given. More than one id3 tag specified for use.\n", me);
	    print_std (-1);
	    exit (EX_USAGE);
	  }
	else
	  ++id3;
      }
    }
  if (input[inputs - 1]->name == NULL)
    {
      if (input[inputs -1]->startframe ||input[inputs -1]->endframe || input[inputs -1]->readframes || input[inputs -1]->use_id3)
	{
	  sprintf (log.buf, "%s: Flags specified for input file, but no input file has been given.\n", me);
	  print_std (-1);
	  exit(EX_USAGE);
	}
      free (input[inputs - 1]);
      inputs--;
    }
  if (!inputs)
    {
      sprintf (log.buf, "%s: No input files specified.\n", me);
      print_std (-1);
      exit (EX_USAGE);
    }
  /* else
    {
      sprintf (log.buf, "inputs: %d\n", inputs);
      print_std (5);
      }*/

  if (!info)
    {
      sprintf (log.buf, "%s: Program from Olli Fromme & _Death_.\n", me);
      print_std (-1);
      
      if (!output->name)
	sprintf (log.buf, "%s: Checking the following mp3 files for header errors:\n", me);
      else
	sprintf(log.buf, "%s: Taking input from:\n", me);
      print_std (1);
      
      for (i = 0; i < inputs; ++i) 
	{
	  sprintf (log.buf, "%s: input %d: %s from frame %ld to %ld [%ld].\n", me, i, input[i]->name, input[i]->startframe, input[i]->endframe, input[i]->readframes);
	  print_std (2);
	}
      
      if (!output->name)
	sprintf (log.buf, "%s: No output file specified.\n", me);
      else
	sprintf (log.buf, "%s: Outputting to %s.\n", me, output->name);
      print_std (1);
    }
}

/*
 * Parses the arguments.
 *
 */

void
parse_args (int argc, char *argv[])
{
  if (argc < 2) 
    usage();
  
  parse_argument(argc, argv) ;
  check_options();
}

/* EOF */
