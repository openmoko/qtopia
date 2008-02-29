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

// !!! IMPORTANT !!!
// This interface is still experimental and subject to change.

#ifndef WINDOWDECORATIONINTERFACE_H
#define WINDOWDECORATIONINTERFACE_H

#include <qtopia/qpeglobal.h>
#include <qtopia/qcom.h>
#include <qpixmap.h>
#include <qpalette.h>
#include <qwsmanager_qws.h>

// {11A45864-4CBA-4DDA-9846-FF234FD307CC} 
#ifndef IID_WindowDecoration
#define IID_WindowDecoration QUuid( 0x11a45864, 0x4cba, 0x4dda, 0x98, 0x46, 0xff, 0x23, 0x4f, 0xd3, 0x07, 0xcc)
#endif

struct QTOPIA_EXPORT WindowDecorationInterface : public QUnknownInterface
{
    virtual ~WindowDecorationInterface() {}

    struct WindowData {
	QRect rect;
	QPalette palette;
	QString caption;
	enum Flags { Maximized=0x01, Dialog=0x02, Active=0x04 };
	Q_UINT32 flags;
	Q_UINT32 reserved;
    };

    enum Metric { TitleHeight, LeftBorder, RightBorder, TopBorder, BottomBorder, OKWidth, CloseWidth, HelpWidth, MaximizeWidth, CornerGrabSize };
    virtual int metric( Metric m, const WindowData * ) const;

    enum Area { Border, Title, TitleText };
    virtual void drawArea( Area a, QPainter *, const WindowData * ) const;

    enum Button { OK, Close, Help, Maximize };
    virtual void drawButton( Button b, QPainter *, const WindowData *, int x, int y, int w, int h, QWSButton::State ) const;

    virtual QRegion mask( const WindowData * ) const;

    virtual QString name() const = 0;
    virtual QPixmap icon() const = 0;
};

#endif
