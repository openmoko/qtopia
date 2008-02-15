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
#ifndef QTOPIACUSTOM_H
#define QTOPIACUSTOM_H

#include <custom-qtopia.h>

// The documentation references these lines (see doc/src/syscust/custom.qdoc)
#ifndef V4L_VIDEO_DEVICE
#define V4L_VIDEO_DEVICE "/dev/video"
#endif

#include <qtopiaglobal.h>

QTOPIABASE_EXPORT int qpe_sysBrightnessSteps();
QTOPIABASE_EXPORT void qpe_setBrightness(int bright);

#endif
