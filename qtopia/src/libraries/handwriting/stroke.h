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

#ifndef QIMPENSTROKE_H_
#define QIMPENSTROKE_H_

#include <qobject.h>
#include <qarray.h>
#include <qlist.h>
#include "signiture.h"

class QIMPenStroke
{
public:
    QIMPenStroke();
    QIMPenStroke( const QIMPenStroke & );
    ~QIMPenStroke(){}

    void clear();
    bool isEmpty() const { return links.isEmpty(); }
    unsigned int length() const { return links.count(); }
    unsigned int match( QIMPenStroke *st );
    const QArray<QIMPenGlyphLink> &chain() const { return links; }
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

    QArray<int> tansig() { createSignatures(); return tsig; } // for debugging
    QArray<int> angnsig() { createSignatures(); return asig; } // for debugging
    QArray<int> dstsig() { createSignatures(); return dsig; } // for debugging

protected:
    void createSignatures();
    void internalAddPoint( QPoint p );

protected:
    QPoint startPoint;
    QPoint lastPoint;
    QArray<QIMPenGlyphLink> links;
    TanSigniture tsig;
    AngleSigniture asig;
    DistSigniture dsig;
    mutable QRect bounding;
    int cheight;
    static bool useVertPos;

    friend QDataStream &operator<< (QDataStream &, const QIMPenStroke &);
    friend QDataStream &operator>> (QDataStream &, QIMPenStroke &);
};

typedef QList<QIMPenStroke> QIMPenStrokeList;
typedef QListIterator<QIMPenStroke> QIMPenStrokeIterator;

QDataStream & operator<< (QDataStream & s, const QIMPenStroke &ws);
QDataStream & operator>> (QDataStream & s, const QIMPenStroke &ws);

#endif

