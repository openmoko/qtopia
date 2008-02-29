/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#ifndef PHOTOTIMER_H
#define PHOTOTIMER_H

#include <QWidget>
#include <QDialog>

class QSpinBox;
class QLCDNumber;
class QAnalogClock;

class PhotoTimer : public QWidget
{
    Q_OBJECT

public:
    PhotoTimer( int timeout,
                int number,
                int interval,
                QWidget* parent = 0,
                Qt::WFlags f = 0 );
signals:
    void takePhoto();

private slots:
    void timeout();

private:
    int mTimeout;
    int mNumber;
    int mInterval;

    QAnalogClock* mClock;
};

class PhotoTimerDialog : public QDialog
{
    Q_OBJECT

public:
    PhotoTimerDialog( QWidget* parent = 0, Qt::WFlags f = 0 );

    int timeout() const;
    int number() const;
    int interval() const;

private slots:
    void timeoutChanged( int timeout );
    void numberChanged( int number );
    void intervalChanged( int interval );

private:
    int mTimeout;
    int mNumber;
    int mInterval;

    QSpinBox* mIntervalSpin;
};

#endif //PHOTOTIMER_H
