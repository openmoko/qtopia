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

#ifndef QPNGIO_H
#define QPNGIO_H

#ifndef QT_H
#include "qimage.h"
#endif // QT_H

#ifndef QT_NO_IMAGEIO_PNG

void qInitPngIO();

class QIODevice;

#ifndef Q_PNGEXPORT
#if !defined(QT_PLUGIN)
#define Q_PNGEXPORT Q_EXPORT
#else
#define Q_PNGEXPORT
#endif
#endif

class Q_PNGEXPORT QPNGImageWriter {
public:
    QPNGImageWriter(QIODevice*);
    ~QPNGImageWriter();

    enum DisposalMethod { Unspecified, NoDisposal, RestoreBackground, RestoreImage };
    void setDisposalMethod(DisposalMethod);
    void setLooping(int loops=0); // 0 == infinity
    void setFrameDelay(int msecs);
    void setGamma(float);

    bool writeImage(const QImage& img, int x, int y);
    bool writeImage(const QImage& img, int quality, int x, int y);
    bool writeImage(const QImage& img)
	{ return writeImage(img, 0, 0); }
    bool writeImage(const QImage& img, int quality)
	{ return writeImage(img, quality, 0, 0); }

    QIODevice* device() { return dev; }

private:
    QIODevice* dev;
    int frames_written;
    DisposalMethod disposal;
    int looping;
    int ms_delay;
    float gamma;
};

class Q_PNGEXPORT QPNGImagePacker : public QPNGImageWriter {
public:
    QPNGImagePacker(QIODevice*, int depth, int convflags);

    void setPixelAlignment(int x);
    bool packImage(const QImage& img);

private:
    QImage previous;
    int depth;
    int convflags;
    int alignx;
};

#endif // QT_NO_IMAGEIO_PNG

#endif // QPNGIO_H
