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

#if defined(QTOPIA_PHONE) && !defined(PHONE_DECORATION_H)
#define PHONE_DECORATION_H

#include <qtopiaglobal.h>
#include <qwindowdecorationinterface.h>
#include <qimage.h>

class PhoneDecorationPrivate;
class DecorationBorderData;

class PhoneDecoration : public QWindowDecorationInterface
{
public:
    PhoneDecoration();
    virtual ~PhoneDecoration();

    virtual int metric( Metric m, const WindowData * ) const;
    virtual void drawArea( Area a, QPainter *, const WindowData * ) const;
    virtual void drawButton( Button b, QPainter *, const WindowData *, int x, int y, int w, int h, QDecoration::DecorationState ) const;
    virtual QRegion mask( const WindowData * ) const;
    virtual QString name() const;
    virtual QPixmap icon() const;

private:
    void drawStretch(QPainter *p, const QRect &r, const DecorationBorderData *data, const QBrush &b, Qt::Orientation o) const;
    QRegion maskStretch(const QRect &r, const DecorationBorderData *data, Qt::Orientation) const;

private:
    PhoneDecorationPrivate *d;
};

#endif
