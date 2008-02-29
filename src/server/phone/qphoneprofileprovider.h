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

#ifndef _QPHONEPROFILEPROVIDER_H_
#define _QPHONEPROFILEPROVIDER_H_

#include <QObject>

class QString;
class QByteArray;
class QPhoneProfileProviderPrivate;
class QPhoneProfileProvider : public QObject
{
Q_OBJECT
public:
    QPhoneProfileProvider(QObject *parent = 0);
    virtual ~QPhoneProfileProvider();

private slots:
    void appMessage(const QString &msg, const QByteArray &data);
    void scheduleActivation();
    void activeChanged();
    void audioProfileChanged();

private:
    Q_DISABLE_COPY(QPhoneProfileProvider);
    QPhoneProfileProviderPrivate *d;
};

#endif // _QPHONEPROFILEPROVIDER_H_
