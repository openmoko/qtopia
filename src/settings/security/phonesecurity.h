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
#ifndef PHONESECURITY_H
#define PHONESECURITY_H

#include <QObject>
#include <qpinmanager.h>

class PhoneSecurity : public QObject {
    Q_OBJECT
public:
    PhoneSecurity(QObject * parent);
    void setLockType(int t);
    void markProtected(int t, bool b, const QString& pw);
    void changePassword(int t, const QString& old, const QString& new2);

signals:
    void changed(bool);
    void locked(bool);
    void lockDone(bool);

private slots:
    void lockStatus(const QString& type, bool enabled );
    void setLockStatusResult(const QString& type, bool valid );
    void changePinResult(const QString& type, bool valid );

private:
    QPinManager *pinManager;
    int locktype;
};

#endif
