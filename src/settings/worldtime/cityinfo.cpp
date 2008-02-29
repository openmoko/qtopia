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

#include "cityinfo.h"

#include <qtimestring.h>
#include <qtimezone.h>
#include <qtopialog.h>

#include <QStyleOption>
#include <QPalette>
#include <QPainter>

CityInfo::CityInfo(QWidget *parent, Qt::WFlags f)
    : QFrame(parent,f)
{
    mUtc = QTimeZone::utcDateTime();
}

void CityInfo::setZone(const QString &zone)
{
    mZone = zone;
    qLog(Time) << "CityInfo()::setZone() " <<
        mZone.toLocal8Bit().constData();
}

void CityInfo::setUtcTime(const QDateTime &dt)
{
    mUtc = dt;
    repaint();
}

QString CityInfo::text() const
{
    QString line;
    QDateTime cityTime;

    if ( !mZone.isNull() ) {
        QTimeZone curZone( mZone.toLocal8Bit().constData() );
        if ( curZone.isValid() )
            cityTime = curZone.fromUtc(mUtc);
        else
            qLog(Time) << "CityInfo()::text() curZone Not valid!";
        line = QTimeString::localHMDayOfWeek(cityTime);
        qLog(Time) << "CityInfo()::text() cityTime=" << cityTime.toString("hh:mm");
        qLog(Time) << "CityInfo()::text() timestring=" << line.toLocal8Bit().constData();
        return line;
    }
    else {
        qLog(Time) << "CityInfo()::text() - mZone is NULL!";
        return QString();
    }
}

void CityInfo::paintEvent( QPaintEvent * )
{
    QPainter p(this);
    QStyleOptionHeader opt;

    opt.palette = palette();
    opt.state = QStyle::State_Enabled;
    opt.state |= QStyle::State_Horizontal;

    QRect cr = contentsRect();
    style()->drawItemText( &p, cr, Qt::AlignRight, opt.palette,
            opt.state, text(), QPalette::ButtonText);

    drawFrame(&p);
}


QSize CityInfo::sizeHint() const
{
    QSize res;
    QFontMetrics fm( font() );
    res.setWidth(fm.width(text())+2);
    res.setHeight(fm.height());
    qLog(Time) << "CityInfo(QFrame)::sizeHint(w=" << res.width() << ", h=" << res.height();
    return res;
}
