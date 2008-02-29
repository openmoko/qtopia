/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef QTOPIATIMER_H
#define QTOPIATIMER_H

#include <QTimer>
#include <qtopiaglobal.h>

class QtopiaTimerPrivate;
class QTOPIABASE_EXPORT  QtopiaTimer : public QObject
{
Q_OBJECT
public:
    QtopiaTimer(QObject *parent = 0);
    virtual ~QtopiaTimer();

    enum Type { Normal            = 0x0000,
                PauseWhenInactive = 0x0001 };

    Type type() const;

    int interval () const;
    bool isActive () const;
    void setInterval(int msec, QtopiaTimer::Type = Normal);

    bool isSingleShot() const;
    void setSingleShot(bool);
   
public slots: 
    void start();
    void stop();
    void start(int msec, QtopiaTimer::Type = Normal);

signals:
    void timeout();

protected:
    virtual void timerEvent(QTimerEvent *);

private slots:
    void activeChanged();

private:
    void disable(bool);
    void enable();

    QtopiaTimerPrivate *d;
};

#endif // QTOPIATIMER_H
