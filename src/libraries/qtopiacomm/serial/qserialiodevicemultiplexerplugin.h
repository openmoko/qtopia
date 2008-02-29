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

#ifndef QSERIALIODEVICEMULTIPLEXERPLUGIN_H
#define QSERIALIODEVICEMULTIPLEXERPLUGIN_H

#include <qfactoryinterface.h>

#include <qtopiaglobal.h>

class QSerialIODevice;
class QSerialIODeviceMultiplexer;

struct QTOPIACOMM_EXPORT QSerialIODeviceMultiplexerPluginInterface : public QFactoryInterface
{
    virtual bool forceGsm0710Stop() = 0;
    virtual bool detect( QSerialIODevice *device ) = 0;
    virtual QSerialIODeviceMultiplexer *create( QSerialIODevice *device ) = 0;
};
#define QSerialIODeviceMultiplexerPluginInterface_iid "com.trolltech.Qtopia.QSerialIODeviceMultiplexerPluginInterface"
Q_DECLARE_INTERFACE(QSerialIODeviceMultiplexerPluginInterface, QSerialIODeviceMultiplexerPluginInterface_iid)

class QTOPIACOMM_EXPORT QSerialIODeviceMultiplexerPlugin : public QObject, public QSerialIODeviceMultiplexerPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QSerialIODeviceMultiplexerPluginInterface:QFactoryInterface)
public:
    QSerialIODeviceMultiplexerPlugin( QObject* parent = 0 );
    ~QSerialIODeviceMultiplexerPlugin();

    QStringList keys() const;
    bool forceGsm0710Stop();
    bool detect( QSerialIODevice *device );
    QSerialIODeviceMultiplexer *create( QSerialIODevice *device );
};

#endif /* QSERIALIODEVICEMULTIPLEXERPLUGIN_H */
