/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#ifndef _e2_TELEPHONYBAR_H_
#define _e2_TELEPHONYBAR_H_

#include <QWidget>
#include <QBrush>
#include <QPixmap>
#include <QValueSpaceItem>
#include <QString>

class E2TelephonyBar : public QWidget
{
Q_OBJECT
public:
    E2TelephonyBar(QWidget *parent = 0, Qt::WFlags = 0);

public slots:
    void batteryChanged();
    void signalChanged();
    void timeChanged();

private:
    virtual void paintEvent(QPaintEvent *);

    QPixmap fillBrush;
    QPixmap batPix;
    QPixmap signalPix;
    QPixmap cBatPix;
    QPixmap cSignalPix;
    QString cTime;
    QValueSpaceItem time;
    QValueSpaceItem battery;
    QValueSpaceItem signal;
};

#endif // _e2_TELEPHONYBAR_H_

