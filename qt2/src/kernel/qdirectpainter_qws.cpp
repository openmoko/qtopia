/****************************************************************************
** $Id: qt/src/kernel/qdirectpainter_qws.cpp   2.3.12   edited 2005-10-27 $
**
** Implementation of QDirectPainter class
**
** Created : 010101
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qdirectpainter_qws.h"

#ifdef _WS_QWS_

#include <qgfxraster_qws.h>

// gain access to some internal QGfx functionality you'll need.
class QDirectPainterGfx : public QGfxRasterBase {
    static void* gfxdata;
public:
    QDirectPainterGfx() : QGfxRasterBase(0,0,0) { }

    void beginTransaction(const QRect& r) { gfxdata=QGfxRasterBase::beginTransaction(r); }
    void endTransaction() { QGfxRasterBase::endTransaction(gfxdata); }
    unsigned char * memory() { return buffer; }
    int numRects() const { return ncliprect; }
    const QRect& rect(int i) const { return cliprect[i]; }
};

void *QDirectPainterGfx::gfxdata = 0;

class QDirectPainterPrivate {
public:
    QDirectPainterPrivate()
    {
	paintOk = TRUE;
	image = 0;
	gfx = 0;
    }

    QDirectPainterGfx* gfx;
    bool paintOk;
    QImage* image;
    QPoint offset;
    int w,h;
};

/*!
    \class QDirectPainter qdirectpainter_qws.h
    \brief The QDirectPainter class provides direct access to the video hardware.

    \ingroup graphics

    Only available in Qt/Embedded.

    When the hardware is known and well defined, as is often the case
    with software for embedded devices, it may be useful to manipulate
    the underlying video hardware directly. In order to do this in a
    way that is co-operative with other applications, you must lock
    the video hardware for exclusive use for a small time while you
    write to it, and you must know the clipping region which is
    allocated to a widget.

    QDirectPainter provides this functionality.

    In the simplest case, you make a QDirectPainter on a widget and
    then, observing the clip region, perform some platform-specific
    operation. For example:
    \code
	void MyWidget::updatePlatformGraphics()
	{
	    QDirectPainter dp( this );
	    for ( int i = 0; i < dp.numRects; i++ ) {
		const QRect& clip = dp.rect(i);
		... // platform specific operation
	    }
	}
    \endcode

    The platform-specific code has access to the display, but should
    only modify graphics in the rectangles specified by numRects() and
    rect(). Note that these rectangles are relative to the entire
    display.

    The offset() function returns the position of the widget relative
    to the entire display, allowing you to offset platform-specific
    operations appropriately. The xOffset() and yOffset() functions
    merely return the component values of offset().

    For simple frame-buffer hardware, the frameBuffer(), lineStep(),
    and depth() functions provide basic access, though some hardware
    configurations are insufficiently specified by such simple
    parameters.

    Note that while a QDirectPainter exists, the entire Qt/Embedded
    window system is locked from use by other applications. Always
    construct the QDirectPainter as an auto (stack) variable, and be
    very careful to write robust and stable code within its scope.
*/

/*!
    Construct a direct painter on \a w. The display is locked and the
    mouse cursor is hidden if it is above \a w.
*/
QDirectPainter::QDirectPainter( const QWidget* w ) :
    QPainter(w,w)
{
    d = new QDirectPainterPrivate;
    d->gfx = (QDirectPainterGfx*)internalGfx();
    d->offset = w->mapToGlobal(QPoint(0,0));
    d->w = w->width();
    d->h = w->height();
    d->gfx->beginTransaction(QRect(d->offset,size()));
}

/*!
    Destroys the direct painter. The mouse cursor is revealed if
    necessary and the display is unlocked.
*/
QDirectPainter::~QDirectPainter()
{
    // Fake a full-refresh (only does something on VNC and VFB GFX)
    setPen(NoPen);
    setBrush(NoBrush);
    d->gfx->fillRect(0,0,d->w,d->h);
    d->gfx->endTransaction();
    delete d;
}


/*!
    Returns a pointer to the framebuffer memory if available.
*/
uchar* QDirectPainter::frameBuffer() { return d->gfx->memory(); }

/*!
    Returns the spacing in bytes from one framebuffer line to the
    next.
*/
int QDirectPainter::lineStep() { return d->gfx->linestep(); }

/*!
    Returns a number that signifies the orientation of the
    framebuffer.
    \table
    \row \i11 0 \i11 no rotation
    \row \i11 1 \i11 90 degrees rotation
    \row \i11 2 \i11 180 degrees rotation
    \row \i11 3 \i11 270 degrees rotation
    \endtable
*/
int QDirectPainter::transformOrientation()
{
    return qt_screen->transformOrientation();
}

/*!
    Returns the number of rectangles in the drawable region.

    \sa rect(), region()
*/
int QDirectPainter::numRects() const { return d->gfx->numRects(); }

/*!
    Returns a reference to rectangle \a i of the drawable region.
    Valid values for \a i are 0..numRects()-1.

    \sa region()
*/
const QRect& QDirectPainter::rect(int i) const { return d->gfx->rect(i); }

/*!
    Returns the bit-depth of the display.
*/
int QDirectPainter::depth() const { return d->gfx->bitDepth(); }

/*!
    Returns the width of the widget drawn upon.
*/
int QDirectPainter::width() const { return d->w; }

/*!
    Returns the height of the widget drawn upon.
*/
int QDirectPainter::height() const { return d->h; }

/*!
    Returns the X-position of the widget relative to the entire
    display.
*/
int QDirectPainter::xOffset() const { return d->offset.x(); }

/*!
    Returns the Y-position of the widget relative to the entire
    display.
*/
int QDirectPainter::yOffset() const { return d->offset.y(); }

/*!
    Returns the position of the widget relative to the entire display.
*/
QPoint QDirectPainter::offset() const { return d->offset; }

/*!
    Returns the size of the widget drawn upon.

    \sa width(), height()
*/
QSize QDirectPainter::size() const { return QSize(d->w,d->h); }

#endif
