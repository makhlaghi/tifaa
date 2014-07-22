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
/* System libraries: */
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
#include "tifaa_code.h"



/********************************************************************
 ************        Prepare the image and object        ************
 ************                 information                ************
 ********************************************************************/
/* This function looks into the directory and outputs the names of all
   the desired files.  */
void
get_images_in_dir(struct Config *conf, glob_t *gl)
{
  /* Declarations: 
     long i;*/
  char dir[LINE_BUFFER];

  /* Make the directory string and run glob */
  strcpy(dir, conf->surv_add);
  strcat(dir, conf->img_pfx);
  glob(dir, 0, 0, gl);

  /* Incase you want to see the results:
     for(i=0;i<(long)gl->gl_pathc;i++)
     printf("%s\n", gl->gl_pathv[i]);*/
}





/* This function will open a FITS file, read the header and output a
 prepared wcsprm
                                         
 Don't forget to free the space after it: 

    status = wcsvfree(&nwcs,&wcs); */
void 
prepare_fitswcs(char *fits_name, fitsfile **fptr, int *f_status, 
		int *w_status, int *nwcs, struct wcsprm **wcs)
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
  *w_status = wcspih(header, nkeys, relax, ctrl, &nreject, nwcs, wcs);
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





/* WCSLIB requires a string made up of all the headers in a fits file,
   this function will get that string using CFITSIO.*/
void
get_imginfo(char *fits_name, double **imginfo, unsigned long zero_pos, 
	    const double res)
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
  prepare_fitswcs(fits_name, &fptr, &f_status, &w_status, &nwcs, &wcs);
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
  (*imginfo)[zero_pos  ]=world[0];
  (*imginfo)[zero_pos+1]=world[1];
  (*imginfo)[zero_pos+2]=naxis1/7200*res; 
  (*imginfo)[zero_pos+3]=naxis2/7200*res;
}





/* This function will read the header of all FITS files in the folder
   and save the necessary information (image name, central position
   RA, central position Dec, NAXIS1 and NAXIS2) into an array.*/
void 
fits_info(struct Config *conf, glob_t *fits_names, 
	  double **imginfo)
{
  /* Declarations: */
  unsigned long i;

  /* Open up space for a table with all the information
     for all the files.*/
  get_images_in_dir(conf, fits_names);
  *imginfo=malloc(fits_names->gl_pathc*NUM_IMAGEINFO_COLS*sizeof(double));

  /* Go over all the images and save their information */
  for(i=0;i<fits_names->gl_pathc;i++)
    get_imginfo(fits_names->gl_pathv[i], imginfo, 
		NUM_IMAGEINFO_COLS*i, conf->res);
}





int 
cmpfunc (const void * a, const void * b)
{
  return ( *(int*)a - *(int*)b );
}





/* This function finds the unique elements in an array, the pointer
   "felem" points to the first desired element in the array the number
   of elements you want to order is given by num_elem and the step
   size tells how far the next element is in the original array (in
   case it is 1D this is 1, if it is 2D, step_size=number of
   columns.  */
void 
find_unique(int *felem, int num_elem, int step_size, 
	    int *unique, int *num_unique, int emptyelem)
{
  /* Declaratins: */
  int *pt, *upt, *temp;

  /* Initialize num_unique to zero. */
  *num_unique=0;

  /* Allocate space to the temp array: */
  temp=malloc(num_elem*sizeof(int));
  for(pt=felem;pt<&felem[num_elem*step_size]; pt+=step_size)
    temp[(pt-felem)/step_size]=*pt;

  /* Set all the elements of uniqe to emptyelem: */
  for(pt=unique; pt<unique+num_elem; pt++) *pt=emptyelem;

  /* Sort the temp array: */
  qsort(temp, num_elem, sizeof(int), cmpfunc);

  /* Check for different elements, */
  *unique=*temp;
  upt=unique+1;
  for(pt=temp;pt<temp+num_elem-1;pt++)
    if (*pt!=*(pt+1)) {*upt++=*(pt+1); (*num_unique)++;}

  /* Free the allocated space: */
  free(temp);
}





/* This function will look at the 4 corners of a square that has been
   identified by the user around the object's RA and Dec. It will then
   find the images that contain at least one of those points.*/
void 
obj_fits(struct ArrayInfo *intable, struct Config *conf, glob_t *fits_names, 
	 double **imginfo, int **which_images)
{
  /* Declarations: */
  double points[8], ra, dec; 
  double hswr, hswd, decr, *po, *im;
  int i, *ipt, step=1, *unique, num_unique;

  /* Allocate space to the which_images array: Any 
     object's region can at most be surrounded by 
     4 images, the first column will show how many 
     images are necessary for each object, initialize
     the whole array to have a value of -1.*/
  *which_images=malloc(WI_COLS*intable->s0*sizeof(int));
  for(ipt=*which_images; ipt<&(*which_images)[WI_COLS*intable->s0];ipt++)
    *ipt=EMPTY_VAL;

  /* Set the half side width in degrees and radians,
     they are done here to simplify the positioning 
     step in the loop. Notice that conf->ps_size was 
     in arcseconds.*/
  hswd=conf->ps_size/7200;
  hswr=hswd*PI/180; 

  /* Go over all the objects and find the images that 
     contain all or part of the desired region around it. */
  for(i=0;i<intable->s0;i++)
    {
      /* For simplification of the points below: */
      ra =intable->d[i*intable->s1+conf->ra_col];
      dec=intable->d[i*intable->s1+conf->dec_col];
      decr=dec*PI/180;

      /* Define the 4 surrounding points, in order they are:*/
      points[0]=ra+hswd/cos(decr-hswr); points[1]=dec-hswd; /*Bottom left */
      points[2]=ra-hswd/cos(decr-hswr); points[3]=dec-hswd; /*Bottom right*/
      points[4]=ra+hswd/cos(decr+hswr); points[5]=dec+hswd; /*Top left    */
      points[6]=ra-hswd/cos(decr+hswr); points[7]=dec+hswd; /*Top right   */

      /* Each object has 4 points around it. See which images contain
	 which point. NOTE: For each point: pRA=*po, pDec=*(po+1) */
      for(po=points;po<&points[8];po+=2)
        {
	  /* NOTE: For each image (ic: image center): 
	     icRA=*im, icDec=*(im+1), img_ax1_half_width=*(im+2), 
	     img_ax2_half_width=*(im+3) */
	  for(im=*imginfo; 
	      im<&((*imginfo)[fits_names->gl_pathc*NUM_IMAGEINFO_COLS]);
	      im+=NUM_IMAGEINFO_COLS)
            {
	      /* First make sure declination is in range, then RA. */
	      if( *(po+1)<=*(im+1)+*(im+3) && *(po+1)>*(im+1)-*(im+3) 
		  && *po<=*im+*(im+2)/cos(*(po+1)*PI/180)
		  && *po>*im-*(im+2)/cos(*(po+1)*PI/180) )
                {
		  (*which_images)[(i*WI_COLS)+((po-points)/2)]
		    =(int)(im-*imginfo)/NUM_IMAGEINFO_COLS;

		  /* Break out of the search, if there are no overlaps 
		     then eachpoint can only be in one image, if there are
		     overlaps then it doesn't matter! We have one image that
		     some of the pixels are in, we don't need two!*/
		  break;
                }
            }
        }
    }

  /* make sure that for each object there 
     is only a unique set of images: */
  unique=malloc(WI_COLS*sizeof(int));
  for(ipt=*which_images; ipt<&(*which_images)[WI_COLS*intable->s0];
      ipt+=WI_COLS)
    {
      /* Get the unique set */
      find_unique(ipt, WI_COLS, step, unique, &num_unique, EMPTY_VAL);

      /* Replace the which_images row: */
      for(i=0;i<WI_COLS;i++) *(ipt+i)=unique[i];
    }
  free(unique);
}




















/********************************************************************
 ************           Make the postage stamps          ************
 ********************************************************************/
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





/* Given the coordiantes of the object, this function finds which
   pixels of the image correspond to which pixels in the cropped image
   using the *pixel and *pixel_c arrays.

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
        int imagesdone, int *zero_flag, long crop_side, int *wr_status)
{
  /* Declarations: */
  long fpixel_ch[2], lpixel_ch[2], inc[2]={1,1}, ch_hw;
  float *to_check, *ipt, nulval=-9999;
  int group=0, naxis=2, hw, anynul=0;

  /* Set the positions and array of the pixels to check: */
  hw=crop_side/2; ch_hw=chk_siz/2;
  fpixel_ch[0]=(hw+1)-ch_hw; fpixel_ch[1]=(hw+1)+ch_hw;
  lpixel_ch[0]=(hw+1)-ch_hw; lpixel_ch[1]=(hw+1)+ch_hw;
  to_check=calloc(chk_siz*chk_siz, sizeof(float));

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
report_prepare_end(double **final_report, int frrow, const int obj_id, 
        int imagesdone, const int zero_flag, int *remove_flag)
{
  if (zero_flag==1)
    {
      printf("%5d:   Central region (at least) is zero!\n",obj_id);
      *remove_flag=1;
      (*final_report)[frrow*FINAL_REPORT_COLS+2]=1;
    }
  else if (imagesdone==0)
    {
      printf("%5d:   Not in field!\n",obj_id);
      *remove_flag=1;
      (*final_report)[frrow*FINAL_REPORT_COLS+2]=2;
    }
  else if (imagesdone==1)
    {
      printf("%5d:   cropped.\n",obj_id);
      (*final_report)[frrow*FINAL_REPORT_COLS+2]=0;
    }
  else if (imagesdone>1)
    {
      printf("%5d:   stiched and cropped (%d images).\n", 
	     obj_id, imagesdone);
      (*final_report)[frrow*FINAL_REPORT_COLS+2]=0;
    }
}





/* This function will pull out those objects that only need one image,
   to be most efficient, it will first open each FITS file and then
   pull out all the objects that are in it.  */
void
needs_one(struct ArrayInfo *intable, struct Config *conf, 
        glob_t *fits_names, int **which_images, double **final_report)
{
  /* Declarations: */
  long naxis=2, crop_side, onaxes[2], inaxes[2], nelements, inc[2]={1,1};
  int zero_flag, remove_flag, nwcs=0, anynul=0, num_unique, img_counter;
  int *unique, *pt, i, obj_id, fr_status, wc_status, wr_status;
  int ncoord=1, nelem=2, stat[NWCSFIX], group=0, imagesdone=1;
  long fpixel_c[2], lpixel_c[2], fpixel_i[2], lpixel_i[2];
  double phi, theta, pixcrd[2], imgcrd[2], world[2]; 
  char fitsname[LINE_BUFFER];
  float *cropped, nulval=-9999;
  fitsfile *write_fptr, *read_fptr;
  struct wcsprm *wcs;

  /* Allocate space for the array of unique images */
  unique=malloc(intable->s0*sizeof(int));  
 
  /* Find the unique images used. Only in the first
     column of which_images, since we are only dealing 
     with objects that are fully in one image. */
  find_unique(*which_images, intable->s0, WI_COLS, 
	      unique, &num_unique, EMPTY_VAL);
  img_counter=1;

  for(pt=unique;pt<unique+intable->s0;pt++)
    printf("%d\n", *pt);

  /* Set the width of the output */
  crop_side=(long)(conf->ps_size/conf->res);
  if (crop_side%2==0) crop_side-=1;
  onaxes[0]=crop_side; onaxes[1]=crop_side;
  nelements=crop_side*crop_side;

  /* Go over all the images and crop any object in them */  
  for(pt=unique;pt<unique+intable->s0;pt++)
    {
      /* In case it gets to the end, break out: */
      if (*pt==EMPTY_VAL) continue;
 
      /* Report the status: */
      printf("\nLooking in: (%d of %d) \n%s\n", img_counter, num_unique+1, 
	     fits_names->gl_pathv[*pt]);

      /* Prepare wcsprm structure and read the image size.*/
      fr_status=0; wc_status=0;
      prepare_fitswcs(fits_names->gl_pathv[*pt], &read_fptr, &fr_status, 
		      &wc_status, &nwcs, &wcs);
      fits_read_key(read_fptr, TLONG, "NAXIS1", &inaxes[0], 
		    NULL, &fr_status);
      fits_read_key(read_fptr, TLONG, "NAXIS2", &inaxes[1], 
		    NULL, &fr_status);
        
      /* Go over all the objects and crop those in this image: */
      for(i=0;i<intable->s0;i++)
        {
	  /* Only Check if the object is fully positioned
	     in this particular image. */
	  if ((*which_images)[i*WI_COLS+1]!=-1 
	      || (*which_images)[i*WI_COLS  ]!=*pt) continue; 

	  /*Set these two flags to zero:*/
	  zero_flag=0; remove_flag=0;

	  /* Set this object's ID, RA and Dec: */
	  obj_id=(int)intable->d[i*intable->s1+conf->id_col];
	  world[0]=intable->d[i*intable->s1+conf->ra_col];
	  world[1]=intable->d[i*intable->s1+conf->dec_col];

	  /* Find the position of the object's RA and Dec: */
	  wc_status = wcss2p(wcs, ncoord, nelem, world, &phi, 
			     &theta, imgcrd, pixcrd, stat); 

	  /* Find the desired pixel ranges in both the input 
	     and output images. */
	  find_desired_pixel_range(pixcrd, inaxes[0], inaxes[1], crop_side,
				   fpixel_i, lpixel_i, fpixel_c, lpixel_c);

	  /* Open space for the array keeping the cropped section. */
	  cropped=malloc((lpixel_i[0]-fpixel_i[0]+1)
			 *(lpixel_i[1]-fpixel_i[1]+1)*sizeof(float));

	  /* Read the pixels in the cropped region: */
	  fits_read_subset_flt(read_fptr, group, naxis, inaxes, 
			       fpixel_i, lpixel_i, inc, nulval, 
			       cropped, &anynul, &fr_status);

	  /* Create the fits image for the cropped array here: */
	  wr_status=0;
	  sprintf(fitsname, "%s%d%s", conf->out_add, obj_id, conf->out_ext);
	  fits_create_file(&write_fptr, fitsname, &wr_status);
	  fits_create_img(write_fptr, FLOAT_IMG, naxis, onaxes, &wr_status);
	  fits_write_img(write_fptr, TFLOAT, 1, nelements, cropped, 
			 &wr_status);            
            
	  /* Save the necessary information in final_report */
	  (*final_report)[i*FINAL_REPORT_COLS]=(double)obj_id;
	  (*final_report)[i*FINAL_REPORT_COLS+1]=imagesdone;

	  /* Check to see if the center of the image is empty or not. */
	  check_center(&write_fptr, onaxes, conf->chk_siz, imagesdone, 
		       &zero_flag, crop_side, &wr_status);

	  /* Close the FITS file and free the cropped space: */
	  fits_close_file(write_fptr, &wr_status);
	  fits_report_error(stderr, wr_status);
	  free(cropped);   

	  /* Report the results on stdout and in final_report: */
	  report_prepare_end(final_report, i, obj_id, imagesdone, 
			     zero_flag, &remove_flag);

	  /* If the object's image is flagged to be removed, do so: */
	  if (remove_flag==1) remove(fitsname);
        }

      /* Close the files and free the spaces: */
      fits_close_file(read_fptr, &fr_status);
      wc_status = wcsvfree(&nwcs, &wcs);
      img_counter++;
    }
}





/* This function will crop objects that need more than one tile. For
   each object, the required section of each tile is brough out and
   placed into the output array. */
void
needs_more_than_one(struct ArrayInfo *intable, struct Config *conf, 
        glob_t *fits_names, int **which_images, double **final_report)
{
  /* Declarations: */
  long naxis=2, crop_side, onaxes[2], inaxes[2], nelements, inc[2]={1,1};
  int i, zero_flag, remove_flag, wr_status, imagesdone, stat[NWCSFIX];
  int obj_id, group=0, fr_status, wc_status, nwcs=0, ncoord=1;
  long fpixel_c[2], lpixel_c[2], fpixel_i[2], lpixel_i[2];
  double phi, theta, pixcrd[2], imgcrd[2], world[2];
  char fitsname[LINE_BUFFER];
  float *cropped, *temparray, nulval=-9999;
  fitsfile *write_fptr, *read_fptr;
  int nelem=2, anynul=0;
  struct wcsprm *wcs;

  /* Set the width of the output */
  crop_side=(long)(conf->ps_size/conf->res);
  if (crop_side%2==0) crop_side-=1;
  onaxes[0]=crop_side; onaxes[1]=crop_side;
  nelements=crop_side*crop_side;

  for(i=0;i<intable->s0;i++)
    {
      /* Set the remove and zero flags to zero: */
      zero_flag=0; remove_flag=0;

      /* In case only one image is needed, the job is already
	 done previously, so there is no need to do it again: 
	 if ((*which_images)[i*WI_COLS+1]==-1) continue; */

      /* Set this object's ID, RA and Dec: */
      obj_id=(int)intable->d[i*intable->s1+conf->id_col];
      world[0]=intable->d[i*intable->s1+conf->ra_col];
      world[1]=intable->d[i*intable->s1+conf->dec_col];

      /* Create the fits image for the cropped array here: */
      wr_status=0;
      sprintf(fitsname, "%s%d%s", conf->out_add, obj_id, conf->out_ext);
      cropped=calloc(crop_side*crop_side, sizeof(float));
      fits_create_file(&write_fptr, fitsname, &wr_status);
      fits_create_img(write_fptr, FLOAT_IMG, naxis, onaxes, &wr_status);
      fits_write_img(write_fptr, TFLOAT, 1, nelements, cropped, &wr_status);

      /* Go over all the images for this object. */
      imagesdone=0;
      while((*which_images)[i*WI_COLS+imagesdone]!=-1 && imagesdone<WI_COLS)
        {
	  /* Prepare wcsprm structure and read the image size.*/
	  fr_status=0; wc_status=0;
	  prepare_fitswcs(fits_names->gl_pathv[(*which_images) 
					       [i*WI_COLS+imagesdone]], 
			  &read_fptr, &fr_status, &wc_status, &nwcs, &wcs);
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
	  temparray=calloc((lpixel_i[0]-fpixel_i[0]+1)
			   *(lpixel_i[1]-fpixel_i[1]+1), sizeof(float));

	  /* Read the pixels in the desired subset: */
	  fits_read_subset_flt(read_fptr, group, naxis, inaxes, fpixel_i, 
			       lpixel_i, inc, nulval, temparray, &anynul, 
			       &fr_status);

	  /* Write that section */
	  fits_write_subset_flt(write_fptr, group, naxis, onaxes, fpixel_c,
				lpixel_c, temparray, &wr_status);

	  /* Free the spaces: */
	  free(temparray);
	  fits_close_file(read_fptr, &fr_status);
	  wc_status = wcsvfree(&nwcs, &wcs);

	  /* Go onto the next image: */
	  imagesdone++;
        }

      /* Save the necessary information in final_report */
      (*final_report)[i*FINAL_REPORT_COLS]=(double)obj_id;
      (*final_report)[i*FINAL_REPORT_COLS+1]=imagesdone;

      /* Check to see if the center of the image is empty or not. */
      check_center(&write_fptr, onaxes, conf->chk_siz, imagesdone, 
		   &zero_flag, crop_side, &wr_status);

      /* Close the FITS file and free the cropped space: */
      fits_close_file(write_fptr, &wr_status);
      fits_report_error(stderr, wr_status);
      free(cropped);   

      /* Report the results on stdout and in final_report: */
      report_prepare_end(final_report, i, obj_id, imagesdone, 
			 zero_flag, &remove_flag);

      /* If the object's image is flagged to be removed, do so: */
      if (remove_flag==1) remove(fitsname);
    }
}





/* Knowing exactly which images are required for each object, this
   function will crop the required region out for each image */
void 
stichcrop(struct ArrayInfo *intable, struct Config *conf,
        glob_t *fits_names, int **which_images, double **final_report)
{
  /* Declarations: */
  int filecheck=0;

  /* Make the file containing the cropped images */
  filecheck=mkdir(conf->out_add, 0777);
  if (filecheck==-1)
    {
      printf("   ## Error: %s could not be made.\n", conf->out_add);
      printf("             It probably already exists, remove it.\n\n");
      exit(EXIT_FAILURE);        
    }

  /* For objects that only need one survey tile: 
     needs_one(intable, conf, fits_names, which_images, final_report);*/

  /* Objects that need more than one survey tile: */
  printf("\nObjects that need more than one image:\n");
  needs_more_than_one(intable, conf, fits_names, 
		      which_images, final_report);
}





/* Print the report.  */
void 
print_report(struct ArrayInfo *intable, struct Config *conf, 
	     double **final_report, time_t *t_start, time_t *t_1, 
	     time_t *t_end, glob_t *fits_names)
{
  /* Declarations: */
  struct ArrayInfo fr;
  char *outname;
  int int_cols[]={0,1,2,-1}, accu_cols[]={-1};
  int space[]={5,10,15}, prec[]={6,8};
  int num_s=0, num_c=0, num_z=0, num_o=0;
  double *pt;
    
  /* Prepare the comments to the report: */
  fr.c=malloc(LINE_BUFFER);
  *fr.c='\0';
  strcat(fr.c,"# Final report of cropping the objects:\n");
  strcat(fr.c,"# Col 0: Object ID\n");
  strcat(fr.c,"# Col 1: Number of images used for this object.\n");
  strcat(fr.c,"# Col 2: Flag = 0 : No problem\n");
  strcat(fr.c,"#             = 1 : Atleast the central region is zero\n");
  strcat(fr.c,"#             = 2 : The object was not in the field.\n");
  if ((long)strlen(fr.c)>LINE_BUFFER-1)
    {
      printf("   ## Error: LINEBUFFER	should be increased\n");
      exit(EXIT_FAILURE);
    }

  /* Set the output name: */  
  outname=malloc(strlen(conf->out_add)+strlen(conf->info_name)+1);
  strcpy(outname, conf->out_add);
  strcat(outname, conf->info_name);

  /* Set all the other structure parameters */
  fr.s0=intable->s0; fr.s1=FINAL_REPORT_COLS;
  fr.d=*final_report; fr.nr=0; fr.r=NULL;

  /* print the result to file: */
  writeasciitable(outname, &fr, int_cols, accu_cols, space, prec);  
  free(fr.c); free(outname); 

  /* Get a summary of the process: */
  for(pt=*final_report; pt<*final_report+(FINAL_REPORT_COLS*intable->s0);
      pt+=FINAL_REPORT_COLS)
    {
      if(*(pt+2)==0 && *(pt+1)==1) num_c++;
      else if(*(pt+2)==0 && *(pt+1)>1) num_s++;
      else if(*(pt+2)==1) num_z++;
      else if(*(pt+2)==2) num_o++;
    }

  /* Print a summary on stdout */
  printf("\n\n\n----------------------------------------\n");
  printf("----------------------------------------\n");
  printf("        %d objects cropped\n", num_c+num_s);
  printf("        and placed in %s\n", conf->out_add);
  printf("----------------------------------------\n");
  printf("---------- Summary: \n");
  printf("Cropped:                 %d\n", num_c);
  printf("Stiched & cropped:       %d\n", num_s);
  printf("Center zero  (no FITS):  %d\n", num_z);
  printf("Out of field (no FITS):  %d\n", num_o);
  printf("----------------------------------------\n");
  printf("---------- Timing report:\n");
  printf("Num survey FITS images:  %d\n",(int)fits_names->gl_pathc);
  printf("Preparing Survey info:   %.0f (seconds)\n", 
	 difftime(*t_1, *t_start));
  printf("Cropping all:            %.0f (seconds)\n", 
	 difftime(*t_end, *t_1));
  printf("----------------------------------------\n");
  printf("----------------------------------------\n\n\n");

}





/* This function will first get all the required information, then it
   will take the desired sections of each image and save the results
   in a FITS file. */
void 
tifaa_crop_from_survey(struct ArrayInfo *intable, 
        struct Config *conf)
{
  /* Arrays: */
  glob_t fits_names;            /* Array of image names. */
  double *imginfo;              /* Information about each image. */
  int *which_images;            /* Which images for which objects. */
  double *final_report;         /* To keep a log for each image */
  time_t t_start, t_1, t_end;   /* To keep track of execution time */

  /* First get the information for all the FITS: */
  time(&t_start);
  fits_info(conf, &fits_names, &imginfo);

  /* Correspond the objects to the FITS images: */
  obj_fits(intable, conf, &fits_names, &imginfo, &which_images);
  time(&t_1);

  /* Allocate space for final_report:*/
  final_report=calloc(intable->s0*FINAL_REPORT_COLS, sizeof(double));

  /* Do the cropping: */
  stichcrop(intable, conf, &fits_names, &which_images, &final_report);
  time(&t_end);

  /* Print the final_report */
  print_report(intable, conf, &final_report, &t_start, 
	       &t_1, &t_end, &fits_names);

  /* Free the spaces allocated: */
  free(final_report);
  globfree(&fits_names);
  free(which_images);
  free(imginfo);
}
