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
#include <math.h>
#include <glob.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>

#include "tifaa.h"
#include "surveyimginfo.h"


/* We have `nimgs` images and we want their indexs to be divided
   between `nthrds` CPU threads. This function will give each image
   index to a thread such that the maximum difference between the
   number of images for each thread is 1. The results will be saved in
   a 2D array of `outlabthrdcols` columns and each row will finish
   with a NONINDEX (see surveyimginfo.h). */
void
prepindexsinthreads(size_t nindexs, size_t nthrds, size_t **outthrds,
		    size_t *outthrdcols)
{
  size_t *sp, *fp;
  size_t i, *thrds, thrdcols;
  *outthrdcols = thrdcols = nindexs/nthrds+2;
  assert( (thrds=*outthrds=malloc(nthrds*thrdcols*sizeof *thrds))!=NULL );
  
  /* Initialize all the elements to NONINDEX. */
  fp=(sp=thrds)+nthrds*thrdcols;
  do *sp=NONINDEX; while(++sp<fp);

  /* Distribute the labels in the threads.  */
  for(i=0;i<nindexs;++i)
    thrds[ (i%nthrds)*thrdcols+(i/nthrds) ] = i;

  /*
  for(i=0;i<nthrds;++i)
    {
      size_t j;
      printf("\n\n############################\n");
      printf("THREAD %lu: \n", i);
      for(j=0;thrds[i*thrdcols+j]!=NONINDEX;j++)
	printf("%lu, ", thrds[i*thrdcols+j]);
      printf("\b\b.\n");
    }
  exit(0);
  */
}





/* This function will open a FITS file, read the header and output a
 prepared wcsprm
                                         
 Don't forget to free the space after it: 

    status = wcsvfree(&nwcs,&wcs); */
void 
prepare_fitswcs(char *fits_name, fitsfile **fptr, int *f_status, 
		int *w_status, int *nwcs, struct wcsprm **wcs,
		pthread_mutex_t *wm)
{
  /* Declaratins: */
  int nkeys=0, relax, ctrl, nreject;
  char *header;

  /********************************************
   ***********   CFITSIO functions:  **********
   ***********   To read the header  **********
   ********************************************/
  fits_open_file(fptr, fits_name, READONLY, f_status);
  fits_hdr2str(*fptr, 1, NULL, 0, &header, &nkeys, f_status);
  if (*f_status!=0)
    {
      fits_report_error(stderr, *f_status);
      exit(EXIT_FAILURE);
    }
 
  /********************************************
   ***********   WCSLIB functions:   **********
   ********* To get the central RA,Dec ********
   ********************************************/
  relax    = WCSHDR_all; /* A macro, to use all informal WCS extensions. */
  ctrl     = 0;          /* Don't report why a keyword wasn't used. */
  nreject  = 0;          /* Number of keywords rejected for syntax. */
  pthread_mutex_lock(wm);
  *w_status = wcspih(header, nkeys, relax, ctrl, &nreject, nwcs, wcs);
  pthread_mutex_unlock(wm);
  if (*w_status!=0)
    {
      fprintf(stderr, "wcspih ERROR %d: %s.\n", *w_status, 
	      wcs_errmsg[*w_status]);
      exit(EXIT_FAILURE);
    }
  free(header);

  /* Initialize the wcsprm struct */
  if ((*w_status = wcsset(*wcs))) 
    {
      fprintf(stderr, "wcsset ERROR %d: %s.\n", *w_status, 
	      wcs_errmsg[*w_status]);
      exit(EXIT_FAILURE);
    }
}





/* This function will save the following information for all the images:
   Column 0: RA of image center.
   Column 1: Dec of image center.
   Column 2: Half width of image (in RA).
   Column 3: Half height of image (in Dec).*/
void
get_imginfo(char *fits_name, double *imginfo, unsigned long zero_pos, 
	    const double res, pthread_mutex_t *wm)
{
  /* For prepare_fitswcs: */
  int nwcs=0, f_status=0, w_status=0;
  struct wcsprm *wcs;
  fitsfile *fptr;

  /* For converting coordinates, note that here we just want to
     convert one point, so ncoord=1, if you want more than one point,
     change this to that number.*/
  int stat[NWCSFIX];
  double phi, theta, pixcrd[2], imgcrd[2], world[2];
  int ncoord=1, nelem=2;

  /* For this function */
  double naxis1, naxis2;

  /* Prepare wcsprm structure: */
  prepare_fitswcs(fits_name, &fptr, &f_status, 
		  &w_status, &nwcs, &wcs, wm);
  fits_read_key(fptr, TDOUBLE, "NAXIS1", &naxis1, NULL, &f_status);
  fits_read_key(fptr, TDOUBLE, "NAXIS2", &naxis2, NULL, &f_status);

  /* Find the central position of the image: */
  pixcrd[0]=naxis1/2; pixcrd[1]=naxis2/2;

  /* Explanations about the parameters in wcsp2s: 

     ncoord: Number of points you want to transform. 
     nelem:  Number of axises for each point. 
     imgcrd: Intermediate coordinates, keep it empty
     phi, theta: Intermediate world coordinates
     world: final world coordinates, 

     keep all of these the same as the declarations as long as you want
     a conversion for one point, for more than one point, just change
     ncoord. */
  w_status = wcsp2s(wcs, ncoord, nelem, pixcrd, imgcrd, 
		    &phi, &theta, world, stat);
  if (w_status!=0)
    {
      fprintf(stderr, "wcsp2s ERROR %d: %s.\n", 
	      w_status, wcs_errmsg[w_status]);
      exit(EXIT_FAILURE);
    }

  /* Free the spaces: */
  w_status = wcsvfree(&nwcs, &wcs);
  fits_close_file(fptr, &f_status);

  /* Save the results into the table. About the last two values: It
     is important to know how far (in degrees) the sides of the images
     are to their center. So here I find the angular distance of half
     the width of the image in each axis (in degrees). */
  imginfo[zero_pos  ] = world[0];
  imginfo[zero_pos+1] = world[1];
  imginfo[zero_pos+2] = naxis1/7200*res; /* 7200=2*3600! */
  imginfo[zero_pos+3] = naxis2/7200*res;
}




















/********************************************************************/
/*****************      Image information      **********************/
/********************************************************************/
void *
imginfothreads(void *inparams)
{
  struct imginfothreadparams *p= (struct imginfothreadparams *)inparams;
  char **imgnames=p->imgnames;
  size_t i, *imgs;

  /* Pull out the row of image indexs for this thread. */
  imgs=&p->imgthrds[p->id*p->thrdcols];

  for(i=0;imgs[i]!=NONINDEX;++i)
    get_imginfo(imgnames[imgs[i]], p->imginfo, 
		imgs[i]*NUM_IMAGEINFO_COLS, p->res, p->wm);

  /* Increment the `done` counter and return. */
  pthread_mutex_lock(p->m);
  ++(*p->done);
  pthread_cond_signal(p->c);
  pthread_mutex_unlock(p->m);
  return NULL;
}





void
getsurveyimageinfo(struct tifaaparams *tp)
{
  size_t *imgthrds, thrdcols;
  size_t nimgs=tp->survglob.gl_pathc;
  char **imgnames=tp->survglob.gl_pathv;

  /* Parameters for parallel processing: */
  pthread_t *t;
  pthread_cond_t cv;
  pthread_attr_t attr;
  size_t done, numactive;
  size_t i, nt=tp->numthrd;
  pthread_mutex_t mtx, wcsmtx;
  struct imginfothreadparams *p;

  /* Threads/mutexs/condition variables initialization. */
  pthread_attr_init(&attr);
  pthread_cond_init(&cv, NULL);
  pthread_mutex_init(&mtx, NULL);
  pthread_mutex_init(&wcsmtx, NULL);
  assert( (t=malloc(nt*sizeof *t))!=NULL );
  assert( (p=malloc(nt*sizeof *p))!=NULL );
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  
  prepindexsinthreads(nimgs, nt, &imgthrds, &thrdcols);

  for(i=0;i<nt;++i)
    {
      p[i].id=i; p[i].imgthrds=imgthrds; p[i].thrdcols=thrdcols;
      p[i].imgnames=imgnames; p[i].imginfo=tp->imginfo;
      p[i].res=tp->res; p[i].c=&cv; p[i].m=&mtx; p[i].done=&done;
      p[i].wm=&wcsmtx;
    }

  /* Initalize `done` and `numactive` for this mesh type. */
  done=numactive=0;

  /* Spin off the threads: */
  for(i=0;i<nt;++i)
    if(imgthrds[i*thrdcols]!=NONINDEX)
      {
	++numactive;
	pthread_create(&t[i], &attr, imginfothreads, &p[i]);
      }

  /* Wait for the threads to finish. */
  pthread_mutex_lock(&mtx);
  while(done<numactive)
    pthread_cond_wait(&cv, &mtx);
  pthread_mutex_unlock(&mtx);

  free(p);
  free(t);
  free(imgthrds);

  /* In case you want to see the image information: 
  for(i=0;i<nimgs;i++)
    printf("%lu: %f %f %f %f\n", i, 
	   p->imginfo[i*NUM_IMAGEINFO_COLS  ], 
	   p->imginfo[i*NUM_IMAGEINFO_COLS+1], 
	   p->imginfo[i*NUM_IMAGEINFO_COLS+2],
	   p->imginfo[i*NUM_IMAGEINFO_COLS+3]);
  */
}




















/********************************************************************/
/*****************      Targets in images      **********************/
/********************************************************************/
/* This function will look at the 4 corners of a square that has been
   identified by the user around the object's RA and Dec. It will then
   find the images that contain at least one of those points.*/
void 
whichimageforwhichtargets(struct tifaaparams *p)
{
  /* Declarations: */
  size_t imindex;
  double *cat=p->cat;
  double hswr, hswd, decr, *po, *pof, *im, *imf;
  double points[8], ra, dec, *imginfo=p->imginfo; 
  size_t cs1=p->cs1, racol=p->ra_col, deccol=p->dec_col;
  size_t i, j, *whichimg=p->whichimg, counter, cs0=p->cs0;

  /* Set the half side width in degrees and radians, they are done
     here to simplify the positioning step in the loop. Notice that
     conf->ps_size was in arcseconds.*/
  hswd=p->ps_size/7200;
  hswr=hswd*M_PI/180; 
  
  /* To simplify the loop to check all images for a target. */
  imf=imginfo+((size_t)(p->survglob.gl_pathc)*NUM_IMAGEINFO_COLS);
  pof=points+8;

  /* Go over all the objects and find the images that 
     contain all or part of the desired region around it. */
  for(i=0;i<cs0;++i)
    {
      /* For simplification of the points below: */
      ra   = cat[i*cs1+racol];
      dec  = cat[i*cs1+deccol];
      decr = dec*M_PI/180;

      /* Define the 4 surrounding points, in order they are:*/
      points[0]=ra+hswd/cos(decr-hswr); points[1]=dec-hswd; /*Bottom left */
      points[2]=ra-hswd/cos(decr-hswr); points[3]=dec-hswd; /*Bottom right*/
      points[4]=ra+hswd/cos(decr+hswr); points[5]=dec+hswd; /*Top left    */
      points[6]=ra-hswd/cos(decr+hswr); points[7]=dec+hswd; /*Top right   */

      /* Each target has 4 points around it. See which images contains
	 which point. NOTE: For each point: pRA=*po, pDec=*(po+1) */
      counter=0;
      po=points;
      do
        {
	  /* NOTE: For each image (ic: image center): 
	     icRA=*im, icDec=*(im+1), img_ax1_half_width=*(im+2), 
	     img_ax2_half_width=*(im+3).*/
	  im=imginfo;
	  do
            {
	      /* First make sure declination is in range, then RA. 
		 `im` is the row of imageinfo that we are now looking at.
	         `po` is the two coordinates of each side.`*/
	      if(    po[1] <= im[1]+im[3] 
		  && po[1] >  im[1]-im[3] 
		  && po[0] <= im[0]+im[2]/cos(po[1]*M_PI/180)
		  && po[0] >  im[0]-im[2]/cos(po[1]*M_PI/180) )
                {
		  imindex=(im-imginfo)/NUM_IMAGEINFO_COLS;
		  for(j=0;j<counter;++j)
		    if(whichimg[i*WI_COLS+j]==imindex)
		      break;
		  if(j==counter) /* Image not yet assigned for target. */
		    whichimg[i*WI_COLS+counter++]=imindex;

		  /* Break out of the search, if there are no overlaps 
		     then eachpoint can only be in one image, if there are
		     overlaps then it doesn't matter! We have one image that
		     some of the pixels are in, we don't need two!*/
		  break;
                }
	      im+=NUM_IMAGEINFO_COLS;
            }
	  while(im<imf);
	  po+=2;
        }
      while(po<pof);
    }

  /* In case you want to see the table: 
  {
    size_t j;
    for(i=0;i<cs0;i++)
      {
	printf("%lu: ", i);
	for(j=0;whichimg[i*WI_COLS+j]!=NONINDEX;j++)
	  printf("%lu, ", whichimg[i*WI_COLS+j]);
	printf("\b\b.\n");
      }
    exit(0);
  }
  */
}
