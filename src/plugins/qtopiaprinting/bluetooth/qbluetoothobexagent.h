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

#ifndef QBLUETOOTHOBEXAGENT_H
#define QBLUETOOTHOBEXAGENT_H

#include <qbluetoothremotedevice.h>
#include <qbluetoothnamespace.h>
#include <qbluetoothsdpquery.h>
#include <qcontent.h>

class QBluetoothObexAgentPrivate;

class QBluetoothObexAgent : public QObject
{
    Q_OBJECT
public:
    QBluetoothObexAgent( const QBluetoothRemoteDevice &remoteDevice,
            QBluetooth::SDPProfile profile = QBluetooth::ObjectPushProfile,
            QObject *parent = 0 );
    ~QBluetoothObexAgent();

    void send( const QString &fileName, const QString &mimeType = QString() );
    void send( const QContent &content );
    void send( const QByteArray &array, const QString &fileName, const QString &mimeType = QString() );
    void sendHtml( const QString &html );

    void setAutoDelete( const bool autoDelete );

public slots:
    void abort();

signals:
    void done( bool error );

private slots:
    void searchComplete( const QBluetoothSdpQueryResult &result );
    void progress( qint64, qint64 );

private:
    void startSearch();
    bool inProgress();

    QBluetoothObexAgentPrivate *d;
};


#endif // QBLUETOOTHOBEXAGENT_H
