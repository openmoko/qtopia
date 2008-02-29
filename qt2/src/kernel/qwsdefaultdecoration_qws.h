/****************************************************************************
** $Id: qt/src/kernel/qwsdefaultdecoration_qws.h   2.3.12   edited 2005-10-27 $
**
** Declaration of Qt/Embedded window manager decorations
**
** Created : 000101
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
#ifndef __QWS_DEFAULT_DECORATION_H__
#define __QWS_DEFAULT_DECORATION_H__

#ifndef QT_H
#include "qwsmanager_qws.h"
#endif // QT_H

#ifndef QT_NO_QWS_MANAGER

#define CORNER_GRAB	16
#define BORDER_WIDTH	4
#define BOTTOM_BORDER_WIDTH	2*BORDER_WIDTH



class QWSDefaultDecoration : public QWSDecoration
{
public:
    QWSDefaultDecoration();
    virtual ~QWSDefaultDecoration();

    virtual QRegion region(const QWidget *, const QRect &rect, Region);
    virtual void paint(QPainter *, const QWidget *);
    virtual void paintButton(QPainter *, const QWidget *, Region, int state);

protected:
    virtual const QPixmap* pixmapFor(const QWidget *, Region, bool, int&, int&);

    /* Added these virtual functions to enable other styles to be added more easily */
    virtual int getTitleWidth(const QWidget *);
    virtual int getTitleHeight(const QWidget *);

#ifndef QT_NO_IMAGEIO_XPM
    virtual const char **menuPixmap();
    virtual const char **closePixmap();
    virtual const char **minimizePixmap();
    virtual const char **maximizePixmap();
    virtual const char **normalizePixmap();
#endif

private:

    static QPixmap * staticMenuPixmap;
    static QPixmap * staticClosePixmap;
    static QPixmap * staticMinimizePixmap;
    static QPixmap * staticMaximizePixmap;
    static QPixmap * staticNormalizePixmap;

};

#endif // QT_NO_QWS_MANAGER

#endif
