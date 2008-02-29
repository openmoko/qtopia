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

/*********************************************************************
** Asteroids(R) is a registered trademark of Atari Corporation.  Please
** note that the Asteroids(R) games included in Qtopia was not developed
** by nor is endorsed by Atari Corporation. Trolltech respectfully
** acknowledges Atari Corporation's ownership of the trademark."
**********************************************************************/

/*
 * KAsteroids - Copyright (c) Martin R. Jones 1997
 *
 * Part of the KDE project
 */

#ifndef __LEDMETER_H__
#define __LEDMETER_H__

#include <qframe.h>
#include <qlist.h>

#define QPtrList QList

class KALedMeter : public QFrame
{
    Q_OBJECT
public:
    KALedMeter( QWidget *parent );

    int range() const { return mRange; }
    void setRange( int r );

    int count() const { return mCount; }
    void setCount( int c );

    int value () const { return mValue; }

    void addColorRange( int pc, const QColor &c );

public slots:
    void setValue( int v );

protected:
    virtual void resizeEvent( QResizeEvent * );
    virtual void drawContents( QPainter * );
    void calcColorRanges();

protected:
    struct ColorRange
    {
	int mPc;
	int mValue;
	QColor mColor;
    };

    int mRange;
    int mCount;
    int mCurrentCount;
    int mValue;
    QPtrList<ColorRange> mCRanges;
};

#endif
