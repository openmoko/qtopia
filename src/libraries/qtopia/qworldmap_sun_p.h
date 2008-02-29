/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QWORLDMAP_SUN_P_H
#define QWORLDMAP_SUN_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

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

#define abs(x) ((x) < 0 ? (-(x)) : x)                     /* Absolute value */
#define sgn(x) (((x) < 0) ? -1 : ((x) > 0 ? 1 : 0))       /* Extract sign */
#define dtr(x) ((x) * (PI / 180.0))                       /* Degree->Radian */
#define rtd(x) ((x) / (PI / 180.0))                       /* Radian->Degree */
#define fixangle(a) ((a) - 360.0 * ((int)floor((a) / 360.0)))  /* Fix angle       */

#define TERMINC  100               /* Circle segments for terminator */

#define PROJINT  (60 * 10)         /* Frequency of seasonal recalculation */


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

#endif //QWORLDMAP_SUN_P_H
