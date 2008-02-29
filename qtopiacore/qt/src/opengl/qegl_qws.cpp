/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtOpenGL module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
** 
** Trolltech ASA (c) 2007
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/


/*
Example QEGL interface layer for the "vanilla" EGL implementation from
Hybrid Graphics, Ltd.
*/

#include "qegl_qws_p.h"
#ifndef Q_USE_EGLWINDOWSURFACE
#include <qimage.h>
#include <qrect.h>
#include <vanilla/eglVanilla.h>
#include <qdebug.h>

static void imgToVanilla(QImage &img, VanillaPixmap *pix)
{

    pix->width = img.width();
    pix->height = img.height();
    pix->stride = img.bytesPerLine();


    if (img.format() == QImage::Format_RGB32 || img.format() == QImage::Format_ARGB32) {
        pix->rSize = pix->gSize = pix->bSize = pix->aSize = 8;
        pix->lSize = 0;
        pix->rOffset = 16;
        pix->gOffset = 8;
        pix->bOffset = 0;
        pix->aOffset = 24;

    } else if (img.format() == QImage::Format_RGB16) {
        pix->rSize = 5;
        pix->gSize = 6;
        pix->bSize = 5;
        pix->aSize = 0;
        pix->lSize = 0;
        pix->rOffset = 11;
        pix->gOffset = 5;
        pix->bOffset = 0;
        pix->aOffset = 0;
    }

    pix->padding = pix->padding2 = 0;

    pix->pixels = img.bits();
}





//NativeWindowType QEGL::createNativeWindow(const QRect&);
//NativeWindowType QEGL::toNativeWindow(QWidget *);

NativePixmapType QEGL::createNativePixmap(QImage * img)
{
    VanillaPixmap *pix = new VanillaPixmap;
    imgToVanilla(*img, pix);
    return pix;
}

void QEGL::destroyNativePixmap(NativePixmapType pix)
{
    delete (VanillaPixmap *)pix;
}
#endif
