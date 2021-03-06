/*********************************************************************
tifaa - Thumbnail images from astronomical archives
A simple set of functions to crop thumbnails from astronomical archives.

Copyright (C) 2013-2014 Mohammad Akhlaghi
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
#include <assert.h>
#include <string.h>
#include <unistd.h>
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
         boundries are included.

   The four long arrays are:

   fpixel_i: First pixel in the input (large) image.
   lpixel_i: Last pixel in the input (large) image.
   fpixel_c: First pixel in the cropped image.
   lpixel_c: Last pixel in the cropped image. 
*/
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
	printf("%5lu:   Central region (at least) is zero!\n", targetindex+1);
      *remove_flag=1;
      log[targetindex*LOG_COLS+2]=1;
    }
  else if (numimg==0)
    {
      if(verb)
	printf("%5lu:   Not in field!\n", targetindex+1);
      *remove_flag=1;
      log[targetindex*LOG_COLS+2]=2;
    }
  else if (numimg==1)
    {
      if(verb)
	printf("%5lu:   cropped.\n", targetindex+1);
      log[targetindex*LOG_COLS+2]=0;
    }
  else if (numimg>1)
    {
      if(verb)
	printf("%5lu:   stiched and cropped (%d images).\n",  
	       targetindex+1, numimg);
      log[targetindex*LOG_COLS+2]=0;
    }
}





void
addheaderinfo(fitsfile *write_fptr, int *wr_status, struct wcsprm *wcs,
	      long *fpixel_i, long *fpixel_c, double *world, 
	      double ps_size, double res)
{
  size_t i;
  time_t rawtime;
  int nkeyrec, h;
  char comment[1000];
  char startblank[]="                   / ";
  char *wcsheader, *cp, *cpf, blankrec[80], titlerec[80];

  time(&rawtime);

  /* Set the last element of the blank array. */
  cpf=blankrec+79;
  *cpf='\0';
  titlerec[79]='\0';
  cp=blankrec; do *cp=' '; while(++cp<cpf);

  /* Delete the comments that already exist: */
  fits_delete_key(write_fptr, "COMMENT", wr_status);
  fits_delete_key(write_fptr, "COMMENT", wr_status);

  /* Add the WCS information: */
  fits_write_record(write_fptr, blankrec, wr_status);
  sprintf(titlerec, "%sWCS INFORMATION", startblank);
  titlerec[strlen(titlerec)]=' ';
  fits_write_record(write_fptr, titlerec, wr_status);
  wcs->crpix[0] -= (fpixel_i[0]-1)+(fpixel_c[0]-1);
  wcs->crpix[1] -= (fpixel_i[1]-1)+(fpixel_c[1]-1);
  wcshdo(0, wcs, &nkeyrec, &wcsheader);
  for(h=0;h<nkeyrec-1;++h)
    {
      cp=&wcsheader[h*80];
      wcsheader[(h+1)*80-1]='\0';
      fits_write_record(write_fptr, cp, wr_status);
    }
  free(wcsheader);

  /*Print all the other information in the header:  */
  fits_write_record(write_fptr, blankrec, wr_status);
  sprintf(titlerec, "%sABOUT THIS THUMBNAIL", startblank);
  for(i=strlen(titlerec);i<79;++i)
    titlerec[i]=' ';
  fits_write_record(write_fptr, titlerec, wr_status);  
  sprintf(comment, "Created with TIFAA %s on %s.", 
	  TIFFAVERSION, ctime(&rawtime));
  fits_write_comment(write_fptr, comment, wr_status);
  sprintf(comment, "RA  of thumbnail center: %13f", world[0]);
  fits_write_comment(write_fptr, comment, wr_status);
  sprintf(comment, "DEC of thumbnail center: %13f", world[1]);
  fits_write_comment(write_fptr, comment, wr_status);
  sprintf(comment, "Thumbnail is %.2f arcseconds across with %.3f "
	  "arcsecond/pixel.", ps_size, res);
  fits_write_comment(write_fptr, comment, wr_status);

  /* Copyright information */
  fits_write_record(write_fptr, blankrec, wr_status);
  sprintf(titlerec, "%sABOUT TIFAA", startblank);
  for(i=strlen(titlerec);i<79;++i)
    titlerec[i]=' ';
  fits_write_record(write_fptr, titlerec, wr_status);
  sprintf(comment, "TIFAA is available under the GNU GPL v3+.");
  fits_write_comment(write_fptr, comment, wr_status);
  sprintf(comment, "https://github.com/makhlaghi/tifaa");
  fits_write_comment(write_fptr, comment, wr_status);
  sprintf(comment, "Copyright 2013-2014, Mohammad Akhlaghi.");
  fits_write_comment(write_fptr, comment, wr_status);
  sprintf(comment, "http://www.astr.tohoku.ac.jp/~akhlaghi/");
  fits_write_comment(write_fptr, comment, wr_status);
}






void *
stitchcroponthread(void *inparam)
{
  struct stitchcropthread *p=(struct stitchcropthread *)inparam;
  struct tifaaparams *tp=p->tp;

  int wwc_stat;
  struct wcsprm *wcs;
  pthread_mutex_t *wcsmtxp=p->wm;
  char **whtnames=tp->wsurvglob.gl_pathv;
  fitsfile *write_fptr, *read_fptr, *wread_fptr;
  size_t racol=tp->ra_col, deccol=tp->dec_col, numimg;
  int stat[NWCSFIX], verb=tp->verb, group=0, anynul=0;
  char fitsname[1000], **imgnames=tp->survglob.gl_pathv;
  size_t *t, *i, *whichimg=tp->whichimg, *log=tp->log, tmpsize;
  int wr_status, fr_status, wc_status, nwcs, ncoord=1, nelem=2;
  char *outname=tp->out_name, *outext=tp->out_ext, *fullheader;
  float *cropped, *tmparray, nulval=-9999, *wtmp, *wf, *wff, *sf;
  double world[2], *cat=tp->cat, phi, theta, imgcrd[2], pixcrd[2];
  size_t zero_flag, remove_flag, cs1=tp->cs1, crop_side=p->crop_side;
  long onaxes[2], nelements, naxis=2, inaxes[2], chk_size=tp->chk_size;
  long fpixel_c[2], lpixel_c[2], fpixel_i[2], lpixel_i[2], inc[2]={1,1};

  /* Set the width of the output */
  onaxes[0]=crop_side; onaxes[1]=crop_side;
  nelements=crop_side*crop_side;

  t=&p->targetthrds[p->id*p->thrdcols];
  do
    {
      /* In case this object doesn't exist in the image range, ignore it. */
      if(whichimg[*t*WI_COLS]==NONINDEX) continue;

      /* Set the remove and zero flags to zero: */
      zero_flag=0; remove_flag=0;

      /* Get this object's RA and Dec: */
      world[0]=cat[*t*cs1+racol];
      world[1]=cat[*t*cs1+deccol];

      /* Create the fits image for the cropped array here: */
      wr_status=0;
      sprintf(fitsname, "%s%lu%s", outname, *t+1, outext);
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
	  prepare_fitswcs(imgnames[*i], &read_fptr, &fr_status, &wc_status, 
			  &nwcs, &wcs, wcsmtxp, &fullheader);
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

	  /* In case you want to multiply by the weight image: */
	  if(tp->weightmultip)
	    {			/* See the comments of what is in `else`. */
	      wwc_stat=0;
	      fits_open_file(&wread_fptr, whtnames[*i], READONLY, &wwc_stat);
	      tmpsize=(lpixel_i[0]-fpixel_i[0]+1)*(lpixel_i[1]-fpixel_i[1]+1);
	      assert( (tmparray=malloc(tmpsize*sizeof *tmparray))!=NULL );
	      assert( (wtmp=malloc(tmpsize*sizeof *wtmp))!=NULL );
	      fits_read_subset_flt(read_fptr, group, naxis, inaxes, fpixel_i, 
				   lpixel_i, inc, nulval, tmparray, &anynul, 
				   &fr_status);
	      fits_read_subset_flt(wread_fptr, group, naxis, inaxes,fpixel_i, 
				   lpixel_i, inc, nulval, wtmp, &anynul, 
				   &wwc_stat);
	      sf=tmparray; wff=(wf=wtmp)+tmpsize; 
	      do *sf++ *= *wf; while(++wf<wff);
	      fits_close_file(wread_fptr, &wwc_stat);
	      free(wtmp);
	    }
	  else
	    {
	      /* Make the array to keep the section pixels. The +1
		 on each axis is explained in the explanations of 
		 find_desired_pixel_range().  */
	      tmparray=malloc((lpixel_i[0]-fpixel_i[0]+1)
			      *(lpixel_i[1]-fpixel_i[1]+1)* sizeof *tmparray);
	      assert(tmparray!=NULL);

	      /* Read the pixels in the desired subset: */
	      fits_read_subset_flt(read_fptr, group, naxis, inaxes, fpixel_i, 
				   lpixel_i, inc, nulval, tmparray, &anynul, 
				   &fr_status);
	    }

	  /* Write that section */
	  fits_write_subset_flt(write_fptr, group, naxis, onaxes, fpixel_c,
				lpixel_c, tmparray, &wr_status);


	  /* Add the WCS header information to the cropped image if
	     this is the first stitch. */
	  if(numimg==0)
	    addheaderinfo(write_fptr, &wr_status, wcs, fpixel_i, fpixel_c,
			  world, tp->ps_size, tp->res);

	  /* Free the spaces: */
	  free(tmparray);
	  free(fullheader);
	  fits_close_file(read_fptr, &fr_status);
	  wc_status = wcsvfree(&nwcs, &wcs);
	  ++numimg;
	}
      while(*(++i)!=NONINDEX);
 
      /* Save the necessary information in the process log */
      log[*t*LOG_COLS  ] = *t+1;
      log[*t*LOG_COLS+1] = numimg;

      /* Check to see if the center of the image is empty or not. */
      check_center(&write_fptr, onaxes, chk_size, numimg, 
		   &zero_flag, crop_side, &wr_status);

      /* Close the FITS file and free the cropped space: */
      fits_close_file(write_fptr, &wr_status);
      fits_report_error(stderr, wr_status);
 
      /* Report the results on stdout and in final_report: */
      report_prepare_end(verb, log, *t, numimg, zero_flag, &remove_flag);

      /* If the image should be removed, do so: */
      if(remove_flag)
	assert(unlink(fitsname)==0);

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
  if(p->verb) 
    {
      sprintf(report, "All %lu target(s) stitched or cropped.", p->cs0);
      reporttiming(&t1, report, 1);
    }

  tiffasavelog(p);
}
