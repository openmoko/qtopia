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

#ifndef __CITYINFO_H__
#define __CITYINFO_H__

// Qt4 Headers
#include <QFrame>
#include <QDateTime>

class CityInfo : public QFrame
{
    Q_OBJECT
public:
    CityInfo(QWidget *parent, Qt::WFlags f = 0 );

    QString zone() const { return mZone; }
    QSize sizeHint() const;
QSizePolicy sizePolicy() const
     { return QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed); }

public slots:
    void setZone(const QString &zone);
    void setUtcTime(const QDateTime &);

protected:
    void paintEvent( QPaintEvent *);

private:
    QString text() const;
    QString mZone;
    QDateTime mUtc;
};

#endif
