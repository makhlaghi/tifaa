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
printhelp(struct tifaaparams *p)
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
	 " -v:\n\tOnly print version and copyright.\n");


  printf("########### On/Off options (don't use arguments):\n"
	 "########### By default these are off.\n");

  printf(" -e:\n\tVerbose mode, reporting every step.\n");

  printf(" -g:\n\tDelete existing postage stamp folder (if exists).\n");


  printf("########### Options with arguments (mandatory):\n");
  
  printf("-c STRING:\n\tInput catalog name.\n\n"

	 "-i INTEGER:\n\tColumn of object IDs.\n\n"

	 "-r INTEGER:\n\tColumn of object RA.\n\n"

	 "-d INTEGER:\n\tColumn of object DEC.\n\n"

	 "-a FLOAT:\n\tResolution of image (arcseconds/pixel).\n\n"

	 "-p FLOAT:\n\tSize of postage stamp (in arcseconds).\n\n"

	 "-s STRING:\n\tFolder containing survey images.\n\n"

	 "-b STRING:\n\tInput image ending in survey folder.\n"
	 "\tUsually surveys provide both `science` images and `weight`\n"
	 "\timages, with this option you can ask for a specific one,\n"
	 "\tfor example `*sci.fits`.\n\n"

	 "-o STRING:\n\tFolder keeping the postage stamps.\n"
	 "\tNote: if the folder already exists, it will be deleted.\n\n"

	 "-f STRING:\n\tPostage stamp extension. In cases where you want\n"
	 "\tboth the weight image and actual image, you can use this option\n"
	 "\tto specify the difference in the postage stamps.\n"
	 "\tAs an examle if you are cropping weight images, the value for\n"
	 "\tthis option could be: `_w.fits`.\n"

	 "-k INTEGER:\n\tDEFAULT: %lu\n"
	 "\tCheck size in the center of the postage stamp.\n"
	 "\tThe survey images on the sides of the survey are usually\n"
         "\tblank (zero). It might happen that a desired galaxy is in\n"
	 "\tsuch regions. If so, you can specify a check size in the\n"
	 "\tcentral pixels of each postage stamp to see if it is blank or\n"
         "\tnot.\n", p->chk_size);
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
  if(p->id_col == DEFAULTIDCOL)
    { 
      if(numargmissing==0)
	{printversioninfo(); printf("Option(s) not set:\n");}
      printf("\t`-i` (id column).\n"); 
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
  if(p->surv_name == DEFAULTSURVNAME)
    { 
      if(numargmissing==0)
	{printversioninfo(); printf("Option(s) not set:\n");}
      printf("\t`-s` (folder of survey images).\n"); 
      ++numargmissing; 
    } 
  if(p->img_pfx == DEFAULTIMGPFX)
    { 
      if(numargmissing==0)
	{printversioninfo(); printf("Option(s) not set:\n");}
      printf("\t`-b` (survey image name ending).\n"); 
      ++numargmissing; 
    } 
  if(p->out_name == DEFAULTOUTNAME)
    {
      if(numargmissing==0)
	{printversioninfo(); printf("Option(s) not set:\n");}
      printf("\t`-p` (output folder name).\n"); 
      ++numargmissing; 
    } 
  if(p->out_ext == DEFAULTOUTEXT)
    { 
      if(numargmissing==0)
	{printversioninfo(); printf("Option not set:\n");}
      printf("\t`-p` (output folder name).\n"); 
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

  /* Check to see if the survey directory exists. */
  if( (dp=opendir(p->surv_name) )==NULL)
    {
      printf("Error: the survey image directory (%s) cannot be opened.\n\n",
	     p->surv_name);
      exit(0);
    }
  else
    closedir(dp);

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
readinputcatalog(struct tifaaparams *p, struct uiparams *up)
{
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
  exit(0);
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
  char *tailptr;
  struct uiparams up;

  /* Set the default parameter values for a check in the end: */
  up.cat_name  = DEFAULTCATNAME;     p->id_col      = DEFAULTIDCOL;
  p->ra_col    = DEFAULTRACOL;       p->dec_col     = DEFAULTDECCOL;
  p->res       = DEFAULTRES;         p->ps_size     = DEFAULTPSSIZE;
  p->surv_name = DEFAULTSURVNAME;    p->img_pfx     = DEFAULTIMGPFX;
  p->out_name  = DEFAULTOUTNAME;     p->out_ext     = DEFAULTOUTEXT;
  p->chk_size  = 3;                  p->info_name   = "psinfo.txt";
  p->verb      = 0;                  up.delpsfolder = 0;

  while( (c=getopt(argc, argv, "hegva:b:c:d:f:i:k:o:p:r:s:")) 
	 != -1 )
    switch(c)
      {
      /* Info options: */
      case 'h':                 /* Print help.  */
	printhelp(p);
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
      case 'i':	                /* Column number of ID (from 1).*/
	checkiflzero(optarg, &tmp, c);	
	p->id_col=tmp-1;
	break;
      case 'r':	                /* Column number of RA (from 1).*/
	checkiflzero(optarg, &tmp, c);	
	p->ra_col=tmp-1;
	break;
      case 'd':	                /* Column number of DEC (from 1).*/
	checkiflzero(optarg, &tmp, c);	
	p->dec_col=tmp-1;
	break;
      case 'a':			/* Resolution of image. */
	p->res=strtof(optarg, &tailptr);
	break;
      case 'p':			/* Postage stamp size in arcseconds. */
	p->ps_size=strtof(optarg, &tailptr);
	break;
      case 's':			/* Folder containing survey images. */
	p->surv_name=optarg;
	break;
      case 'b':			/* Image prefix in survey folder. */
	p->img_pfx=optarg;
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

  checkifparamtersset(p, &up);
  checkfilesanddirectories(p, &up);
  readinputcatalog(p, &up);
}
