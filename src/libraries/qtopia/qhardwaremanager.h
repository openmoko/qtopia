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

#ifndef QHARDWAREMANAGER_H
#define QHARDWAREMANAGER_H

// Local includes
#include "qabstractipcinterfacegroupmanager.h"

class QHardwareManagerPrivate;
class QTOPIA_EXPORT QHardwareManager : public QAbstractIpcInterfaceGroupManager
{
    Q_OBJECT
public:
    explicit QHardwareManager( const QString& interface, QObject *parent = 0 );
    virtual ~QHardwareManager();

    QString interface() const;
    QStringList providers() const;

    template <typename T> static QStringList providers()
    {
        QAbstractIpcInterfaceGroupManager man( "/Hardware/Accessories" );
        return man.supports<T>();
    }

signals:
    void providerAdded( const QString& id );
    void providerRemoved( const QString& id );
private:
    QHardwareManagerPrivate* d;
};

#endif //QHARDWAREMANAGER_H
