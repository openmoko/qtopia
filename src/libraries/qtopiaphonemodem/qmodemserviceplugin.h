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

#ifndef QMODEMSERVICEPLUGIN_H
#define QMODEMSERVICEPLUGIN_H

#include <qfactoryinterface.h>

#include <qtopiaglobal.h>

class QModemService;
class QSerialIODeviceMultiplexer;

struct QTOPIAPHONEMODEM_EXPORT QModemServicePluginInterface : public QFactoryInterface
{
    virtual bool supports( const QString& manufacturer ) = 0;
    virtual QModemService *create
            ( const QString& service, QSerialIODeviceMultiplexer *mux,
              QObject *parent ) = 0;
};
#define QModemServicePluginInterface_iid "com.trolltech.Qtopia.QModemServicePluginInterface"
Q_DECLARE_INTERFACE(QModemServicePluginInterface, QModemServicePluginInterface_iid)

class QTOPIAPHONEMODEM_EXPORT QModemServicePlugin : public QObject, public QModemServicePluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QModemServicePluginInterface:QFactoryInterface)
public:
    explicit QModemServicePlugin( QObject* parent = 0 );
    ~QModemServicePlugin();

    QStringList keys() const;
    bool supports( const QString& manufacturer );
    QModemService *create( const QString& service,
                           QSerialIODeviceMultiplexer *mux,
                           QObject *parent );
};

#endif /* QMODEMSERVICEPLUGIN_H */
