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
#include <qdplugin.h>
#include <qtopiadesktoplog.h>
#include <qcopchannel_qd.h>
#include <trace.h>
#include <QVariant>

class Qtopia4Device : public QDDevPlugin
{
    Q_OBJECT
    QD_CONSTRUCT_PLUGIN(Qtopia4Device,QDDevPlugin)
public:
    // QDPlugin
    QString id() { return "com.trolltech.plugin.dev.qtopia4"; }
    QString displayName() { return tr("Qtopia 4.3 Device"); }

    QString model() { return mModel; }
    QString system() { return mSystem; }
    quint32 version() { return mVersion; }
    QString versionString() { return mVersionString; }
    QPixmap icon() { return QPixmap(":image/appicon"); }
    int port() { return 4243; }

    // Doers

    void probe( QDConPlugin *con );
    void disassociate( QDConPlugin *con );

    void requestCardInfo() {}
    void requestInstallLocations() {}
    void requestAllDocLinks() {}
    void installPackage() {}
    void removePackage() {}
    void setSyncDate() {}

private slots:
    void message( const QString &message, const QByteArray &data );

private:
    QDConPlugin *connection;

    QString mModel;
    QString mSystem;
    quint32 mVersion;
    QString mVersionString;
};

QD_REGISTER_PLUGIN(Qtopia4Device)

void Qtopia4Device::probe( QDConPlugin *con )
{
    if ( con->conProperty("system") == "Qtopia" && con->conProperty("protocol") == "2" ) {
        if ( con->claim( this ) ) {
            mModel = con->conProperty("model");
            mSystem = con->conProperty("system");
            mVersionString = con->conProperty("version");
            mVersion = QVariant(con->conProperty("hexversion")).toUInt();
            connection = con;
            connect( connection, SIGNAL(receivedMessage(QString,QByteArray)),
                    this, SLOT(message(QString,QByteArray)) );
            connection->connected( this );
        }
    }
}

void Qtopia4Device::disassociate( QDConPlugin *con )
{
    Q_ASSERT( connection == con );
    disconnect( connection, SIGNAL(receivedMessage(QString,QByteArray)),
            this, SLOT(message(QString,QByteArray)) );
    connection = 0;
}

void Qtopia4Device::message( const QString &message, const QByteArray &data )
{
    QDataStream stream( data );
    if ( message == "forwardedMessage(QString,QString,QByteArray)" ) {
        QString _channel;
        QString _message;
        QByteArray _data;
        stream >> _channel >> _message >> _data;
        TRACE(QDDev) << "Qtopia4Device::message" << "channel" << _channel << "message" << _message << "data" << _data;
        QCopChannel::send( _channel, _message, _data );
    }
}

#include "qtopia4device.moc"
