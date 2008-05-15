/****************************************************************************
**
** Copyright (C) 2008-2008 TROLLTECH ASA. All rights reserved.
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

#include <qtopiaglobal.h>
#include <qcopchannel_qd.h>
#include <QtopiaIpcEnvelope>

class device_updaterPlugin : public QObject
{
    Q_OBJECT
public:
    device_updaterPlugin( QObject *parent = 0 )
        : QObject( parent )
    {
        QCopChannel *chan = new QCopChannel( "QPE/Application/packagemanager", this );
        connect( chan, SIGNAL(received(const QString&,const QByteArray&)), this, SLOT(received(const QString&,const QByteArray&)) );
    }

    ~device_updaterPlugin()
    {
    }

private slots:
    void received( const QString &message, const QByteArray &data )
    {
        if ( message == "PackageManager::installPackageConfirm(QString)" ) {
            QString file;
            QDataStream stream( data );
            stream >> file;
            // No worries about circular delivery because this does not talk to the "local" QCop classes
            QtopiaIpcEnvelope e("QPE/Application/packagemanager", "PackageManager::installPackageConfirm(QString)");
            e << file;
        }
    }

};

QTOPIA_EXPORT_PLUGIN(device_updaterPlugin)

#include "main.moc"