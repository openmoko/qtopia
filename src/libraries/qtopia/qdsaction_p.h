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

#ifndef QDS_ACTION_P_H
#define QDS_ACTION_P_H

// Local includes
#include "qdsaction.h"
#include "qdsserviceinfo.h"
#include "qdsdata.h"

// Qt includes
#include <QObject>
#include <QUuid>

// Qtopia includes
#include <QMimeType>

// ============================================================================
//
//  Forward class declarations
//
// ============================================================================

class QDSData;
class QTimer;
class QUuid;
class QEventLoop;
class QtopiaIpcAdaptor;

// ============================================================================
//
//  QDSActionPrivate
//
// ============================================================================

class QDSActionPrivate : public QObject
{
    Q_OBJECT

public:
    QDSActionPrivate();
    QDSActionPrivate( const QDSActionPrivate& other );
    QDSActionPrivate( const QString& name,
                      const QString& service );
    QDSActionPrivate( const QDSServiceInfo& serviceInfo );
    ~QDSActionPrivate();

    // Methods
    bool requestActive();
    void emitRequest();
    void emitRequest( const QDSData& data, const QByteArray& auxiliary );
    void reset();
    void connectToAction( QDSAction* action );

    // Data members
    QLocalUniqueId      mId;
    QDSServiceInfo      mServiceInfo;
    QtopiaIpcAdaptor*   mResponseChannel;
    QTimer*             mTimer;
    QEventLoop*         mEventLoop;
    QDSData             mResponseData;
    QString             mErrorMsg;
    int                 mResponseCode;

    static QUniqueIdGenerator mIdGen;

private slots:
    void heartbeatSlot();
    void responseSlot();
    void responseSlot( const QDSData& responseData );
    void requestTimeoutSlot();
    void errorSlot( const QString& message );

signals:
    void response( const QLocalUniqueId& actionId );
    void response( const QLocalUniqueId& actionId, const QDSData& responseData );
    void error( const QLocalUniqueId& actionId, const QString& message );

private:
    void connectToChannel();
    void disconnectFromChannel();
    void startTimer();

    QString responseChannel();
};

#endif //QDS_ACTION_P_H
