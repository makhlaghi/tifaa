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
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "attaavv.h"
#include "tifaa.h"
#include "ui.h"



/*************************************************************/
/***************    Print version and help   *****************/
/*************************************************************/
/* s1_e0 tells it if it is printing the dashes at the start or the
   end of the report. */
void
printdashes(int s1_e0)
{
  size_t i=0;
  if(s1_e0) printf("\n\n");
  while(i++<NUMDASHES) printf("-");
  if(!s1_e0) printf("\n\n");
  else printf("\n");
}





void
printversioninfo()
{
  printf("\n\nTIFAA %s\n"
	 "============\n"
	 "Cut out and stich postage stamps from surveys.\n"
	 "\nCopyright (C) 2014  Mohammad Akhlaghi\n"
	 "This program comes with ABSOLUTELY NO WARRANTY.\n"
	 "This is free software, and you are welcome to\n"
	 "modify and redistribute it under the\n"
	 "GNU Public License v3 or later.\n\n\n", TIFFAVERSION);
}





void
printhelp(struct tifaaparams *p, struct uiparams *up)
{
  printversioninfo();
  printf("\nOptions are classified into three groups:\n"
	 "  1. Won't run NoiseChisel.\n"
	 "  2. Not needing arguments (on/off switches). \n"
	 "  3. Needing arguments.\n\n\n");

  printf("########### Options that won't run TIFAA:\n"
	 " -h:\n\tPrint this help message.\n"
	 "\tNote that if any option values are set prior to '-h'\n"
	 "\tTheir values will be shown as default values.\n"
	 "\tTherefore, it is best to run this option alone.\n"
	 " -v:\n\tOnly print version and copyright.\n\n");


  printf("########### On/Off options (don't use arguments):\n"
	 "########### By default these are off.\n");

  printf(" -e:\n\tVerbose mode, reporting every step.\n");

  printf(" -g:\n\tDelete existing postage stamp folder (if exists).\n\n");


  printf("########### Options with arguments (mandatory):\n");
  
  printf("-c STRING:\n\tInput catalog name.\n\n"

	 "-r INTEGER:\n\tColumn of object RA (counting starts from 0)\n\n"

	 "-d INTEGER:\n\tColumn of object DEC (counting starts from 0).\n\n"

	 "-a FLOAT:\n\tResolution of image (arcseconds/pixel).\n\n"

	 "-p FLOAT:\n\tSize of postage stamp (in arcseconds).\n\n"

	 "-s STRING:\n\tWild card based survey image names.\n"
	 "\tFor example if your survey images are in the directory\n"
	 "\t`/SURVEY/` and all your survey images end in `sci.fits`\n"
	 "\tthen the value for this option would be: `/SURVEY/*sci.fits`.\n\n"

	 "-t FLOAT:\n\tDEFAULT: %.2f\n"
	 "\tThe number of threads you want TIFAA to use. The given value\n"
	 "\twill be multiplied with your environment variable `NCORES`,\n"
	 "\twhich is the number of threads your operating system has\n"
	 "\tavailable. In case you want to only use one thread, set this"
	 "\toption to 0.\n\n"

	 "-o STRING:\n\tDEFAULT: `%s`\n"
	 "\tFolder keeping the postage stamps.\n"
	 "\tNote that it has to end with a slash (`/`). It will be created\n"
	 "\tif it doesn't exist. If it does exist, the `-g` option decides\n"
	 "\tthe fate of the existing folder. Any image names that already\n"
	 "\texist in this folder and have the same name as the those that\n"
	 "\tTIFFA will produce will be deleted.\n\n"

	 "-f STRING:\n\tDEFAULT: `%s`\n"
	 "\tPostage stamp extension. In cases where you want\n"
	 "\tboth the weight image and actual image, you can use this option\n"
	 "\tto specify the difference in the postage stamps.\n"
	 "\tAs an examle if you are cropping weight images, the value for\n"
	 "\tthis option could be: `_w.fits`.\n\n"

	 "-k INTEGER:\n\tDEFAULT: %lu\n"
	 "\tCheck size in the center of the postage stamp.\n"
	 "\tThe survey images on the sides of the survey are usually\n"
         "\tblank (zero). It might happen that a desired galaxy is in\n"
	 "\tsuch regions. If so, you can specify a check size in the\n"
	 "\tcentral pixels of each postage stamp to see if it is blank or\n"
         "\tnot.\n\n", up->thrdmultip, p->out_name, p->out_ext, p->chk_size);
}




















/*************************************************************/
/************   Check and apply the parameters   *************/
/*************************************************************/
void
checkifparamtersset(struct tifaaparams *p, struct uiparams *up)
{
  int numargmissing=0;

  if(up->cat_name == DEFAULTCATNAME)
    { 
      if(numargmissing==0)
	{printversioninfo(); printf("Option(s) not set:\n");}
      printf("\t`-c` (catalog name).\n"); 
      ++numargmissing; 
    }
  if(p->ra_col == DEFAULTRACOL)
    { 
      if(numargmissing==0)
	{printversioninfo(); printf("Option(s) not set:\n");}
      printf("\t`-r` (RA column).\n"); 
      ++numargmissing; 
    }
  if(p->dec_col == DEFAULTDECCOL)
    { 
      if(numargmissing==0)
	{printversioninfo(); printf("Option(s) not set:\n");}
      printf("\t`-d` (DEC column).\n"); 
      ++numargmissing; 
    }
  if(p->res == DEFAULTRES)
    { 
      if(numargmissing==0)
	{printversioninfo(); printf("Option(s) not set:\n");}
      printf("\t`-a` (image resolution).\n"); 
      ++numargmissing; 
    } 
  if(p->ps_size == DEFAULTPSSIZE)
    { 
      if(numargmissing==0)
	{printversioninfo(); printf("Option(s) not set:\n");}
      printf("\t`-p` (postage stamp size) not set.\n"); 
      ++numargmissing; 
    } 
  if(up->surv_name == DEFAULTSURVNAME)
    { 
      if(numargmissing==0)
	{printversioninfo(); printf("Option not set:\n");}
      printf("\t`-s` (wild card of survey images).\n"); 
      ++numargmissing; 
    } 
  if(numargmissing)
    {
      printf("\nError: %d required option%s(above) not provided.\n\n", 
	     numargmissing, numargmissing > 1 ? "s " : " " );
      exit(EXIT_FAILURE);
    }
}





void
checkfilesanddirectories(struct tifaaparams *p, struct uiparams *up)
{
  DIR *dp;
  FILE *fp;
  char command[10000];

  /* Check if the input catalog exists. */
  if( ( fp=fopen(up->cat_name, "r") )==NULL)
    {
      printf("Error: Cannot open provided catalog file: %s\n\n", 
	     up->cat_name);
      exit(EXIT_FAILURE);
    }
  else
    fclose(fp);

  /* Check the postage stamp directory. If it is asked to delete it,
     then do so, if not, just make sure it is there and everything is
     fine. If it is not there, make it. */
  if( (dp=opendir(p->out_name) )==NULL)
    {
      sprintf(command, "mkdir %s", p->out_name);
      system(command);
    }
  else
    {
      closedir(dp);
      if(up->delpsfolder)
	{
	  sprintf(command, "rm -rf %s", p->out_name);
	  system(command);
	  sprintf(command, "mkdir %s", p->out_name);
	  system(command);
	}
    }
  
}





void
readinputcatalogandimgnames(struct tifaaparams *p, struct uiparams *up)
{
  int globout;
  struct ArrayInfo ai;
  
  readasciitable(up->cat_name, &ai);

  p->cat=ai.d;
  p->cs0=ai.s0;
  p->cs1=ai.s1;
  assert( (ai.d=malloc(10*sizeof *ai.d))!=NULL );
  freeasciitable(&ai);

  /* In case you want to check the read array:
  {
    size_t i,j;
    for(i=0;i<p->cs0;++i)
      {
	for(j=0;j<p->cs1;++j)
	  printf("%.2f ", p->cat[i*p->cs1+j]);
	printf("\n");
      }
  }
  */

  /* Successful result will be zero, so if it is not successful, it
     will output a non-zero value. */
  globout=glob(up->surv_name, GLOB_NOSORT, NULL, &p->survglob);
  if(globout)
    {
      printf("\n\nError in expanding the given wildcard:\n%s\n", 
	     up->surv_name);
      if (globout==GLOB_ABORTED)
	printf("----The directory could not be opened.");
      else if(globout==GLOB_NOMATCH)
	printf("----There were no matches\n\n");
      else if (globout==GLOB_NOSPACE)
	printf("----Not enough space to allocate the names.\n\n");
      exit(EXIT_FAILURE);
    } 

  /* Incase you want to see the results:
  {
    size_t i;
    printf("gl_pathc: %lu\n", (size_t)p->survglob.gl_pathc);
    for(i=0;i<(size_t)p->survglob.gl_pathc;i++)
      printf("%lu: %s\n", i, p->survglob.gl_pathv[i]);
  }
  */
}





void
allocateinternalarrays(struct tifaaparams *p)
{
  size_t numimg, *sp, *fp;

  /* Allocate the array to keep all the image information. */
  numimg=p->survglob.gl_pathc;
  p->imginfo=malloc(numimg*NUM_IMAGEINFO_COLS*sizeof *p->imginfo);
  assert(p->imginfo!=NULL);

  /* Allocate and initialize the array to keep the image indexs that
     are needed for every target in the catalog (so it has to have the
     same number of rows as the catalog, but with WI_COLS columns. */
  p->whichimg=malloc(p->cs0*WI_COLS*sizeof *p->whichimg);
  assert(p->whichimg!=NULL);
  fp=(sp=p->whichimg)+p->cs0*WI_COLS;
  do *sp=NONINDEX; while(++sp<fp);

  /* Allocate space for the log table (showing the final status of
     each target's postage stamp. */
  assert( ( p->log=calloc(p->cs0*LOG_COLS, sizeof *p->log) )!=NULL );
}

















/*************************************************************/
/***************  Read the input parameters  *****************/
/*************************************************************/
void
checkifelzero(char *optarg, int *var, int opt)
{
  long tmp;
  char *tailptr;
  tmp=strtol(optarg, &tailptr, 0);
  if(tmp<0)
    {
      printf("\n\n Error: argument to -%c ", opt); 
      printf("should be >=0, it is: %ld\n\n", tmp);
      exit(EXIT_FAILURE);
    }
  *var=tmp;  
}





void
checkiflzero(char *optarg, int *var, int opt)
{
  long tmp;
  char *tailptr;
  tmp=strtol(optarg, &tailptr, 0);
  if(tmp<=0)
    {
      printf("\n\n Error: argument to -%c ", opt); 
      printf("should be >0, it is: %ld\n\n", tmp);
      exit(EXIT_FAILURE);
    }
  *var=tmp;  
}





void
setparams(int argc, char *argv[], struct tifaaparams *p)
{
  int c, tmp;
  struct uiparams up;
  char *tailptr, *envvalue;

  /* Set the default parameter values for a check in the end: */
  up.cat_name   = DEFAULTCATNAME;     p->ra_col    = DEFAULTRACOL;       
  p->dec_col    = DEFAULTDECCOL;      p->res       = DEFAULTRES;         
  p->ps_size    = DEFAULTPSSIZE;      up.surv_name = DEFAULTSURVNAME;    
  p->out_name   = "./PS/";            p->out_ext     = ".fits";          
  p->chk_size   = 3;                  p->info_name   = "psinfo.txt";     
  p->verb       = 0;                  up.delpsfolder = 0;                
  up.thrdmultip = 0;

  while( (c=getopt(argc, argv, "hegva:c:d:f:k:o:p:r:s:t:")) 
	 != -1 )
    switch(c)
      {
      /* Info options: */
      case 'h':                 /* Print help.  */
	printhelp(p, &up);
	exit(EXIT_SUCCESS);
	break;
      case 'v':                 /* Print version.  */
	printversioninfo();
	exit(EXIT_SUCCESS);
	break;

      /* No argument options: */
      case 'e':	                /* Verbose mode. */
	p->verb=1;
	break;
      case 'g':
	up.delpsfolder=1;
	break;


      /* Options with arguments: */
      case 'c':	                /* Input catalog name */
	up.cat_name=optarg;
	break;
      case 'r':	                /* Column number of RA (from 0).*/
	checkiflzero(optarg, &tmp, c);	
	p->ra_col=tmp;
	break;
      case 'd':	                /* Column number of DEC (from 0).*/
	checkiflzero(optarg, &tmp, c);	
	p->dec_col=tmp;
	break;
      case 'a':			/* Resolution of image. */
	p->res=strtof(optarg, &tailptr);
	break;
      case 'p':			/* Postage stamp size in arcseconds. */
	p->ps_size=strtof(optarg, &tailptr);
	break;
      case 's':			/* Folder containing survey images. */
	up.surv_name=optarg;
	break;
      case 't':
	up.thrdmultip=strtof(optarg, &tailptr);;
	break;
      case 'o':			/* Folder keeping cropped images. */
	p->out_name=optarg;
	break;
      case 'f':			/* ending of output file name. */
	p->out_ext=optarg;
	break;
      case 'k':
	checkifelzero(optarg, &tmp, c);	
	p->chk_size=tmp;
	break;


      /* Unrecognized options: */
      case '?':
	fprintf(stderr, "Unknown option: '-%c'.\n\n", optopt);
	exit(EXIT_FAILURE);
      default:
	abort();
      }

  /* Set the default number of threads based on the NCORES environment
     variable. The number of meshes, if p->mesh.numthrd==0, then only
     use one thread. */
  if(up.thrdmultip!=0.0f)
    {
      envvalue=getenv("NCORES");
      checkifelzero(envvalue, &tmp, ' '); /* Never smaller than zero! */
      p->numthrd = (float) tmp * up.thrdmultip;
    }
  else
    p->numthrd=1;

  checkifparamtersset(p, &up);
  checkfilesanddirectories(p, &up);
  readinputcatalogandimgnames(p, &up);
  allocateinternalarrays(p);
}






/* Free all the allocated arrays in tifaaparams. */
void
freeparams(struct tifaaparams *p)
{
  free(p->cat);
  free(p->log);
  free(p->imginfo);
  free(p->whichimg);
  globfree(&p->survglob);
}
