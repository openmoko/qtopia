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
#ifndef QCATEGORYSTORESERVER_P_H
#define QCATEGORYSTORESERVER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qtopia/private/qdocumentserverchannel_p.h>
#include <private/qunixsocketserver_p.h>
#include <private/qunixsocket_p.h>

#include <QThread>

class QCategoryStoreServer : public QDocumentServerHost
{
    Q_OBJECT
public:
    QCategoryStoreServer( QObject *parent = 0 );

protected:
    virtual QDocumentServerMessage invokeMethod( const QDocumentServerMessage &message );

private slots:
    void categoriesChanged();
};

class QCategoryStoreSocketServer : public QUnixSocketServer
{
public:
    QCategoryStoreSocketServer( QObject *parent = 0 );

protected:
    virtual void incomingConnection( int socketDescriptor );
};

class QCategoryStoreServerTask : public QThread
{
public:
    QCategoryStoreServerTask( QObject *parent = 0 );

protected:
    virtual void run();
};

#endif
