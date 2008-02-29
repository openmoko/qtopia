/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS and its licensors.
** All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See below for additional copyright and license information
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
// -*- c++ -*-

#ifndef KRFBSERVERINFO_H
#define KRFBSERVERINFO_H

#include <qstring.h>

typedef unsigned char CARD8;
typedef unsigned short CARD16;
typedef unsigned long CARD32;

class KRFBPixelFormat
{
public:
  CARD8 bpp;
  CARD8 depth;
  CARD8 bigEndian;
  CARD8 trueColor;
  CARD16 redMax;
  CARD16 greenMax;
  CARD16 blueMax;
  CARD8 redShift;
  CARD8 greenShift;
  CARD8 blueShift;
  CARD8 padding[3]; // 3 bytes padding
};

/**
 * Information sent by the server in its init message.
 */
class KRFBServerInfo : public KRFBPixelFormat
{
public:
  CARD16 width;
  CARD16 height;
  CARD32 nameLength;
  QString name;
};

#endif // KRFBSERVERINFO_H


