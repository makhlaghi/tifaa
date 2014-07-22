/*********************************************************************
attaavv - ascii table to array and vice versa.
Library to read ascii tables of any size into 1D C arrays.

Copyright (C) 2013 Mohammad Akhlaghi
Tohoku University Astronomical Institute, Sendai, Japan.
http://astr.tohoku.ac.jp/~akhlaghi/

attaavv is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

attaavv is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "attaavv.h"





/* Get the comments string, add it to the comments array of
   pointers.  */
void 
ForComments(char *comments, long int *buff_comments, char *str)
{
  /* If it is getting longer than the actual length, make the comments
     array longer: */
  if ((long)(strlen(comments)+strlen(str))>=*buff_comments) 
    {
      *buff_comments+=BUFFER_NUM;
      comments = realloc(comments, *buff_comments*sizeof(char));
      assert(comments != NULL);
    }

  /* Copy the string into the */
  strcat(comments, str);
}





/*  Get the first data string and see how many columns of data there
    are in it.  */
void 
CountCols(char *str, int *s1, double **table, 
        long int buff_num_rows)
{
  /* The original string is copied, because strtok
     will be used on it again in a later function.  */
  char str_cpy[MAX_ROW_CHARS];
  strcpy(str_cpy, str);
  if(strtok(str_cpy," \n,")==NULL) return ;
  ++(*s1);
  while (1)
    {
      if(strtok(NULL," \n,")==NULL ) break;
      ++(*s1);
    }

  /* Assign space to the table:  */
  *table=malloc(buff_num_rows*(*s1)*sizeof(double)); 
  if (*table==NULL)
    { 
      printf("\n### ERROR: malloc failed to create a data table\n");
      exit(EXIT_FAILURE);
    }
}





/* This function will print a message notifying the user that a
   replacement has taken place because the specified element in the
   data was not a number */
void 
replacenan(struct ArrayInfo *intable, int col, char *strdata, 
        long int *buff_num_replacements)
{
  int *temp_i_pt;
 
  /* If this is the first replacement, open up space for the
     array keeping the positions of the replacements: */
  if (intable->nr==0)
    {
      intable->r=malloc(*buff_num_replacements*sizeof(int));
      if (intable->r==NULL)
        { 
	  printf("\n### ERROR: malloc failed to ");
	  printf("create a replacement table\n");
	  exit(EXIT_FAILURE);
        }
    }

  /* Do the replacement: */
  intable->d[intable->s0*intable->s1+col]=(double) CHAR_REPLACEMENT;
#if 0
  /* Report it. If CHAR_REPLACEMENT is zero, abort the program: */
  printf("### Not a number:\n");
  printf("###        \"%s\" in (%d, %d)\n", strdata, intable->s0, col);
  if (CHAR_REPLACEMENT==0) exit(EXIT_FAILURE);
  printf("###        replaced with"); 
  printf(" %f.\n", (double) CHAR_REPLACEMENT);
  printf("###        (CHAR_REPLACEMENT macro)\n\n");
#endif
  strdata=NULL;
  /* Save the position of the replaced element and 
     increment the number of replacements */
  intable->r[2*intable->nr]=intable->s0;
  intable->r[2*intable->nr+1]=col;
  intable->nr++;

  /* Increase the size of the replacements array if necessary: */
  if (2*intable->nr>=*buff_num_replacements)
    {
      *buff_num_replacements+=2*BUFFER_NUM;
      temp_i_pt=realloc(intable->r, *buff_num_replacements*sizeof(int));
      if(temp_i_pt!=NULL) intable->r=temp_i_pt;
      else 
        {
	  printf("\n### Error: Replacements array ");
	  printf("could not be reallocated.\n\n");
	  exit(EXIT_FAILURE);
        }

    }
}





/* Knowing the number of columns, this function will read in each
   row.  */
void 
AddRow(struct ArrayInfo *intable, long int *buff_num_rows, 
        long int *buff_num_replacements, char *str)
{
  /* Declarations: */ 
  int num_cols=1, z_index;
  char *strdata, *tempstr="";
  char **ExtraString=&tempstr;
  double *temp_d_pt;

  /* Set the index of the zeroth element in the row to be added:*/
  z_index=intable->s0*intable->s1;

  /* Check if there is anything in this row: */
  if((strdata=strtok(str," \n,"))==NULL) return ;

  /* Read the first element of the row: */
  intable->d[z_index]=strtod(strdata, ExtraString);
  if (strlen(*ExtraString)>0)
    replacenan(intable, 0, strdata, buff_num_replacements);

  /* Continue with the rest: */
  while (1)
    {
      /* Incase it is the end of the line, break out: */
      if( (strdata=strtok(NULL," \n,"))==NULL ) break;

      /* put the element in the array, if it is not a number
	 inform the user and replace it with CHAR_REPLACEMENT*/
      intable->d[z_index+num_cols++]=strtod(strdata, ExtraString);
      if (strlen(*ExtraString)>0)
	replacenan(intable, num_cols-1, strdata, buff_num_replacements);
     
      /* Incase the number of columns has exceeded the desired value
	 abort the program and inform the user. */
      if (num_cols>intable->s1)
        {
	  printf("### Error: Too many data in row");
	  printf("%d (starting from 1).\n",intable->s0+1);
	  printf("### --------------should have");
	  printf("%d but has %d.\n", intable->s1, num_cols);
	  exit(EXIT_FAILURE);
        }        
    }

  /* Check if the number of rows was not smaller than 
     what was expected. In a normal file the last line 
     will have 1 from the initialization.*/
  if (num_cols<intable->s1 && num_cols>1)
    {
      printf("### Error: Too few data in data row");
      printf("%d (starting from 1).\n",intable->s0+1);
      printf("### --------------should have");
      printf("%d but has %d.\n", intable->s1, num_cols);
      exit(EXIT_FAILURE);
    }        
    
  /* Add to the number of saved rows. The condition is here
     because I don't want to count the last (empty) row. */
  if (num_cols==intable->s1) intable->s0++;

  /* Check to see if the buffer size has not been exceeded.
     If it has, add space to the table array for the next rows. */
  if (intable->s0>=*buff_num_rows)
    {
      *buff_num_rows+=BUFFER_NUM;
      temp_d_pt=realloc(intable->d, *buff_num_rows*intable->s1*sizeof(double));
      if(temp_d_pt!=NULL) intable->d=temp_d_pt;
      else 
        {
	  printf("\n### Error: Data array could not be reallocated.\n\n");
	  exit(EXIT_FAILURE);
        }
    }
}





/* All of the arrays have some extra space correct this so all of them
   finish on their last valuable element.  */
void
correctsizes(struct ArrayInfo *intable)
{
  /* Shrink the data array to the correct size: */
  if (intable->s0!=0 && intable->s1!=0)
    {
      intable->d=realloc(intable->d, intable->s0*intable->s1*sizeof(double));
      if(intable->d == NULL)
        {
	  printf("\n### Error: Data array could ");
	  printf("not be reallocated.\n\n");
	  exit(EXIT_FAILURE);
        }
    }

  /* Shrink the comments array: */
  intable->c=realloc(intable->c, strlen(intable->c)*sizeof(char));
  if(intable->c == NULL)
    {
      printf("\n### Error: Comments could ");
      printf("not be reallocated.\n\n");
      exit(EXIT_FAILURE);
    }

  /* Shrink the replacements array: */
  if (intable->nr!=0)
    {
      intable->r=realloc(intable->r, 2*intable->nr*sizeof(int));
      if(intable->r == NULL)
        {
	  printf("\n### Error: Replacements array could ");
	  printf("not be reallocated.\n\n");
	  exit(EXIT_FAILURE);
        }
    }

}





/* Read an ascii table into an array and also give the number of rows
   and columns in the array. In order to use this function you have to
   initialize the comments and table variables. Here is an example
   program:
    #include <stdlib.h> 
    #include "attaavv.h"

    int main (void)
    {
	char input_name[]="./data/cat.txt";
	char output_name[]="tempcat.txt";
	int int_cols[]={0,2,-1}, accu_cols[]={4,-1};
	int space[]={5,10,15}, prec[]={6,8};

	struct ArrayInfo intable;

	readasciitable(input_name, &intable);

	writeasciitable(output_name, &intable, int_cols, 
			accu_cols, space, prec);

	freeasciitable(&intable);
	return 0;
    }  */
void 
readasciitable (const char *filename, struct ArrayInfo *intable)
{
  /* Declarations: */
  long line_counter=0;
  long buff_num_rows=BUFFER_NUM;
  long buff_comments=BUFFER_NUM;
  long buff_num_replacements=2*BUFFER_NUM; /* Has to be even */
  char str[MAX_ROW_CHARS];
  FILE *fp=fopen(filename, "r");

  /* Check if the file opening was successful: */
  if (fp==NULL)
    {
      printf("\n### Failed to open input file: %s\n", filename);
      exit(EXIT_FAILURE);
    }

  /* Initialize all the sizes in the structure 
     to zero for later steps */
  intable->c=malloc(buff_comments*sizeof(char));
  intable->s0=0;
  intable->s1=0;
  intable->nr=0;
    
  /* Go over the input file line by line and read
     the comments and data into the given arrays. */
  while(!feof(fp))
    {
      /* Read line by line: */
      fgets(str, sizeof(str), fp);
      ++line_counter;

      /* Incase the length of the string is comparable
	 to the macro defining the string length, abort the
	 program and notify the user. 10 is just an arbitrary
	 number, if one row is so close, others might exceed.
	 it is not worth the risk to continue. */
      if ((long int) strlen(str)>MAX_ROW_CHARS-10)
        {
	  printf("### Error: The number of characters in\n");
	  printf("    line %ld are very near the buffer\n", line_counter); 
	  printf("    limit set by MAX_ROW_CHARS, make it larger.\n\n");
	  exit(EXIT_FAILURE);
        }

      /* Incase the line is a comment line: */
      if(str[0]==COMMENT_SIGN) 
	ForComments(intable->c, &buff_comments, str);

      /* If a line doesn't begin with a COMMENT_SIGN, it is 
	 read as data and put into an array of data values. */
      else
        {
	  /* If this is the first data row, count how many 
	     columns it has to make the correct input format */
	  if (intable->s1==0) 
	    CountCols(str, &intable->s1, &intable->d, buff_num_rows);

	  /* We now have an initial array to begin with,
	     we will fill it up with the rows */
	  AddRow(intable, &buff_num_rows, &buff_num_replacements, str);
        }
    }

  /* Correct the sizes of the arrays: */
  correctsizes(intable);

  /* Close the file and return 0 (meaning success)  */
  fclose(fp);

  /* Report the result: 
  printf("\n\n----------------------\n");
  printf("Completed reading %s\n", filename);
  printf("   Shape of table: (%d, %d).\n", intable->s0, intable->s1);
  printf("   Number of replaced elements: %ld.\n", intable->nr);
  printf("----------------------\n\n");*/
}





/* This function gets the formatting settings of the array as required
   by writeasciitable and makes an array of formatting conditions that
   is suitable for printing.  */
void 
DoFormatting(int numcols, char **fmt, char *fmt_all, int *int_cols, 
        int *accu_cols, int *space, int *prec)
{
  int i,j, found=0;
  char intstr[10], eacustr[10], otherstr[10];

  /* Define the formating for each kind of data: */
  sprintf(intstr, "%%-%d.0f", space[0]);
  sprintf(otherstr, "%%-%d.%dlg", space[1], prec[0]);
  sprintf(eacustr, "%%-%d.%df", space[2], prec[1]);

  /* Initialize the format array: */
  for (i=0;i<numcols;++i)
    {
      fmt[i]=&fmt_all[i*10];

      /* Check if an int should be placed: */
      found=0;
      for(j=0;j<numcols;++j)
        {
	  if (int_cols[j]<0) break;
	  if (i==int_cols[j]) 
            {
	      strcpy(fmt[i], intstr);
	      found=1;break;
            }
        }
      if (found==1) continue;

      /* Check if an extra precision should be placed: */
      found=0;
      for(j=0;j<numcols;++j)
        {
	  if (accu_cols[j]<0) break;
	  if (i==accu_cols[j]) 
            {
	      strcpy(fmt[i], eacustr);
	      found=1;break;
            }
        }
      if (found==1) continue;
 
      strcpy(fmt[i], otherstr);
    }
}






/* Write an array to an ascii file. The example bellow assumes your
array has come from readasciitable. Explanations of the input format
arrays: int_colss: The columns that are integers. accu_rows: The
columns that require extra accuracy.  NOTE: For the two arrays above,
the last element has to be -1.  space: The minimum field width given
to the three kinds of numbers, the first element is the space for
integers, the second for floats (normal numbers) and the third is for
those numbers that require more accuracy.  prec: This is only for the
last two kinds of numbers, it shows the decimal point precision that
will be used for their printing in order.  */
void 
writeasciitable(const char *filename, struct ArrayInfo *intable, 
        int *int_cols, int *accu_cols, int *space, int *prec)
{
  /* Make an array of strings to hold the 
     formattings for each row. */
  int i,j;
  char **fmt=malloc(intable->s1*sizeof(char *));
  char *fmt_all=malloc(10*intable->s1*sizeof(char));
    
  /* Open the output file: */
  FILE *fp=fopen(filename, "w");
  /* Check if the file opening was successful: */
  if (fp==NULL)
    {
      printf("\n### attaavv.c: writeasciitable\n\
               Failed to open output file:"); 
      printf("%s\n", filename);
      exit(EXIT_FAILURE);
    }
    
  /* Prepare the formatting for each column */
  DoFormatting(intable->s1, fmt, fmt_all, int_cols, 
	       accu_cols, space, prec);

  /* Print the headers to file: */   
  fprintf(fp, "%s", intable->c);

  /* Print the data to file: */
  for(i=0;i<intable->s0;++i)
    {
      for(j=0;j<intable->s1;++j) 
	fprintf(fp, fmt[j], intable->d[i*intable->s1+j]);
      fprintf(fp, "\n");
    }

  /* Report the result: 
     printf("\n\n----------------------\n");
     printf("Completed writing %s\n", filename);
     printf("   Shape of table: (%d, %d).\n", intable->s0, intable->s1);
     printf("----------------------\n\n");*/

  /* Close the file and free all pointers: */
  free(fmt_all);
  free(fmt);
  fclose(fp);    
}





/* Free the space that was created by during readasciitable.  */
void 
freeasciitable (struct ArrayInfo *intable)
{
  free(intable->d); 
  if (intable->nr>0) free(intable->r); 
  free(intable->c);
}
