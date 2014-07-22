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

#define TIFFAVERSION        "v0.2"

/* Macros: */
#define NUM_IMAGEINFO_COLS  4
#define WI_COLS             4
#define FINAL_REPORT_COLS   3
#define LINE_BUFFER         1000
#define PI                  3.14159265359
#define MAX_FILE_NAME_SIZE  256
#define EMPTY_VAL           -1

struct tifaaparams
{
  int        verb;  /* ==1: report steps. ==0 don't.                  */

  double     *cat;  /* Data of catalog.                               */
  size_t      cs0;  /* Number of rows in the catalog.                 */
  size_t      cs1;  /* Number of columns in the catalog.              */
  size_t   id_col;  /* Catalog ID column                              */
  size_t   ra_col;  /* Catalog RA column                              */
  size_t  dec_col;  /* Catalog Dec column                             */

  double      res;  /* Resolution in arcseconds                       */
  double  ps_size;  /* Postage stamp size (in arcseconds).            */
  char *surv_name;  /* Folder containing archive images.              */
  char   *img_pfx;  /* Prefix showing the image files in the archive,
		       in other words: (replace the *) "##*".         */
  char  *out_name;  /* Folder keeping the cropped images              */
  char   *out_ext;  /* Ending of output file name                     */
  size_t chk_size;  /* width of a box to check for zeros              */
  char *info_name;  /* File keeping the results log.                  */
};

/* Function declarations: */
void 
tifaa(struct tifaaparams *p);

#endif
