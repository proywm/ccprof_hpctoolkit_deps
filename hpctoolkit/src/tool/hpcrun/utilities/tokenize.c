// -*-Mode: C++;-*- // technically C99

// * BeginRiceCopyright *****************************************************
//
// $HeadURL$
// $Id$
//
// --------------------------------------------------------------------------
// Part of HPCToolkit (hpctoolkit.org)
//
// Information about sources of support for research and development of
// HPCToolkit is at 'hpctoolkit.org' and in 'README.Acknowledgments'.
// --------------------------------------------------------------------------
//
// Copyright ((c)) 2002-2017, Rice University
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// * Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
//
// * Neither the name of Rice University (RICE) nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// This software is provided by RICE and contributors "as is" and any
// express or implied warranties, including, but not limited to, the
// implied warranties of merchantability and fitness for a particular
// purpose are disclaimed. In no event shall RICE or contributors be
// liable for any direct, indirect, incidental, special, exemplary, or
// consequential damages (including, but not limited to, procurement of
// substitute goods or services; loss of use, data, or profits; or
// business interruption) however caused and on any theory of liability,
// whether in contract, strict liability, or tort (including negligence
// or otherwise) arising in any way out of the use of this software, even
// if advised of the possibility of such damage.
//
// ******************************************************* EndRiceCopyright *

// Simple utility for iterating the tokens out of a given string
// FIXME: don't use strtok(), don't use single static buffer,
// factor out delimiters, etc.

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "tokenize.h"

#define MIN(a,b)  (((a)<=(b))?(a):(b))

#define EVENT_DELIMITER '@'
#define PREFIX_FREQUENCY 'f'


static char *tmp;
static char *tk;
static char *last;

static char *sep1 = " ,;";

char *
start_tok(char *lst)
{
  tmp = strdup(lst);
  tk  = strtok_r(tmp,sep1,&last);
  return tk;
}

int
more_tok(void)
{
  if (! tk){
    free(tmp);
  }
  return (tk != NULL);
}

char *
next_tok(void)
{
  tk = strtok_r(NULL,sep1,&last);
  return tk;
}


/**
 * extract the threshold
 */
// Returns:
//   2 if event has explicit frequency
//   1 if event has explicit period threshold,
//   0 if using default.
int
hpcrun_extract_threshold(const char *in, long *th, long def)
{
  if (in == NULL) {
    *th = def;
    return 0;
  }

  int is_period = in[0] == 'f';
  int pos       = 0;
  if (is_period) {
    pos = 1;
  }
  long num = strtol(in + pos, (char **)NULL, 10);
  *th = (num == 0 ? def : num);

  if (is_period)
    return 2;

  return 1;
}

// event option syntax is event_name @ [f] threshold
//   if the f indicator exist, the number is the frequency, otherwise
//   it's a period number
// Returns: 
//   2 if event has explicit frequency
//   1 if event has explicit period threshold, 
//   0 if using default.
// 
int
hpcrun_extract_ev_thresh(const char *in, int evlen, char *ev, long *th, long def)
{
  unsigned int threshold_pos = 0;
  int result = 0;
  unsigned int len = strlen(in);

  char *dlm = strrchr(in, EVENT_DELIMITER);
  if (!dlm) {
    dlm = strrchr(in, ':');
  }
  if (dlm) {
    if (isdigit(dlm[1]) || dlm[1] == PREFIX_FREQUENCY) {
      // we probably have threshold number
      len = MIN(dlm - in, evlen);
      strncpy(ev, in, len);
      ev[len] = '\0';

      result = hpcrun_extract_threshold(dlm+1+threshold_pos, th, def);
      return result;
    }
    if (dlm[0] == EVENT_DELIMITER) {
      len = MIN(dlm - in, evlen);
    }
  }
  // no threshold or the threshold is not a number
  strncpy(ev, in, len);
  ev[len] = '\0';
  *th = def;
  
  return 0;
}

//
// Check  "event" part of candidate string
// for an **exact** match event_name
//
// candidate strings may be either a single event name, or EVENT_NAME@SOMETHING or EVENT_NAME:SOMETHING
//
bool
hpcrun_ev_is(const char* candidate, const char* event_name)
{
  return (strstr(candidate, event_name) == candidate) && strchr("@:",candidate[strlen(event_name)]);
}

#if TEST_TOKENIZE

int
main (int argc, char *argv[])
{
  const char *tokens[]={ "one", "two@", "three@3", "four@f4", "five@f", "six@s", "seven::perf",
      "perf::eight@", "perf::nine@f200", "perf:ten@10" };
  const int elem = 10;
  int i, res;
  long th;
  char ev[100];

  for(i=0; i<elem; i++) {
    res = hpcrun_extract_ev_thresh(tokens[i], 100, ev, &th, -1);
    printf("%d: %s --> ev: %s, t: %ld, r: %d\n", i, tokens[i], ev, th, res);
  }
}
#endif
