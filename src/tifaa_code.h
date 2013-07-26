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
along with this program.  If not, see <http://www.gnu.org/licenses/>.

**********************************************************************/

#ifndef TIFAA_H
#define TIFAA_H

/* Libraries to include: 
System libraries: */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <glob.h>
#include <sys/stat.h>
#include <time.h>

/* Installed libraries: */
#include <fitsio.h>
#include <wcslib/wcshdr.h>
#include <wcslib/wcsfix.h>
#include <wcslib/wcs.h>

/* My libraries: */
#include "attaavv.h"

/* Macros: */
#define NUM_IMAGEINFO_COLS  4
#define WI_COLS             4
#define FINAL_REPORT_COLS   3
#define LINE_BUFFER         1000
#define PI                  3.14159265359
#define MAX_FILE_NAME_SIZE  256
#define EMPTY_VAL           -1

/* This structure is used to keep the information
in the configuration file:*/
struct Config 
{                     /************************************/
    long     id_col;  /* Catalog ID column                */
    long     ra_col;  /* Catalog RA column                */
    long    dec_col;  /* Catalog Dec column               */
    double      res;  /* Resolution in arcseconds         */
    double  ps_size;  /* Postage stamp size               */
    char   *cat_add;  /* Address of catalog               */
    char  *surv_add;  /* Folder containing archive images.*/
    char   *out_add;  /* Folder keeping the cropped images*/
    char   *out_ext;  /* Ending of output file name       */
    long    chk_siz;  /* width of a box to check for zeros*/
    char *info_name;  /* File keeping the results log.    */
    char   *img_pfx;  /* Prefix showing the image files   *
                         in the archive, in other words:  *
                         (replace the *) "##*".           */
};                    /************************************/

/* Function declarations: */
void tifaa_crop_from_survey(struct ArrayInfo *, struct Config *);
void tifaa_read_config(char *, struct Config *);
void free_config(struct Config *);

#endif
