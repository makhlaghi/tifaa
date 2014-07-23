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
#ifndef SURVEYIMGINFO_H
#define SURVEYIMGINFO_H

#include <fitsio.h>
#include <wcslib/wcshdr.h>
#include <wcslib/wcsfix.h>
#include <wcslib/wcs.h>

struct imginfothreadparams
{
  size_t           id; /* Thread ID.                                   */
  size_t    *imgthrds; /* Image indexs for each thread.                */
  size_t     thrdcols; /* Number of columns in imgthrds.               */
  char     **imgnames; /* Array pointing to image names.               */
  double     *imginfo; /* Array to keep the information on each image. */
  double          res; /* Resolution of the image.                     */
  size_t        *done; /* Pointer to number of complete threads.       */
  pthread_cond_t   *c; /* Pointer to the general conditional variable. */
  pthread_mutex_t  *m; /* Pointer to the general mutex variable.       */
  pthread_mutex_t *wm; /* Pointer to the general mutex variable.       */
};

void
prepindexsinthreads(size_t nindexs, size_t nthrds, size_t **outthrds,
		    size_t *outthrdcols);

void
prepare_fitswcs(char *fits_name, fitsfile **fptr, int *f_status, 
		int *w_status, int *nwcs, struct wcsprm **wcs,
		pthread_mutex_t *wm);

void
getsurveyimageinfo(struct tifaaparams *tp);

void 
whichimageforwhichtargets(struct tifaaparams *p);

#endif
