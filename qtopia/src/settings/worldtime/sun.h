/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
/*
 * Sun clock definitions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#ifndef PI
#define PI 3.14159265358979323846
#endif

#ifndef E
#define E 2.7182818284590452354
#endif

#define abs(x) ((x) < 0 ? (-(x)) : x)			  /* Absolute value */
#define sgn(x) (((x) < 0) ? -1 : ((x) > 0 ? 1 : 0))	  /* Extract sign */
#define dtr(x) ((x) * (PI / 180.0))			  /* Degree->Radian */
#define rtd(x) ((x) / (PI / 180.0))			  /* Radian->Degree */
#define fixangle(a) ((a) - 360.0 * (qFloor((a) / 360.0)))  /* Fix angle	  */

#define TERMINC  100		   /* Circle segments for terminator */

#define PROJINT  (60 * 10)	   /* Frequency of seasonal recalculation */


#ifdef __cplusplus
extern "C" {
#endif
double jtime(struct tm *t);
double kepler(double m, double ecc);
void sunpos(double jd, int apparent, double *ra, double *dec, double *rv, double *slong);
void projillum(short *wtab, int xdots, int ydots, double dec);
#ifdef __cplusplus
};
#endif
