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

#ifndef __QWLAN_REGISTRATION__H
#define __QWLAN_REGISTRATION__H

#include <custom.h>

#ifndef NO_WIRELESS_LAN

#include <QCommInterface>
#include <qtopiaglobal.h>

class QWlanRegistrationPrivate;
class QTOPIACOMM_EXPORT QWlanRegistration : public QCommInterface 
{
    Q_OBJECT
public:
    explicit QWlanRegistration( const QString& devHandle,
                        QObject* parent = 0, QAbstractIpcInterface::Mode mode = QAbstractIpcInterface::Client );
    ~QWlanRegistration();

    QString currentESSID() const;
    QStringList knownESSIDs() const;
    
signals:
    void accessPointChanged();

private:
    QWlanRegistrationPrivate* dptr;
};
#endif //NO_WIRELESS_LAN
#endif //__QWLAN_REGISTRATION__H
