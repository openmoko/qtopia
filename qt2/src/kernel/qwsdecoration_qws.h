/****************************************************************************
** $Id: qt/src/kernel/qwsdecoration_qws.h   2.3.12   edited 2005-10-27 $
**
** Definition of QWSDecoration class.
**
** Created : 20000308
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
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
#ifndef QWSDECORATION_H
#define QWSDECORATION_H

#ifndef QT_H
#include "qregion.h"
#endif // QT_H

class QPopupMenu;

/*
 Implements decoration styles
*/
class QWSDecoration
{
public:
    QWSDecoration() {}
    virtual ~QWSDecoration() {}

    enum Region { None=0, All=1, Title=2, Top=3, Bottom=4, Left=5, Right=6,
		TopLeft=7, TopRight=8, BottomLeft=9, BottomRight=10,
		Close=11, Minimize=12, Maximize=13, Normalize=14,
		Menu=15, LastRegion=Menu };

    virtual QRegion region(const QWidget *, const QRect &rect, Region r=All) = 0;
    virtual void close( QWidget * );
    virtual void minimize( QWidget * );
    virtual void maximize( QWidget * );
#ifndef QT_NO_POPUPMENU
    virtual QPopupMenu *menu(const QWidget *, const QPoint &);
#endif
    virtual void paint(QPainter *, const QWidget *) = 0;
    virtual void paintButton(QPainter *, const QWidget *, Region, int state) = 0;
};


#endif
