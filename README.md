tifaa
=======

tifaa - Thumbnail images from astronomical archives.

About tifaa:
-----------

The sizes of wide field astronomical surveys that are archived online
are usually too large (>2Gb) to download completely over the HTTP
protocol. For that purpose these surveys create tiles over the field
and those tiles are available for download. Here is an example of the
tiles of the HST/ACS tiles of the GOODS-North field:

![Tiled image of GOODS-North](https://raw.github.com/makhlaghi/tifaa/master/ReadmeImages/CFS.jpg)


Such servers usually have a cut out tool, but the cut out works on
each tile. So, for example, In [the above
survey](http://archive.stsci.edu/prepds/goods/) if you enter these
coordinates: `189.1572459`,`62.268763` in their [cutout
tool](http://archive.stsci.edu/eidol_v2.php), with a width of `45.03`
(1501 pixels) you will get (for example in the `i` band):

![Example web cutout](https://raw.github.com/makhlaghi/tifaa/master/ReadmeImages/CFSweb.png)

This is a big problem for those studies including objects that are too
close to the tile borders. Since some of the objects I am working on
had such a condition I wrote a simple C program (using `cfitsio` and
`wcslib`) to crop galaxies out of the tiled images while correcting
for such cases by taking different parts of the final image from
different tiles.

![Example tiffa output](https://raw.github.com/makhlaghi/tifaa/master/ReadmeImages/CFShere.png)

In the current version of the program you have to download all the
tiles in order for this program to work. later on, I might add the
ability to directly read the `FITS` files from the online archive, but
that will definitely be much slower than the current version, because
the tiles are usually very large and downloading them can take
considerable time. If you are working a lot on a specific survey you
will need to have the tiles any way.

`tifaa` will write a corrected WCS header information to the cropped
images so the pixels in a cropped and stitched images have exactly the
same celestial coordinates as they would in the larger survey images. 

As an option (`-w`), `tifaa` can also multiply the weight images and
the science images in a survey so that the resulting image (especially
in HST surveys) will be in units of counts and not counts/second.
 
Prerequisites 
-------------

`tifaa` relies on two packages defining the FITS standard and WCS
standard.  The former is used to define astronomical FITS images. The
latter (World Coordiante System) is used to correlate pixel locations
on an image to physical coordinates. The two packages I am using are
the `cfitsio` and the `wcslib` packages, which are the de-facto
standards in astronomy. I have explained the complete procedure of how
to install these packages in the webpage below. You should be able to
install them successfully using the instructions there.
http://astr.tohoku.ac.jp/~akhlaghi/cfitsiowcslibinstall.html

Installation and running
------------------------

After installing `cfitsio` and `wcslib`, making `tifaa` is very easy,
simply run the command below in the downloaded directory. I assume the
`$` and `#` show your shell prompt, the former as a user and the
latter as root. You don't have to type them!

    $ make

In case you want to have system wide access to to `tifaa` (not just in
the directory you have built it in), run the following command in that
same folder. After this, you can complete delete the building
directory.

    $ su
    # make install

to run `tifaa` you need to specify some command line options. The
[POSIX argument syntax
conventions](http://www.gnu.org/software/libc/manual/html_node/Argument-Syntax.html#Argument-Syntax)
apply to the options. They are completely explained in the section
below.


Input options:
--------------

If you type `tifaa -h`, a full list of options will be displayed and
explained. Here is a short summary. If any of the mandatory options
are not provided, `tifaa` will notify you that they are missing.

Options that don't run `tifaa`:
* `-h`: Print the help to explain all the options.
* `-v`: View the version information of `tifaa` that you are running.

On/Off options (no value required):
* `-e`: Verbose mode (print information as `tifaa` is running).
* `-g`: Delete possibly existing output directory.

Mandatory options with arguments:
* `-c`: Name of catalog (ASCII table) you want thumbnails from.
* `-r`: Column (starting from zero) of RA in catalog.
* `-d`: Column (starting from zero) of Dec in catalog.
* `-a`: Resolution of image (in arcseconds/pixel).
* `-p`: Size of thumbnail image in arcseconds.
* `-s`: String (with wildcards) showing the images you want crops from.

Optional options with arguments:
* `-w`: Wildcard representation of weight images. 
* `-t`: Number of CPU threads to use.
* `-o`: Name of folder to keep the output thumbnails images.
* `-f`: Ouput thumbnail name ending.
* `-k`: Central pixels to check if thumbnail is not blank.

Output:
-------

A sample output as seen in my shell when verbose mode was on, can be
seen below after I requested it to crop 145 objects from a survey.

    ----------------------------------------------------------------------
    TIFAA v0.3 (1 threads) started on Wed Jul 23 21:23:22 2014
      - WCS info of 81 image(s) has been read.   in 2.675826 seconds
      - Target/image correspondance found.       in 0.000211 seconds
        .
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
        36:   stiched and cropped (4 images).
        37:   cropped.
        38:   cropped.
        39:   cropped.
        40:   cropped.
        41:   cropped.
        .
        .abridged
        .
      - All 145 target(s) stitched or cropped.   in 23.957981 seconds
    TIFFA finished in:  27.330405 (seconds)
    ----------------------------------------------------------------------

A file placed in `OUTPUT_ADDRESS/tifaalog.txt` (`OUTPUT_ADDRESS` is
the value of the option `-o`) is also created that contains a report
of how many images were used for each object and if it has a `.fits`
file associated with it or not with a flag explained in the header of
that file. A few lines of that file for my request looks like this:

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


Future updates:
---------------

In the future, I will implement several updates.  Not necessarily in
the written order.

 1. Fix any shortcommings it might have that I have not noticed so far.
 2. Use GNU autotools to make the program for easier installation.

Comments and suggestions:
-------------------------

I hope `tifaa` will be useful for those of you that are confronted
with the same problem I was! If you find any problems in this program
please contact me so I can correct them. I would also be very glad to
hear any suggestions or comments you might have, thank you.

makhlaghi@gmail.com 

akhlaghi@astr.tohoku.ac.jp

http://astr.tohoku.ac.jp/~akhlaghi/

Copyright:
----------

Copyright (C) 2013-2014 Mohammad Akhlaghi

Tohoku University Astronomical Institute

http://astr.tohoku.ac.jp/~akhlaghi/

`tifaa` is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

`tifaa` is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with `tifaa`.  If not, see <http://www.gnu.org/licenses/>.
