/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#ifndef QDS_ACTION_REQUEST_P_H
#define QDS_ACTION_REQUEST_P_H

// Local includes
#include "qdsserviceinfo.h"
#include "qdsdata.h"

// Qt includes
#include <QObject>

// ============================================================================
//
//  QDSHeartBeat
//
// ============================================================================

class QDSHeartBeat : public QObject
{
    Q_OBJECT
public:
    explicit QDSHeartBeat( QObject* parent = 0 );
    explicit QDSHeartBeat( const QString& channel, QObject* parent = 0 );
    QDSHeartBeat( const QDSHeartBeat& other );

    const QDSHeartBeat& operator=( const QDSHeartBeat& other );

private slots:
    void beat();

private:
    QString mChannel;
    QTimer* mTimer;
};

// ============================================================================
//
//  QDSActionRequest
//
// ============================================================================

class QDSActionRequestPrivate : public QObject
{
    Q_OBJECT

public:

    QDSActionRequestPrivate();
    QDSActionRequestPrivate( const QDSActionRequestPrivate& other );
    QDSActionRequestPrivate( const QDSServiceInfo& serviceInfo,
                             const QDSData& requestData,
                             const QByteArray& auxiliary,
                             const QString& channel );
    ~QDSActionRequestPrivate();

    QDSServiceInfo mServiceInfo;
    QDSData        mRequestData;
    QDSData        mResponseData;
    QByteArray     mAuxData;
    QString        mChannel;
    bool           mComplete;
    QString        mErrorMessage;

    QDSHeartBeat mHeartBeat;

    void emitResponse();
    void emitError( const QString& error );
};

#endif //QDS_ACTION_REQUEST_P_H
