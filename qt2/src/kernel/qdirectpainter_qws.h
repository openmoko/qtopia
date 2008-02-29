/****************************************************************************
** $Id: qt/src/kernel/qdirectpainter_qws.h   2.3.12   edited 2005-10-27 $
**
** Definition of QDirectPainter class
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

#ifndef QDIRECTPAINTER_H
#define QDIRECTPAINTER_H

#ifndef QT_H
#include <qpainter.h>
#endif // QT_H

#ifdef _WS_QWS_

class QDirectPainterPrivate;


class Q_EXPORT QDirectPainter : public QPainter
{
public:
    QDirectPainter( const QWidget* );
    ~QDirectPainter();

    uchar* frameBuffer();
    int lineStep();
    int transformOrientation();

    int numRects() const;
    const QRect& rect(int i) const;
    QRegion region();

    int depth() const;
    int width() const;
    int height() const;
    int xOffset() const;
    int yOffset() const;

    QPoint offset() const;
    QSize size() const;

private:
    QDirectPainterPrivate* d;
};

#endif

#endif // QDIRECTPAINTER_H
