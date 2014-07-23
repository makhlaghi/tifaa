/*********************************************************************
timing - For reporting timing in the functions.

Copyright (C) 2013 Mohammad Akhlaghi
Tohoku University Astronomical Institute, Sendai, Japan.
http://astr.tohoku.ac.jp/~akhlaghi/

timing is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

timing is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with timing. If not, see <http://www.gnu.org/licenses/>.

**********************************************************************/
#ifndef TIMING_H
#define TIMING_H

#include <sys/time.h>

void
reporttiming(struct timeval *t1, char *jobname, size_t level);

#endif
