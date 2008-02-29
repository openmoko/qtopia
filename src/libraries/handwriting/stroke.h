/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#ifndef QIMPENSTROKE_H_
#define QIMPENSTROKE_H_

#include <qobject.h>
#include <qlist.h>
#include <qiterator.h>
#include <qpoint.h>
#include <qrect.h>

#include <qtopiaglobal.h>

#include "signature.h"

class QTOPIAHW_EXPORT QIMPenStroke
{
public:
    QIMPenStroke();
    QIMPenStroke( const QIMPenStroke & );
    ~QIMPenStroke(){}

    void clear();
    bool isEmpty() const { return links.isEmpty(); }
    unsigned int length() const { return links.count(); }
    unsigned int match( QIMPenStroke *st );
    const QVector<QIMPenGlyphLink> &chain() const { return links; }
    QPoint startingPoint() const { return startPoint; }
    void setStartingPoint( const QPoint &p ) { startPoint = p; }
    QRect boundingRect() const;
    QPoint center() const;

    int canvasHeight() const { return cheight; }
    void setCanvasHeight(int h) { cheight = h; }

    static void setUseCanvasPosition(bool b) { useVertPos = b; }
    static bool useCanvasPosition() { return useVertPos; }

    QIMPenStroke &operator=( const QIMPenStroke &s );

    void beginInput( QPoint p );
    bool addPoint( QPoint p );
    void endInput();

    QVector<int> tansig() { createSignatures(); return tsig; } // for debugging
    QVector<int> angnsig() { createSignatures(); return asig; } // for debugging
    QVector<int> dstsig() { createSignatures(); return dsig; } // for debugging

protected:
    void createSignatures();
    void internalAddPoint( QPoint p );

protected:
    QPoint startPoint;
    QPoint lastPoint;
    QVector<QIMPenGlyphLink> links;
    TanSignature tsig;
    AngleSignature asig;
    DistSignature dsig;
    mutable QRect bounding;
    int cheight;
    static bool useVertPos;

    friend QDataStream &operator<< (QDataStream &, const QIMPenStroke &);
    friend QDataStream &operator>> (QDataStream &, QIMPenStroke &);
};

typedef QList<QIMPenStroke *> QIMPenStrokeList;

typedef QList<QIMPenStroke *>::iterator QIMPenStrokeIterator;
typedef QList<QIMPenStroke *>::const_iterator QIMPenStrokeConstIterator;

QTOPIAHW_EXPORT QDataStream & operator<< (QDataStream & s, const QIMPenStroke &ws);
QTOPIAHW_EXPORT QDataStream & operator>> (QDataStream & s, const QIMPenStroke &ws);

#endif

