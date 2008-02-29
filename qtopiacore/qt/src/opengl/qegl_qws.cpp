/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
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
