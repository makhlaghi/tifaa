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
along with this program. If not, see <http://www.gnu.org/licenses/>.

**********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "attaavv.h"
#include "tifaa_code.h"

int 
main (int argc, char *argv[])
{
    /* Declare structures */
    struct ArrayInfo intable;
    struct Config conf;

    /* Check if there is only one argument: */
    if (argc!=2)
    {
        printf("\n   ### Error: Only one option should be used.\n");
        printf("              (configuration file name)\n\n");
        exit(EXIT_FAILURE);
    }

    /* Prepare the necessary information:*/
    tifaa_read_config(argv[1], &conf);
    readasciitable(conf.cat_add, &intable);
    tifaa_crop_from_survey(&intable, &conf);

    /* Free all the allocated space: */
    free_config(&conf);
    freeasciitable(&intable);

    /* Return 0 for success.*/
    return 0;
}

