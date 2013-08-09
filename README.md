tifaa
=======

tifaa - Thumbnail images from astronomical archives.

----------------------------------------
About tifaa
----------------------------------------
The sizes of wide field astronomical surveys that are archived
online are usually too large (>2Gb) to download completely over the 
HTTP protocol. For that purpose these surveys create tiles over
the field and those tiles are available for download. These online
archives also have cutout tools available to cutout a particular 
section of the field for a specific study. But in some cases these
cutout tools only cut out the region in one tile. So if an object
lies close to the boundary of a tile, some of the cutout will be
empty! You can see a demonstration in the webpage below:
http://astr.tohoku.ac.jp/~akhlaghi/Crop_from_Survey.html

This is a big problem for those studies including objects that 
are too close to the tile borders. Since some of the objects I 
am working on had such a condition I wrote a simple C program 
(using `cfitsio` and `wcslib`) to crop galaxies out of the tiled 
images while correcting for such cases by taking different parts
of the final image from different tiles. In the current version 
of the program you have to download all the tiles in order for 
this program to work. later on, I will add the ability to 
directly read the `FITS` files from the online archive, but that 
will definitely be much slower than the current version. If you 
are working a lot on a specific survey you will need to have the 
tiles any way.
 
----------------------------------------
Prerequisites 
----------------------------------------
`tifaa` relies on two packages defining the FITS standard and WCS standard. 
The former is used to define astronomical images, and the latter the 
World Coordiante System, used to correlate pixel locations on an image 
to physical coordinates. The two packages I am using are the `cfitsio` 
and the `wcslib` packages. I have explained the complete procedure of how 
to install these packages in the webpage below. You should be able to install 
them successfully using the instructions there.
http://astr.tohoku.ac.jp/~akhlaghi/cfitsiowcslibinstall.html

----------------------------------------
Installation and running
----------------------------------------
After installing `cfitsio` and `wcslib`, making `tifaa` is 
very easy, simply run the command below. I assume the `$` and 
`#` show your shell prompt, the former as a user and the latter 
as root. You don't have to type them! 

    $ make

To know more about the warnings, refer to the last paragraph of
this section.

In case you want to have system wide access to to `tifaa`, run the following
command in that same folder. 

    $ su
    # mv tifaa /usr/local/bin

To run `tifaa`, you simply have to run the following command. If you
don't have a system wide installation, simply run the following 
command with `./` before `tifaa` in the same directory you executed `make`.

    $ tifaa configure.txt

Explanation about the configuration file can be seen below. An error
message will be displayed if any more options are passed onto `tifaa`.

After the compilation and linking (the `make` procedure) several 
object files (ending with `.o`) will be created, to remove them 
you can simply run:

    $ rm *.o

The warnings that show up after running `make` and complain about 
statically linked applications shouldn't bother you. If you don't 
want them, remove the `-static` option on the second line of 
`makefile` (starting with `gcc`). A static library will be self 
contained and will not be dependent on your particular system 
libraries. You can take the executables on similar machines that 
don't have the `cfitsio` or `libwcs` packages installed. The speed
is also very slightly faster in a statically shared program. The 
negative side is that with statically shared libraries in the 
executable, if you update `cfitsio` or `wcslib` you will have to 
run `make` again to keep `tifaa` updated about the new versions.

----------------------------------------
Input configuration file
----------------------------------------
The configuration file may have any name, you just have to specify that
name after the `tifaa` command. The configuration file contains the 12 
inputs you have to provide to tifaa. The parameters of the configuration 
file are fairly descriptive an example is shown below:

    CATALOG_ADDRESS = catalog.txt
    SURVEY_ADDRESS  = /directory/you/have/saved/the/survey/images/
    OUTPUT_ADDRESS  = ./PS/
    OUTPUT_EXTEN    = .fits
    IMG_PREFIX      = *sci.fits     #This varies from survey to survey
    IMG_INFO        = imginfo.txt   
    ID_COLUMN       = 0
    RA_COLUMN       = 1
    DEC_COLUMN      = 2
    RESOLUTION      = 0.03          # In arcseconds
    PS_SIZE         = 7             # In arcseconds
    CHECK_SIZE      = 3             # Odd number

`IMG_PREFIX` is the prefix of the desired FITS images in 
`SURVEY_ADDRESS`. For example if you want to have thumbnails of the 
weight images too, you have to change the value of this parameter to 
`*wht.fits` (assuming your survey's weight images end with `wht.fits`).

`IMG_INFO` is a text file that will be created in the end, containing 
a simple log of the image cropping for each object. It is explained further
below. The text file will be saved in `OUTPUT_ADDRESS`.

`PS_SIZE` is the width of the square thumbnail (or postage stamp). 

`CHECK_SIZE` is the size of the central region to check if there are non
zero pixels covering the center or not. Since the tiles are all squares,
on the sides of the survey you usually end up with empty space (zero valued
pixels). This parameter is to prevent objects that are in those sections of
a field being in the final cropped images. All such objects will be flagged
in the final report.

----------------------------------------
Output:
----------------------------------------
A sample output as seen in my shell can be seen below after I requested
it to crop 283 objects from a survey.

    .abridged
    .
    28:   cropped.
    29:   cropped.
    30:   cropped.
    31:   stiched and cropped (2 images).
    32:   cropped.
    33:   cropped.
    34:   cropped.
    35:   cropped.
    36:   stiched and cropped (2 images).
    37:   cropped.
    38:   cropped.
    39:   cropped.
    40:   cropped.
    41:   cropped.
    .
    .abridged
    .
    ----------------------------------------
    ----------------------------------------
            283 objects cropped
            and placed in ./PS/
    ----------------------------------------
        ---------- Summary: 
    Cropped:                 274
    Stiched & cropped:       9
    Center zero  (no FITS):  0
    Out of field (no FITS):  0
    ----------------------------------------
    ---------- Timing report:
    Num survey FITS images:  81
    Preparing Survey info:   0 (seconds)
    Cropping all:            28 (seconds)
    ----------------------------------------
    ----------------------------------------


A file placed in `OUTPUT_ADDRESS/IMG_INFO` is also created that contains
a report of how many image were used for each object and if it has a `.fits`
file associated with it or not with a flag explained in the header of that
file. A few lines of that file for my request looks like this:

    # Final report of cropping the objects:
    # Col 0: Object ID
    # Col 1: Number of images used for this object.
    # Col 2: Flag = 0 : No problem
    #             = 1 : Atleast the central region is zero
    #             = 2 : The object was not in the field.
    (abrdiged)...
    15   1    0    
    16   1    0    
    17   1    0    
    18   1    0    
    19   2    0    
    20   1    0    
    21   1    0   
    ...(abrdiged)

----------------------------------------
Executable file(s):
----------------------------------------
Running the executables doesn't need any prerequisites since they have been
statically compiled. Just make sure you run the executable belonging to your
operating system. So far I only have 

1. RedHat_Linux_86_64 (works on RedHat, Fedora, CentOS, Scientific Linux, etc)

I will add more in the future.

----------------------------------------
Future updates:
----------------------------------------
In the future, I will implement several updates. 
Not necessarily in the written order.

 1. Fix any shortcommings it might have that I have not noticed so far.
 2. Add WCS information to the cropped images.
 3. Speed up the program with more efficient algorithms and parallel processing.
 4. Use GNU autotools to make the program for easier installation.
 5. Add option to use online images so downloading whole survey tiles won't be necessary.

----------------------------------------
Comments and suggestions:
----------------------------------------
I hope this program will be useful for those of you that are confronted 
with the same problem I was! If you find any problems in this program
please contact me so I can correct them. I would also be very glad to
hear any suggestions or comments you might have, thank you.

makhlaghi@gmail.com 

akhlaghi@astr.tohoku.ac.jp

http://astr.tohoku.ac.jp/~akhlaghi/

----------------------------------------
Copyright:
----------------------------------------
Copyright (C) 2013 Mohammad Akhlaghi

Tohoku University Astronomical Institute

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
