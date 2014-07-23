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
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with tifaa.  If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/
#ifndef TIFAAUI_H
#define TIFAAUI_H

#define NUMDASHES       70

#define DEFAULTCATNAME  NULL
#define DEFAULTRACOL    (size_t)(-1)
#define DEFAULTDECCOL   (size_t)(-1)
#define DEFAULTRES      -1.0f
#define DEFAULTPSSIZE   -1.0f
#define DEFAULTSURVNAME NULL
#define DEFAULTIMGPFX   NULL
#define DEFAULTOUTNAME  NULL
#define DEFAULTOUTEXT   NULL

struct uiparams
{
  char   *cat_name;  /* Address of catalog                             */
  int  delpsfolder;  /* ==0: don't. ==1: do.                           */
  char  *surv_name;  /* Folder containing archive images.              */
  float thrdmultip;  /* Multiple of NCORES to use threads.             */
};


void
printdashes(int s1_e0);

void
setparams(int argc, char *argv[], struct tifaaparams *p);

void
freeparams(struct tifaaparams *p);


#endif
