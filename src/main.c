/*********************************************************************
tifaa - Thumbnail images from astronomical archives
A simple set of functions to crop thumbnails from astronomical archives.

Copyright (C) 2013 Mohammad Akhlaghi
Tohoku University Astronomical Institute, Sendai, Japan.
http://astr.tohoku.ac.jp/~akhlaghi/

tifaa is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

tifaa is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this tifaa. If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "tifaa.h"

#include "ui.h"			/* needs tifaa.h */

int 
main (int argc, char *argv[])
{
  struct tifaaparams p;
  
  /* Read the input parameters*/
  setparams(argc, argv, &p);

  /* Run TIFAA 
  tifaa(&p);
  */

  /* Free all non-freed allocations. */
  free(p.cat);

  /* Return 0 for success.*/
  return 0;
}

