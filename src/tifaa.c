/*********************************************
 *********************************************
 *********************************************
 **          Mohammad Akhalaghi             **
 **   http://astr.tohoku.ac.jp/~akhlaghi/   **
 **              July 2013                  **
 *********************************************
 *********************************************
 *********************************************/
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

