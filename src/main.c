/*********************************************************************
TIFFA - Thumbnail images from astronomical archives

Copyright (C) 2013-2014 Mohammad Akhlaghi
Tohoku University Astronomical Institute, Sendai, Japan.
http://astr.tohoku.ac.jp/~akhlaghi/

TIFFA is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TIFFA is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TIFFA. If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/
#define _REENTRANT

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "tifaa.h"
#include "timing.h"

#include "ui.h"			/* needs tifaa.h */

int 
main (int argc, char *argv[])
{
  time_t rawtime;
  struct timeval t1;
  struct tifaaparams p;

  /* Set the starting time.*/
  time(&rawtime);
  gettimeofday(&t1, NULL);

  /* Read the input parameters*/
  setparams(argc, argv, &p);
  printdashes(1);
  printf("TIFAA %s (%lu threads) started on %s", 
	 TIFFAVERSION, p.numthrd, ctime(&rawtime));
  
  /* Run TIFAA */
  tifaa(&p);
  
  /* Free all non-freed allocations. */
  freeparams(&p);

  /* Print the final message. */
  reporttiming(&t1, "TIFFA finished in: ", 0);
  printdashes(0);

  /* Return 0 for success.*/
  return 0;
}

