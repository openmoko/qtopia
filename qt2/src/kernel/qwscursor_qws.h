/****************************************************************************
** $Id: qt/src/kernel/qwscursor_qws.h   2.3.12   edited 2005-10-27 $
**
** Definition of QWSCursor class
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

#ifndef QWSCURSOR_H
#define QWSCURSOR_H

#ifndef QT_H
#include <qimage.h>
#endif // QT_H

class QWSCursor : public Qt
{
public:
    QWSCursor() {}
    QWSCursor(const uchar *data, const uchar *mask,
                int width, int height, int hotX, int hotY)
	{ set(data, mask, width, height, hotX, hotY); }

    void set(const uchar *data, const uchar *mask,
		int width, int height, int hotX, int hotY);

    QPoint hotSpot() const { return hot; }
    QImage &image() { return cursor; }
    const QRegion region() const { return rgn; }

    static QWSCursor *systemCursor(int id);

private:
    static void createSystemCursor( int id );	
    void createDropShadow(int dropx, int dropy);

private:
    QPoint hot;
    QImage cursor;
    QRegion rgn;
};



#endif // QWSCURSOR_H
