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
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>

#include "tifaa.h"
#include "timing.h"
#include "surveyimginfo.h"




/******************************************************************/
/****************        Stich and crop       *********************/
/******************************************************************/
/* The FITS standard assumes that pixels are counted from their center
   and in integer values. So the bottom left corner of a FITS image
   (in a continues space) has coordinates: (0.5,0.5). In converting
   from WCS to pixel, wcslib gives continues values, this function
   converts that into actual array elements.  */
void 
convert_double_to_long_in_FITS(const double a, long *b)
{
  /* Declarations: */
  *b=(long)a;
  if (a-(*b)>0.5) (*b)++;
}





/* Given the coordiantes of the object (of type double), this function
   finds which pixels of the image correspond to which pixels in the
   cropped image using the *pixel and *pixel_c arrays.

   NOTE: I am going with the cfitsio standard here: the desired region
         boundries are included.*/
void 
find_desired_pixel_range(double *pixcrd, const long naxis1,
        const long naxis2, const long crop_side, long *fpixel_i, 
        long *lpixel_i, long *fpixel_c, long *lpixel_c)
{
  /* Declarations: */
  int hw;
  long lpixcrd[2];
  hw=crop_side/2;

  /* Convert pixcrd from double to long based on the 
     FITS standard: */
  convert_double_to_long_in_FITS(pixcrd[0], &lpixcrd[0]);
  convert_double_to_long_in_FITS(pixcrd[1], &lpixcrd[1]);

  /* Set the initial values for the cropped array: */
  fpixel_c[0]=1;          fpixel_c[1]=1;
  lpixel_c[0]=crop_side;  lpixel_c[1]=crop_side;

  /* Set the initial values for the actual image: */
  fpixel_i[0]=lpixcrd[0]-hw; fpixel_i[1]=lpixcrd[1]-hw;
  lpixel_i[0]=lpixcrd[0]+hw; lpixel_i[1]=lpixcrd[1]+hw;

  /* Check the four corners to see if they should be adjusted:
     To understand the first, look at this, suppose | separates pixels:
     |-2|-1| 0|| 1| 2| 3| 4|  (survey image)
     ||1 | 2| 3|  4| 5| 6| 7|  (crop image)
     the || shows where the image actually begins. So when fpixel_i is
     negative, e.g., fpixel_i=-2, then the pixel in the cropped image 
     we want to begin with, that corresponds to 1 in the survey image is:
     fpixel_c= 4 = 2 + -1*fpixel_i.*/
  if (fpixel_i[0]<0) 
    {    
      fpixel_c[0]=-1*fpixel_i[0]+2;
      fpixel_i[0]=1;
    }
  if (fpixel_i[1]<0) 
    {
      fpixel_c[1]=-1*fpixel_i[1]+2;
      fpixel_i[1]=1; 
    }
  /*The same principle applies to the end of an image. Take "s"
    is the maximum size along a specific axis in the survey image 
    and "c" is the size along the same axis on the cropped image.
    Assume the the cropped region's last pixel in that axis will
    be 2 pixels larger than s:
    |1|.....|s-3|s-2|s-1|s  ||s+1|s+2| (survey image)
    |c-3|c-2| c-1| c ||
    So you see that if the outer pixel is n pixels away then in
    the cropped image we should only look upto c-n.*/
  if (lpixel_i[0]>naxis1) 
    {
      lpixel_c[0]=crop_side-(lpixel_i[0]-naxis1);
      lpixel_i[0]=naxis1;
    }
  if (lpixel_i[1]>naxis2) 
    {
      lpixel_c[1]=crop_side-(lpixel_i[1]-naxis2);
      lpixel_i[1]=naxis2;
    }

  /* In case you wish to see the results (declare "junk"!).
     The +1 in the final size section is because of the
     cfitsio standard. An array of size 3 will have elements 
     {1,2,3}, but the last element subtracted from the first is 
     3-1=2, which doesn't show the number of pixels in it, it 
     has to be increased by a unit! 
     printf(" n1: %ld,  n2: %ld\n", naxis1, naxis2);
     printf("if0: %ld, if1: %ld\n", fpixel_i[0], fpixel_i[1]);
     printf("il0: %ld, il1: %ld\n", lpixel_i[0], lpixel_i[1]);
     printf("cf0: %ld, cf1: %ld\n", fpixel_c[0], fpixel_c[1]);
     printf("cl0: %ld, cl1: %ld\n", lpixel_c[0], lpixel_c[1]);
     printf("\nThe sides of the desired region:\n");
     printf("Num pixels in survey image: (ax1=%ld) * (ax2=%ld)\n", 
     lpixel_i[0]-fpixel_i[0]+1, lpixel_i[1]-fpixel_i[1]+1);
     printf("Num pixels in Cropped image: (ax1=%ld) * (ax2=%ld)\n", 
     lpixel_c[0]-fpixel_c[0]+1, lpixel_c[1]-fpixel_c[1]+1);
     scanf("%d", &junk);*/
}





/* Check the central pixels of the finalized image to see if they
   aren't zero, if they are, set the remove variable on (=1). Note
   that hw+1 is the central pixel. */
void
check_center(fitsfile **write_fptr, long *onaxes, long chk_siz,
	     int imagesdone, size_t *zero_flag, long crop_side, 
	     int *wr_status)
{
  /* Declarations: */
  long fpixel_ch[2], lpixel_ch[2], inc[2]={1,1}, ch_hw;
  float *to_check, *ipt, nulval=-9999;
  int group=0, naxis=2, hw, anynul=0;

  /* Set the positions and array of the pixels to check: */
  hw=crop_side/2; ch_hw=chk_siz/2;
  fpixel_ch[0]=(hw+1)-ch_hw; fpixel_ch[1]=(hw+1)+ch_hw;
  lpixel_ch[0]=(hw+1)-ch_hw; lpixel_ch[1]=(hw+1)+ch_hw;
  assert( (to_check=calloc(chk_siz*chk_siz, sizeof *to_check) )!=NULL );

  /* Get those pixels: */
  fits_read_subset_flt(*write_fptr, group, naxis, onaxes, fpixel_ch, 
		       lpixel_ch, inc, nulval, to_check, &anynul, wr_status);

  /* Check them to see if they are zero or not: */
  for(ipt=to_check;ipt<&to_check[chk_siz*chk_siz];ipt++)
    if (*ipt!=0) break;
  if (ipt==&to_check[chk_siz*chk_siz] && imagesdone>0) 
    *zero_flag=1;

  /* Free the allocated space: */
  free(to_check);
}





/* This function will report the result for each image and set the
   remove_flag */
void 
report_prepare_end(int verb, size_t *log, size_t targetindex, int numimg, 
		   size_t zero_flag, size_t *remove_flag)
{
  if (zero_flag==1)
    {
      if(verb)
	printf("%5lu:   Central region (at least) is zero!\n", targetindex);
      *remove_flag=1;
      log[targetindex*LOG_COLS+2]=1;
    }
  else if (numimg==0)
    {
      if(verb)
	printf("%5lu:   Not in field!\n", targetindex);
      *remove_flag=1;
      log[targetindex*LOG_COLS+2]=2;
    }
  else if (numimg==1)
    {
      if(verb)
	printf("%5lu:   cropped.\n", targetindex);
      log[targetindex*LOG_COLS+2]=0;
    }
  else if (numimg>1)
    {
      if(verb)
	printf("%5lu:   stiched and cropped (%d images).\n",  
	       targetindex, numimg);
      log[targetindex*LOG_COLS+2]=0;
    }
}





void *
stitchcroponthread(void *inparam)
{
  struct stitchcropthread *p=(struct stitchcropthread *)inparam;

  struct wcsprm *wcs;
  pthread_mutex_t *wcsmtxp=p->wm;
  fitsfile *write_fptr, *read_fptr;
  float *cropped, *tmparray, nulval=-9999;
  char *outname=p->tp->out_name, *outext=p->tp->out_ext;
  int stat[NWCSFIX], verb=p->tp->verb, group=0, anynul=0;
  char fitsname[1000], **imgnames=p->tp->survglob.gl_pathv;
  size_t racol=p->tp->ra_col, deccol=p->tp->dec_col, numimg;
  size_t *t, *i, *whichimg=p->tp->whichimg, *log=p->tp->log;
  int wr_status, fr_status, wc_status, nwcs, ncoord=1, nelem=2;
  double world[2], *cat=p->tp->cat, phi, theta, imgcrd[2], pixcrd[2];
  long fpixel_c[2], lpixel_c[2], fpixel_i[2], lpixel_i[2], inc[2]={1,1};
  size_t zero_flag, remove_flag, cs1=p->tp->cs1, crop_side=p->crop_side;
  long onaxes[2], nelements, naxis=2, inaxes[2], chk_size=p->tp->chk_size;

  /* Set the width of the output */
  onaxes[0]=crop_side; onaxes[1]=crop_side;
  nelements=crop_side*crop_side;

  t=&p->targetthrds[p->id*p->thrdcols];
  do
    {
      /* Set the remove and zero flags to zero: */
      zero_flag=0; remove_flag=0;

      /* Get this object's RA and Dec: */
      world[0]=cat[*t*cs1+racol];
      world[1]=cat[*t*cs1+deccol];

      /* Create the fits image for the cropped array here: */
      wr_status=0;
      sprintf(fitsname, "%s%lu%s", outname, *t, outext);
      assert( (cropped=calloc(nelements, sizeof *cropped))!=NULL );
      fits_create_file(&write_fptr, fitsname, &wr_status);
      fits_create_img(write_fptr, FLOAT_IMG, naxis, onaxes, &wr_status);
      fits_write_img(write_fptr, TFLOAT, 1, nelements, cropped, &wr_status);

      /* Go over all the images for this object. */
      numimg=0;      
      i=&whichimg[*t*WI_COLS];
      do
	{
	  /* Prepare wcsprm structure and read the image size.*/
	  fr_status=0; wc_status=0;
	  prepare_fitswcs(imgnames[*i], &read_fptr, &fr_status, 
			  &wc_status, &nwcs, &wcs, wcsmtxp);
	  fits_read_key(read_fptr, TLONG, "NAXIS1", &inaxes[0], 
			NULL, &fr_status);
	  fits_read_key(read_fptr, TLONG, "NAXIS2", &inaxes[1], 
			NULL, &fr_status);

	  /* Find the position of the object's RA and Dec: */
	  wc_status = wcss2p(wcs, ncoord, nelem, world, &phi, 
			     &theta, imgcrd, pixcrd, stat); 

	  /* Find the desired pixel ranges in both the input 
	     and output images. */
	  find_desired_pixel_range(pixcrd, inaxes[0], inaxes[1], crop_side,
				   fpixel_i, lpixel_i, fpixel_c, lpixel_c);

	  /* Make the array to keep the section pixels. The +1
	     on each axis is explained in the explanations of 
	     find_desired_pixel_range, in short it  */
	  tmparray=calloc((lpixel_i[0]-fpixel_i[0]+1)
			  *(lpixel_i[1]-fpixel_i[1]+1), sizeof *tmparray);
	  assert(tmparray!=NULL);

	  /* Read the pixels in the desired subset: */
	  fits_read_subset_flt(read_fptr, group, naxis, inaxes, fpixel_i, 
			       lpixel_i, inc, nulval, tmparray, &anynul, 
			       &fr_status);

	  /* Write that section */
	  fits_write_subset_flt(write_fptr, group, naxis, onaxes, fpixel_c,
				lpixel_c, tmparray, &wr_status);

	  /* Free the spaces: */
	  free(tmparray);
	  fits_close_file(read_fptr, &fr_status);
	  wc_status = wcsvfree(&nwcs, &wcs);
	  ++numimg;
	}
      while(*(++i)!=NONINDEX);
 
      /* Save the necessary information in the process log */
      log[*t*LOG_COLS  ] = *t;
      log[*t*LOG_COLS+1] = numimg;

      /* Check to see if the center of the image is empty or not. */
      check_center(&write_fptr, onaxes, chk_size, numimg, 
		   &zero_flag, crop_side, &wr_status);

      /* Close the FITS file and free the cropped space: */
      fits_close_file(write_fptr, &wr_status);
      fits_report_error(stderr, wr_status);
 
      /* Report the results on stdout and in final_report: */
      report_prepare_end(verb, log, *t, numimg, zero_flag, &remove_flag);

      free(cropped);
    }
  while(*(++t)!=NONINDEX);

  /* Increment the `done` counter and return. */
  pthread_mutex_lock(p->m);
  ++(*p->done);
  pthread_cond_signal(p->c);
  pthread_mutex_unlock(p->m);
  return NULL;
}








void
stitchandcrop(struct tifaaparams *tp)
{
  size_t *targetthrds, thrdcols, crop_side;

  /* Parameters for parallel processing: */
  pthread_t *t;
  pthread_cond_t cv;
  pthread_attr_t attr;
  size_t done, numactive;
  size_t i, nt=tp->numthrd;
  pthread_mutex_t mtx, wcsmtx;
  struct stitchcropthread *p;

  /* Find the size of the output images: */
  crop_side=tp->ps_size / tp->res;
  if (crop_side%2==0) crop_side-=1;

  /* Threads/mutexs/condition variables initialization. */
  pthread_attr_init(&attr);
  pthread_cond_init(&cv, NULL);
  pthread_mutex_init(&mtx, NULL);
  pthread_mutex_init(&wcsmtx, NULL);
  assert( (t=malloc(nt*sizeof *t))!=NULL );
  assert( (p=malloc(nt*sizeof *p))!=NULL );
  pthread_attr_setstacksize(&attr, 10*crop_side*crop_side);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  
  prepindexsinthreads(tp->cs0, nt, &targetthrds, &thrdcols);

  for(i=0;i<nt;++i)
    {
      p[i].id=i; p[i].targetthrds=targetthrds; p[i].thrdcols=thrdcols;
      p[i].tp=tp; p[i].c=&cv; p[i].m=&mtx; p[i].done=&done;
      p[i].wm=&wcsmtx; p[i].crop_side=crop_side;
    }

  /* Initalize `done` and `numactive` for this mesh type. */
  done=numactive=0;

  /* Spin off the threads: */
  for(i=0;i<nt;++i)
    if(targetthrds[i*thrdcols]!=NONINDEX)
      {
	++numactive;
	pthread_create(&t[i], &attr, stitchcroponthread, &p[i]);
      }

  /* Wait for the threads to finish. */
  pthread_mutex_lock(&mtx);
  while(done<numactive)
    pthread_cond_wait(&cv, &mtx);
  pthread_mutex_unlock(&mtx);

  free(p);
  free(t);
  free(targetthrds);
}




















/******************************************************************/
/****************        Main function        *********************/
/******************************************************************/
void
tiffasavelog(struct tifaaparams *p)
{
  FILE *fp;
  char logname[1000];
  size_t i, *log=p->log;
  
  sprintf(logname, "%stifaalog.txt", p->out_name);
  assert( (fp=fopen(logname, "w"))!=NULL );

  fprintf(fp, 
	  "# Final report of cropping the objects:\n"
	  "# Col 0: Object ID\n"
          "# Col 1: Number of images used for this object.\n"
	  "# Col 2: Flag = 0 : No problem\n"
	  "#             = 1 : The central region is zero\n"
	  "#             = 2 : The object was not in the field.\n");
  for(i=0;i<p->cs0;++i)
    fprintf(fp, "%-6lu %-5lu %-5lu\n", log[i*LOG_COLS], 
	    log[i*LOG_COLS+1], log[i*LOG_COLS+2]);

  fclose(fp);
}





void
tifaa(struct tifaaparams *p)
{
  char report[100];
  struct timeval t1;

  /* Get the image information. */
  if(p->verb) gettimeofday(&t1, NULL);
  getsurveyimageinfo(p);
  if(p->verb) 
    {
      sprintf(report, "WCS info of %lu image(s) has been read.", 
	      (size_t)(p->survglob.gl_pathc));
      reporttiming(&t1, report, 1);
    }

  /* Find which image is needed for which object. */
  if(p->verb) gettimeofday(&t1, NULL);
  whichimageforwhichtargets(p);
  if(p->verb) reporttiming(&t1, "Target/image correspondance found.", 1);

  /* Stitch or crop the targets out of the images. */
  if(p->verb) gettimeofday(&t1, NULL);
  stitchandcrop(p);
  if(p->verb) reporttiming(&t1, "All targets stitched or cropped.", 1);

  tiffasavelog(p);
}
