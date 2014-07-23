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

#ifndef TIFAA_H
#define TIFAA_H

#include <glob.h>

#define TIFFAVERSION        "v0.2"

#define NONINDEX            (size_t)(-1)
#define NUM_IMAGEINFO_COLS  4
#define WI_COLS             8
#define LOG_COLS            3





struct tifaaparams
{
  /* Paramters given by user: */
  size_t    numthrd;  /* Number of threads to use.                      */
  int          verb;  /* ==1: report steps. ==0 don't.                  */
  double       *cat;  /* Data of catalog.                               */
  size_t        cs0;  /* Number of rows in the catalog.                 */
  size_t        cs1;  /* Number of columns in the catalog.              */
  size_t     ra_col;  /* Catalog RA column                              */
  size_t    dec_col;  /* Catalog Dec column                             */
  double        res;  /* Resolution in arcseconds                       */
  double    ps_size;  /* Postage stamp size (in arcseconds).            */
  glob_t   survglob;  /* glob structure of input images.                */
  char    *out_name;  /* Folder keeping the cropped images              */
  char     *out_ext;  /* Ending of output file name                     */
  long     chk_size;  /* width of a box to check for zeros              */
  char   *info_name;  /* File keeping the results log.                  */

  /* Internal parameters:  */
  double   *imginfo;  /* Necessary information for each image.          */
  size_t  *whichimg;  /* Array saying which images for which target.    */
  size_t       *log;  /* Log for all the objects.                       */
};





struct stitchcropthread
{
  size_t              id; /* ID of thread.                            */
  size_t    *targetthrds; /* Which target for which thread.           */
  size_t        thrdcols; /* Number of columns in targetthrd.         */
  size_t       crop_side; /* Side of the cropped region in pixels.    */
  struct tifaaparams *tp; /* All available parameters.                */

  size_t           *done; /* Counter of number of compelted threads.  */
  pthread_mutex_t     *m; /* Thread mutex.                            */
  pthread_mutex_t    *wm; /* WCS mutex.                               */
  pthread_cond_t      *c; /* Conditional variable.                    */
};

/* Function declarations: */
void 
tifaa(struct tifaaparams *p);

#endif
